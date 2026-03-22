# UDP Video Streaming - Installation & Usage

## Overview
The ESP32-CAM now supports UDP video streaming on port 8889, which handles packet loss much better than HTTP streaming.

## Requirements
Install OpenCV and numpy for Python:

```powershell
pip install opencv-python numpy
```

## Usage

### 1. Start UDP Video Viewer
**Easy way (recommended - no need to find IP):**
```powershell
python udp_video_viewer.py esp32-cam-4wd.local
```

**Or with IP address:**
```powershell
python udp_video_viewer.py 192.168.220.231
```

### 2. Control Robot (separate terminal)
**Easy way (recommended):**
```powershell
python test_udp_control.py esp32-cam-4wd.local
```

**Or with IP address:**
```powershell
python test_udp_control.py 192.168.220.231
```

## Key Features

### UDP Video Advantages:
- **Better packet loss handling** - Missing packets don't block the stream
- **Lower latency** - No TCP retransmissions
- **Smoother playback** - Continues even with WiFi interference
- **Real-time performance** - ~20 FPS at QVGA resolution

### Controls (test_udp_control.py):
- **W / ↑** - Forward
- **S / ↓** - Backward
- **A / ←** - Left
- **D / →** - Right
- **Q** - Forward Left (gentle arc)
- **E** - Forward Right (gentle arc)
- **X** - Stop
- **R** - LED Toggle
- **1** - Speed Down
- **2** - Speed Up
- **ESC** - Quit

### UDP Ports:
- **8888** - Control commands
- **8889** - Video streaming

## How It Works

1. **Client sends "START"** to UDP port 8889
2. **ESP32-CAM sends frame header** with size and packet count
3. **ESP32-CAM sends frame data** in ~1400 byte chunks (safe MTU)
4. **Client reassembles packets** and decodes JPEG
5. **Missing packets = skip frame** (no waiting/blocking)

## Troubleshooting

### No video appearing:
- Check firewall allows UDP port 8889
- Try using mDNS hostname: `esp32-cam-4wd.local` (easier than finding IP)
- If hostname doesn't work, find your robot's IP from router settings
- Check serial output for "[VIDEO] Client connected" message

### Choppy video:
- Move closer to WiFi router
- Reduce WiFi interference
- Lower resolution in web interface (use QVGA for best UDP performance)

### Video stops when sending many commands:
- This should now be fixed with yield() calls
- UDP video runs independently of control commands

## Technical Details

- **Frame rate**: 20 FPS (50ms interval)
- **Packet size**: 1400 bytes (MTU-safe)
- **Protocol**: UDP with packet numbering
- **Encoding**: JPEG from OV2640 camera
- **Resolution**: Configurable via web interface (QVGA recommended for UDP)

## Comparison: HTTP vs UDP

| Feature | HTTP MJPEG | UDP Streaming |
|---------|------------|---------------|
| Latency | Higher (TCP) | Lower |
| Packet Loss | Blocks/Retries | Skips frame |
| Interference | Sensitive | Resilient |
| Smoothness | Can freeze | Always flows |
| CPU Usage | Higher | Lower |

## Note
Both HTTP streaming (port 80) and UDP streaming (port 8889) work simultaneously. Use whichever works best for your network conditions!
