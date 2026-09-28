#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <wininet.h>
#include <ws2tcpip.h>
#include <stdint.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wininet.lib")

#define FIREBASE_HOST "piccione-3c3f6-default-rtdb.europe-west1.firebasedatabase.app"
#define PICCIONE_MAGIC 0x50494343 // "PICC"

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;
    uint8_t type;
    uint8_t state;
    uint16_t code;
    int16_t dx;
    int16_t dy;
} PiccionePacket;
#pragma pack(pop)

void GetLocalIP(char *ipBuffer, int maxLen) {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        struct addrinfo hints = {0}, *res = NULL;
        hints.ai_family = AF_INET;
        if (getaddrinfo(hostname, NULL, &hints, &res) == 0) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
            inet_ntop(AF_INET, &(ipv4->sin_addr), ipBuffer, maxLen);
            freeaddrinfo(res);
            return;
        }
    }
    strcpy_s(ipBuffer, maxLen, "127.0.0.1");
}

void PublishToFirebase(const char *pairCode, const char *ip) {
    HINTERNET hNet = InternetOpenA("p4rr0t", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hNet) return;

    HINTERNET hConnect = InternetConnectA(hNet, FIREBASE_HOST, INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (hConnect) {
        char path[256];
        sprintf_s(path, sizeof(path), "/sessions/%s.json", pairCode);
        HINTERNET hRequest = HttpOpenRequestA(hConnect, "PUT", path, NULL, NULL, NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, 0);
        if (hRequest) {
            char postData[256];
            sprintf_s(postData, sizeof(postData), "{\"ip\":\"%s\"}", ip);
            const char *headers = "Content-Type: application/json\r\n";
            HttpSendRequestA(hRequest, headers, (DWORD)strlen(headers), (LPVOID)postData, (DWORD)strlen(postData));
            InternetCloseHandle(hRequest);
        }
        InternetCloseHandle(hConnect);
    }
    InternetCloseHandle(hNet);
}

void HandleClient(SOCKET clientSocket) {
    char buffer[sizeof(PiccionePacket)];
    while (1) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) break;

        if (bytesReceived == sizeof(PiccionePacket)) {
            PiccionePacket *pkt = (PiccionePacket*)buffer;
            if (pkt->magic == PICCIONE_MAGIC) {
                INPUT input = { 0 };
                if (pkt->type == 1) { // KEYBOARD
                    input.type = INPUT_KEYBOARD;
                    input.ki.wScan = pkt->code;
                    input.ki.dwFlags = KEYEVENTF_SCANCODE;
                    if (pkt->state == 0) input.ki.dwFlags |= KEYEVENTF_KEYUP;
                    SendInput(1, &input, sizeof(INPUT));
                } else if (pkt->type == 2) { // MOUSE MOVE
                    input.type = INPUT_MOUSE;
                    input.mi.dx = pkt->dx;
                    input.mi.dy = pkt->dy;
                    input.mi.dwFlags = MOUSEEVENTF_MOVE;
                    SendInput(1, &input, sizeof(INPUT));
                } else if (pkt->type == 3) { // MOUSE BUTTON
                    input.type = INPUT_MOUSE;
                    BOOL isDown = (pkt->state == 1);
                    if (pkt->code == 1) input.mi.dwFlags = isDown ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
                    else if (pkt->code == 2) input.mi.dwFlags = isDown ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
                    SendInput(1, &input, sizeof(INPUT));
                } else if (pkt->type == 4) { // MOUSE WHEEL
                    input.type = INPUT_MOUSE;
                    input.mi.mouseData = pkt->dy * 120;
                    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
                    SendInput(1, &input, sizeof(INPUT));
                }
            }
        }
    }
    closesocket(clientSocket);
}

void StartReceiver(const char *pairCode) {
    printf("[*] Starting integrated Piccione receiver with Firebase pairing: %s\n", pairCode);

    // Initialize WinSock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("[-] WSAStartup failed.\n");
        return;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        printf("[-] Socket creation failed.\n");
        WSACleanup();
        return;
    }

    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(9000); // Default port

    if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("[-] Bind failed.\n");
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("[-] Listen failed.\n");
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    char localIp[64];
    GetLocalIP(localIp, sizeof(localIp));
    printf("[+] Receiver listening on port 9000 (Local IP: %s). Publishing to Firebase...\n", localIp);

    // Publish IP to Firebase
    PublishToFirebase(pairCode, localIp);

    while (1) {
        SOCKADDR_IN clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) continue;

        printf("[+] Sender connected!\n");
        HandleClient(clientSocket);
        printf("[-] Sender disconnected. Waiting for reconnection...\n");
    }

    closesocket(serverSocket);
    WSACleanup();
}
