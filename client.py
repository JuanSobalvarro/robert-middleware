import os
import sys
import zmq
from typing import Optional

# Configuration
ZMQ_ENDPOINT = os.getenv("ROBERT_ZMQ_ENDPOINT", "tcp://172.26.155.178:42069")

class RobeRTClient:
    def __init__(self, endpoint: str):
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REQ)
        self.socket.setsockopt(zmq.RCVTIMEO, 5000)  # 5s timeout
        self.endpoint = endpoint

    def connect(self):
        print(f"[*] Connecting to RobeRT Middleware at {self.endpoint}...")
        self.socket.connect(self.endpoint)

    def send(self, command: str) -> str:
        try:
            self.socket.send_string(command)
            return self.socket.recv_string()
        except zmq.Again:
            return "ERROR: Middleware timeout (Is the server running?)"
        except Exception as e:
            return f"ERROR: {str(e)}"

    def close(self):
        self.socket.close()
        self.context.term()

def get_input(prompt: str, default: str) -> str:
    user_input = input(f"{prompt} [{default}]: ").strip()
    return user_input if user_input else default

def format_target_csv(label: str) -> str:
    print(f"\n--- {label} Configuration ---")
    # Using standard ABB default values for an IRB 140
    x = get_input("  x (mm)", "400.0")
    y = get_input("  y (mm)", "0.0")
    z = get_input("  z (mm)", "500.0")
    q1 = get_input("  q1", "1.0")
    q2 = get_input("  q2", "0.0")
    q3 = get_input("  q3", "0.0")
    q4 = get_input("  q4", "0.0")
    cf1 = get_input("  cf1", "0")
    cf4 = get_input("  cf4", "0")
    cf6 = get_input("  cf6", "0")
    cfx = get_input("  cfx", "0")
    return f"{x},{y},{z},{q1},{q2},{q3},{q4},{cf1},{cf4},{cf6},{cfx}"

def display_menu():
    print("\n" + "═"*45)
    print("  RobeRT INDUSTRIAL MIDDLEWARE CLI v1.0  ")
    print("═"*45)
    menu = {
        "1": "MoveL (Linear)",
        "2": "MoveJ (Joint-Cartesian)",
        "3": "MoveC (Circular)",
        "4": "MoveAbsJ (Joint-Axis)",
        "5": "Set Speed",
        "6": "Set Precision",
        "7": "Emergency STOP",
        "8": "PING (Heartbeat)",
        "9": "CUSTOM Raw Command",
        "0": "EXIT"
    }
    for k, v in menu.items():
        print(f" {k}. {v}")
    print("═"*45)

def main():
    client = RobeRTClient(ZMQ_ENDPOINT)
    client.connect()

    try:
        while True:
            display_menu()
            choice = input("\nSelect Option > ").strip()

            command = ""
            if choice == "1":
                command = f"MOVEL|{format_target_csv('MoveL')}"
            elif choice == "2":
                command = f"MOVEJ|{format_target_csv('MoveJ')}"
            elif choice == "3":
                cir = format_target_csv("MoveC (Circle Point)")
                to = format_target_csv("MoveC (Dest Point)")
                command = f"MOVEC|{cir}|{to}"
            elif choice == "4":
                print("\n--- MoveAbsJ (Degrees) ---")
                j = [get_input(f"  j{i}", "0.0") for i in range(1, 7)]
                command = f"MOVEABSJ|{','.join(j)}"
            elif choice == "5":
                val = get_input("Speed (mm/s)", "100")
                command = f"SETSPEED|{val}"
            elif choice == "6":
                val = get_input("Precision (fine/z10/z50)", "z10")
                command = f"SETPRECISION|{val}"
            elif choice == "7":
                command = "STOP|NONE"
            elif choice == "8":
                command = "PING|NONE"
            elif choice == "9":
                command = input("Raw Message > ").strip()
            elif choice == "0" or choice.lower() == "exit":
                client.send("EXIT|NONE")
                break
            else:
                print("[!] Invalid selection.")
                continue

            print(f"[*] Sending: {command}")
            response = client.send(command)
            
            # Color coding status (simplified for terminal)
            status_prefix = "[+]" if "ACK" in response or "PONG" in response else "[!]"
            print(f"{status_prefix} Middleware Response: {response}")

    except KeyboardInterrupt:
        print("\n[!] Disconnecting...")
    finally:
        client.close()

if __name__ == "__main__":
    main()