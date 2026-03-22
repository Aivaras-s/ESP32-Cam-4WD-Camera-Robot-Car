# ESP32-CAM 4WD Robot Firmware - Release Package

## Overview
Complete firmware package for ESP32-CAM based 4-wheel drive robot with camera streaming, motor control, and WiFi management.

## Quick Start

### 🌐 Web-Based Flashing (Easiest - No Installation!)

**Use your browser to flash directly:**

1. Open **https://esptool.spacehuhn.com/** (Chrome or Edge required)
2. Plug in your ESP32-CAM via USB
3. Select the device from the dropdown
4. Add files at these addresses:
   - `bootloader.bin` → **0x1000**
   - `partitions.bin` → **0x8000**
   - `firmware.bin` → **0x10000**
5. Click **Program** ✅

**For complete instructions,** see `FLASH_INSTRUCTIONS.md` (Option 4: Web-based esptool.spacehuhn.com)

---

### Command-Line Flashing

1. **Flash the firmware** using one of these methods:
   - See `FLASH_INSTRUCTIONS.md` for detailed steps
   - Recommended: esptool.py command-line tool
   - Alternative: ESP32 Flash Download Tool (Windows GUI)

2. **Connect to setup AP:**
   - SSID: `ESP32-CAM-Setup`
   - Password: `12345678`
   - Open browser: http://192.168.4.1

3. **Configure WiFi** and start using your robot!

## 📦 GitHub Release

**Pre-built Firmware Files** are available in GitHub Releases for quick flashing without compilation.

### Release Contents
- ✅ **bootloader.bin** - ESP32 bootloader
- ✅ **partitions.bin** - Partition table (OTA enabled)
- ✅ **firmware.bin** - Main application firmware (1.9MB app partition)
- 📖 **FLASH_INSTRUCTIONS.md** - Complete flashing guide
- 📋 **README.md** - Quick reference

### How to Use Release Files

**For Windows/Mac/Linux users:**
1. Download the three `.bin` files from GitHub Release
2. Open https://esptool.spacehuhn.com/ in Chrome or Edge
3. Connect your ESP32-CAM
4. Select the three files with addresses shown above
5. Click Program

**No compilation needed!** The firmware is ready to flash directly onto your device.

## Package Contents

### Firmware Files
- `firmware.bin` (988KB) - Main application
- `bootloader.bin` (17KB) - ESP32 bootloader
- `partitions.bin` (3KB) - OTA partition table

### Documentation
- `FLASH_INSTRUCTIONS.md` - Detailed flashing guide with tool recommendations
- `OTA_INSTRUCTIONS.md` - Wireless update procedures
- `README.md` - This file

## Key Features

### WiFi & Network
- 🌐 AP mode for initial setup (ESP32-CAM-Setup)
- 🔌 Client mode with auto-reconnect
- 🔄 WiFi roaming optimization (gratuitous ARP)
- 📡 mDNS hostname: esp32-cam-4wd.local
- 🛡️ Fast WiFi scanning and signal-based connection

### Camera
- 📷 MJPEG streaming (port 81)
- 🎥 Web interface with live video
- ⚙️ Adjustable quality and frame size
- 🚀 8-71 fps performance

### Motor Control
- 🎮 4-wheel drive with joystick interface
- ⚡ Three speed levels (Low/Medium/High)
- 🛑 Safety watchdog (auto-stop after 1s)
- 🌐 WebSocket low-latency control
- 📡 UDP control support (port 8888)

### System Features
- 🔧 Over-The-Air (OTA) updates
- 💾 Non-volatile WiFi credential storage
- 🔍 WiFi signal strength monitoring
- 📊 WebSocket ping/pong latency measurement
- 🎨 Custom robot favicon
- 🔐 Secure OTA with password protection

## Technical Specifications

### Hardware
- **Board:** AI-Thinker ESP32-CAM
- **MCU:** ESP32 (240MHz dual-core)
- **RAM:** 320KB SRAM
- **Flash:** 4MB
- **Camera:** OV2640
- **WiFi:** 802.11 b/g/n

### Partition Layout (OTA Enabled)
- **NVS:** 20KB @ 0x9000 (WiFi credentials)
- **OTA Data:** 8KB @ 0xE000 (boot selector)
- **App0:** 1.9MB @ 0x10000 (active firmware)
- **App1:** 1.9MB @ 0x1F0000 (update slot)
- **SPIFFS:** 192KB @ 0x3D0000 (file system)

### Network Ports
- **HTTP:** 80 (web interface)
- **MJPEG:** 81 (camera stream)
- **WebSocket:** /ws (motor control)
- **UDP:** 8888 (control protocol)
- **OTA:** 3232 (wireless updates)

## Default Credentials

### Access Point Mode
- **SSID:** ESP32-CAM-Setup
- **Password:** 12345678
- **Gateway:** 192.168.4.1

### OTA Updates
- **Password:** robot123 ⚠️ **Change this for production!**
- **Hostname:** esp32-cam-4wd

## Flashing Quick Reference

### Windows (esptool.py)
```powershell
python -m esptool --chip esp32 --port COM9 --baud 460800 write_flash 0x1000 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
```

### Linux/Mac (esptool.py)
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 write_flash 0x1000 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
```

See `FLASH_INSTRUCTIONS.md` for complete commands with all parameters.

## Usage Examples

### Access Web Interface
```
http://esp32-cam-4wd.local
or
http://192.168.x.x (check your router)
```

### View Camera Stream
```
http://esp32-cam-4wd.local:81/stream
```

### UDP Joystick Control (Python example)
```python
import socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
# Send joystick command: X=128 (center), Y=200 (forward)
sock.sendto(b'JOY:128,200', ('192.168.x.x', 8888))
```

### OTA Update (PlatformIO)
```bash
platformio run -e esp32cam_ota --target upload --upload-port esp32-cam-4wd.local
```

## Troubleshooting

### Flash Failed
- Check USB connection and drivers (CP2102/CH340)
- Try lower baud rate: 115200 instead of 460800
- Ensure device is in flash mode (GPIO 0 to GND while booting)
- Verify power supply (5V 2A minimum)

### WiFi Connection Issues
- Device will fall back to AP mode automatically
- Connect to ESP32-CAM-Setup AP
- Reconfigure WiFi via web interface
- Check WiFi credentials for typos

### Camera Not Working
- Verify AI-Thinker board model
- Check camera ribbon cable connection
- Ensure proper power supply (camera needs stable 5V)
- Check serial output for initialization errors

### OTA Update Fails
- Verify device is on same network
- Check hostname resolution: `ping esp32-cam-4wd.local`
- Try using IP address instead of hostname
- Ensure OTA password matches (default: robot123)
- Check firewall allows port 3232

## Development

### Serial Monitor
```bash
platformio device monitor --port COM9 --baud 115200 --rts 0 --dtr 0 --filter direct
```

### Enable Debug Output
In source code, set:
```cpp
#define DEBUG_ENABLED 1
```

### Source Code Repository
This firmware was built with PlatformIO. To modify:
1. Install PlatformIO IDE or CLI
2. Open project folder
3. Edit `src/robotuko_kodas.cpp`
4. Build: `platformio run -e esp32cam`
5. Upload via USB: `platformio run -e esp32cam --target upload`
6. Upload via OTA: `platformio run -e esp32cam_ota --target upload`

## Version History

### v1.0 (Current Release)
- ✅ WiFi captive portal with AP fallback
- ✅ Dynamic WiFi configuration and storage
- ✅ Camera MJPEG streaming
- ✅ WebSocket motor control with joystick
- ✅ OTA update support with dual-bank partitions
- ✅ WiFi roaming optimization (gratuitous ARP)
- ✅ WebSocket latency measurement
- ✅ Safety features (motor watchdog, reconnection logic)
- ✅ UDP control protocol

## Safety Notes

1. **Change default passwords** before deploying:
   - OTA password in `robotuko_kodas.cpp`
   - AP password if exposing to others

2. **Motor safety:**
   - 1-second watchdog automatically stops motors
   - Always test in safe environment first
   - Keep emergency power disconnect accessible

3. **Network security:**
   - Device has no authentication on web interface
   - Only use on trusted networks
   - Consider adding authentication for production use

4. **Power requirements:**
   - Minimum 5V 2A power supply
   - Use quality USB cable or direct power
   - Motors may cause voltage drops affecting camera

## Support & Resources

### Tools
- **esptool.py:** https://github.com/espressif/esptool
- **ESP32 Flash Tool:** https://www.espressif.com/en/support/download/other-tools
- **PlatformIO:** https://platformio.org/

### Drivers
- **CP2102:** https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
- **CH340:** http://www.wch-ic.com/downloads/CH341SER_EXE.html

### Espressif Resources
- **ESP32 Docs:** https://docs.espressif.com/projects/esp-idf/en/latest/esp32/
- **Arduino-ESP32:** https://github.com/espressif/arduino-esp32

## License & Credits

Built with:
- ESP32 Arduino Core
- ESP32 Camera Driver
- ArduinoOTA
- ESPAsyncWebServer concepts

---

**Happy Robotics! 🤖**
