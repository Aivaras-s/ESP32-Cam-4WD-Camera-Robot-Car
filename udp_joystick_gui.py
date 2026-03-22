#!/usr/bin/env python3
"""
ESP32-CAM Robot UDP Controller with GUI
Features:
- Visual draggable joystick
- HTTP video stream display
- Speed control buttons
- LED toggle
- Connection status
"""

import socket
import threading
import time
from io import BytesIO
import tkinter as tk
from tkinter import ttk
import urllib.request
import urllib.error

try:
    from PIL import Image, ImageTk
    PIL_AVAILABLE = True
except ImportError:
    PIL_AVAILABLE = False
    print("PIL not installed - video stream disabled")
    print("Install with: pip install pillow")

# ESP32 Configuration
ESP32_IP = "192.168.220.231"
ESP32_UDP_PORT = 8888
ESP32_STREAM_URL = f"http://{ESP32_IP}:81/stream"

class JoystickController:
    def __init__(self, master):
        self.master = master
        self.master.title("ESP32-CAM Robot Controller")
        self.master.geometry("900x700")
        self.master.configure(bg='#2b2b2b')
        
        # Control state
        self.joystick_x = 0  # -100 to 100
        self.joystick_y = 0  # -100 to 100
        self.speed_level = 1
        self.led_state = False
        self.is_dragging = False
        self.running = True
        
        # UDP socket
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.settimeout(0.1)
        
        # Video stream
        self.video_thread = None
        self.current_frame = None
        
        self.setup_ui()
        self.start_control_loop()
        if PIL_AVAILABLE:
            self.start_video_stream()
        
    def setup_ui(self):
        """Setup the GUI layout"""
        # Main container
        main_frame = tk.Frame(self.master, bg='#2b2b2b')
        main_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Left panel - Video stream
        left_frame = tk.Frame(main_frame, bg='#1a1a1a', relief=tk.RIDGE, bd=2)
        left_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=(0, 5))
        
        # Video label
        tk.Label(left_frame, text="Live Video Stream", bg='#1a1a1a', fg='white', 
                font=('Arial', 12, 'bold')).pack(pady=5)
        
        self.video_label = tk.Label(left_frame, bg='black')
        self.video_label.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        if not PIL_AVAILABLE:
            tk.Label(self.video_label, text="Install Pillow for video:\npip install pillow", 
                    bg='black', fg='red', font=('Arial', 14)).pack(expand=True)
        
        # Right panel - Controls
        right_frame = tk.Frame(main_frame, bg='#2b2b2b')
        right_frame.pack(side=tk.RIGHT, fill=tk.BOTH, padx=(5, 0))
        
        # Status panel
        status_frame = tk.LabelFrame(right_frame, text="Status", bg='#2b2b2b', fg='white',
                                     font=('Arial', 10, 'bold'))
        status_frame.pack(fill=tk.X, pady=(0, 10))
        
        self.status_label = tk.Label(status_frame, text="Connecting...", bg='#2b2b2b', 
                                     fg='yellow', font=('Arial', 10))
        self.status_label.pack(pady=5)
        
        self.position_label = tk.Label(status_frame, text="X: 0  Y: 0", bg='#2b2b2b',
                                       fg='#4a9eff', font=('Courier', 10))
        self.position_label.pack(pady=2)
        
        # Joystick panel
        joystick_frame = tk.LabelFrame(right_frame, text="Joystick", bg='#2b2b2b', fg='white',
                                       font=('Arial', 10, 'bold'))
        joystick_frame.pack(fill=tk.BOTH, expand=True, pady=(0, 10))
        
        # Joystick canvas
        self.joystick_size = 250
        self.joystick_center = self.joystick_size // 2
        self.joystick_radius = self.joystick_size // 2 - 10
        self.stick_radius = 25
        
        self.canvas = tk.Canvas(joystick_frame, width=self.joystick_size, 
                               height=self.joystick_size, bg='#1a1a1a', 
                               highlightthickness=0)
        self.canvas.pack(padx=10, pady=10)
        
        # Draw joystick base
        self.canvas.create_oval(5, 5, self.joystick_size-5, self.joystick_size-5,
                               outline='#4a9eff', width=3)
        
        # Draw crosshair
        self.canvas.create_line(self.joystick_center, 0, self.joystick_center, 
                               self.joystick_size, fill='#555555', dash=(2, 2))
        self.canvas.create_line(0, self.joystick_center, self.joystick_size, 
                               self.joystick_center, fill='#555555', dash=(2, 2))
        
        # Draw direction markers
        self.canvas.create_text(self.joystick_center, 15, text="FWD", fill='#888888')
        self.canvas.create_text(self.joystick_center, self.joystick_size-15, text="BACK", fill='#888888')
        self.canvas.create_text(15, self.joystick_center, text="L", fill='#888888')
        self.canvas.create_text(self.joystick_size-15, self.joystick_center, text="R", fill='#888888')
        
        # Joystick stick
        self.stick = self.canvas.create_oval(
            self.joystick_center - self.stick_radius,
            self.joystick_center - self.stick_radius,
            self.joystick_center + self.stick_radius,
            self.joystick_center + self.stick_radius,
            fill='#4a9eff', outline='white', width=2
        )
        
        # Bind mouse events
        self.canvas.bind('<Button-1>', self.on_joystick_press)
        self.canvas.bind('<B1-Motion>', self.on_joystick_drag)
        self.canvas.bind('<ButtonRelease-1>', self.on_joystick_release)
        
        # Control buttons panel
        control_frame = tk.LabelFrame(right_frame, text="Controls", bg='#2b2b2b', fg='white',
                                      font=('Arial', 10, 'bold'))
        control_frame.pack(fill=tk.X, pady=(0, 10))
        
        # Speed controls
        speed_frame = tk.Frame(control_frame, bg='#2b2b2b')
        speed_frame.pack(pady=5)
        
        tk.Label(speed_frame, text="Speed:", bg='#2b2b2b', fg='white',
                font=('Arial', 10)).pack(side=tk.LEFT, padx=5)
        
        self.speed_var = tk.StringVar(value="1")
        for i in range(1, 4):
            rb = tk.Radiobutton(speed_frame, text=str(i), variable=self.speed_var,
                              value=str(i), command=self.on_speed_change,
                              bg='#2b2b2b', fg='white', selectcolor='#4a9eff',
                              font=('Arial', 10, 'bold'))
            rb.pack(side=tk.LEFT, padx=2)
        
        # LED toggle
        self.led_button = tk.Button(control_frame, text="LED: OFF", command=self.toggle_led,
                                    bg='#555555', fg='white', font=('Arial', 10, 'bold'),
                                    width=15, height=2)
        self.led_button.pack(pady=5)
        
        # Stop button
        stop_button = tk.Button(control_frame, text="STOP", command=self.emergency_stop,
                               bg='#e74c3c', fg='white', font=('Arial', 12, 'bold'),
                               width=15, height=2)
        stop_button.pack(pady=5)
        
        # Info label
        info_frame = tk.Frame(right_frame, bg='#2b2b2b')
        info_frame.pack(fill=tk.X)
        
        info_text = "Drag joystick to control\nRelease to stop"
        tk.Label(info_frame, text=info_text, bg='#2b2b2b', fg='#888888',
                font=('Arial', 9), justify=tk.CENTER).pack(pady=5)
    
    def on_joystick_press(self, event):
        """Handle joystick press"""
        self.is_dragging = True
        self.update_joystick_position(event.x, event.y)
    
    def on_joystick_drag(self, event):
        """Handle joystick drag"""
        if self.is_dragging:
            self.update_joystick_position(event.x, event.y)
    
    def on_joystick_release(self, event):
        """Handle joystick release"""
        self.is_dragging = False
        self.joystick_x = 0
        self.joystick_y = 0
        self.update_stick_visual()
    
    def update_joystick_position(self, x, y):
        """Update joystick position from mouse coordinates"""
        # Calculate offset from center
        dx = x - self.joystick_center
        dy = y - self.joystick_center
        
        # Calculate distance from center
        distance = (dx**2 + dy**2)**0.5
        
        # Constrain to joystick radius
        if distance > self.joystick_radius:
            angle = abs(dy / dx) if dx != 0 else float('inf')
            dx = self.joystick_radius * (dx / distance)
            dy = self.joystick_radius * (dy / distance)
            distance = self.joystick_radius
        
        # Convert to -100 to 100 range
        # X: -100 (left) to +100 (right)
        # Y: -100 (back/down) to +100 (forward/up) - inverted because canvas Y is top-down
        self.joystick_x = int((dx / self.joystick_radius) * 100)
        self.joystick_y = -int((dy / self.joystick_radius) * 100)  # Invert Y
        
        # Apply deadzone
        if abs(self.joystick_x) < 10:
            self.joystick_x = 0
        if abs(self.joystick_y) < 10:
            self.joystick_y = 0
        
        self.update_stick_visual()
    
    def update_stick_visual(self):
        """Update visual position of joystick stick"""
        # Convert joystick values back to canvas coordinates
        canvas_x = self.joystick_center + (self.joystick_x / 100.0) * self.joystick_radius
        canvas_y = self.joystick_center - (self.joystick_y / 100.0) * self.joystick_radius
        
        # Update stick position
        self.canvas.coords(self.stick,
                          canvas_x - self.stick_radius,
                          canvas_y - self.stick_radius,
                          canvas_x + self.stick_radius,
                          canvas_y + self.stick_radius)
        
        # Update position label
        self.position_label.config(text=f"X: {self.joystick_x:4d}  Y: {self.joystick_y:4d}")
    
    def on_speed_change(self):
        """Handle speed level change"""
        self.speed_level = int(self.speed_var.get())
        self.send_command(f"SPEED{self.speed_level}")
    
    def toggle_led(self):
        """Toggle LED state"""
        self.led_state = not self.led_state
        self.send_command("LED")
        self.led_button.config(
            text=f"LED: {'ON' if self.led_state else 'OFF'}",
            bg='#27ae60' if self.led_state else '#555555'
        )
    
    def emergency_stop(self):
        """Emergency stop"""
        self.joystick_x = 0
        self.joystick_y = 0
        self.is_dragging = False
        self.update_stick_visual()
        self.send_command("STOP")
    
    def send_command(self, command):
        """Send UDP command to ESP32"""
        try:
            self.sock.sendto(command.encode(), (ESP32_IP, ESP32_UDP_PORT))
            return True
        except Exception as e:
            print(f"Error sending command: {e}")
            return False
    
    def control_loop(self):
        """Main control loop - sends joystick position"""
        last_x = 0
        last_y = 0
        last_send_time = 0
        
        while self.running:
            current_time = time.time()
            
            # Send joystick position if changed OR every 500ms (to keep watchdog happy)
            should_send = (self.joystick_x != last_x or self.joystick_y != last_y or 
                          current_time - last_send_time > 0.5)
            
            if should_send:
                command = f"JOY:{self.joystick_x},{self.joystick_y}"
                if self.send_command(command):
                    self.master.after(0, lambda: self.status_label.config(
                        text="Connected ✓", fg='#27ae60'))
                    last_send_time = current_time
                else:
                    self.master.after(0, lambda: self.status_label.config(
                        text="Connection Error", fg='#e74c3c'))
                
                last_x = self.joystick_x
                last_y = self.joystick_y
            
            time.sleep(0.05)  # 20Hz update rate
    
    def start_control_loop(self):
        """Start the control loop thread"""
        control_thread = threading.Thread(target=self.control_loop, daemon=True)
        control_thread.start()
    
    def video_stream_loop(self):
        """Video stream loop"""
        stream = None
        error_count = 0
        max_errors = 5
        
        while self.running:
            try:
                if stream is None:
                    # Open stream
                    stream = urllib.request.urlopen(ESP32_STREAM_URL, timeout=5)
                    error_count = 0
                
                # Read frame with multipart MJPEG parsing
                bytes_data = b''
                while self.running:
                    chunk = stream.read(4096)
                    if not chunk:
                        break
                    
                    bytes_data += chunk
                    
                    # Find JPEG boundaries
                    a = bytes_data.find(b'\xff\xd8')  # JPEG start
                    b = bytes_data.find(b'\xff\xd9')  # JPEG end
                    
                    if a != -1 and b != -1 and b > a:
                        jpg = bytes_data[a:b+2]
                        bytes_data = bytes_data[b+2:]
                        
                        # Decode and display frame
                        try:
                            image = Image.open(BytesIO(jpg))
                            # Resize to fit window (max 640x480)
                            image.thumbnail((640, 480), Image.Resampling.LANCZOS)
                            photo = ImageTk.PhotoImage(image)
                            
                            self.video_label.config(image=photo)
                            self.video_label.image = photo
                            
                            # Frame displayed successfully, clear error count
                            error_count = 0
                        except Exception as e:
                            # Skip silently - likely incomplete frame
                            pass
                        
                        # Limit buffer size
                        if len(bytes_data) > 100000:
                            bytes_data = b''
                        
                        break
                
                time.sleep(0.001)  # Small delay
                
            except Exception as e:
                print(f"Video stream error: {e}")
                error_count += 1
                if stream:
                    stream.close()
                    stream = None
                
                if error_count >= max_errors:
                    self.master.after(0, lambda: self.video_label.config(
                        text=f"Video stream error\n{ESP32_STREAM_URL}\n\nRetrying...",
                        fg='red', bg='black'))
                    time.sleep(2)
                    error_count = 0
                else:
                    time.sleep(0.5)
    
    def start_video_stream(self):
        """Start video stream thread"""
        self.video_thread = threading.Thread(target=self.video_stream_loop, daemon=True)
        self.video_thread.start()
    
    def on_closing(self):
        """Handle window closing"""
        self.running = False
        self.send_command("STOP")
        time.sleep(0.1)
        self.sock.close()
        self.master.destroy()

def main():
    root = tk.Tk()
    app = JoystickController(root)
    root.protocol("WM_DELETE_WINDOW", app.on_closing)
    root.mainloop()

if __name__ == "__main__":
    main()
