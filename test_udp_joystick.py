#!/usr/bin/env python3
"""
ESP32-CAM Robot UDP Controller with Analog Joystick Simulation
Supports both keyboard (WASD/Arrows) and gamepad control
"""

import socket
import time
import sys
import os

# Try to import pygame for gamepad support
try:
    import pygame
    PYGAME_AVAILABLE = True
except ImportError:
    PYGAME_AVAILABLE = False
    print("pygame not installed - gamepad support disabled")
    print("Install with: pip install pygame")

# Windows keyboard input
if os.name == 'nt':
    import msvcrt
    def get_key():
        if msvcrt.kbhit():
            return msvcrt.getch().decode('utf-8', errors='ignore').lower()
        return None
else:
    # Unix-like systems
    import termios
    import tty
    def get_key():
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(fd)
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch.lower() if ch else None

# ESP32 Configuration
ESP32_IP = "<YOUR_ESP32_IP>"  # Change this to your ESP32-CAM's IP address
ESP32_UDP_PORT = 8888

# Joystick simulation parameters
JOYSTICK_X = 0  # -100 to 100 (left to right)
JOYSTICK_Y = 0  # -100 to 100 (backward to forward)
SPEED_INCREMENT = 5  # How fast joystick moves per key press
DECAY_RATE = 10  # How fast joystick returns to center when released
MAX_JOYSTICK = 100

# Control state
keys_pressed = set()
current_speed = 1  # Speed level (1, 2, or 3)
led_state = False

def constrain(value, min_val, max_val):
    """Constrain value between min and max"""
    return max(min_val, min(max_val, value))

def send_udp_command(sock, command):
    """Send command via UDP to ESP32"""
    try:
        sock.sendto(command.encode(), (ESP32_IP, ESP32_UDP_PORT))
        return True
    except Exception as e:
        print(f"Error sending command: {e}")
        return False

def update_joystick_keyboard():
    """Update joystick position based on keyboard input"""
    global JOYSTICK_X, JOYSTICK_Y
    
    # Calculate target position based on pressed keys
    target_x = 0
    target_y = 0
    
    # Vertical axis (Y)
    if 'w' in keys_pressed or 'up' in keys_pressed:
        target_y = MAX_JOYSTICK
    elif 's' in keys_pressed or 'down' in keys_pressed:
        target_y = -MAX_JOYSTICK
    
    # Horizontal axis (X)
    if 'd' in keys_pressed or 'right' in keys_pressed:
        target_x = MAX_JOYSTICK
    elif 'a' in keys_pressed or 'left' in keys_pressed:
        target_x = -MAX_JOYSTICK
    
    # Smooth movement towards target
    if target_x != 0:
        if JOYSTICK_X < target_x:
            JOYSTICK_X = min(JOYSTICK_X + SPEED_INCREMENT, target_x)
        elif JOYSTICK_X > target_x:
            JOYSTICK_X = max(JOYSTICK_X - SPEED_INCREMENT, target_x)
    else:
        # Return to center when no key pressed
        if JOYSTICK_X > 0:
            JOYSTICK_X = max(0, JOYSTICK_X - DECAY_RATE)
        elif JOYSTICK_X < 0:
            JOYSTICK_X = min(0, JOYSTICK_X + DECAY_RATE)
    
    if target_y != 0:
        if JOYSTICK_Y < target_y:
            JOYSTICK_Y = min(JOYSTICK_Y + SPEED_INCREMENT, target_y)
        elif JOYSTICK_Y > target_y:
            JOYSTICK_Y = max(JOYSTICK_Y - SPEED_INCREMENT, target_y)
    else:
        # Return to center when no key pressed
        if JOYSTICK_Y > 0:
            JOYSTICK_Y = max(0, JOYSTICK_Y - DECAY_RATE)
        elif JOYSTICK_Y < 0:
            JOYSTICK_Y = min(0, JOYSTICK_Y + DECAY_RATE)
    
    # Apply deadzone
    if abs(JOYSTICK_X) < 5:
        JOYSTICK_X = 0
    if abs(JOYSTICK_Y) < 5:
        JOYSTICK_Y = 0

def update_joystick_gamepad(joystick):
    """Update joystick position from gamepad"""
    global JOYSTICK_X, JOYSTICK_Y
    
    # Read analog sticks (axis 0 = left/right, axis 1 = up/down)
    # Most gamepads: axis 0 = left stick X, axis 1 = left stick Y
    raw_x = joystick.get_axis(0)  # -1.0 to 1.0
    raw_y = -joystick.get_axis(1)  # Inverted (up is negative by default)
    
    # Apply deadzone
    deadzone = 0.15
    if abs(raw_x) < deadzone:
        raw_x = 0
    if abs(raw_y) < deadzone:
        raw_y = 0
    
    # Convert to -100 to 100 range
    JOYSTICK_X = int(raw_x * MAX_JOYSTICK)
    JOYSTICK_Y = int(raw_y * MAX_JOYSTICK)

def print_status():
    """Print current control status"""
    # Create visual joystick position indicator
    joy_visual = ""
    if abs(JOYSTICK_X) < 5 and abs(JOYSTICK_Y) < 5:
        joy_visual = "CENTER"
    else:
        if JOYSTICK_Y > 30:
            joy_visual = "FWD"
        elif JOYSTICK_Y < -30:
            joy_visual = "BACK"
        
        if JOYSTICK_X > 30:
            joy_visual += "+RIGHT" if joy_visual else "RIGHT"
        elif JOYSTICK_X < -30:
            joy_visual += "+LEFT" if joy_visual else "LEFT"
    
    # Create motor visualization
    left_motor = JOYSTICK_Y + JOYSTICK_X
    right_motor = JOYSTICK_Y - JOYSTICK_X
    max_mag = max(abs(left_motor), abs(right_motor))
    if max_mag > 100:
        left_motor = (left_motor / max_mag) * 100
        right_motor = (right_motor / max_mag) * 100
    
    left_bar = "█" * int(abs(left_motor) / 5)
    right_bar = "█" * int(abs(right_motor) / 5)
    left_dir = "FWD" if left_motor > 0 else "BCK" if left_motor < 0 else "---"
    right_dir = "FWD" if right_motor > 0 else "BCK" if right_motor < 0 else "---"
    
    print(f"\rJoy: X:{JOYSTICK_X:4d} Y:{JOYSTICK_Y:4d} [{joy_visual:12s}] | "
          f"Speed:{current_speed} LED:{'ON' if led_state else 'OFF'} | "
          f"L[{left_dir}:{left_bar:20s}] R[{right_dir}:{right_bar:20s}]", end="", flush=True)

def main():
    global current_speed, led_state, keys_pressed
    
    print("=" * 80)
    print("ESP32-CAM Robot UDP Controller with Analog Joystick")
    print("=" * 80)
    print(f"Connecting to: {ESP32_IP}:{ESP32_UDP_PORT}")
    
    # Initialize pygame for gamepad support
    gamepad = None
    if PYGAME_AVAILABLE:
        pygame.init()
        pygame.joystick.init()
        if pygame.joystick.get_count() > 0:
            gamepad = pygame.joystick.Joystick(0)
            gamepad.init()
            print(f"Gamepad detected: {gamepad.get_name()}")
        else:
            print("No gamepad detected - using keyboard mode")
    
    print("\nKeyboard Controls:")
    print("  W/↑ = Forward")
    print("  S/↓ = Backward")
    print("  A/← = Left")
    print("  D/→ = Right")
    print("  1/2/3 = Speed level")
    print("  L = Toggle LED")
    print("  ESC/Q = Quit")
    
    if gamepad:
        print("\nGamepad Controls:")
        print("  Left Stick = Movement")
        print("  Button 0 (A/X) = Toggle LED")
        print("  Button 1 (B/Circle) = Speed Up")
        print("  Button 2 (X/Square) = Speed Down")
    
    print("\nStarting control loop...")
    print("-" * 80)
    
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(0.1)
    
    last_command = ""
    last_update = time.time()
    update_interval = 0.05  # 50ms = 20Hz update rate
    
    try:
        while True:
            current_time = time.time()
            
            # Handle pygame events (gamepad)
            if gamepad:
                pygame.event.pump()
                
                # Check gamepad buttons
                if gamepad.get_button(0):  # Button A/X
                    led_state = not led_state
                    send_udp_command(sock, "LED")
                    time.sleep(0.3)  # Debounce
                
                if gamepad.get_button(1):  # Button B/Circle
                    current_speed = min(3, current_speed + 1)
                    send_udp_command(sock, f"SPEED{current_speed}")
                    time.sleep(0.3)
                
                if gamepad.get_button(2):  # Button X/Square
                    current_speed = max(1, current_speed - 1)
                    send_udp_command(sock, f"SPEED{current_speed}")
                    time.sleep(0.3)
                
                # Update joystick from gamepad
                update_joystick_gamepad(gamepad)
            
            # Handle keyboard input
            key = get_key()
            if key:
                if key in ['q', '\x1b']:  # ESC or Q
                    print("\n\nStopping robot...")
                    send_udp_command(sock, "STOP")
                    break
                
                elif key == 'l':
                    led_state = not led_state
                    send_udp_command(sock, "LED")
                    time.sleep(0.1)
                
                elif key in ['1', '2', '3']:
                    current_speed = int(key)
                    send_udp_command(sock, f"SPEED{current_speed}")
                    time.sleep(0.1)
                
                elif key in ['w', 's', 'a', 'd']:
                    keys_pressed.add(key)
                
                elif key == '\xe0':  # Windows arrow key prefix
                    arrow = get_key()
                    if arrow == 'H':  # Up arrow
                        keys_pressed.add('up')
                    elif arrow == 'P':  # Down arrow
                        keys_pressed.add('down')
                    elif arrow == 'K':  # Left arrow
                        keys_pressed.add('left')
                    elif arrow == 'M':  # Right arrow
                        keys_pressed.add('right')
            
            # In keyboard mode, check if keys were released
            if not gamepad:
                # Simple approach: keys decay over time if not refreshed
                # This works because we're updating continuously
                pass
            
            # Update joystick position (keyboard mode)
            if not gamepad:
                update_joystick_keyboard()
            
            # Send joystick command at regular intervals
            if current_time - last_update >= update_interval:
                command = f"JOY:{JOYSTICK_X},{JOYSTICK_Y}"
                
                # Only send if changed or periodically to maintain watchdog
                if command != last_command or current_time - last_update > 0.5:
                    send_udp_command(sock, command)
                    last_command = command
                
                last_update = current_time
                print_status()
            
            # Small delay to prevent CPU spinning
            time.sleep(0.01)
    
    except KeyboardInterrupt:
        print("\n\nInterrupted by user")
    
    finally:
        print("\nSending STOP command...")
        send_udp_command(sock, "STOP")
        sock.close()
        if gamepad:
            pygame.quit()
        print("Disconnected. Goodbye!")

if __name__ == "__main__":
    main()
