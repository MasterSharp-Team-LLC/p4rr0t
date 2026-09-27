#!/usr/bin/env python3
"""
Piccione Sender (Banana Pi M2 Ultra / Any Linux)
Captures USB keyboards & mice directly from Linux kernel (/dev/input/evdev)
and streams inputs to Windows Receiver via Pair Code or Target IP.
"""

import sys
import time
import socket
import struct
import json
import select
import urllib.request

from firebase_config import FIREBASE_CONFIG

try:
    import evdev
    from evdev import InputDevice, ecodes, list_devices
except ImportError:
    print("[-] Error: 'evdev' module not found.")
    print("    Install it using: sudo apt update && sudo apt install python3-evdev")
    sys.exit(1)

MAGIC = 0x50344D32
DEFAULT_PORT = 9876

# Event Types
EVENT_TYPE_KEYBOARD = 1
EVENT_TYPE_MOUSE_MOVE = 2
EVENT_TYPE_MOUSE_BUTTON = 3
EVENT_TYPE_MOUSE_WHEEL = 4

# Mouse Buttons
MOUSE_BTN_LEFT = 1
MOUSE_BTN_RIGHT = 2
MOUSE_BTN_MIDDLE = 3
MOUSE_BTN_SIDE = 4
MOUSE_BTN_EXTRA = 5

def lookup_firebase_session(pair_code):
    db_url = FIREBASE_CONFIG.get("databaseURL", "").rstrip("/")
    if not db_url or "your-app" in db_url:
        return None

    encoded_code = urllib.parse.quote(pair_code)
    url = f"{db_url}/sessions/{encoded_code}.json"
    req = urllib.request.Request(url, method="GET")

    try:
        with urllib.request.urlopen(req) as resp:
            if resp.status == 200:
                return json.loads(resp.read().decode("utf-8"))
    except Exception:
        return None

def pack_packet(event_type, state, code, dx, dy):
    timestamp = int(time.time() * 1000) & 0xFFFFFFFF
    return struct.pack("<IBBHiiI", MAGIC, event_type, state, code, dx, dy, timestamp)

def main():
    if hasattr(sys.stdout, "reconfigure"):
        try:
            sys.stdout.reconfigure(encoding="utf-8")
        except Exception:
            pass

    print("===========================================")
    print("  Piccione Sender (Linux)")
    print("===========================================")

    target_ip = None
    target_port = DEFAULT_PORT

    if len(sys.argv) > 1:
        target_input = sys.argv[1].strip()
    else:
        target_input = input("Enter Pair Code OR Windows Local IP: ").strip()

    if "." in target_input:
        target_ip = target_input
    else:
        print(f"[*] Looking up Pair Code '{target_input}' in Firebase...")
        session = lookup_firebase_session(target_input)
        if session and session.get("status") == "online":
            target_ip = session["ip"]
            target_port = session.get("port", DEFAULT_PORT)
            print(f"[+] Discovered Windows PC ({session.get('hostname', 'PC')}) at {target_ip}:{target_port}!")
        else:
            print("[-] Pairing code not found in Firebase.")
            sys.exit(1)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    devices = []
    for path in list_devices():
        try:
            dev = InputDevice(path)
            capabilities = dev.capabilities()
            is_kb = ecodes.EV_KEY in capabilities and ecodes.KEY_A in capabilities.get(ecodes.EV_KEY, [])
            is_mouse = ecodes.EV_REL in capabilities and ecodes.REL_X in capabilities.get(ecodes.EV_REL, [])

            if is_kb or is_mouse:
                print(f"[+] Found device: {dev.name} ({path})")
                dev.grab()
                devices.append(dev)
        except Exception:
            pass

    if not devices:
        print("[-] No keyboard or mouse input devices found in /dev/input/. Run with 'sudo'.")
        sys.exit(1)

    print(f"\n[+] Streaming inputs to {target_ip}:{target_port}... Press Ctrl+C to stop.")

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
                            elif event.code == ecodes.BTN_SIDE: btn_code = MOUSE_BTN_SIDE
                            elif event.code == ecodes.BTN_EXTRA: btn_code = MOUSE_BTN_EXTRA

                            pkt = pack_packet(EVENT_TYPE_MOUSE_BUTTON, 1 if event.value != 0 else 0, btn_code, 0, 0)
                            sock.sendto(pkt, (target_ip, target_port))
                        else:
                            pkt = pack_packet(EVENT_TYPE_KEYBOARD, event.value, event.code, 0, 0)
                            sock.sendto(pkt, (target_ip, target_port))

                    elif event.type == ecodes.EV_REL:
                        if event.code == ecodes.REL_X:
                            pkt = pack_packet(EVENT_TYPE_MOUSE_MOVE, 0, 0, event.value, 0)
                            sock.sendto(pkt, (target_ip, target_port))
                        elif event.code == ecodes.REL_Y:
                            pkt = pack_packet(EVENT_TYPE_MOUSE_MOVE, 0, 0, 0, event.value)
                            sock.sendto(pkt, (target_ip, target_port))
                        elif event.code == ecodes.REL_WHEEL:
                            pkt = pack_packet(EVENT_TYPE_MOUSE_WHEEL, 0, 0, 0, event.value)
                            sock.sendto(pkt, (target_ip, target_port))
                        elif event.code == ecodes.REL_HWHEEL:
                            pkt = pack_packet(EVENT_TYPE_MOUSE_WHEEL, 0, 0, event.value, 0)
                            sock.sendto(pkt, (target_ip, target_port))

    except KeyboardInterrupt:
        print("\n[*] Stopping sender...")
    finally:
        for dev in devices:
            try:
                dev.ungrab()
            except Exception:
                pass

if __name__ == "__main__":
    main()