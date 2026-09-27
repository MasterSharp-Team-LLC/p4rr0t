#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wininet.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <cstdint>

#include "protocol.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(linker, "/subsystem:windows")
#pragma comment(linker, "/ENTRY:WinMainCRTStartup")

#define STATIC_PAIR_CODE "piccione"
#define FIREBASE_DB_HOST "piccione-3c3f6-default-rtdb.europe-west1.firebasedatabase.app"

struct KeyMapping {
    WORD scanCode;
    bool isExtended;
};

// Map Linux EVDEV KEY_* codes to Windows Scan Codes + Extended flag
static std::unordered_map<uint16_t, KeyMapping> g_keyMap;

void init_keymap() {
    // Standard keys
    g_keyMap[1] = { 0x01, false }; // ESC
    g_keyMap[2] = { 0x02, false }; // 1
    g_keyMap[3] = { 0x03, false }; // 2
    g_keyMap[4] = { 0x04, false }; // 3
    g_keyMap[5] = { 0x05, false }; // 4
    g_keyMap[6] = { 0x06, false }; // 5
    g_keyMap[7] = { 0x07, false }; // 6
    g_keyMap[8] = { 0x08, false }; // 7
    g_keyMap[9] = { 0x09, false }; // 8
    g_keyMap[10] = { 0x0A, false }; // 9
    g_keyMap[11] = { 0x0B, false }; // 0
    g_keyMap[12] = { 0x0C, false }; // MINUS
    g_keyMap[13] = { 0x0D, false }; // EQUAL
    g_keyMap[14] = { 0x0E, false }; // BACKSPACE
    g_keyMap[15] = { 0x0F, false }; // TAB
    g_keyMap[16] = { 0x10, false }; // Q
    g_keyMap[17] = { 0x11, false }; // W
    g_keyMap[18] = { 0x12, false }; // E
    g_keyMap[19] = { 0x13, false }; // R
    g_keyMap[20] = { 0x14, false }; // T
    g_keyMap[21] = { 0x15, false }; // Y
    g_keyMap[22] = { 0x16, false }; // U
    g_keyMap[23] = { 0x17, false }; // I
    g_keyMap[24] = { 0x18, false }; // O
    g_keyMap[25] = { 0x19, false }; // P
    g_keyMap[26] = { 0x1A, false }; // LEFTBRACE
    g_keyMap[27] = { 0x1B, false }; // RIGHTBRACE
    g_keyMap[28] = { 0x1C, false }; // ENTER
    g_keyMap[29] = { 0x1D, false }; // LEFTCTRL
    g_keyMap[30] = { 0x1E, false }; // A
    g_keyMap[31] = { 0x1F, false }; // S
    g_keyMap[32] = { 0x20, false }; // D
    g_keyMap[33] = { 0x21, false }; // F
    g_keyMap[34] = { 0x22, false }; // G
    g_keyMap[35] = { 0x23, false }; // H
    g_keyMap[36] = { 0x24, false }; // J
    g_keyMap[37] = { 0x25, false }; // K
    g_keyMap[38] = { 0x26, false }; // L
    g_keyMap[39] = { 0x27, false }; // SEMICOLON
    g_keyMap[40] = { 0x28, false }; // APOSTROPHE
    g_keyMap[41] = { 0x29, false }; // GRAVE
    g_keyMap[42] = { 0x2A, false }; // LEFTSHIFT
    g_keyMap[43] = { 0x2B, false }; // BACKSLASH
    g_keyMap[44] = { 0x2C, false }; // Z
    g_keyMap[45] = { 0x2D, false }; // X
    g_keyMap[46] = { 0x2E, false }; // C
    g_keyMap[47] = { 0x2F, false }; // V
    g_keyMap[48] = { 0x30, false }; // B
    g_keyMap[49] = { 0x31, false }; // N
    g_keyMap[50] = { 0x32, false }; // M
    g_keyMap[51] = { 0x33, false }; // COMMA
    g_keyMap[52] = { 0x34, false }; // DOT
    g_keyMap[53] = { 0x35, false }; // SLASH
    g_keyMap[54] = { 0x36, false }; // RIGHTSHIFT
    g_keyMap[55] = { 0x37, false }; // KPASTERISK
    g_keyMap[56] = { 0x38, false }; // LEFTALT
    g_keyMap[57] = { 0x39, false }; // SPACE
    g_keyMap[58] = { 0x3A, false }; // CAPSLOCK
    g_keyMap[59] = { 0x3B, false }; // F1
    g_keyMap[60] = { 0x3C, false }; // F2
    g_keyMap[61] = { 0x3D, false }; // F3
    g_keyMap[62] = { 0x3E, false }; // F4
    g_keyMap[63] = { 0x3F, false }; // F5
    g_keyMap[64] = { 0x40, false }; // F6
    g_keyMap[65] = { 0x41, false }; // F7
    g_keyMap[66] = { 0x42, false }; // F8
    g_keyMap[67] = { 0x43, false }; // F9
    g_keyMap[68] = { 0x44, false }; // F10
    g_keyMap[87] = { 0x57, false }; // F11
    g_keyMap[88] = { 0x58, false }; // F12

    // Extended Keys
    g_keyMap[96] = { 0x1C, true };  // KPENTER
    g_keyMap[97] = { 0x1D, true };  // RIGHTCTRL
    g_keyMap[98] = { 0x35, true };  // KPSLASH
    g_keyMap[100] = { 0x38, true }; // RIGHTALT
    g_keyMap[102] = { 0x47, true }; // HOME
    g_keyMap[103] = { 0x48, true }; // UP
    g_keyMap[104] = { 0x49, true }; // PAGEUP
    g_keyMap[105] = { 0x4B, true }; // LEFT
    g_keyMap[106] = { 0x4D, true }; // RIGHT
    g_keyMap[107] = { 0x4F, true }; // END
    g_keyMap[108] = { 0x50, true }; // DOWN
    g_keyMap[109] = { 0x51, true }; // PAGEDOWN
    g_keyMap[110] = { 0x52, true }; // INSERT
    g_keyMap[111] = { 0x53, true }; // DELETE
    g_keyMap[125] = { 0x5B, true }; // LEFTMETA (Win)
    g_keyMap[126] = { 0x5C, true }; // RIGHTMETA (Win)
}

std::string GetLocalIP() {
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET) return "127.0.0.1";
    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr("8.8.8.8");
    sa.sin_port = htons(80);
    if (connect(s, (sockaddr*)&sa, sizeof(sa)) == SOCKET_ERROR) {
        closesocket(s);
        return "127.0.0.1";
    }
    sockaddr_in name{};
    int namelen = sizeof(name);
    getsockname(s, (sockaddr*)&name, &namelen);
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &name.sin_addr, ipStr, sizeof(ipStr));
    closesocket(s);
    return std::string(ipStr);
}

std::string UrlEncode(const std::string& str) {
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++) {
        if (isalnum((unsigned char)str[i]) || str[i] == '-' || str[i] == '_' || str[i] == '.' || str[i] == '~')
            strTemp += str[i];
        else if (str[i] == ' ')
            strTemp += "%20";
        else {
            char buf[4];
            snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)str[i]);
            strTemp += buf;
        }
    }
    return strTemp;
}

bool RegisterFirebaseSession(const std::string& pairCode, const std::string& localIp, int port) {
    if (std::string(FIREBASE_DB_HOST).empty()) return false;

    HINTERNET hNet = InternetOpenA("PiccioneReceiver", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hNet) return false;

    HINTERNET hConnect = InternetConnectA(hNet, FIREBASE_DB_HOST, INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hNet);
        return false;
    }

    std::string path = "/sessions/" + UrlEncode(pairCode) + ".json";
    HINTERNET hRequest = HttpOpenRequestA(hConnect, "PUT", path.c_str(), NULL, NULL, NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hNet);
        return false;
    }

    char hostname[256];
    DWORD size = sizeof(hostname);
    GetComputerNameA(hostname, &size);

    char jsonBuf[512];
    snprintf(jsonBuf, sizeof(jsonBuf), "{\"ip\":\"%s\",\"port\":%d,\"hostname\":\"%s\",\"status\":\"online\"}", localIp.c_str(), port, hostname);

    std::string headers = "Content-Type: application/json\r\n";
    BOOL sent = HttpSendRequestA(hRequest, headers.c_str(), (DWORD)headers.length(), (LPVOID)jsonBuf, (DWORD)strlen(jsonBuf));

    DWORD statusCode = 0;
    DWORD statusSize = sizeof(statusCode);
    HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &statusSize, NULL);

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hNet);

    return sent && (statusCode == 200);
}

void process_packet(const PiccionePacket& pkt) {
    if (pkt.magic != PICCIONE_MAGIC) return;

    INPUT input = { 0 };

    if (pkt.type == EVENT_TYPE_KEYBOARD) {
        auto it = g_keyMap.find(pkt.code);
        WORD scanCode = 0;
        bool isExtended = false;

        if (it != g_keyMap.end()) {
            scanCode = it->second.scanCode;
            isExtended = it->second.isExtended;
        }
        else {
            scanCode = static_cast<WORD>(pkt.code);
        }

        input.type = INPUT_KEYBOARD;
        input.ki.wScan = scanCode;
        input.ki.dwFlags = KEYEVENTF_SCANCODE;

        if (isExtended) {
            input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
        }

        if (pkt.state == 0) {
            // Key Up
            input.ki.dwFlags |= KEYEVENTF_KEYUP;
        }

        SendInput(1, &input, sizeof(INPUT));
    }
    else if (pkt.type == EVENT_TYPE_MOUSE_MOVE) {
        input.type = INPUT_MOUSE;
        input.mi.dx = pkt.dx;
        input.mi.dy = pkt.dy;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        SendInput(1, &input, sizeof(INPUT));
    }
    else if (pkt.type == EVENT_TYPE_MOUSE_BUTTON) {
        input.type = INPUT_MOUSE;
        bool isDown = (pkt.state == 1);

        switch (pkt.code) {
        case MOUSE_BTN_LEFT:
            input.mi.dwFlags = isDown ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
            break;
        case MOUSE_BTN_RIGHT:
            input.mi.dwFlags = isDown ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
            break;
        case MOUSE_BTN_MIDDLE:
            input.mi.dwFlags = isDown ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
            break;
        case MOUSE_BTN_SIDE:
            input.mi.dwFlags = isDown ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP;
            input.mi.mouseData = XBUTTON1;
            break;
        case MOUSE_BTN_EXTRA:
            input.mi.dwFlags = isDown ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP;
            input.mi.mouseData = XBUTTON2;
            break;
        default:
            return;
        }
        SendInput(1, &input, sizeof(INPUT));
    }
    else if (pkt.type == EVENT_TYPE_MOUSE_WHEEL) {
        if (pkt.dy != 0) {
            input.type = INPUT_MOUSE;
            input.mi.mouseData = pkt.dy * WHEEL_DELTA;
            input.mi.dwFlags = MOUSEEVENTF_WHEEL;
            SendInput(1, &input, sizeof(INPUT));
        }
        if (pkt.dx != 0) {
            input.type = INPUT_MOUSE;
            input.mi.mouseData = pkt.dx * WHEEL_DELTA;
            input.mi.dwFlags = MOUSEEVENTF_HWHEEL;
            SendInput(1, &input, sizeof(INPUT));
        }
    }
}

int main(int argc, char* argv[]) {
    HWND hwnd = GetConsoleWindow();
    if (hwnd != NULL) {
        ShowWindow(hwnd, SW_HIDE);
    }

    int port = PICCIONE_DEFAULT_PORT;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        }
        catch (...) {}
    }

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[-] WSAStartup failed!" << std::endl;
        return 1;
    }

    std::cout << "===========================================\n";
    std::cout << "  Piccione C++ Receiver (Windows 10/11)\n";
    std::cout << "===========================================\n";

    init_keymap();

    std::string pairCode = STATIC_PAIR_CODE;
    std::string localIp = GetLocalIP();
    std::cout << "[*] Local IP: " << localIp << ":" << port << "\n";
    std::cout << "[*] Registering Pair Code '" << pairCode << "' with Firebase...\n";

    bool registered = RegisterFirebaseSession(pairCode, localIp, port);
    if (registered) {
        std::cout << "[+] Successfully registered in Firebase!\n";
        std::cout << "[+] STATIC PAIR CODE: " << pairCode << "\n";
    }
    else {
        std::cout << "[!] Firebase Registration warning (check database rules or host).\n";
    }

    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "[-] Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        system("pause");
        return 1;
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[-] Bind failed on port " << port << " (Error: " << WSAGetLastError() << ")\n";
        std::cerr << "    Another instance of Piccione Receiver or python script is already using port 9876.\n";
        closesocket(sockfd);
        WSACleanup();
        system("pause");
        return 1;
    }

    std::cout << "\n[+] C++ Receiver listening for UDP packets on port " << port << "...\n";
    std::cout << "[*] Press Ctrl+C to exit.\n" << std::endl;

    PiccionePacket pkt{};
    sockaddr_in clientAddr{};

    char clientIp[INET_ADDRSTRLEN];
    bool connected = false;

    while (true) {
        // CRITIC FIX: Bisogna reimpostare la dimensione della struttura ad ogni ciclo di recvfrom
        int clientAddrLen = sizeof(clientAddr);

        int bytesRecv = recvfrom(sockfd, (char*)&pkt, sizeof(pkt), 0, (sockaddr*)&clientAddr, &clientAddrLen);
        if (bytesRecv == sizeof(PiccionePacket)) {
            if (!connected) {
                inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, sizeof(clientIp));
                std::cout << "[+] Connected to Sender at " << clientIp << ":" << ntohs(clientAddr.sin_port) << std::endl;
                connected = true;
            }
            process_packet(pkt);
        }
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
