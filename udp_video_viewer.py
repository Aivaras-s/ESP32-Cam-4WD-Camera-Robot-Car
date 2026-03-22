#!/usr/bin/env python3
"""
ESP32-CAM UDP Video Viewer
Receives JPEG frames via UDP and displays them with OpenCV
Better packet loss handling than HTTP streaming
"""

import socket
import cv2
import numpy as np
import sys
import struct
import time

UDP_VIDEO_PORT = 8889
BUFFER_SIZE = 2048

class UDPVideoReceiver:
    def __init__(self, esp32_ip, port=UDP_VIDEO_PORT):
        self.esp32_ip = esp32_ip
        self.port = port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.settimeout(1.0)
        self.running = False
        self.frame_buffer = {}
        self.expected_packets = 0
        self.frame_size = 0
        
    def start_stream(self):
        """Send START command to ESP32-CAM"""
        try:
            self.sock.sendto(b"START", (self.esp32_ip, self.port))
            print(f"[INFO] Sent START command to {self.esp32_ip}:{self.port}")
            self.running = True
        except Exception as e:
            print(f"[ERROR] Failed to start stream: {e}")
            return False
        return True
    
    def stop_stream(self):
        """Send STOP command to ESP32-CAM"""
        try:
            self.sock.sendto(b"STOP", (self.esp32_ip, self.port))
            print("[INFO] Sent STOP command")
            self.running = False
        except Exception as e:
            print(f"[ERROR] Failed to stop stream: {e}")
    
    def receive_frame(self):
        """Receive a complete JPEG frame via UDP packets"""
        self.frame_buffer = {}
        self.expected_packets = 0
        self.frame_size = 0
        packets_received = 0
        timeout_start = time.time()
        
        while self.running:
            try:
                data, addr = self.sock.recvfrom(BUFFER_SIZE)
                
                # Check if this is a header packet (starts with 0xFF 0xD8)
                if len(data) >= 8 and data[0] == 0xFF and data[1] == 0xD8:
                    # Parse header
                    self.frame_size = struct.unpack('>I', data[2:6])[0]
                    self.expected_packets = struct.unpack('>H', data[6:8])[0]
                    self.frame_buffer = {}
                    packets_received = 0
                    timeout_start = time.time()
                    continue
                
                # Data packet (packet_num + data)
                if len(data) >= 2 and self.expected_packets > 0:
                    packet_num = struct.unpack('>H', data[0:2])[0]
                    self.frame_buffer[packet_num] = data[2:]
                    packets_received += 1
                    
                    # Check if we have all packets
                    if packets_received >= self.expected_packets:
                        # Reconstruct frame
                        frame_data = b''
                        for i in range(self.expected_packets):
                            if i in self.frame_buffer:
                                frame_data += self.frame_buffer[i]
                            else:
                                # Missing packet - skip this frame
                                print(f"[WARN] Missing packet {i}/{self.expected_packets}")
                                return None
                        
                        # Decode JPEG
                        return cv2.imdecode(np.frombuffer(frame_data, dtype=np.uint8), cv2.IMREAD_COLOR)
                
                # Timeout if taking too long
                if time.time() - timeout_start > 2.0:
                    if packets_received > 0:
                        print(f"[WARN] Frame timeout: {packets_received}/{self.expected_packets} packets")
                    return None
                    
            except socket.timeout:
                continue
            except Exception as e:
                print(f"[ERROR] Receive error: {e}")
                return None
        
        return None
    
    def close(self):
        """Close socket"""
        self.stop_stream()
        self.sock.close()

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <ESP32_IP>")
        print(f"Example: {sys.argv[0]} 192.168.220.231")
        sys.exit(1)
    
    esp32_ip = sys.argv[1]
    
    print("=== ESP32-CAM UDP Video Viewer ===")
    print(f"Connecting to: {esp32_ip}:{UDP_VIDEO_PORT}")
    print("Press 'q' to quit\n")
    
    receiver = UDPVideoReceiver(esp32_ip)
    
    if not receiver.start_stream():
        sys.exit(1)
    
    cv2.namedWindow("ESP32-CAM UDP Stream", cv2.WINDOW_NORMAL)
    
    frame_count = 0
    fps_start = time.time()
    fps = 0
    
    try:
        while True:
            frame = receiver.receive_frame()
            
            if frame is not None:
                frame_count += 1
                
                # Calculate FPS
                elapsed = time.time() - fps_start
                if elapsed >= 1.0:
                    fps = frame_count / elapsed
                    frame_count = 0
                    fps_start = time.time()
                
                # Display FPS on frame
                cv2.putText(frame, f"FPS: {fps:.1f}", (10, 30), 
                           cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)
                
                cv2.imshow("ESP32-CAM UDP Stream", frame)
            
            # Check for quit
            if cv2.waitKey(1) & 0xFF == ord('q'):
                print("\n[INFO] Quitting...")
                break
                
    except KeyboardInterrupt:
        print("\n[INFO] Interrupted by user")
    finally:
        receiver.close()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
