import subprocess
import time
import os

def is_notepad_running():
    try:
        output = subprocess.check_output("tasklist", creationflags=subprocess.CREATE_NO_WINDOW).decode()
        return "notepad.exe" in output.lower()
    except Exception:
        return False

def main():
    print("[*] Starting Notepad Respawn Test Guard (Active for 20 seconds)...")
    print("[*] Launching initial notepad.exe...")

    # Start initial notepad
    subprocess.Popen(["notepad.exe"])

    start_time = time.time()
    duration = 20  # seconds

    while time.time() - start_time < duration:
        time.sleep(0.5)
        if not is_notepad_running():
            print("[+] Notepad was closed (likely by p4rr0t.exe)! Respawning so fallbacks can be tested...")
            try:
                subprocess.Popen(["notepad.exe"])
            except Exception as e:
                print(f"[-] Failed to restart notepad: {e}")
            time.sleep(1)

    print("[*] 20 seconds elapsed. Stopping Notepad Respawn Guard.")

if __name__ == "__main__":
    main()
