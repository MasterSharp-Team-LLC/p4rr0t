#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <thread>
#include <chrono>

#include <rtc/rtc.hpp>

#include "protocol.h"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(linker, "/subsystem:windows")

#define STATIC_PAIR_CODE "piccione"
#define FIREBASE_DB_HOST "piccione-3c3f6-default-rtdb.europe-west1.firebasedatabase.app"

struct KeyMapping {
    WORD scanCode;
    bool isExtended;
};

static std::unordered_map<uint16_t, KeyMapping> g_keyMap;

void init_keymap() {
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

bool FirebasePut(const std::string& path, const std::string& jsonPayload) {
    HINTERNET hNet = InternetOpenA("PiccioneWebRTC", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hNet) return false;
    HINTERNET hConnect = InternetConnectA(hNet, FIREBASE_DB_HOST, INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) { InternetCloseHandle(hNet); return false; }

    HINTERNET hRequest = HttpOpenRequestA(hConnect, "PUT", path.c_str(), NULL, NULL, NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, 0);
    if (!hRequest) { InternetCloseHandle(hConnect); InternetCloseHandle(hNet); return false; }

    std::string headers = "Content-Type: application/json\r\n";
    BOOL sent = HttpSendRequestA(hRequest, headers.c_str(), (DWORD)headers.length(), (LPVOID)jsonPayload.c_str(), (DWORD)jsonPayload.length());

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hNet);
    return sent;
}

std::string FirebaseGet(const std::string& path) {
    HINTERNET hNet = InternetOpenA("PiccioneWebRTC", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hNet) return "";
    HINTERNET hConnect = InternetConnectA(hNet, FIREBASE_DB_HOST, INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) { InternetCloseHandle(hNet); return ""; }

    HINTERNET hRequest = HttpOpenRequestA(hConnect, "GET", path.c_str(), NULL, NULL, NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, 0);
    if (!hRequest) { InternetCloseHandle(hConnect); InternetCloseHandle(hNet); return ""; }

    std::string response = "";
    if (HttpSendRequestA(hRequest, NULL, 0, NULL, 0)) {
        char buffer[1024];
        DWORD bytesRead = 0;
        while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            response += buffer;
        }
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hNet);
    return response;
}

std::string ExtractJsonField(const std::string& json, const std::string& field) {
    std::string key = "\"" + field + "\":\"";
    size_t start = json.find(key);
    if (start == std::string::npos) return "";
    start += key.length();
    size_t end = json.find("\"", start);
    if (end == std::string::npos) return "";
    return json.substr(start, end - start);
}

void process_packet(const PiccionePacket& pkt) {
    if (pkt.magic != PICCIONE_MAGIC) return;

    INPUT input = { 0 };

    if (pkt.type == EVENT_TYPE_KEYBOARD) {
        auto it = g_keyMap.find(pkt.code);
        WORD scanCode = (it != g_keyMap.end()) ? it->second.scanCode : static_cast<WORD>(pkt.code);
        bool isExtended = (it != g_keyMap.end()) ? it->second.isExtended : false;

        input.type = INPUT_KEYBOARD;
        input.ki.wScan = scanCode;
        input.ki.dwFlags = KEYEVENTF_SCANCODE;
        if (isExtended) input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
        if (pkt.state == 0) input.ki.dwFlags |= KEYEVENTF_KEYUP;

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
        case MOUSE_BTN_LEFT: input.mi.dwFlags = isDown ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP; break;
        case MOUSE_BTN_RIGHT: input.mi.dwFlags = isDown ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP; break;
        case MOUSE_BTN_MIDDLE: input.mi.dwFlags = isDown ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP; break;
        case MOUSE_BTN_SIDE:
            input.mi.dwFlags = isDown ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP;
            input.mi.mouseData = XBUTTON1;
            break;
        case MOUSE_BTN_EXTRA:
            input.mi.dwFlags = isDown ? MOUSEEVENTF_XDOWN : MOUSEEVENTF_XUP;
            input.mi.mouseData = XBUTTON2;
            break;
        default: return;
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
    std::cout << "===========================================\n";
    std::cout << "  Piccione C++ WebRTC Receiver (Windows)\n";
    std::cout << "===========================================\n";

    init_keymap();

    std::string pairCode = (argc > 1) ? argv[1] : STATIC_PAIR_CODE;
    std::cout << "[*] Pair Code: " << pairCode << "\n";

    rtc::Configuration config;
    config.iceServers.emplace_back("stun:stun.l.google.com:19302");

    auto pc = std::make_shared<rtc::PeerConnection>(config);

    pc->onStateChange([](rtc::PeerConnection::State state) {
        std::cout << "[*] WebRTC State: " << state << std::endl;
        });

    auto dc = pc->createDataChannel("events");

    dc->onOpen([&]() {
        std::cout << "\n[+] WebRTC DataChannel Opened! Connected to Linux Sender!\n";
        });

    dc->onMessage([&](std::variant<rtc::binary, rtc::string> message) {
        if (std::holds_alternative<rtc::binary>(message)) {
            auto data = std::get<rtc::binary>(message);
            if (data.size() == sizeof(PiccionePacket)) {
                PiccionePacket pkt;
                std::memcpy(&pkt, data.data(), sizeof(PiccionePacket));
                process_packet(pkt);
            }
        }
        });

    pc->onLocalDescription([&](rtc::Description description) {
        std::string sdpStr = std::string(description);

        std::string escapedSdp = "";
        for (char c : sdpStr) {
            if (c == '\r') continue;
            if (c == '\n') escapedSdp += "\\n";
            else escapedSdp += c;
        }

        std::string payload = "{\"sdp\":\"" + escapedSdp + "\",\"type\":\"offer\"}";
        std::cout << "[*] Sending WebRTC Offer to Firebase for session '" << pairCode << "'...\n";

        if (FirebasePut("/webrtc/" + pairCode + "/offer.json", payload)) {
            std::cout << "[+] Offer uploaded successfully to Firebase!\n";
        }
        else {
            std::cerr << "[-] Error while uploading the Offer to Firebase.\n";
        }
        });

    pc->setLocalDescription();

    std::cout << "[*] Waiting for the WebRTC Answer from the Sender...\n";
    std::string answerSdp = "";
    while (answerSdp.empty()) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::string response = FirebaseGet("/webrtc/" + pairCode + "/answer.json");
        if (!response.empty() && response != "null") {
            answerSdp = ExtractJsonField(response, "sdp");
        }
    }

    std::string unescapedAnswer = "";
    for (size_t i = 0; i < answerSdp.length(); ++i) {
        if (answerSdp[i] == '\\' && i + 1 < answerSdp.length() && answerSdp[i + 1] == 'n') {
            unescapedAnswer += "\r\n";
            i++;
        }
        else {
            unescapedAnswer += answerSdp[i];
        }
    }

    std::cout << "[+] Answer received from Firebase! Configuring Remote Description...\n";
    pc->setRemoteDescription(rtc::Description(unescapedAnswer, "answer"));

    std::cout << "[*] Streaming started. Press ENTER to exit.\n";
    std::cin.get();

    return 0;
}