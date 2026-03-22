# ESP32-CAM Robot - Three Code Versions

## Overview
This project now has **three different firmware versions** you can choose from based on your needs.

---

## Version 1: ESP-IDF HTTP Server (Original - CURRENTLY ACTIVE)
**File:** `src/robotuko_kodas.cpp` (current)  
**Backup:** `src/robotuko_kodas.cpp.backup`

### Features:
✅ **ESP-IDF native HTTP server** (esp_http_server.h)  
✅ **Separate streaming port** (Port 80 for control, Port 81 for stream)  
✅ **UDP control** (Port 8888)  
✅ **Analog virtual joystick** interface  
✅ **MJPEG streaming** with resolution selector  
✅ **Motor watchdog** (1 second timeout)  
✅ **WiFi/latency status** display  

### Technical:
- Uses: `httpd_handle_t` for HTTP server
- RAM: **17.5%** (57,444 bytes)
- Flash: **28.3%** (889,073 bytes)
- Streaming: Blocking MJPEG on separate port

### Best For:
- **Stable HTTP streaming** with separate video port
- **Traditional ESP32 development** style
- **Lower memory overhead**

---

## Version 2: AsyncWebServer + WebSocket
**File:** `src/robotuko_kodas.cpp.async_version`

### Features:
✅ **AsyncWebServer** (ESPAsyncWebServer-esphome)  
✅ **WebSocket** real-time control (ws://)  
✅ **UDP control** (Port 8888)  
✅ **UDP video streaming** (Port 8889) - NEW!  
✅ **Non-blocking MJPEG** streaming  
✅ **Analog virtual joystick** interface  
✅ **Packet loss resistant** video  

### Technical:
- Uses: `AsyncWebServer`, `AsyncWebSocket`, `AsyncMjpegResponse`
- RAM: **15.4%** (50,396 bytes)
- Flash: **29.8%** (937,217 bytes)
- Streaming: Async chunked + UDP option

### Best For:
- **Multiple simultaneous clients**
- **WebSocket real-time control** from browser
- **UDP video** for poor WiFi conditions
- **Non-blocking operations**

---

## Version 3: AsyncWebServer (No WebSocket/UDP Video)
**File:** *(can be created by removing WS/UDP video from Version 2)*

### Features:
✅ **AsyncWebServer** only  
✅ **HTTP REST API** control (/action?cmd=)  
✅ **UDP control** (Port 8888)  
✅ **Non-blocking MJPEG** streaming  
✅ **Analog virtual joystick** interface  
✅ **Simplified codebase**  

### Best For:
- **Simplest async implementation**
- **HTTP-only control**
- **No WebSocket overhead**

---

## Quick Comparison

| Feature | Version 1 (ESP-IDF) | Version 2 (Async+WS+UDP) | Version 3 (Async Only) |
|---------|---------------------|--------------------------|------------------------|
| **HTTP Server** | ESP-IDF Native | AsyncWebServer | AsyncWebServer |
| **WebSocket** | ❌ No | ✅ Yes | ❌ No |
| **UDP Control** | ✅ Port 8888 | ✅ Port 8888 | ✅ Port 8888 |
| **UDP Video** | ❌ No | ✅ Port 8889 | ❌ No |
| **MJPEG Stream** | Blocking (Port 81) | Non-blocking (Port 80) | Non-blocking (Port 80) |
| **Virtual Joystick** | ✅ Yes | ✅ Yes | ✅ Yes |
| **RAM Usage** | 17.5% | 15.4% | ~15% |
| **Flash Usage** | 28.3% | 29.8% | ~28% |
| **Multiple Clients** | Limited | Excellent | Good |
| **WiFi Resilience** | Good | Excellent | Good |

---

## How to Switch Between Versions

### Activate Version 1 (ESP-IDF - Current):
```powershell
Copy-Item "src\robotuko_kodas.cpp.backup" "src\robotuko_kodas.cpp" -Force
platformio run --target upload --upload-port COM11
```

### Activate Version 2 (AsyncWebServer + WS + UDP):
```powershell
Copy-Item "src\robotuko_kodas.cpp.async_version" "src\robotuko_kodas.cpp" -Force
platformio run --target upload --upload-port COM11
```

---

## Current Active Version
**Version 1: ESP-IDF HTTP Server with Joystick** ✅

This is the traditional ESP-IDF implementation with the new analog joystick control added. It uses:
- Native ESP32 HTTP server (`esp_http_server.h`)
- Separate streaming port (81)
- UDP control (8888)
- Virtual joystick interface

---

## Testing Recommendations

1. **Try Version 1** (current) for stable traditional operation
2. **Try Version 2** if you need:
   - Browser WebSocket control
   - UDP video for bad WiFi
   - Multiple simultaneous viewers
3. **Use UDP control script** (`test_udp_control.py`) with any version

---

## Access Points

### Web Interface:
```
http://192.168.220.231       # Main control page (all versions)
http://192.168.220.231:81    # Stream only (Version 1)
ws://192.168.220.231/ws      # WebSocket (Version 2 only)
```

### UDP Control:
```powershell
python test_udp_control.py 192.168.220.231
```

### UDP Video (Version 2 only):
```powershell
python udp_video_viewer.py 192.168.220.231
```

---

## Recommendation

**Start with Version 1** (currently active) - it's the most tested and stable version with the new joystick control. Switch to Version 2 if you experience video freezing issues or need WebSocket features.

All versions include:
- ✅ Analog joystick control
- ✅ UDP command support
- ✅ Speed control (Low/Medium/High)
- ✅ LED toggle
- ✅ Motor watchdog safety
- ✅ Resolution selector
