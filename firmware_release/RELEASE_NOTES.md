# ESP32-CAM 4WD Camera Robot - Release v1.0

**Release Date:** November 24, 2025

## 🎉 Welcome

This is the official release package for the enhanced ESP32-CAM 4WD Camera Robot firmware. No compilation needed - just download and flash!

---

## 📥 Quick Start

### 1️⃣ Download the Firmware Files

Download these three files from this release:
- `bootloader.bin`
- `partitions.bin`
- `firmware.bin`

### 2️⃣ Flash Using Web Flasher (Easiest!)

1. Go to: **https://esptool.spacehuhn.com/**
   - Use Google Chrome or Microsoft Edge
   
2. Plug in your ESP32-CAM via USB

3. Select your device from the dropdown

4. Add the three files with these addresses:
   | Address | File |
   |---------|------|
   | 0x1000 | `bootloader.bin` |
   | 0x8000 | `partitions.bin` |
   | 0x10000 | `firmware.bin` |

5. Click **Program** and wait for success ✅

### 3️⃣ First Time Setup

After flashing:
1. Device boots into AP (Access Point) mode
2. Connect to WiFi: `ESP32-CAM-Setup` / Password: `12345678`
3. Open: http://192.168.4.1
4. Configure your WiFi network
5. Device will reboot and connect
6. Access at: http://esp32-cam-4wd.local

---

## ✨ Features

### Connectivity & Control
- 🌐 WiFi AP mode for easy setup (no pre-configuration needed)
- 📡 Automatic client mode with roaming support
- 🎮 Web-based virtual joystick interface
- 🕹️ UDP control protocol for low-latency commands
- ⚡ WebSocket real-time communication
- 🔄 Over-The-Air (OTA) wireless updates

### Camera & Streaming
- 📷 MJPEG video streaming with adjustable quality
- 🎥 Live preview in web browser
- 🚀 ~20 FPS at QVGA resolution
- 📊 Real-time performance monitoring

### Motor Control
- 🎯 4-wheel drive with independent motor control
- 🚗 Joystick-based movement (forward, backward, turn)
- ⚙️ Three speed levels: Low / Medium / High
- 🛑 Safety watchdog (auto-stops motors after 1 second idle)
- 📱 UDP control with Python scripts included

### Hardware
- ✅ AI-Thinker ESP32-CAM (4MB flash)
- ✅ OV2640 camera module
- ✅ RGB LED control
- ✅ 4 DC motor drivers
- ✅ mDNS hostname support: `esp32-cam-4wd.local`

---

## 📋 What's Included

### Binary Files (for flashing)
- **bootloader.bin** (17 KB) - ESP32 bootloader
- **partitions.bin** (3 KB) - OTA partition table
- **firmware.bin** (988 KB) - Main application firmware

### Documentation
- **FLASH_INSTRUCTIONS.md** - Detailed flashing guide (4 methods)
- **README.md** - Complete feature documentation
- **OTA_INSTRUCTIONS.md** - Wireless update guide

---

## 🔧 Technical Details

### Memory Layout
- **Total Flash:** 4MB
- **NVS (WiFi Settings):** 20KB @ 0x9000
- **OTA Data:** 8KB @ 0xE000
- **App0 (Active):** 1.9MB @ 0x10000
- **App1 (Update):** 1.9MB @ 0x1F0000
- **SPIFFS (File System):** 192KB @ 0x3D0000

### Network Ports
- **HTTP Web UI:** Port 80
- **MJPEG Stream:** Port 81
- **UDP Control:** Port 8888
- **UDP Video:** Port 8889
- **OTA Updates:** Port 3232

### Performance
- **WiFi:** 802.11 b/g/n @ 2.4GHz
- **Streaming:** ~20 FPS at QVGA, ~10 FPS at VGA
- **Control Latency:** <50ms (UDP mode)
- **Motor Response:** Instantaneous

---

## 🚀 System Requirements

### For Flashing
- **Operating System:** Windows, macOS, or Linux
- **Browser:** Google Chrome or Microsoft Edge (for web flasher)
- **USB Cable:** Any USB-A to Micro-USB cable
- **USB Driver:** CP2102 or CH340 (auto-installed on most systems)

### For Using the Robot
- **WiFi Network:** 2.4GHz WiFi (802.11 b/g/n)
- **Power:** 5V 2A USB power or 7.4V LiPo battery
- **Display:** Any device with a web browser

---

## 🎮 How to Use

### Via Web Interface
1. Open `http://esp32-cam-4wd.local` in your browser
2. Click and drag the joystick to control movement
3. Watch the camera feed in the top-right
4. Toggle LED, check WiFi signal strength

### Via UDP Control (Python)
```bash
# Download the included Python scripts
python test_udp_control.py esp32-cam-4wd.local
```

**Keyboard Controls:**
- W/↑ - Forward
- S/↓ - Backward  
- A/← - Turn Left
- D/→ - Turn Right
- 1/2 - Speed control
- R - Toggle LED
- X - Stop motors

### Via Video Streaming
```bash
python udp_video_viewer.py esp32-cam-4wd.local
```

---

## ⚠️ Safety & Security

### Important Notes

1. **Change Default Passwords** (for production use):
   - AP Mode Password: `12345678`
   - OTA Update Password: `robot123`
   - Edit source code to change these permanently

2. **Motor Safety:**
   - Motors auto-stop after 1 second without commands
   - Always test in open space first
   - Keep hands clear of rotating wheels
   - Disconnect power when not in use

3. **Network Security:**
   - Device has NO authentication on web interface
   - Only use on trusted/private networks
   - Not recommended for internet-exposed deployments
   - Consider adding password protection for production

4. **Power Requirements:**
   - Minimum: 5V 2A USB power
   - Recommended: 7.4V 2S LiPo battery for mobile use
   - Low power can cause camera/motor failure
   - Use quality USB cables to prevent voltage drops

---

## 🔄 Updating Firmware

### Via OTA (Over-The-Air)
Once on your WiFi network, update wirelessly without USB cable:

**Option 1: PlatformIO (if you have source code)**
```bash
platformio run -e esp32cam_ota --target upload --upload-port esp32-cam-4wd.local
```

**Option 2: Web Flasher**
1. Go to https://esptool.spacehuhn.com/
2. Select device on WiFi network
3. Flash new firmware (device must be on same network)

### Via USB (Local)
1. Download new firmware release
2. Flash as described in Quick Start section above

---

## 🐛 Troubleshooting

### Device Not Detected
- Try different USB port
- Try different USB cable (USB 3.0 sometimes has issues)
- Install CP2102 or CH340 drivers
- Check Device Manager (Windows) for unknown devices

### Flashing Failed
- Disconnect and reconnect USB cable
- Try lower baud rate (115200 instead of 460800)
- Close other serial monitor programs
- Ensure device has power and is fully connected

### WiFi Connection Issues
- Device will fall back to AP mode automatically
- Connect to `ESP32-CAM-Setup` AP and reconfigure
- Check WiFi SSID/password for typos
- Try moving closer to router
- Check router supports 2.4GHz (not 5GHz only)

### Camera Not Working
- Verify camera ribbon cable is properly seated
- Check power supply (needs stable 5V)
- Try reflashing firmware
- Check serial output for initialization errors

### Joystick/Motor Not Responding
- Device may be in AP mode - connect to WiFi first
- Check motor power supply
- Verify USB power is adequate (minimum 5V 2A)
- Make sure motors are properly wired

### Can't Access Web Interface
- Try using IP address instead of hostname
- Check device is connected to same WiFi network
- Refresh browser page
- Clear browser cache
- Try different browser

---

## 📞 Support

### Getting Help

1. **Check Documentation:**
   - FLASH_INSTRUCTIONS.md - Detailed flashing guides
   - OTA_INSTRUCTIONS.md - Update procedures
   - README.md - Complete feature reference

2. **Enable Debug Output:**
   - Edit source code: `#define DEBUG_ENABLED 1`
   - Rebuild and reflash
   - Monitor serial output at 115200 baud

3. **Serial Monitor Command:**
   ```bash
   platformio device monitor --port COM9 --baud 115200 --rts 0 --dtr 0 --filter direct
   ```

### Tools & Resources

- **Web Flasher:** https://esptool.spacehuhn.com/
- **esptool.py:** https://github.com/espressif/esptool
- **PlatformIO:** https://platformio.org/
- **ESP32 Documentation:** https://docs.espressif.com/projects/esp-idf/
- **Arduino-ESP32:** https://github.com/espressif/arduino-esp32

### USB Drivers

- **CP2102 Drivers:** https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
- **CH340 Drivers:** http://www.wch-ic.com/downloads/CH341SER_EXE.html

---

## 📊 Hardware Compatibility

### ✅ Confirmed Working
- AI-Thinker ESP32-CAM (recommended)
- OV2640 camera module
- Standard ESP32 camera drivers
- 4MB flash (minimum requirement)

### ⚠️ May Have Issues
- Other ESP32 boards (pins may differ)
- Non-OV2640 cameras (different initialization)
- Boards with < 4MB flash (size issues)

---

## 🔄 Version Information

- **Firmware Version:** v1.0
- **Release Date:** November 24, 2025
- **Board:** AI-Thinker ESP32-CAM
- **Flash Size:** 4MB (1.9MB app partitions)
- **Base Framework:** Arduino-ESP32
- **HTTP Server:** ESP-IDF native implementation

### Features in This Release
- ✅ WiFi AP/Client mode with auto-fallback
- ✅ MJPEG streaming with configurable quality
- ✅ Web-based joystick interface
- ✅ UDP control protocol
- ✅ OTA wireless updates
- ✅ mDNS hostname support (esp32-cam-4wd.local)
- ✅ WiFi roaming optimization
- ✅ Motor safety watchdog
- ✅ Signal strength monitoring
- ✅ Dual communication protocols

---

## 📝 License & Credits

This firmware builds upon:
- **ESP32 Arduino Core** - https://github.com/espressif/arduino-esp32
- **ESP32 Camera Driver** - ESP-IDF components
- **ArduinoOTA** - Standard Arduino framework

All modifications and enhancements are provided as-is for educational and personal use.

---

## 🎯 Next Steps

1. ✅ **Flash the firmware** (see Quick Start)
2. ✅ **Configure WiFi** (see First Time Setup)
3. ✅ **Access web interface** at http://esp32-cam-4wd.local
4. ✅ **Test camera** and motor control
5. ✅ **Enjoy your robot!** 🤖

**Questions or issues?** Check the troubleshooting section above or review the detailed documentation files.

---

**Happy Robotics! 🤖✨**

For source code and additional information, visit the main project repository.
