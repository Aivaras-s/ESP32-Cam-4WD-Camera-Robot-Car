#!/usr/bin/env python3
"""
UDP Control Test Script for ESP32-CAM Robot
Usage: python test_udp_control.py <ESP32_IP> [command]
Commands: FWD, BWD, LEFT, RIGHT, STOP, LED_ON, LED_OFF, SPEED+, SPEED-
"""

import socket
import sys
import time

UDP_PORT = 8888
TIMEOUT = 2.0

def send_udp_command(ip, command):
    """Send a command via UDP to the ESP32-CAM"""
    try:
        # Create UDP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.settimeout(TIMEOUT)
        
        # Send command
        message = command.encode('utf-8')
        sock.sendto(message, (ip, UDP_PORT))
        print(f"[UDP] Sent '{command}' to {ip}:{UDP_PORT}")
        
        # Wait for acknowledgment
        try:
            data, server = sock.recvfrom(1024)
            print(f"[UDP] Received ACK: {data.decode('utf-8')} from {server[0]}:{server[1]}")
            return True
        except socket.timeout:
            print("[UDP] No acknowledgment received (timeout)")
            return False
            
    except Exception as e:
        print(f"[ERROR] {e}")
        return False
    finally:
        sock.close()

def interactive_mode(ip):
    """Interactive control mode"""
    print("\n=== ESP32-CAM UDP Control ===")
    print(f"Connected to: {ip}:{UDP_PORT}")
    print("\nCommands:")
    print("  w/↑ - Forward")
    print("  s/↓ - Backward")
    print("  a/← - Left")
    print("  d/→ - Right")
    print("  q   - Forward Left")
    print("  e   - Forward Right")
    print("  x   - Stop")
    print("  r   - LED Toggle")
    print("  1   - Speed Down (instant)")
    print("  2   - Speed Up (instant)")
    print("  ESC - Quit")
    print("\nPress keys to control the robot:\n")
    
    led_state = False
    last_command = None
    last_send_time = 0
    send_interval = 0.5  # Send command every 500ms to keep robot moving
    
    try:
        import msvcrt  # Windows only
        while True:
            current_time = time.time()
            keys_pressed = set()  # Track keys in this iteration
            
            # Read all available keys
            while msvcrt.kbhit():
                key_bytes = msvcrt.getch()
                
                # Handle special keys (arrows, etc.) - they send 0xe0 or 0x00 first
                if key_bytes in (b'\xe0', b'\x00'):
                    if msvcrt.kbhit():
                        key_bytes = msvcrt.getch()  # Get the actual key code
                    else:
                        continue
                
                # Try to decode as UTF-8, skip if it fails
                try:
                    key = key_bytes.decode('utf-8').lower()
                except UnicodeDecodeError:
                    # Check for arrow key scan codes
                    key_code = ord(key_bytes)
                    if key_code == 72:  # Up arrow
                        key = 'up'
                    elif key_code == 80:  # Down arrow
                        key = 'down'
                    elif key_code == 75:  # Left arrow
                        key = 'left'
                    elif key_code == 77:  # Right arrow
                        key = 'right'
                    else:
                        continue
                
                # Handle immediate commands (speed, LED, quit)
                if key == '2':
                    send_udp_command(ip, "SPEED+")
                    continue
                elif key == '1':
                    send_udp_command(ip, "SPEED-")
                    continue
                elif key == 'r':
                    led_state = not led_state
                    send_udp_command(ip, "LED_ON" if led_state else "LED_OFF")
                    continue
                elif key == '\x1b':  # ESC key
                    print("\nExiting...")
                    send_udp_command(ip, "STOP")  # Stop on exit
                    return
                
                # Add movement keys to pressed set
                if key in ['w', 'up', 's', 'down', 'a', 'left', 'd', 'right', 'q', 'e', 'x']:
                    keys_pressed.add(key)
            
            # Build combined command from pressed keys
            command = None
            
            # Check for stop first (highest priority)
            if 'x' in keys_pressed:
                command = "STOP"
            # Q and E as direct forward-left/forward-right commands
            elif 'q' in keys_pressed:
                command = "FWD+LEFT"
            elif 'e' in keys_pressed:
                command = "FWD+RIGHT"
            # Check for forward/backward + left/right combinations
            elif ('w' in keys_pressed or 'up' in keys_pressed) and ('d' in keys_pressed or 'right' in keys_pressed):
                command = "FWD+RIGHT"
            elif ('w' in keys_pressed or 'up' in keys_pressed) and ('a' in keys_pressed or 'left' in keys_pressed):
                command = "FWD+LEFT"
            elif ('s' in keys_pressed or 'down' in keys_pressed) and ('d' in keys_pressed or 'right' in keys_pressed):
                command = "BWD+RIGHT"
            elif ('s' in keys_pressed or 'down' in keys_pressed) and ('a' in keys_pressed or 'left' in keys_pressed):
                command = "BWD+LEFT"
            # Single direction commands
            elif 'w' in keys_pressed or 'up' in keys_pressed:
                command = "FWD"
            elif 's' in keys_pressed or 'down' in keys_pressed:
                command = "BWD"
            elif 'a' in keys_pressed or 'left' in keys_pressed:
                command = "LEFT"
            elif 'd' in keys_pressed or 'right' in keys_pressed:
                command = "RIGHT"
            
            # Send command periodically or when changed
            if command:
                # Send immediately if command changed or it's time to refresh
                if command != last_command or (current_time - last_send_time) >= send_interval:
                    send_udp_command(ip, command)
                    last_command = command
                    last_send_time = current_time
            
            time.sleep(0.05)  # Small delay to prevent flooding
                    
    except ImportError:
        print("\n[INFO] Interactive mode requires Windows. Use command-line mode instead.")
        print(f"Example: python {sys.argv[0]} {ip} FWD")

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <ESP32_IP> [command]")
        print(f"Example: {sys.argv[0]} 192.168.1.100 FWD")
        print(f"         {sys.argv[0]} 192.168.1.100  (interactive mode)")
        sys.exit(1)
    
    esp32_ip = sys.argv[1]
    
    if len(sys.argv) >= 3:
        # Single command mode
        command = sys.argv[2].upper()
        send_udp_command(esp32_ip, command)
    else:
        # Interactive mode
        interactive_mode(esp32_ip)

if __name__ == "__main__":
    main()
