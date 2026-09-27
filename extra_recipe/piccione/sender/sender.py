#!/usr/bin/env python3
"""
🐦 Piccione Global WebRTC Sender (Banana Pi / Linux)
Works across DIFFERENT NETWORKS / INTERNET!
Connects to Windows PC WebRTC DataChannel via Firebase signaling.
"""

import sys
import os
import time
import asyncio
import struct
import select
import json
import urllib.request

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from firebase_config import FIREBASE_CONFIG

try:
    from aiortc import RTCPeerConnection, RTCSessionDescription
except ImportError:
    print("[-] Error: 'aiortc' package not found.")
    print("    Install it using: sudo apt update && pip install aiortc")
    sys.exit(1)

try:
    import evdev
    from evdev import InputDevice, ecodes, list_devices
except ImportError:
    print("[-] Error: 'evdev' module not found.")
    print("    Install it using: sudo apt update && sudo apt install python3-evdev")
    sys.exit(1)

MAGIC = 0x50344D32

EVENT_TYPE_KEYBOARD = 1
EVENT_TYPE_MOUSE_MOVE = 2
EVENT_TYPE_MOUSE_BUTTON = 3
EVENT_TYPE_MOUSE_WHEEL = 4

MOUSE_BTN_LEFT = 1
MOUSE_BTN_RIGHT = 2
MOUSE_BTN_MIDDLE = 3

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

def pack_packet(event_type, state, code, dx, dy):
    timestamp = int(time.time() * 1000) & 0xFFFFFFFF
    return struct.pack("<IBBHiiI", MAGIC, event_type, state, code, dx, dy, timestamp)

async def main():
    print("===========================================")
    print("  Piccione Global WebRTC Sender (Linux)")
    print("===========================================")

    if len(sys.argv) > 1:
        pair_code = sys.argv[1].strip()
    else:
        pair_code = input("Enter Pair Code: ").strip()

    print(f"[*] Fetching WebRTC offer for session '{pair_code}' from Firebase...")
    offer_data = None
    while offer_data is None:
        offer_data = get_firebase_data(f"webrtc/{pair_code}/offer")
        if offer_data is None:
            print("[-] Waiting for WebRTC offer from Windows receiver...")
            await asyncio.sleep(2)

    pc = RTCPeerConnection()
    channel_future = asyncio.Future()

    @pc.on("datachannel")
    def on_datachannel(channel):
        print("[+] WebRTC DataChannel OPEN! Connected across the Internet!")
        channel_future.set_result(channel)

    offer = RTCSessionDescription(sdp=offer_data["sdp"], type=offer_data["type"])
    await pc.setRemoteDescription(offer)

    answer = await pc.createAnswer()
    await pc.setLocalDescription(answer)

    set_firebase_data(f"webrtc/{pair_code}/answer", {
        "sdp": pc.localDescription.sdp,
        "type": pc.localDescription.type
    })

    print("[*] Connecting via WebRTC across the Internet...")
    channel = await channel_future

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
        print("[-] No keyboard or mouse input devices found. Run with 'sudo'.")
        sys.exit(1)

    print("\n[+] Streaming inputs across the Internet... Press Ctrl+C to stop.")

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
                            pkt = pack_packet(EVENT_TYPE_MOUSE_BUTTON, 1 if event.value != 0 else 0, btn_code, 0, 0)
                            channel.send(pkt)
                        else:
                            pkt = pack_packet(EVENT_TYPE_KEYBOARD, event.value, event.code, 0, 0)
                            channel.send(pkt)

                    elif event.type == ecodes.EV_REL:
                        if event.code == ecodes.REL_X:
                            pkt = pack_packet(EVENT_TYPE_MOUSE_MOVE, 0, 0, event.value, 0)
                            channel.send(pkt)
                        elif event.code == ecodes.REL_Y:
                            pkt = pack_packet(EVENT_TYPE_MOUSE_MOVE, 0, 0, 0, event.value)
                            channel.send(pkt)
                        elif event.code == ecodes.REL_WHEEL:
                            pkt = pack_packet(EVENT_TYPE_MOUSE_WHEEL, 0, 0, 0, event.value)
                            channel.send(pkt)

            await asyncio.sleep(0.001)

    except KeyboardInterrupt:
        print("\n[*] Stopping sender...")
    finally:
        for dev in devices:
            try:
                dev.ungrab()
            except Exception:
                pass

if __name__ == "__main__":
    asyncio.run(main())
