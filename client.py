import socket
import sys

ROBOT_IP = '127.0.0.1'
ROBOT_PORT = 5000

COMMANDS = {
    "MOVEJ": "Send MOVEJ command with X,Y,Z coordinates",
    "ORIGIN": "Send ORIGIN command to set current position as origin",
    "HOME": "Send HOME command to move robot to home position",
    "CUSTOM/RAW": "Send a custom command string to the robot",
    "EXIT": "Disconnect and exit the CLI"
}

def display_menu():
    print("\n" + "="*30)
    print(" ABB VIRTUAL CONTROLLER CLI ")
    print("="*30)
    for i, (cmd, desc) in enumerate(COMMANDS.items(), start=1):
        print(f"{i}. {cmd}: {desc}")
    print("="*30)

def main():
    # Initialize the socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    print(f"Connecting to Virtual Controller at {ROBOT_IP}:{ROBOT_PORT}...")
    try:
        client_socket.connect((ROBOT_IP, ROBOT_PORT))
        print("Successfully connected!")
    except ConnectionRefusedError:
        print("\nError: Connection refused.")
        print("Make sure the RAPID program is running and waiting at SocketAccept.")
        sys.exit(1)

    # Main CLI Loop
    try:
        while True:
            display_menu()
            choice = input("Select an option (1-5): ").strip()
            
            command_str = ""
            
            if choice == '1':
                # Gather coordinates
                try:
                    x = float(input("Enter X coordinate (mm): "))
                    y = float(input("Enter Y coordinate (mm): "))
                    z = float(input("Enter Z coordinate (mm): "))
                    # Construct the comma-separated string
                    command_str = f"MOVEJ,{x},{y},{z}"
                except ValueError:
                    print("Invalid input! Please enter numeric values.")
                    continue

            elif choice == '2':
                command_str = "ORIGIN"  
                    
            elif choice == '3':
                command_str = "HOME"
                
            elif choice == '4':
                command_str = input("Enter raw command string: ").strip()
                if not command_str:
                    continue
                    
            elif choice == '5':
                command_str = "EXIT"
                
            else:
                print("Invalid choice. Please select 1-5.")
                continue

            # Send the encoded string to the robot
            print(f"\nSending -> {command_str}")
            client_socket.sendall(command_str.encode('ascii'))
            
            # If EXIT was sent, break the loop and close out
            if command_str == "EXIT":
                print("Disconnect signal sent. Exiting CLI...")
                break
                
            # Wait for the RAPID server to finish processing and send "ACK"
            print("Waiting for acknowledgment from controller...")
            response = client_socket.recv(1024).decode('ascii')
            print(f"Robot replied <- {response}")

    except KeyboardInterrupt:
        print("\nCLI forcefully interrupted by user.")
    except Exception as e:
        print(f"\nAn unexpected network error occurred: {e}")
    finally:
        print("Closing socket connection.")
        client_socket.close()

if __name__ == "__main__":
    main()