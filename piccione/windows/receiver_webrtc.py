#!/usr/bin/env python3
"""
🐦 Piccione Global WebRTC Receiver (Windows 10/11)
Works across DIFFERENT NETWORKS / INTERNET without router port forwarding!
Uses Firebase for WebRTC SDP signaling, STUN for NAT hole punching,
and injects mouse/keyboard inputs via Win32 SendInput API.
"""

import sys
import os
import asyncio
import json
import struct
import time
import urllib.request
import ctypes
from ctypes import wintypes

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from firebase_config import FIREBASE_CONFIG

try:
    from aiortc import RTCPeerConnection, RTCSessionDescription
except ImportError:
    print("[-] Error: 'aiortc' package not found.")
    print("    Install it using: pip install aiortc")
    sys.exit(1)

# =========================================================
# CONFIGURATION
# =========================================================
STATIC_PAIR_CODE = "piccione"
# =========================================================

MAGIC = 0x50344D32

# Event Types
EVENT_TYPE_KEYBOARD = 1
EVENT_TYPE_MOUSE_MOVE = 2
EVENT_TYPE_MOUSE_BUTTON = 3
EVENT_TYPE_MOUSE_WHEEL = 4

MOUSE_BTN_LEFT = 1
MOUSE_BTN_RIGHT = 2
MOUSE_BTN_MIDDLE = 3

INPUT_MOUSE = 0
INPUT_KEYBOARD = 1

KEYEVENTF_EXTENDEDKEY = 0x0001
KEYEVENTF_KEYUP = 0x0002
KEYEVENTF_SCANCODE = 0x0008

MOUSEEVENTF_MOVE = 0x0001
MOUSEEVENTF_LEFTDOWN = 0x0002
MOUSEEVENTF_LEFTUP = 0x0004
MOUSEEVENTF_RIGHTDOWN = 0x0008
MOUSEEVENTF_RIGHTUP = 0x0010
MOUSEEVENTF_MIDDLEDOWN = 0x0020
MOUSEEVENTF_MIDDLEUP = 0x0040
MOUSEEVENTF_WHEEL = 0x0800
WHEEL_DELTA = 120

class MOUSEINPUT(ctypes.Structure):
    _fields_ = [
        ("dx", wintypes.LONG),
        ("dy", wintypes.LONG),
        ("mouseData", wintypes.DWORD),
        ("dwFlags", wintypes.DWORD),
        ("time", wintypes.DWORD),
        ("dwExtraInfo", ctypes.POINTER(ctypes.c_ulong))
    ]

class KEYBDINPUT(ctypes.Structure):
    _fields_ = [
        ("wVk", wintypes.WORD),
        ("wScan", wintypes.WORD),
        ("dwFlags", wintypes.DWORD),
        ("time", wintypes.DWORD),
        ("dwExtraInfo", ctypes.POINTER(ctypes.c_ulong))
    ]

class HARDWAREINPUT(ctypes.Structure):
    _fields_ = [
        ("uMsg", wintypes.DWORD),
        ("wParamL", wintypes.WORD),
        ("wParamH", wintypes.WORD)
    ]

class _INPUT_UNION(ctypes.Union):
    _fields_ = [
        ("mi", MOUSEINPUT),
        ("ki", KEYBDINPUT),
        ("hi", HARDWAREINPUT)
    ]

class INPUT(ctypes.Structure):
    _fields_ = [
        ("type", wintypes.DWORD),
        ("u", _INPUT_UNION)
    ]

SendInput = ctypes.windll.user32.SendInput
SendInput.argtypes = [wintypes.UINT, ctypes.POINTER(INPUT), ctypes.c_int]
SendInput.restype = wintypes.UINT

KEY_MAP = {
    1: (0x01, False), 2: (0x02, False), 3: (0x03, False), 4: (0x04, False),
    5: (0x05, False), 6: (0x06, False), 7: (0x07, False), 8: (0x08, False),
    9: (0x09, False), 10: (0x0A, False), 11: (0x0B, False), 12: (0x0C, False),
    13: (0x0D, False), 14: (0x0E, False), 15: (0x0F, False), 16: (0x10, False),
    17: (0x11, False), 18: (0x12, False), 19: (0x13, False), 20: (0x14, False),
    21: (0x15, False), 22: (0x16, False), 23: (0x17, False), 24: (0x18, False),
    25: (0x19, False), 26: (0x1A, False), 27: (0x1B, False), 28: (0x1C, False),
    29: (0x1D, False), 30: (0x1E, False), 31: (0x1F, False), 32: (0x20, False),
    33: (0x21, False), 34: (0x22, False), 35: (0x23, False), 36: (0x24, False),
    37: (0x25, False), 38: (0x26, False), 39: (0x27, False), 40: (0x28, False),
    41: (0x29, False), 42: (0x2A, False), 43: (0x2B, False), 44: (0x2C, False),
    45: (0x2D, False), 46: (0x2E, False), 47: (0x2F, False), 48: (0x30, False),
    49: (0x31, False), 50: (0x32, False), 51: (0x33, False), 52: (0x34, False),
    53: (0x35, False), 54: (0x36, False), 55: (0x37, False), 56: (0x38, False),
    57: (0x39, False), 58: (0x3A, False), 59: (0x3B, False), 60: (0x3C, False),
    61: (0x3D, False), 62: (0x3E, False), 63: (0x3F, False), 64: (0x40, False),
    65: (0x41, False), 66: (0x42, False), 67: (0x43, False), 68: (0x44, False),
    87: (0x57, False), 88: (0x58, False),
    96: (0x1C, True), 97: (0x1D, True), 98: (0x35, True), 100: (0x38, True),
    102: (0x47, True), 103: (0x48, True), 104: (0x49, True), 105: (0x4B, True),
    106: (0x4D, True), 107: (0x4F, True), 108: (0x50, True), 109: (0x51, True),
    110: (0x52, True), 111: (0x53, True), 125: (0x5B, True), 126: (0x5C, True)
}

def set_firebase_data(path, data):
    db_url = FIREBASE_CONFIG.get("databaseURL", "").rstrip("/")
    if not db_url or "your-app" in db_url: return False
    url = f"{db_url}/{path}.json"
    req = urllib.request.Request(url, data=json.dumps(data).encode("utf-8"), method="PUT")
    req.add_header("Content-Type", "application/json")
    try:
        with urllib.request.urlopen(req, timeout=5) as resp:
            return resp.status == 200
    except Exception:
        return False

def get_firebase_data(path):
    db_url = FIREBASE_CONFIG.get("databaseURL", "").rstrip("/")
    if not db_url or "your-app" in db_url: return None
    url = f"{db_url}/{path}.json"
    req = urllib.request.Request(url, method="GET")
    try:
        with urllib.request.urlopen(req, timeout=5) as resp:
            if resp.status == 200:
                return json.loads(resp.read().decode("utf-8"))
    except Exception:
        return None
    return None

def inject_packet(data):
    if len(data) != 20: return
    magic, evt_type, state, code, dx, dy, ts = struct.unpack("<IBBHiiI", data)
    if magic != MAGIC: return

    inp = INPUT()
    if evt_type == EVENT_TYPE_KEYBOARD:
        scan_code, is_ext = KEY_MAP.get(code, (code, False))
        inp.type = INPUT_KEYBOARD
        inp.u.ki.wScan = scan_code
        flags = KEYEVENTF_SCANCODE
        if is_ext: flags |= KEYEVENTF_EXTENDEDKEY
        if state == 0: flags |= KEYEVENTF_KEYUP
        inp.u.ki.dwFlags = flags
        SendInput(1, ctypes.byref(inp), ctypes.sizeof(INPUT))

    elif evt_type == EVENT_TYPE_MOUSE_MOVE:
        inp.type = INPUT_MOUSE
        inp.u.mi.dx = dx
        inp.u.mi.dy = dy
        inp.u.mi.dwFlags = MOUSEEVENTF_MOVE
        SendInput(1, ctypes.byref(inp), ctypes.sizeof(INPUT))

    elif evt_type == EVENT_TYPE_MOUSE_BUTTON:
        inp.type = INPUT_MOUSE
        is_down = (state == 1)
        if code == MOUSE_BTN_LEFT:
            inp.u.mi.dwFlags = MOUSEEVENTF_LEFTDOWN if is_down else MOUSEEVENTF_LEFTUP
        elif code == MOUSE_BTN_RIGHT:
            inp.u.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN if is_down else MOUSEEVENTF_RIGHTUP
        elif code == MOUSE_BTN_MIDDLE:
            inp.u.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN if is_down else MOUSEEVENTF_MIDDLEUP
        SendInput(1, ctypes.byref(inp), ctypes.sizeof(INPUT))

    elif evt_type == EVENT_TYPE_MOUSE_WHEEL:
        if dy != 0:
            inp.type = INPUT_MOUSE; inp.u.mi.mouseData = dy * WHEEL_DELTA; inp.u.mi.dwFlags = MOUSEEVENTF_WHEEL; SendInput(1, ctypes.byref(inp), ctypes.sizeof(INPUT))

async def main():
    print("===========================================")
    print("  Piccione WebRTC Internet Receiver")
    print("===========================================")
    print(f"[*] STATIC PAIR CODE: {STATIC_PAIR_CODE}")

    pc = RTCPeerConnection()
    channel = pc.createDataChannel("input")

    @channel.on("open")
    def on_open():
        print("[+] WebRTC DataChannel OPEN! Connected across the Internet!")

    @channel.on("message")
    def on_message(message):
        if isinstance(message, bytes):
            inject_packet(message)

    offer = await pc.createOffer()
    await pc.setLocalDescription(offer)

    set_firebase_data(f"webrtc/{STATIC_PAIR_CODE}/offer", {
        "sdp": pc.localDescription.sdp,
        "type": pc.localDescription.type
    })

    print("[*] Published WebRTC Offer to Firebase. Waiting for sender...")

    answer_data = None
    while answer_data is None:
        await asyncio.sleep(1)
        answer_data = get_firebase_data(f"webrtc/{STATIC_PAIR_CODE}/answer")

    answer = RTCSessionDescription(sdp=answer_data["sdp"], type=answer_data["type"])
    await pc.setRemoteDescription(answer)
    print("[+] Established WebRTC P2P connection over the Internet!")

    while True:
        await asyncio.sleep(3600)

if __name__ == "__main__":
    asyncio.run(main())
