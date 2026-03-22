# OTA (Over-The-Air) Update Instructions

## What is OTA?
OTA allows you to upload new firmware wirelessly without connecting USB cable to your ESP32-CAM.

## Configuration
- **Hostname**: `esp32-cam-4wd.local` (or use your ESP32's IP address if mDNS doesn't work)
- **Default Password**: `12345678` (defined in code - IMPORTANT: Change `DEVICE_PASSWORD` in robotuko_kodas.cpp before deployment!)
- **Port**: 3232 (default ArduinoOTA port)

## How to Upload via OTA

### Method 1: Using PlatformIO (Recommended)

1. **Make sure ESP32-CAM is connected to WiFi and running**
   - Check serial monitor shows: `[OTA] Ready for updates`

2. **Upload via OTA**:
   ```powershell
   platformio run --target upload
   ```
   
   The platformio.ini is already configured to use OTA by default.

3. **To switch back to USB upload** (if needed):
   ```powershell
   platformio run --target upload --upload-port COM9
   ```

### Method 2: Using IP Address Instead of Hostname

If mDNS doesn't work (esp32-cam-4wd.local not resolving), edit `platformio.ini`:

```ini
upload_port = <YOUR_ESP32_IP>  ; Replace with your ESP32's IP address
```

### Method 3: Using Arduino IDE

1. Open Arduino IDE
2. Go to Tools → Port
3. You should see: `esp32-cam-4wd at <IP_ADDRESS>` (Network Port)
4. Select it and upload normally

## Troubleshooting

### OTA Device Not Found
- Check ESP32-CAM is powered on and connected to WiFi
- Check serial monitor shows `[OTA] Ready for updates`
- Try using IP address instead of hostname
- Make sure PC and ESP32-CAM are on same network

### Authentication Failed
- Password in platformio.ini must match password in code (both use `DEVICE_PASSWORD` constant)
- Current password: `12345678` (default - should be changed for security)
- Change in both places if needed:
  - Code: `const char *DEVICE_PASSWORD = "12345678";` (at top of robotuko_kodas.cpp)
  - platformio.ini: `--auth=12345678`

### Connection Timeout
- Check firewall isn't blocking port 3232
- ESP32-CAM and PC must be on same WiFi network
- Try rebooting ESP32-CAM

## Benefits of OTA

✅ No USB cable needed
✅ Update robot while it's moving
✅ Update from anywhere on same network
✅ Faster development cycle
✅ Perfect for deployed/mounted devices

## Security Note

**Change the OTA password!** The default `robot123` is not secure.

Edit in `robotuko_kodas.cpp`:
```cpp
ArduinoOTA.setPassword("your-secure-password-here");
```

And in `platformio.ini`:
```ini
--auth=your-secure-password-here
```
