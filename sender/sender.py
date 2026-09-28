#!/usr/bin/env python3
"""
🐦 Piccione TCP Socket Sender (Banana Pi / Linux)
Fetches target Windows IP from Firebase, connects via TCP socket,
and streams evdev hardware mouse/keyboard events in real-time.
"""

import sys
import os
import time
import socket
import struct
import select
import json
import urllib.request

try:
    import evdev
    from evdev import InputDevice, ecodes, list_devices
except ImportError:
    print("[-] Error: 'evdev' module not found.")
    print("    Install it using: sudo apt update && sudo apt install python3-evdev")
    sys.exit(1)

FIREBASE_HOST = "piccione-3c3f6-default-rtdb.europe-west1.firebasedatabase.app"
PICCIONE_MAGIC = 0x50494343 # "PICC"

EVENT_TYPE_KEYBOARD = 1
EVENT_TYPE_MOUSE_MOVE = 2
EVENT_TYPE_MOUSE_BUTTON = 3
EVENT_TYPE_MOUSE_WHEEL = 4

MOUSE_BTN_LEFT = 1
MOUSE_BTN_RIGHT = 2
MOUSE_BTN_MIDDLE = 3

def get_session_info(code):
    try:
        slug = code.strip().replace(" ", "-").lower()
        url = f"https://{FIREBASE_HOST}/sessions/{slug}.json"
        req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
        with urllib.request.urlopen(req, timeout=3) as response:
            data = response.read().decode()
            if data and data != "null":
                return json.loads(data)
    except Exception:
        pass
    return None

def pack_packet(event_type, state, code, dx=0, dy=0):
    return struct.pack("<I B B H h h", PICCIONE_MAGIC, event_type, state, code, int(dx), int(dy))

def main():
    print("===========================================")
    print("  Piccione TCP Socket Sender (Linux)")
    print("===========================================")

    pair_code = sys.argv[1].strip() if len(sys.argv) > 1 else "piccione"
    print(f"[*] Fetching target IP for session '{pair_code}' from Firebase...")

    session_info = None
    while session_info is None or "ip" not in session_info:
        session_info = get_session_info(pair_code)
        if session_info is None or "ip" not in session_info:
            print("[-] Waiting for Windows PC to publish IP to Firebase...")
            time.sleep(2)

    target_ip = session_info["ip"]
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 9000

    print(f"[+] Retrieved target IP from Firebase: {target_ip}")
    print(f"[*] Connecting via TCP to {target_ip}:{port}...")

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((target_ip, port))
        print("[+] Connected successfully via TCP socket!")
    except Exception as e:
        print(f"[-] Connection failed: {e}")
        sys.exit(1)

    devices = []
    for path in list_devices():
        try:
            dev = InputDevice(path)
            capabilities = dev.capabilities()
            is_kb = ecodes.EV_KEY in capabilities and ecodes.KEY_A in capabilities.get(ecodes.EV_KEY, [])
            is_mouse = ecodes.EV_REL in capabilities and ecodes.REL_X in capabilities.get(ecodes.EV_REL, [])
            if is_kb or is_mouse:
                print(f"[+] Found input device: {dev.name} ({path})")
                dev.grab()
                devices.append(dev)
        except Exception:
            pass

    if not devices:
        print("[-] No keyboard or mouse input devices found. Try running with 'sudo'.")
        sock.close()
        sys.exit(1)

    print("\n[+] Streaming hardware inputs over TCP socket... Press Ctrl+C to stop.")

    dev_map = {dev.fd: dev for dev in devices}

    try:
        while True:
            r, _, _ = select.select(dev_map.keys(), [], [], 0.005)
            for fd in r:
                dev = dev_map[fd]
                for event in dev.read():
                    if event.type == ecodes.EV_KEY:
                        if ecodes.BTN_MOUSE <= event.code < ecodes.BTN_JOYSTICK:
                            btn_code = MOUSE_BTN_LEFT
                            if event.code == ecodes.BTN_RIGHT: btn_code = MOUSE_BTN_RIGHT
                            elif event.code == ecodes.BTN_MIDDLE: btn_code = MOUSE_BTN_MIDDLE
                            pkt = pack_packet(EVENT_TYPE_MOUSE_BUTTON, 1 if event.value != 0 else 0, btn_code)
                            sock.sendall(pkt)
                        else:
                            pkt = pack_packet(EVENT_TYPE_KEYBOARD, event.value, event.code)
                            sock.sendall(pkt)

                    elif event.type == ecodes.EV_REL:
                        if event.code == ecodes.REL_X:
                            pkt = pack_packet(EVENT_TYPE_MOUSE_MOVE, 0, 0, dx=event.value, dy=0)
                            sock.sendall(pkt)
                        elif event.code == ecodes.REL_Y:
                            pkt = pack_packet(EVENT_TYPE_MOUSE_MOVE, 0, 0, dx=0, dy=event.value)
                            sock.sendall(pkt)
                        elif event.code == ecodes.REL_WHEEL:
                            pkt = pack_packet(EVENT_TYPE_MOUSE_WHEEL, 0, 0, dx=0, dy=event.value)
                            sock.sendall(pkt)

            time.sleep(0.001)

    except KeyboardInterrupt:
        print("\n[*] Stopping sender script...")
    finally:
        sock.close()
        for dev in devices:
            try:
                dev.ungrab()
            except Exception:
                pass

if __name__ == "__main__":
    main()
