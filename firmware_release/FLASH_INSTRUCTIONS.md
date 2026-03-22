# ESP32-CAM Robot Firmware - Flash Instructions

## Firmware Version
- **Date**: November 24, 2025
- **Features**: WiFi AP/Client mode, Camera streaming, Motor control, WebSocket, OTA updates, WiFi roaming
- **Board**: AI-Thinker ESP32-CAM
- **Flash Size**: 4MB

## Files Included
- `bootloader.bin` - ESP32 bootloader
- `partitions.bin` - Partition table (OTA enabled)
- `firmware.bin` - Main application firmware

## Recommended Flashing Tools

### Option 1: esptool.py (Cross-platform, Recommended)

**Install:**
```bash
pip install esptool
```

**Flash Command:**
```bash
esptool.py --chip esp32 --port COM9 --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
```

**Windows PowerShell:**
```powershell
python -m esptool --chip esp32 --port COM9 --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
```

**Linux/Mac:**
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size 4MB 0x1000 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
```

### Option 2: ESP32 Flash Download Tool (Windows GUI)

**Download:** https://www.espressif.com/en/support/download/other-tools

**Steps:**
1. Open Flash Download Tool
2. Select "ESP32" chip type
3. Add files with these addresses:
   - `bootloader.bin` @ 0x1000
   - `partitions.bin` @ 0x8000
   - `firmware.bin` @ 0x10000
4. Select COM port (e.g., COM9)
5. Set baud rate: 460800
6. Flash settings:
   - SPI Speed: 40MHz
   - SPI Mode: DIO
   - Flash Size: 4MB
7. Click "START" to flash

### Option 3: PlatformIO (For Developers)

If you have the source code:
```bash
platformio run -e esp32cam --target upload --upload-port COM9
```

### Option 4: Web-based esptool.spacehuhn.com (Browser - No Installation)

**🌟 Easiest Method - No Installation Required!**

This is a web-based tool that works in **Google Chrome** or **Microsoft Edge** browsers. No software installation needed!

**Steps:**

1. **Prepare the files:**
   - Download all three files from this folder:
     - `bootloader.bin`
     - `partitions.bin`
     - `firmware.bin`
   - Save them to your computer in an easy-to-find location

2. **Open the web flasher:**
   - Go to: https://esptool.spacehuhn.com/
   - Use **Google Chrome** or **Microsoft Edge** (other browsers may not work)

3. **Connect your ESP32-CAM:**
   - Plug the ESP32-CAM into your computer via USB cable
   - Wait 1-2 seconds for the device to be recognized

4. **Select the COM port:**
   - Click the dropdown menu in the web tool (usually says "Select a device")
   - Choose your ESP32-CAM device (often shows as "USB JTAG/serial debug unit" or "COM9")
   - If you don't see it, try a different USB cable or USB port

5. **Add firmware files:**
   - You will see fields for different memory addresses. Fill them in as follows:

   | Address | File | 
   |---------|------|
   | 0x1000 | Select `bootloader.bin` |
   | 0x8000 | Select `partitions.bin` |
   | 0x10000 | Select `firmware.bin` |

   **How to add files:**
   - Click on each address field
   - Click "Browse" or the file selector button
   - Navigate to and select the corresponding `.bin` file

6. **Configure flash settings:**
   - **Baud Rate:** 460800 (usually pre-selected)
   - **Flash Mode:** DIO
   - **Flash Frequency:** 40MHz
   - **Flash Size:** Keep auto-detected or set to 4MB

7. **Start flashing:**
   - Click the "Program" or "Flash" button
   - Wait for the progress bar to complete (usually 10-30 seconds)
   - You should see a success message when done

8. **Verify successful flash:**
   - The device should reboot automatically
   - Check the LED on the ESP32-CAM (should blink briefly)
   - If you see errors, try again at 115200 baud rate

**Troubleshooting:**

- **Device not detected:** Try a different USB port or cable
- **Connection timeout:** Disconnect and reconnect the USB cable, then refresh the browser
- **Permission denied error:** Close any other programs using the COM port
- **Flashing failed:** Retry at 115200 baud rate instead of 460800

**Browser Requirements:**
- ✅ Google Chrome (recommended)
- ✅ Microsoft Edge (Chromium-based)
- ❌ Firefox (WebSerial not supported)
- ❌ Safari (WebSerial not supported)

## First Time Setup

After flashing, the device will:

1. **Boot into AP mode** (no WiFi configured)
   - SSID: `ESP32-CAM-Setup`
   - Password: `12345678`
   
2. **Connect to the AP** and open browser to:
   - http://192.168.4.1 (default AP address)
   
3. **Configure WiFi** on the setup page
   - Enter your WiFi SSID and password
   - Click "Save WiFi Settings"
   - Device will reboot and connect to your WiFi

4. **Access the robot** at:
   - http://esp32-cam-4wd.local (if mDNS works)
   - Or check your router for the IP address

## Features

### WiFi Management
- ✅ AP mode for initial setup
- ✅ Client mode with auto-reconnect (5 attempts)
- ✅ WiFi roaming with gratuitous ARP
- ✅ Hostname: `esp32-cam-4wd`
- ✅ Delete WiFi settings option

### Camera
- ✅ MJPEG streaming on port 81
- ✅ Web interface with live video feed
- ✅ Adjustable quality and framesize

### Motor Control
- ✅ 4-wheel drive control
- ✅ Joystick interface (web-based)
- ✅ Speed control (Low/Medium/High)
- ✅ Safety watchdog (auto-stop after 1s)
- ✅ UDP control support (port 8888)

### Network Features
- ✅ WebSocket communication (low latency)
- ✅ WiFi signal strength monitoring
- ✅ Ping/latency measurement
- ✅ OTA (Over-The-Air) updates

## OTA Updates

Once flashed, future updates can be done wirelessly:

```bash
# Using PlatformIO
platformio run -e esp32cam_ota --target upload --upload-port esp32-cam-4wd.local

# Or using IP address (replace with your ESP32's actual IP)
platformio run -e esp32cam_ota --target upload --upload-port <YOUR_ESP32_IP>
```

**OTA Password:** `robot123` (change in code for security!)

## Troubleshooting

### Device not booting
- Check power supply (needs 5V 2A minimum)
- Verify flash was successful
- Try re-flashing at lower baud rate (115200)

### Can't connect to WiFi
- Device will return to AP mode if WiFi fails
- Connect to `ESP32-CAM-Setup` AP
- Reconfigure WiFi settings
- Check SSID/password are correct

### Camera not working
- Ensure AI-Thinker board is selected
- Camera module properly connected
- Check serial output for errors

### Serial Port Issues (Windows)
- Install CP2102 or CH340 USB drivers
- Check Device Manager for COM port
- Try different USB cable/port

## Default Credentials

### Access Point Mode
- **SSID:** ESP32-CAM-Setup
- **Password:** 12345678
- **IP:** 192.168.4.1

### OTA Updates
- **Password:** robot123
- **Port:** 3232

### Web Interface
- **Port:** 80 (HTTP)
- **Stream Port:** 81 (MJPEG)
- **UDP Control:** 8888

## Pin Configuration

### Camera Pins (AI-Thinker)
- PWDN: GPIO 32
- RESET: -1 (not used)
- XCLK: GPIO 0
- SIOD: GPIO 26
- SIOC: GPIO 27
- Y9-Y2: GPIO 35,34,39,36,21,19,18,5
- VSYNC: GPIO 25
- HREF: GPIO 23
- PCLK: GPIO 22

### Motor Pins
- **Right Motor:** GPIO 14, 15
- **Left Motor:** GPIO 13, 12
- **LED:** GPIO 4

## Support

For issues or questions, check serial monitor at 115200 baud with DEBUG_ENABLED=1 in the source code.

**Serial Monitor Command:**
```bash
platformio device monitor --port COM9 --baud 115200 --rts 0 --dtr 0 --filter direct
```
