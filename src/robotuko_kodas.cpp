/*
  ESP32-CAM Robot - Based on CameraWebServer Example
  Added: Motor control for robot
*/

#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <Preferences.h>
#include "esp_timer.h"
#include "img_converters.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "lwip/etharp.h"
#include <ESPmDNS.h>
#include <ArduinoOTA.h>

// Debug control - set to 0 to disable all debug output
#define DEBUG_ENABLED 0

// ===== CONFIGURATION - CHANGE THESE BEFORE DEPLOYMENT =====
// WiFi and OTA password - used for both Access Point and OTA updates
// IMPORTANT: Change this to a secure password before deploying to production!
const char *DEVICE_PASSWORD = "12345678";
// ============================================================

// WiFi credentials - will be loaded from preferences
char ssid[32] = "";
char password[64] = "";

// Access Point credentials
const char *ap_ssid = "ESP32-CAM-Setup";
const char *ap_password = DEVICE_PASSWORD;

// WiFi mode
bool isAPMode = false;

// WiFi reconnection tracking
int wifiReconnectAttempts = 0;
const int MAX_RECONNECT_ATTEMPTS = 5;
unsigned long lastWiFiCheck = 0;
const unsigned long WIFI_CHECK_INTERVAL = 10000; // Check WiFi every 10 seconds

// Preferences storage
Preferences preferences;

// DNS Server for captive portal
DNSServer dnsServer;
const byte DNS_PORT = 53;

// Camera pins (AI-Thinker ESP32-CAM)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// Motor pins
#define MOTOR_R_PIN_1 14
#define MOTOR_R_PIN_2 15
#define MOTOR_L_PIN_1 13
#define MOTOR_L_PIN_2 12
#define LED_GPIO_NUM 4

// Motor speed
int motorSpeed = 170;

// HTTP ports
#define HTTP_PORT 80
#define STREAM_PORT 81

// UDP Control Port
#define UDP_CONTROL_PORT 8888

// Power saving
volatile bool clientConnected = false;
volatile unsigned long lastClientActivity = 0;
#define IDLE_TIMEOUT_MS 30000  // 30 seconds of inactivity

// Motor watchdog - safety feature to stop motors if commands stop
volatile unsigned long lastMotorCommand = 0;
#define MOTOR_WATCHDOG_TIMEOUT_MS 1000  // 1 second
volatile bool motorsActive = false;

// HTTP streaming
#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

// UDP server
WiFiUDP udpServer;

// Forward declarations
void startCameraServer();
void processCommand(const char* cmd);
bool loadWiFiCredentials();
void saveWiFiCredentials(const char* newSsid, const char* newPassword);
void startAccessPoint();
void startWiFiClient();
void sendGratuitousARP();
void WiFiEvent(WiFiEvent_t event);

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // Initialize motor pins
  pinMode(MOTOR_R_PIN_1, OUTPUT);
  pinMode(MOTOR_R_PIN_2, OUTPUT);
  pinMode(MOTOR_L_PIN_1, OUTPUT);
  pinMode(MOTOR_L_PIN_2, OUTPUT);
  pinMode(LED_GPIO_NUM, OUTPUT);

  #if DEBUG_ENABLED
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();
  #endif

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    #if DEBUG_ENABLED
      Serial.printf("Camera init failed with error 0x%x", err);
    #endif
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  // drop down frame size for higher initial frame rate
  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }
  s->set_vflip(s, 1);

  // Load WiFi credentials from preferences
  if (loadWiFiCredentials() && strlen(ssid) > 0) {
    // Try to connect to saved WiFi
    #if DEBUG_ENABLED
      Serial.printf("Connecting to WiFi: %s\n", ssid);
    #endif
    
    startWiFiClient();
    
    // Wait up to 15 seconds for connection
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
      delay(500);
      #if DEBUG_ENABLED
        Serial.print(".");
      #endif
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      #if DEBUG_ENABLED
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
      #endif
      isAPMode = false;
      
      // Setup OTA updates (only in client mode)
      // Note: OTA password is kept secure and defined at the top of this file (DEVICE_PASSWORD)
      ArduinoOTA.setHostname("esp32-cam-4wd");
      ArduinoOTA.setPassword(DEVICE_PASSWORD);
      
      ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
          type = "sketch";
        } else { // U_SPIFFS
          type = "filesystem";
        }
        #if DEBUG_ENABLED
          Serial.println("[OTA] Start updating " + type);
          Serial.println("[OTA] Stopping camera and servers to free memory...");
        #endif
        
        // Stop camera to free memory
        esp_camera_deinit();
        
        // Stop HTTP servers
        if (camera_httpd != NULL) {
          httpd_stop(camera_httpd);
          camera_httpd = NULL;
        }
        if (stream_httpd != NULL) {
          httpd_stop(stream_httpd);
          stream_httpd = NULL;
        }
        
        // Stop motors
        analogWrite(MOTOR_R_PIN_1, 0);
        analogWrite(MOTOR_R_PIN_2, 0);
        analogWrite(MOTOR_L_PIN_1, 0);
        analogWrite(MOTOR_L_PIN_2, 0);
        
        #if DEBUG_ENABLED
          Serial.println("[OTA] Services stopped, ready for update");
        #endif
      });
      
      ArduinoOTA.onEnd([]() {
        #if DEBUG_ENABLED
          Serial.println("\n[OTA] Update complete!");
        #endif
      });
      
      ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        #if DEBUG_ENABLED
          Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
        #endif
      });
      
      ArduinoOTA.onError([](ota_error_t error) {
        #if DEBUG_ENABLED
          Serial.printf("[OTA] Error[%u]: ", error);
          if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
          else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
          else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
          else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
          else if (error == OTA_END_ERROR) Serial.println("End Failed");
        #endif
      });
      
      ArduinoOTA.begin();
      
      #if DEBUG_ENABLED
        Serial.println("[OTA] Ready for updates - Test v1.1");
        Serial.println("[OTA] Hostname: esp32-cam-4wd");
        Serial.println("[OTA] Testing wireless upload capability!");
      #endif
    } else {
      #if DEBUG_ENABLED
        Serial.println("");
        Serial.println("WiFi connection failed, starting AP mode");
      #endif
      startAccessPoint();
    }
  } else {
    #if DEBUG_ENABLED
      Serial.println("No WiFi credentials found, starting AP mode");
    #endif
    startAccessPoint();
  }

  startCameraServer();

  // Start UDP server (only in client mode)
  if (!isAPMode) {
    udpServer.begin(UDP_CONTROL_PORT);
  }

  #if DEBUG_ENABLED
    if (isAPMode) {
      Serial.println("========================================");
      Serial.println("AP Mode - WiFi Configuration Portal");
      Serial.print("Connect to WiFi: ");
      Serial.println(ap_ssid);
      Serial.print("Password: ");
      Serial.println(ap_password);
      Serial.print("Go to: http://");
      Serial.print(WiFi.softAPIP());
      Serial.println("/");
      Serial.println("========================================");
    } else {
      Serial.println("========================================");
      Serial.print("Camera Ready! Use 'http://");
      Serial.print(WiFi.localIP());
      Serial.println("' to connect");
      Serial.print("UDP Control Port: ");
      Serial.println(UDP_CONTROL_PORT);
      Serial.println("========================================");
    }
  #endif
}

void loop() {
  // Handle OTA updates (only in client mode)
  if (!isAPMode) {
    ArduinoOTA.handle();
  }
  
  // Handle DNS requests in AP mode for captive portal
  if (isAPMode) {
    dnsServer.processNextRequest();
  }
  
  // Monitor WiFi connection and attempt reconnection
  if (!isAPMode && (millis() - lastWiFiCheck > WIFI_CHECK_INTERVAL)) {
    lastWiFiCheck = millis();
    
    if (WiFi.status() != WL_CONNECTED) {
      wifiReconnectAttempts++;
      
      #if DEBUG_ENABLED
        Serial.printf("[WiFi] Connection lost! Reconnection attempt %d/%d\n", 
                      wifiReconnectAttempts, MAX_RECONNECT_ATTEMPTS);
      #endif
      
      if (wifiReconnectAttempts >= MAX_RECONNECT_ATTEMPTS) {
        #if DEBUG_ENABLED
          Serial.println("[WiFi] Max reconnection attempts reached. Rebooting...");
        #endif
        delay(1000);
        ESP.restart();
      } else {
        // Try to reconnect
        WiFi.disconnect();
        delay(100);
        WiFi.begin(ssid, password);
        
        // Wait up to 10 seconds for reconnection
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
          delay(500);
          attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
          #if DEBUG_ENABLED
            Serial.println("[WiFi] Reconnected successfully!");
            Serial.print("[WiFi] IP address: ");
            Serial.println(WiFi.localIP());
          #endif
          
          // Send gratuitous ARP after reconnection
          delay(100);
          sendGratuitousARP();
          
          wifiReconnectAttempts = 0; // Reset counter on successful reconnection
        }
      }
    } else {
      // Connection is good, reset counter
      wifiReconnectAttempts = 0;
    }
  }
  
  // Handle UDP control packets (only in client mode)
  int packetSize = 0;
  if (!isAPMode) {
    packetSize = udpServer.parsePacket();
  }
  if (packetSize) {
    char incomingPacket[32];
    int len = udpServer.read(incomingPacket, sizeof(incomingPacket) - 1);
    if (len > 0) {
      incomingPacket[len] = '\0';  // Null-terminate the string
      
      #if DEBUG_ENABLED
        Serial.printf("[UDP] Received: %s from %s:%d\n", 
                      incomingPacket, 
                      udpServer.remoteIP().toString().c_str(), 
                      udpServer.remotePort());
      #endif
      
      processCommand(incomingPacket);
      
      // Send acknowledgment back to sender
      udpServer.beginPacket(udpServer.remoteIP(), udpServer.remotePort());
      udpServer.write((const uint8_t*)"OK", 2);
      udpServer.endPacket();
      
      // Update activity for power saving
      lastClientActivity = millis();
    }
  }
  
  // Motor watchdog: Stop motors if no commands received for 1 second
  if (motorsActive && (millis() - lastMotorCommand > MOTOR_WATCHDOG_TIMEOUT_MS)) {
    #if DEBUG_ENABLED
      Serial.println("[WATCHDOG] No motor commands for 1s - EMERGENCY STOP!");
    #endif
    analogWrite(MOTOR_R_PIN_1, 0);
    analogWrite(MOTOR_R_PIN_2, 0);
    analogWrite(MOTOR_L_PIN_1, 0);
    analogWrite(MOTOR_L_PIN_2, 0);
    motorsActive = false;
  }
  
  // Power saving: Monitor client activity
  static unsigned long lastIdleCheck = 0;
  static bool wasIdle = false;
  
  if (millis() - lastIdleCheck > 5000) {  // Check every 5 seconds
    lastIdleCheck = millis();
    
    bool isIdle = !clientConnected && (millis() - lastClientActivity > IDLE_TIMEOUT_MS);
    
    if (isIdle && !wasIdle) {
      #if DEBUG_ENABLED
        Serial.println("[POWER SAVE] No clients for 30s - entering low power mode");
        Serial.println("[POWER SAVE] WiFi still active, motors disabled, camera on standby");
      #endif
      
      // Stop motors if running
      analogWrite(MOTOR_R_PIN_1, 0);
      analogWrite(MOTOR_R_PIN_2, 0);
      analogWrite(MOTOR_L_PIN_1, 0);
      analogWrite(MOTOR_L_PIN_2, 0);
      motorsActive = false;
      
      // Turn off LED
      digitalWrite(LED_GPIO_NUM, LOW);
      
      wasIdle = true;
    } else if (!isIdle && wasIdle) {
      #if DEBUG_ENABLED
        Serial.println("[POWER SAVE] Client activity detected - resuming normal operation");
      #endif
      wasIdle = false;
    }
    
    // Memory status logging
    #if DEBUG_ENABLED
      if (!wasIdle) {
        Serial.printf("[STATUS] Heap: %d, PSRAM: %d, Client: %s, Motors: %s\n", 
                      ESP.getFreeHeap(), 
                      ESP.getFreePsram(),
                      clientConnected ? "Connected" : "Idle",
                      motorsActive ? "Active" : "Stopped");
      }
    #endif
  }
  
  delay(100);
}

// URL decode helper function
void urlDecode(char* dst, const char* src, size_t maxLen) {
  size_t i = 0, j = 0;
  while (src[i] && j < maxLen - 1) {
    if (src[i] == '%' && src[i+1] && src[i+2]) {
      // Decode %XX hex sequence
      char hex[3] = {src[i+1], src[i+2], 0};
      dst[j++] = (char)strtol(hex, NULL, 16);
      i += 3;
    } else if (src[i] == '+') {
      // Convert + to space
      dst[j++] = ' ';
      i++;
    } else {
      dst[j++] = src[i++];
    }
  }
  dst[j] = '\0';
}

// WiFi Management Functions
bool loadWiFiCredentials() {
  preferences.begin("wifi", true); // Read-only mode
  preferences.getString("ssid", ssid, sizeof(ssid));
  preferences.getString("password", password, sizeof(password));
  preferences.end();
  
  #if DEBUG_ENABLED
    Serial.printf("Loaded credentials - SSID: %s\n", ssid);
  #endif
  
  return (strlen(ssid) > 0);
}

void saveWiFiCredentials(const char* newSsid, const char* newPassword) {
  preferences.begin("wifi", false); // Read-write mode
  preferences.putString("ssid", newSsid);
  preferences.putString("password", newPassword);
  preferences.end();
  
  strcpy(ssid, newSsid);
  strcpy(password, newPassword);
  
  #if DEBUG_ENABLED
    Serial.printf("Saved credentials - SSID: %s\n", ssid);
  #endif
}

void startAccessPoint() {
  isAPMode = true;
  WiFi.setHostname("esp32-cam-4wd");  // Set hostname before AP configuration
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  
  #if DEBUG_ENABLED
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
  #endif
  
  // Start DNS server for captive portal
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
}

// Send gratuitous ARP to update network devices
void sendGratuitousARP() {
  #if DEBUG_ENABLED
    Serial.println("[WiFi] Sending gratuitous ARP to update network...");
  #endif
  
  // Get network interface
  struct netif *netif = netif_default;
  if (netif != NULL && netif_is_up(netif)) {
    // Send gratuitous ARP (announces our IP to all devices)
    etharp_gratuitous(netif);
    delay(10);
    etharp_gratuitous(netif); // Send twice for reliability
    
    #if DEBUG_ENABLED
      Serial.println("[WiFi] Gratuitous ARP sent");
    #endif
  }
}

// WiFi event handler for roaming optimization
void WiFiEvent(WiFiEvent_t event) {
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      #if DEBUG_ENABLED
        Serial.print("[WiFi] Connected! IP: ");
        Serial.println(WiFi.localIP());
        Serial.print("[WiFi] Gateway: ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("[WiFi] BSSID: ");
        Serial.println(WiFi.BSSIDstr());
      #endif
      
      // Send gratuitous ARP immediately after getting IP
      sendGratuitousARP();
      
      // Re-announce mDNS (end first to ensure clean state)
      MDNS.end();
      if (MDNS.begin("esp32-cam-4wd")) {
        MDNS.addService("http", "tcp", 80);
        #if DEBUG_ENABLED
          Serial.println("[mDNS] Service announced: esp32-cam-4wd.local");
        #endif
      } else {
        #if DEBUG_ENABLED
          Serial.println("[mDNS] Failed to initialize");
        #endif
      }
      
      wifiReconnectAttempts = 0;
      break;
      
    case SYSTEM_EVENT_STA_DISCONNECTED:
      #if DEBUG_ENABLED
        Serial.println("[WiFi] Disconnected from AP");
      #endif
      break;
      
    default:
      break;
  }
}

void startWiFiClient() {
  WiFi.setHostname("esp32-cam-4wd");  // Set hostname BEFORE mode configuration
  WiFi.mode(WIFI_STA);
  
  // Configure for fast roaming
  WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
  WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
  
  // Set aggressive reconnect timing for faster roaming
  wifi_config_t conf;
  esp_wifi_get_config(WIFI_IF_STA, &conf);
  conf.sta.listen_interval = 3; // Check beacon every 3 intervals (faster detection)
  esp_wifi_set_config(WIFI_IF_STA, &conf);
  
  // Disable power save for more responsive connection
  WiFi.setSleep(false);
  
  // Register WiFi event handler
  WiFi.onEvent(WiFiEvent);
  
  WiFi.begin(ssid, password);
}

// Process control commands
void processCommand(const char* cmd) {
  if (strcmp(cmd, "FWD") == 0) {
    lastMotorCommand = millis();
    motorsActive = true;
    analogWrite(MOTOR_R_PIN_1, 0);
    analogWrite(MOTOR_R_PIN_2, motorSpeed);
    analogWrite(MOTOR_L_PIN_1, motorSpeed);
    analogWrite(MOTOR_L_PIN_2, 0);
  }
  else if (strcmp(cmd, "BWD") == 0) {
    lastMotorCommand = millis();
    motorsActive = true;
    analogWrite(MOTOR_R_PIN_1, motorSpeed);
    analogWrite(MOTOR_R_PIN_2, 0);
    analogWrite(MOTOR_L_PIN_1, 0);
    analogWrite(MOTOR_L_PIN_2, motorSpeed);
  }
  else if (strcmp(cmd, "LEFT") == 0) {
    lastMotorCommand = millis();
    motorsActive = true;
    analogWrite(MOTOR_R_PIN_1, 0);
    analogWrite(MOTOR_R_PIN_2, motorSpeed);
    analogWrite(MOTOR_L_PIN_1, 0);
    analogWrite(MOTOR_L_PIN_2, motorSpeed);
  }
  else if (strcmp(cmd, "RIGHT") == 0) {
    lastMotorCommand = millis();
    motorsActive = true;
    analogWrite(MOTOR_R_PIN_1, motorSpeed);
    analogWrite(MOTOR_R_PIN_2, 0);
    analogWrite(MOTOR_L_PIN_1, motorSpeed);
    analogWrite(MOTOR_L_PIN_2, 0);
  }
  else if (strcmp(cmd, "STOP") == 0) {
    motorsActive = false;
    analogWrite(MOTOR_R_PIN_1, 0);
    analogWrite(MOTOR_R_PIN_2, 0);
    analogWrite(MOTOR_L_PIN_1, 0);
    analogWrite(MOTOR_L_PIN_2, 0);
  }
  else if (strcmp(cmd, "LED_ON") == 0) {
    digitalWrite(LED_GPIO_NUM, HIGH);
  }
  else if (strcmp(cmd, "LED_OFF") == 0) {
    digitalWrite(LED_GPIO_NUM, LOW);
  }
  else if (strcmp(cmd, "SPEED+") == 0) {
    motorSpeed = constrain(motorSpeed + 85, 85, 255);
  }
  else if (strcmp(cmd, "SPEED-") == 0) {
    motorSpeed = constrain(motorSpeed - 85, 85, 255);
  }
  else if (strncmp(cmd, "JOY:", 4) == 0) {
    // Joystick command format: JOY:X,Y (X and Y from -100 to 100)
    int x = 0, y = 0;
    if (sscanf(cmd + 4, "%d,%d", &x, &y) == 2) {
      x = constrain(x, -100, 100);
      y = constrain(y, -100, 100);
      
      #if DEBUG_ENABLED
        Serial.printf("[JOY] X:%d Y:%d\n", x, y);
      #endif
      
      // Calculate motor speeds based on joystick position
      // Y axis: forward/backward (-100 to 100)
      // X axis: left/right steering (-100 to 100)
      
      if (abs(x) < 10 && abs(y) < 10) {
        // Deadzone - stop motors
        motorsActive = false;
        analogWrite(MOTOR_R_PIN_1, 0);
        analogWrite(MOTOR_R_PIN_2, 0);
        analogWrite(MOTOR_L_PIN_1, 0);
        analogWrite(MOTOR_L_PIN_2, 0);
      } else {
        lastMotorCommand = millis();
        motorsActive = true;
        
        // Differential drive algorithm
        // Convert joystick X,Y to left/right motor speeds (-100 to +100)
        // Y = forward/backward, X = turn left/right
        
        float leftMotor = y + x;   // Left motor: forward + turn component
        float rightMotor = y - x;  // Right motor: forward - turn component
        
        // Normalize if any value exceeds 100
        float maxMagnitude = max(abs(leftMotor), abs(rightMotor));
        if (maxMagnitude > 100) {
          leftMotor = (leftMotor / maxMagnitude) * 100;
          rightMotor = (rightMotor / maxMagnitude) * 100;
        }
        
        // Map to actual motor speed
        int leftSpeed = map(abs(leftMotor), 0, 100, 0, motorSpeed);
        int rightSpeed = map(abs(rightMotor), 0, 100, 0, motorSpeed);
        
        // Apply motor directions
        // Left motor
        if (leftMotor > 0) {
          // Left forward
          analogWrite(MOTOR_L_PIN_1, leftSpeed);
          analogWrite(MOTOR_L_PIN_2, 0);
        } else {
          // Left backward
          analogWrite(MOTOR_L_PIN_1, 0);
          analogWrite(MOTOR_L_PIN_2, leftSpeed);
        }
        
        // Right motor
        if (rightMotor > 0) {
          // Right forward
          analogWrite(MOTOR_R_PIN_1, 0);
          analogWrite(MOTOR_R_PIN_2, rightSpeed);
        } else {
          // Right backward
          analogWrite(MOTOR_R_PIN_1, rightSpeed);
          analogWrite(MOTOR_R_PIN_2, 0);
        }
      }
    }
  }
  else if (strcmp(cmd, "REBOOT") == 0) {
    #if DEBUG_ENABLED
      Serial.println("Rebooting...");
    #endif
    delay(100);
    ESP.restart();
  }
}

// WiFi Configuration page
static const char PROGMEM CONFIG_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32-CAM WiFi Settings</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta charset="UTF-8">
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      margin: 0;
      padding: 20px;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: #fff;
      min-height: 100vh;
    }
    .container {
      max-width: 400px;
      margin: 0 auto;
      background: rgba(255, 255, 255, 0.95);
      padding: 30px;
      border-radius: 15px;
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.3);
      color: #333;
    }
    h1 {
      color: #667eea;
      margin-bottom: 10px;
    }
    .subtitle {
      color: #666;
      font-size: 14px;
      margin-bottom: 20px;
    }
    .current-wifi {
      background: #e8f4f8;
      padding: 15px;
      border-radius: 8px;
      margin-bottom: 20px;
      text-align: left;
    }
    .current-wifi h3 {
      margin: 0 0 10px 0;
      color: #667eea;
    }
    .current-wifi p {
      margin: 5px 0;
      color: #555;
    }
    input {
      width: 100%;
      padding: 12px;
      margin: 10px 0;
      border: 2px solid #ddd;
      border-radius: 8px;
      box-sizing: border-box;
      font-size: 16px;
    }
    input:focus {
      outline: none;
      border-color: #667eea;
    }
    button {
      width: 100%;
      padding: 14px;
      border: none;
      border-radius: 8px;
      cursor: pointer;
      font-size: 16px;
      font-weight: bold;
      margin-top: 10px;
      transition: transform 0.1s;
    }
    .btn-primary {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
    }
    .btn-danger {
      background: linear-gradient(135deg, #e74c3c 0%, #c0392b 100%);
      color: white;
    }
    .btn-secondary {
      background: #6c757d;
      color: white;
    }
    button:hover {
      transform: scale(1.02);
    }
    button:active {
      transform: scale(0.98);
    }
    .status {
      margin-top: 20px;
      padding: 10px;
      border-radius: 8px;
      display: none;
    }
    .status.success {
      background: #d4edda;
      color: #155724;
      display: block;
    }
    .status.error {
      background: #f8d7da;
      color: #721c24;
      display: block;
    }
    a {
      color: #667eea;
      text-decoration: none;
      display: inline-block;
      margin-top: 15px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>⚙️ WiFi Settings</h1>
    <p class="subtitle">ESP32-CAM-4WD Configuration</p>
    
    <div class="current-wifi" id="currentWifi">
      <h3>Current Connection</h3>
      <p id="wifiStatus">Loading...</p>
    </div>
    
    <form id="wifiForm">
      <h3 style="color: #667eea; margin-bottom: 10px;">Change WiFi Network</h3>
      <input type="text" id="ssid" name="ssid" placeholder="WiFi Network Name (SSID)" required>
      <input type="password" id="password" name="password" placeholder="WiFi Password" required>
      <button type="submit" class="btn-primary">Save & Connect</button>
    </form>
    
    <button class="btn-danger" id="deleteBtn" onclick="deleteWiFi()">🗑️ Delete WiFi Settings</button>
    <button class="btn-secondary" onclick="location.href='/'">← Back to Robot</button>
    
    <div id="status" class="status"></div>
  </div>

  <script>
    // Check current WiFi status
    fetch('/wifi-info')
      .then(response => response.json())
      .then(data => {
        const statusEl = document.getElementById('wifiStatus');
        if (data.connected) {
          statusEl.innerHTML = '<strong>SSID:</strong> ' + data.ssid + '<br><strong>IP:</strong> ' + data.ip + '<br><strong>Hostname:</strong> ' + data.hostname + '<br><strong>Signal:</strong> ' + data.rssi + ' dBm';
        } else {
          statusEl.textContent = 'Not connected (AP Mode)';
        }
      })
      .catch(err => {
        document.getElementById('wifiStatus').textContent = 'Unable to fetch status';
      });

    document.getElementById('wifiForm').addEventListener('submit', function(e) {
      e.preventDefault();
      
      const ssid = document.getElementById('ssid').value;
      const password = document.getElementById('password').value;
      const statusDiv = document.getElementById('status');
      
      statusDiv.className = 'status';
      statusDiv.textContent = 'Saving and connecting to ' + ssid + '...';
      statusDiv.classList.add('success');
      
      fetch('/save-wifi?ssid=' + encodeURIComponent(ssid) + '&password=' + encodeURIComponent(password))
        .then(response => response.text())
        .then(data => {
          statusDiv.textContent = 'WiFi configured! Device rebooting... Please reconnect to your network.';
          statusDiv.classList.add('success');
        })
        .catch(error => {
          statusDiv.textContent = 'Error: ' + error.message;
          statusDiv.className = 'status error';
        });
    });

    function deleteWiFi() {
      if (confirm('Delete WiFi settings? Device will reboot into AP mode.')) {
        const statusDiv = document.getElementById('status');
        statusDiv.className = 'status';
        statusDiv.textContent = 'Deleting WiFi settings...';
        statusDiv.classList.add('success');
        
        fetch('/delete-wifi')
          .then(response => response.text())
          .then(data => {
            statusDiv.textContent = 'WiFi settings deleted! Device rebooting into AP mode...';
            statusDiv.classList.add('success');
          })
          .catch(error => {
            statusDiv.textContent = 'Error: ' + error.message;
            statusDiv.className = 'status error';
          });
      }
    }
  </script>
</body>
</html>
)rawliteral";

// Web interface
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32-CAM Robot</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <meta charset="UTF-8">
  <style>
    * {
      -webkit-tap-highlight-color: transparent;
      -webkit-touch-callout: none;
      -webkit-user-select: none;
      user-select: none;
    }
    body {
      font-family: -apple-system, BlinkMacSystemFont, Arial, sans-serif;
      text-align: center;
      margin: 0;
      padding: 10px;
      background: #1a1a1a;
      color: #fff;
      touch-action: manipulation;
    }
    h1 {
      font-size: 24px;
      margin: 10px 0;
      color: #4a9eff;
    }
    .badge {
      display: inline-block;
      background: #27ae60;
      color: white;
      padding: 5px 10px;
      border-radius: 5px;
      font-size: 12px;
      margin: 5px;
    }
    .badge.http { background: #3498db; }
    .badge.udp { background: #9b59b6; }
    .badge.ws { background: #27ae60; }
    .badge.disconnected { background: #e74c3c; }
    #photo {
      width: 100%;
      max-width: 640px;
      height: auto;
      border: 2px solid #2f4468;
      border-radius: 8px;
      margin: 10px auto;
      display: block;
      background: #000;
    }
    .reboot-button {
      width: 85px;
      height: 85px;
      background: linear-gradient(145deg, #e74c3c, #c0392b);
      color: white;
      border: none;
      border-radius: 12px;
      cursor: pointer;
      font-size: 16px;
      font-weight: bold;
      touch-action: manipulation;
      transition: all 0.1s ease;
      display: flex;
      align-items: center;
      justify-content: center;
      box-shadow: 0 4px 8px rgba(0,0,0,0.3);
    }
    .quality-button {
      background: linear-gradient(145deg, #3498db, #2980b9);
      color: white;
      border: none;
      border-radius: 6px;
      cursor: pointer;
      font-size: 11px;
      font-weight: bold;
      touch-action: manipulation;
      transition: all 0.1s ease;
      padding: 5px 10px;
      display: inline-block;
      box-shadow: 0 2px 4px rgba(0,0,0,0.3);
    }
    .quality-button.high {
      background: linear-gradient(145deg, #27ae60, #219a52);
    }
    .status-bar {
      position: fixed;
      bottom: 0;
      left: 0;
      right: 0;
      background: #2c3e50;
      padding: 10px;
      color: white;
      font-size: 12px;
      display: flex;
      justify-content: space-around;
      align-items: center;
      flex-wrap: wrap;
      gap: 8px;
    }
    .wifi-strong { color: #27ae60; }
    .wifi-medium { color: #f1c40f; }
    .wifi-weak { color: #e74c3c; }
    .latency-good { color: #27ae60; }
    .latency-medium { color: #f1c40f; }
    .latency-poor { color: #e74c3c; }
    .ws-connected { color: #27ae60; }
    .ws-disconnected { color: #e74c3c; }
    .ws-connecting { color: #f1c40f; }
    .control-container {
      max-width: 640px;
      margin: 20px auto;
      padding: 10px;
    }
    .joystick-container {
      position: relative;
      width: 250px;
      height: 250px;
      margin: 20px auto;
      background: radial-gradient(circle, #1e2d47, #0d1621);
      border-radius: 50%;
      border: 3px solid #2f4468;
      box-shadow: 0 8px 16px rgba(0,0,0,0.5), inset 0 0 20px rgba(0,0,0,0.3);
      touch-action: none;
    }
    .joystick-base {
      position: absolute;
      width: 100%;
      height: 100%;
      border-radius: 50%;
    }
    .joystick-handle {
      position: absolute;
      width: 80px;
      height: 80px;
      background: radial-gradient(circle at 30% 30%, #4a9eff, #2f4468);
      border: 3px solid #4a9eff;
      border-radius: 50%;
      top: 50%;
      left: 50%;
      transform: translate(-50%, -50%);
      cursor: grab;
      box-shadow: 0 4px 12px rgba(74, 158, 255, 0.4);
      transition: box-shadow 0.1s;
    }
    .joystick-handle:active {
      cursor: grabbing;
      box-shadow: 0 0 20px rgba(74, 158, 255, 0.8);
    }
    .joystick-crosshair {
      position: absolute;
      width: 2px;
      height: 100%;
      background: rgba(74, 158, 255, 0.3);
      left: 50%;
      transform: translateX(-50%);
    }
    .joystick-crosshair.h {
      width: 100%;
      height: 2px;
      top: 50%;
      transform: translateY(-50%);
      left: 0;
    }
    .joystick-info {
      text-align: center;
      margin-top: 15px;
      font-size: 14px;
      color: #4a9eff;
      font-family: monospace;
    }
    .button-row {
      display: flex;
      justify-content: center;
      gap: 10px;
      margin: 15px 0;
      flex-wrap: wrap;
    }
    .button, .led-button, .reboot-button {
      border: none;
      text-align: center;
      font-size: 16px;
      cursor: pointer;
      padding: 12px 24px;
      border-radius: 8px;
      touch-action: manipulation;
      transition: all 0.1s ease;
      font-weight: bold;
      box-shadow: 0 4px 8px rgba(0,0,0,0.3);
    }
    .button {
      background: linear-gradient(145deg, #2f4468, #1e2d47);
      color: white;
    }
    .button:active {
      transform: scale(0.95);
    }
    .led-button {
      background: linear-gradient(145deg, #555, #444);
      color: white;
    }
    .led-on {
      background: linear-gradient(145deg, #ffd700, #f0c40f);
      color: #000;
    }
    .forward { grid-area: forward; }
    .led { grid-area: led; font-size: 16px; }
    .left { grid-area: left; }
    .stop { grid-area: stop; background: #c0392b; font-size: 32px; }
    .right { grid-area: right; }
    .backward { grid-area: backward; }
    .plus { grid-area: plus; font-size: 28px; }
    .minus { grid-area: minus; font-size: 28px; }
    .info {
      margin: 10px;
      font-size: 14px;
      color: #4a9eff;
    }
    select {
      background: #2f4468;
      color: white;
      border: 1px solid #4a9eff;
      border-radius: 6px;
      padding: 8px;
      font-size: 14px;
      cursor: pointer;
      margin: 10px;
    }
  </style>
</head>
<body>
  <h1>ESP32-CAM Robot Control</h1>
  
  <div style="margin: 10px 0;">
    <a href="/wifi-config" style="color: #4a9eff; text-decoration: none; font-size: 14px;">⚙️ WiFi Settings</a>
  </div>
  
  <img src="" id="photo" alt="Camera Stream">
  
  <div class="info">
    <label for="resolution">Resolution: </label>
    <select id="resolution">
      <option value="5">QVGA (320x240) - Default</option>
      <option value="6">CIF (400x296)</option>
      <option value="8">VGA (640x480)</option>
      <option value="9">SVGA (800x600)</option>
      <option value="10">XGA (1024x768)</option>
      <option value="11">HD (1280x720)</option>
      <option value="12">SXGA (1280x1024)</option>
      <option value="13">UXGA (1600x1200)</option>
    </select>
    | Speed: <span id="speedText">Medium</span>
  </div>

  <div class="control-container">
    <div class="joystick-container" id="joystickContainer">
      <div class="joystick-base">
        <div class="joystick-crosshair"></div>
        <div class="joystick-crosshair h"></div>
      </div>
      <div class="joystick-handle" id="joystickHandle"></div>
    </div>
    
    <div class="joystick-info">
      X: <span id="joyX">0</span> | Y: <span id="joyY">0</span>
    </div>

    <div class="button-row">
      <button class="button" data-action="SPEED-">Speed −</button>
      <button id="ledButton" class="led-button">LED OFF</button>
      <button class="button" data-action="SPEED+">Speed +</button>
    </div>
    
    <div class="button-row">
      <button id="rebootButton" class="reboot-button">REBOOT ESP32</button>
    </div>
  </div>

  <div class="status-bar">
    <div id="wifiStatus">WiFi: --</div>
    <div id="latencyStatus">Latency: --</div>
    <div id="wsStatus">WebSocket: --</div>
  </div>

  <script>
    const imgElement = document.getElementById('photo');
    imgElement.style.display = 'block';
    const wifiStatusEl = document.getElementById('wifiStatus');
    const latencyStatusEl = document.getElementById('latencyStatus');
    const wsStatusEl = document.getElementById('wsStatus');

    document.addEventListener('touchstart', (e) => {
      if (e.target.tagName === 'BUTTON') e.preventDefault();
    }, { passive: false });
    document.addEventListener('touchmove', (e) => e.preventDefault(), { passive: false });

    // Resolution selector
    const resolutionSelect = document.getElementById('resolution');
    resolutionSelect.addEventListener('change', function() {
      fetch('/action?framesize=' + this.value)
        .then(() => {
          setTimeout(() => startStream(), 500);
        });
    });

    // Start HTTP stream
    function startStream() {
      imgElement.src = 'http://' + window.location.hostname + ':81/stream?' + Date.now();
    }
    
    // Handle HTTP stream errors and auto-reconnect
    imgElement.addEventListener('error', function() {
      setTimeout(() => {
        imgElement.src = 'http://' + window.location.hostname + ':81/stream?' + Date.now();
      }, 1000);
    });

    // Connection quality monitoring - using WebSocket ping/pong for accurate latency
    let lastPingTime = 0;
    let pendingPing = false;
    let latencyHistory = [];
    const MAX_LATENCY_SAMPLES = 5;
    
    function checkConnection() {
      if (useWebSocket && websocket && websocket.readyState === WebSocket.OPEN) {
        // Use WebSocket for low-latency ping measurement
        lastPingTime = performance.now();
        pendingPing = true;
        websocket.send('ping');
      } else {
        // Fallback: measure HTTP round-trip time (less accurate)
        const startTime = performance.now();
        fetch('/action?cmd=ping', { 
          method: 'GET',
          cache: 'no-cache'
        })
          .then(response => {
            if (response.ok) {
              const latency = performance.now() - startTime;
              updateLatencyStatus(latency);
            }
          })
          .catch(err => {
            latencyStatusEl.textContent = 'Latency: -- (Disconnected)';
            latencyStatusEl.className = 'latency-poor';
          });
      }
    }

    function updateLatencyStatus(latency) {
      // Add to history and calculate moving average for smoother display
      latencyHistory.push(latency);
      if (latencyHistory.length > MAX_LATENCY_SAMPLES) {
        latencyHistory.shift();
      }
      
      const avgLatency = latencyHistory.reduce((a, b) => a + b, 0) / latencyHistory.length;
      
      let status = '';
      let className = '';
      
      if (avgLatency < 50) {
        status = 'Excellent';
        className = 'latency-good';
      } else if (avgLatency < 100) {
        status = 'Good';
        className = 'latency-good';
      } else if (avgLatency < 200) {
        status = 'Fair';
        className = 'latency-medium';
      } else {
        status = 'Poor';
        className = 'latency-poor';
      }

      latencyStatusEl.textContent = `Latency: ${Math.round(avgLatency)}ms (${status})`;
      latencyStatusEl.className = className;
    }

    // WiFi signal strength monitoring
    function updateWifiStatus() {
      if (useWebSocket && websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send('wifi-status');
      } else {
        fetch('/wifi-status')
          .then(response => response.json())
          .then(data => {
            processWifiStatus(data);
          })
          .catch(err => {
            wifiStatusEl.textContent = 'WiFi: Disconnected';
            wifiStatusEl.className = 'wifi-weak';
          });
      }
    }
    
    function processWifiStatus(data) {
      let strength = '';
      let className = '';
      
      if (data.rssi > -50) {
        strength = 'Excellent';
        className = 'wifi-strong';
      } else if (data.rssi > -70) {
        strength = 'Good';
        className = 'wifi-medium';
      } else {
        strength = 'Weak';
        className = 'wifi-weak';
      }

      wifiStatusEl.textContent = `WiFi: ${strength} (${data.rssi}dBm)`;
      wifiStatusEl.className = className;
    }
    
    // WebSocket connection
    let websocket = null;
    let useWebSocket = false;
    let wsReconnectAttempts = 0;
    const MAX_WS_RECONNECT = 10;
    
    function updateWSStatus(status, className) {
      wsStatusEl.textContent = 'WS: ' + status;
      wsStatusEl.className = className;
    }
    
    function connectWebSocket() {
      const wsUrl = 'ws://' + window.location.hostname + ':80/ws';
      console.log('[WS] Connecting to:', wsUrl);
      updateWSStatus('Connecting...', 'ws-connecting');
      
      try {
        websocket = new WebSocket(wsUrl);
      } catch (e) {
        console.error('[WS] Failed to create WebSocket:', e);
        updateWSStatus('Failed', 'ws-disconnected');
        return;
      }
      
      websocket.onopen = function() {
        console.log('[WS] Connected successfully');
        updateWSStatus('Connected ✓', 'ws-connected');
        useWebSocket = true;
        wsReconnectAttempts = 0;
      };
      
      websocket.onclose = function(event) {
        console.log('[WS] Disconnected. Code:', event.code, 'Reason:', event.reason);
        updateWSStatus('Disconnected', 'ws-disconnected');
        useWebSocket = false;
        
        if (wsReconnectAttempts < MAX_WS_RECONNECT) {
          wsReconnectAttempts++;
          const delay = Math.min(3000 * wsReconnectAttempts, 10000);
          console.log('[WS] Reconnecting in', delay, 'ms (attempt', wsReconnectAttempts, ')');
          setTimeout(connectWebSocket, delay);
        } else {
          updateWSStatus('Failed (using HTTP)', 'ws-disconnected');
        }
      };
      
      websocket.onerror = function(error) {
        console.error('[WS] Error:', error);
        updateWSStatus('Error', 'ws-disconnected');
      };
      
      websocket.onmessage = function(event) {
        const data = event.data;
        
        // Handle ping/pong response for latency measurement
        if (data === 'pong' && pendingPing) {
          const latency = performance.now() - lastPingTime;
          updateLatencyStatus(latency);
          pendingPing = false;
          console.log('[WS] Ping/Pong latency:', Math.round(latency) + 'ms');
        }
        // Handle WiFi status response
        else if (data.startsWith('{') && data.includes('rssi')) {
          try {
            const wifiData = JSON.parse(data);
            processWifiStatus(wifiData);
          } catch (e) {
            console.error('[WS] Failed to parse WiFi status:', e);
          }
        }
        else {
          console.log('[WS] RX:', data);
        }
      };
    }
    
    connectWebSocket();
    
    function sendCommand(cmd) {
      if (useWebSocket && websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(cmd);
      } else {
        const xhr = new XMLHttpRequest();
        xhr.timeout = 500;
        xhr.open('GET', '/action?cmd=' + cmd, true);
        xhr.send();
      }
    }

    // Motor command repeat system
    let activeMotorCommand = null;
    let motorCommandInterval = null;

    function startMotorCommand(cmd) {
      activeMotorCommand = cmd;
      sendCommand(cmd);
      
      // Send command every 800ms while button is held
      if (motorCommandInterval) clearInterval(motorCommandInterval);
      motorCommandInterval = setInterval(() => {
        if (activeMotorCommand) {
          sendCommand(activeMotorCommand);
        }
      }, 800);  // 800ms - less than 1 second watchdog timeout
    }

    function stopMotorCommand() {
      activeMotorCommand = null;
      if (motorCommandInterval) {
        clearInterval(motorCommandInterval);
        motorCommandInterval = null;
      }
      sendCommand('STOP');
    }

    window.onload = function() {
      // Start video stream
      imgElement.src = 'http://' + window.location.hostname + ':81/stream';
      
      // Start connection monitoring
      setInterval(checkConnection, 2500);  // Check connection every 2.5 seconds
      setInterval(updateWifiStatus, 10000); // Update WiFi status every 10 seconds
      
      // Initial status updates
      updateWifiStatus();
      checkConnection();
      
      // Reload stream if it fails to load
      imgElement.onerror = function() {
        setTimeout(() => {
          imgElement.src = 'http://' + window.location.hostname + ':81/stream?' + Date.now();
        }, 1000);
      };
    };

    let currentSpeed = 170;
    const speedLevels = { 85: 'Low', 170: 'Medium', 255: 'High' };

    function updateSpeedDisplay() {
      document.getElementById('speedText').textContent = speedLevels[currentSpeed] || 'Medium';
    }

    // Joystick control
    const joystickContainer = document.getElementById('joystickContainer');
    const joystickHandle = document.getElementById('joystickHandle');
    const joyXDisplay = document.getElementById('joyX');
    const joyYDisplay = document.getElementById('joyY');
    
    let isDragging = false;
    let joystickX = 0;
    let joystickY = 0;
    let joystickInterval = null;
    const maxDistance = 85; // Max pixels from center
    
    function updateJoystick(clientX, clientY) {
      const rect = joystickContainer.getBoundingClientRect();
      const centerX = rect.left + rect.width / 2;
      const centerY = rect.top + rect.height / 2;
      
      let deltaX = clientX - centerX;
      let deltaY = centerY - clientY; // Inverted for Y axis
      
      const distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY);
      
      if (distance > maxDistance) {
        const angle = Math.atan2(deltaY, deltaX);
        deltaX = Math.cos(angle) * maxDistance;
        deltaY = Math.sin(angle) * maxDistance;
      }
      
      joystickHandle.style.transform = `translate(calc(-50% + ${deltaX}px), calc(-50% + ${-deltaY}px))`;
      
      joystickX = Math.round((deltaX / maxDistance) * 100);
      joystickY = Math.round((deltaY / maxDistance) * 100);
      
      joyXDisplay.textContent = joystickX;
      joyYDisplay.textContent = joystickY;
    }
    
    function resetJoystick() {
      joystickHandle.style.transform = 'translate(-50%, -50%)';
      joystickX = 0;
      joystickY = 0;
      joyXDisplay.textContent = '0';
      joyYDisplay.textContent = '0';
      sendCommand('JOY:0,0');
    }
    
    function startJoystickUpdates() {
      if (joystickInterval) clearInterval(joystickInterval);
      joystickInterval = setInterval(() => {
        if (isDragging) {
          sendCommand(`JOY:${joystickX},${joystickY}`);
        }
      }, 100);
    }
    
    joystickHandle.addEventListener('mousedown', (e) => {
      isDragging = true;
      startJoystickUpdates();
      e.preventDefault();
    });
    
    joystickHandle.addEventListener('touchstart', (e) => {
      isDragging = true;
      startJoystickUpdates();
      e.preventDefault();
    }, { passive: false });
    
    document.addEventListener('mousemove', (e) => {
      if (isDragging) {
        updateJoystick(e.clientX, e.clientY);
      }
    });
    
    document.addEventListener('touchmove', (e) => {
      if (isDragging && e.touches.length > 0) {
        updateJoystick(e.touches[0].clientX, e.touches[0].clientY);
      }
    }, { passive: false });
    
    document.addEventListener('mouseup', () => {
      if (isDragging) {
        isDragging = false;
        resetJoystick();
        if (joystickInterval) {
          clearInterval(joystickInterval);
          joystickInterval = null;
        }
      }
    });
    
    document.addEventListener('touchend', () => {
      if (isDragging) {
        isDragging = false;
        resetJoystick();
        if (joystickInterval) {
          clearInterval(joystickInterval);
          joystickInterval = null;
        }
      }
    });

    // LED button
    let ledState = false;
    const ledButton = document.getElementById('ledButton');
    ledButton.addEventListener('click', (e) => {
      e.preventDefault();
      ledState = !ledState;
      ledButton.classList.toggle('led-on', ledState);
      ledButton.textContent = ledState ? 'LED ON' : 'LED OFF';
      sendCommand(ledState ? 'LED_ON' : 'LED_OFF');
    });

    // Reboot button
    const rebootButton = document.getElementById('rebootButton');
    rebootButton.addEventListener('click', (e) => {
      e.preventDefault();
      if (confirm('Reboot ESP32-CAM?')) {
        sendCommand('REBOOT');
      }
    });

    // Speed buttons
    const buttons = document.querySelectorAll('.button[data-action]');
    buttons.forEach(button => {
      const action = button.getAttribute('data-action');
      
      button.addEventListener('click', (e) => {
        e.preventDefault();
        if (action === 'SPEED+') {
          currentSpeed = Math.min(currentSpeed + 85, 255);
          updateSpeedDisplay();
          sendCommand(action);
        } else if (action === 'SPEED-') {
          currentSpeed = Math.max(currentSpeed - 85, 85);
          updateSpeedDisplay();
          sendCommand(action);
        }
      });
    });
  </script>
</body>
</html>
)rawliteral";

// Favicon handler - returns a robot emoji as SVG
static esp_err_t favicon_handler(httpd_req_t *req) {
  const char* favicon_svg = R"rawliteral(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100">
  <defs>
    <linearGradient id="robotGrad" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" style="stop-color:#ff3333;stop-opacity:1" />
      <stop offset="100%" style="stop-color:#cc0000;stop-opacity:1" />
    </linearGradient>
    <linearGradient id="bodyGrad" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" style="stop-color:#33ff33;stop-opacity:1" />
      <stop offset="100%" style="stop-color:#00cc00;stop-opacity:1" />
    </linearGradient>
  </defs>
  <!-- Robot head - RED -->
  <rect x="25" y="30" width="50" height="45" rx="8" fill="url(#robotGrad)" stroke="#ffff00" stroke-width="3"/>
  <!-- Antenna - YELLOW -->
  <line x1="50" y1="20" x2="50" y2="30" stroke="#ffff00" stroke-width="3"/>
  <circle cx="50" cy="18" r="4" fill="#ffff00"/>
  <!-- Eyes - YELLOW -->
  <circle cx="38" cy="45" r="5" fill="#ffff00"/>
  <circle cx="62" cy="45" r="5" fill="#ffff00"/>
  <circle cx="38" cy="45" r="2" fill="#000"/>
  <circle cx="62" cy="45" r="2" fill="#000"/>
  <!-- Smile - YELLOW -->
  <path d="M 35 60 Q 50 68 65 60" stroke="#ffff00" stroke-width="3" fill="none" stroke-linecap="round"/>
  <!-- Body - GREEN -->
  <rect x="30" y="75" width="40" height="12" rx="4" fill="url(#bodyGrad)" stroke="#ffff00" stroke-width="2"/>
  <!-- Wheels - RED with YELLOW outline -->
  <circle cx="35" cy="90" r="8" fill="#ff3333" stroke="#ffff00" stroke-width="3"/>
  <circle cx="65" cy="90" r="8" fill="#ff3333" stroke="#ffff00" stroke-width="3"/>
</svg>
)rawliteral";
  
  httpd_resp_set_type(req, "image/svg+xml");
  httpd_resp_set_hdr(req, "Cache-Control", "public, max-age=86400");
  return httpd_resp_send(req, favicon_svg, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t config_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, CONFIG_HTML, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t wifi_info_handler(httpd_req_t *req) {
  char response[256];
  if (!isAPMode && WiFi.status() == WL_CONNECTED) {
    snprintf(response, sizeof(response), 
             "{\"connected\":true,\"ssid\":\"%s\",\"ip\":\"%s\",\"hostname\":\"esp32-cam-4wd\",\"rssi\":%d}",
             WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.RSSI());
  } else {
    snprintf(response, sizeof(response), "{\"connected\":false}");
  }
  
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, response, strlen(response));
}

static esp_err_t delete_wifi_handler(httpd_req_t *req) {
  #if DEBUG_ENABLED
    Serial.println("[WiFi Config] Deleting WiFi credentials");
  #endif
  
  preferences.begin("wifi", false);
  preferences.clear();
  preferences.end();
  
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_send(req, "OK - Rebooting", HTTPD_RESP_USE_STRLEN);
  
  delay(1000);
  ESP.restart();
  
  return ESP_OK;
}

static esp_err_t save_wifi_handler(httpd_req_t *req) {
  char query[256];
  int len = httpd_req_get_url_query_len(req) + 1;
  
  if (len > sizeof(query)) {
    #if DEBUG_ENABLED
      Serial.println("[WiFi Config] Query string too long");
    #endif
    httpd_resp_send_404(req);
    return ESP_OK;
  }

  if (httpd_req_get_url_query_str(req, query, len) == ESP_OK) {
    char encodedSsid[64] = {0};
    char encodedPassword[128] = {0};
    char newSsid[32] = {0};
    char newPassword[64] = {0};
    
    #if DEBUG_ENABLED
      Serial.printf("[WiFi Config] Received query: %s\n", query);
    #endif
    
    if (httpd_query_key_value(query, "ssid", encodedSsid, sizeof(encodedSsid)) == ESP_OK &&
        httpd_query_key_value(query, "password", encodedPassword, sizeof(encodedPassword)) == ESP_OK) {
      
      // URL decode the credentials
      urlDecode(newSsid, encodedSsid, sizeof(newSsid));
      urlDecode(newPassword, encodedPassword, sizeof(newPassword));
      
      #if DEBUG_ENABLED
        Serial.printf("[WiFi Config] Decoded SSID: %s\n", newSsid);
        Serial.printf("[WiFi Config] Password length: %d\n", strlen(newPassword));
      #endif
      
      if (strlen(newSsid) == 0) {
        #if DEBUG_ENABLED
          Serial.println("[WiFi Config] Error: SSID is empty");
        #endif
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "SSID cannot be empty");
        return ESP_OK;
      }
      
      saveWiFiCredentials(newSsid, newPassword);
      
      httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
      httpd_resp_send(req, "OK - Rebooting", HTTPD_RESP_USE_STRLEN);
      
      #if DEBUG_ENABLED
        Serial.println("[WiFi Config] Credentials saved, rebooting in 2 seconds...");
      #endif
      
      // Reboot after a short delay
      delay(2000);
      ESP.restart();
      
      return ESP_OK;
    } else {
      #if DEBUG_ENABLED
        Serial.println("[WiFi Config] Error: Missing SSID or password in query");
      #endif
    }
  } else {
    #if DEBUG_ENABLED
      Serial.println("[WiFi Config] Error: Failed to get query string");
    #endif
  }

  httpd_resp_send_404(req);
  return ESP_OK;
}

static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;
  struct timeval _timestamp;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t *_jpg_buf = NULL;
  char *part_buf[128];

  // Mark client as connected for power saving
  clientConnected = true;
  lastClientActivity = millis();
  #if DEBUG_ENABLED
    Serial.println("Stream client connected");
  #endif

  static int64_t last_frame = 0;
  if (!last_frame) {
    last_frame = esp_timer_get_time();
  }

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) {
    clientConnected = false;
    return res;
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "X-Framerate", "60");

  while (true) {
    lastClientActivity = millis();  // Update activity timestamp
    
    fb = esp_camera_fb_get();
    if (!fb) {
      #if DEBUG_ENABLED
        Serial.println("Camera capture failed");
      #endif
      res = ESP_FAIL;
    } else {
      _timestamp.tv_sec = fb->timestamp.tv_sec;
      _timestamp.tv_usec = fb->timestamp.tv_usec;
      if (fb->format != PIXFORMAT_JPEG) {
        bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
        esp_camera_fb_return(fb);
        fb = NULL;
        if (!jpeg_converted) {
          #if DEBUG_ENABLED
            Serial.println("JPEG compression failed");
          #endif
          res = ESP_FAIL;
        }
      } else {
        _jpg_buf_len = fb->len;
        _jpg_buf = fb->buf;
      }
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 128, _STREAM_PART, _jpg_buf_len, _timestamp.tv_sec, _timestamp.tv_usec);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if (fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if (_jpg_buf) {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if (res != ESP_OK) {
      #if DEBUG_ENABLED
        Serial.println("Send frame failed");
      #endif
      break;
    }
    int64_t fr_end = esp_timer_get_time();

    int64_t frame_time = fr_end - last_frame;
    last_frame = fr_end;
    frame_time /= 1000;
    
    // Log every 100 frames
    #if DEBUG_ENABLED
      static int frame_count = 0;
      frame_count++;
      if (frame_count % 100 == 0) {
        Serial.printf("MJPG: %uB %ums (%.1ffps)\n", 
                      (uint32_t)(_jpg_buf_len), 
                      (uint32_t)frame_time, 
                      1000.0 / (uint32_t)frame_time);
      }
    #endif
  }

  // Client disconnected
  clientConnected = false;
  #if DEBUG_ENABLED
    Serial.println("Stream client disconnected");
  #endif
  return res;
}

static esp_err_t action_handler(httpd_req_t *req) {
  char query[128];
  int len = httpd_req_get_url_query_len(req) + 1;
  
  // Track client activity for power saving
  lastClientActivity = millis();
  
  if (len > sizeof(query)) {
    httpd_resp_send_404(req);
    return ESP_OK;
  }

  if (httpd_req_get_url_query_str(req, query, len) == ESP_OK) {
    char action[32] = {0};
    if (httpd_query_key_value(query, "cmd", action, sizeof(action)) == ESP_OK) {
      processCommand(action);
    } else if (httpd_query_key_value(query, "framesize", action, sizeof(action)) == ESP_OK) {
      // Handle resolution change
      int framesize = atoi(action);
      sensor_t *s = esp_camera_sensor_get();
      if (s) {
        // Framesize enum: 0-13 are valid values
        // FRAMESIZE_96X96,     // 0
        // FRAMESIZE_QQVGA,     // 1 (160x120)
        // FRAMESIZE_QCIF,      // 2 (176x144)
        // FRAMESIZE_HQVGA,     // 3 (240x176)
        // FRAMESIZE_240X240,   // 4
        // FRAMESIZE_QVGA,      // 5 (320x240)
        // FRAMESIZE_CIF,       // 6 (400x296)
        // FRAMESIZE_HVGA,      // 7 (480x320)
        // FRAMESIZE_VGA,       // 8 (640x480)
        // FRAMESIZE_SVGA,      // 9 (800x600)
        // FRAMESIZE_XGA,       // 10 (1024x768)
        // FRAMESIZE_HD,        // 11 (1280x720)
        // FRAMESIZE_SXGA,      // 12 (1280x1024)
        // FRAMESIZE_UXGA       // 13 (1600x1200)
        
        if (framesize >= 0 && framesize <= 13) {
          s->set_framesize(s, (framesize_t)framesize);
          #if DEBUG_ENABLED
            Serial.printf("Resolution changed to framesize: %d\n", framesize);
          #endif
        } else {
          #if DEBUG_ENABLED
            Serial.printf("Invalid framesize: %d\n", framesize);
          #endif
        }
      }
    }
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

static esp_err_t snapshot_handler(httpd_req_t *req) {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=snapshot.jpg");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  
  esp_err_t res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
  return res;
}

static esp_err_t wifi_status_handler(httpd_req_t *req) {
  char response[64];
  snprintf(response, sizeof(response), "{\"rssi\":%d}", WiFi.RSSI());
  
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, response, strlen(response));
}

// WebSocket handler
static esp_err_t ws_handler(httpd_req_t *req) {
  // Check if this is a WebSocket handshake
  if (req->method == HTTP_GET) {
    #if DEBUG_ENABLED
      Serial.println("[WS] WebSocket handshake - client connecting");
    #endif
    return ESP_OK;
  }
  
  // This is a WebSocket frame
  httpd_ws_frame_t ws_pkt;
  memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
  ws_pkt.type = HTTPD_WS_TYPE_TEXT;
  
  esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
  if (ret != ESP_OK) {
    #if DEBUG_ENABLED
      Serial.println("[WS] Failed to get frame len");
    #endif
    return ret;
  }
  
  if (ws_pkt.len) {
    uint8_t *buf = (uint8_t*)calloc(1, ws_pkt.len + 1);
    if (buf == NULL) {
      #if DEBUG_ENABLED
        Serial.println("[WS] Failed to allocate memory");
      #endif
      return ESP_ERR_NO_MEM;
    }
    ws_pkt.payload = buf;
    
    ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
    if (ret != ESP_OK) {
      #if DEBUG_ENABLED
        Serial.println("[WS] Failed to receive frame");
      #endif
      free(buf);
      return ret;
    }
    
    buf[ws_pkt.len] = '\0';
    
    #if DEBUG_ENABLED
      Serial.printf("[WS] RX: %s\n", buf);
    #endif
    
    // Handle special status commands
    char response[128];
    const char *resp_ptr = "OK";
    
    if (strcmp((char*)buf, "ping") == 0) {
      // Respond to ping
      strcpy(response, "pong");
      resp_ptr = response;
    }
    else if (strcmp((char*)buf, "wifi-status") == 0) {
      // Send WiFi status as JSON
      snprintf(response, sizeof(response), "{\"rssi\":%d}", WiFi.RSSI());
      resp_ptr = response;
    }
    else {
      // Process motor/LED commands
      processCommand((char*)buf);
    }
    
    // Send response
    httpd_ws_frame_t ws_resp;
    memset(&ws_resp, 0, sizeof(httpd_ws_frame_t));
    ws_resp.type = HTTPD_WS_TYPE_TEXT;
    ws_resp.payload = (uint8_t*)resp_ptr;
    ws_resp.len = strlen(resp_ptr);
    httpd_ws_send_frame(req, &ws_resp);
    
    free(buf);
    lastClientActivity = millis();
  }
  
  return ESP_OK;
}

void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = HTTP_PORT;
  config.max_open_sockets = 7;  // Increased for WebSocket support
  config.max_uri_handlers = 16;  // Increased from default 8 to accommodate WebSocket
  config.stack_size = 8192;  // Increased from 4096 for high-res streaming
  config.lru_purge_enable = true;  // Enable purging of LRU socket

  httpd_uri_t favicon_uri = {
    .uri = "/favicon.ico",
    .method = HTTP_GET,
    .handler = favicon_handler,
    .user_ctx = NULL
  };

  httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_handler,
    .user_ctx = NULL
  };

  httpd_uri_t cmd_uri = {
    .uri = "/action",
    .method = HTTP_GET,
    .handler = action_handler,
    .user_ctx = NULL
  };

  httpd_uri_t snapshot_uri = {
    .uri = "/snapshot",
    .method = HTTP_GET,
    .handler = snapshot_handler,
    .user_ctx = NULL
  };

  httpd_uri_t wifi_status_uri = {
    .uri = "/wifi-status",
    .method = HTTP_GET,
    .handler = wifi_status_handler,
    .user_ctx = NULL
  };

  httpd_uri_t config_uri = {
    .uri = "/wifi-config",
    .method = HTTP_GET,
    .handler = config_handler,
    .user_ctx = NULL
  };

  httpd_uri_t wifi_info_uri = {
    .uri = "/wifi-info",
    .method = HTTP_GET,
    .handler = wifi_info_handler,
    .user_ctx = NULL
  };

  httpd_uri_t save_wifi_uri = {
    .uri = "/save-wifi",
    .method = HTTP_GET,
    .handler = save_wifi_handler,
    .user_ctx = NULL
  };

  httpd_uri_t delete_wifi_uri = {
    .uri = "/delete-wifi",
    .method = HTTP_GET,
    .handler = delete_wifi_handler,
    .user_ctx = NULL
  };

  httpd_uri_t ws_uri = {
    .uri = "/ws",
    .method = HTTP_GET,
    .handler = ws_handler,
    .user_ctx = NULL,
    .is_websocket = true
  };

  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &favicon_uri);
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &config_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
    httpd_register_uri_handler(camera_httpd, &snapshot_uri);
    httpd_register_uri_handler(camera_httpd, &wifi_status_uri);
    httpd_register_uri_handler(camera_httpd, &wifi_info_uri);
    httpd_register_uri_handler(camera_httpd, &save_wifi_uri);
    httpd_register_uri_handler(camera_httpd, &delete_wifi_uri);
    
    esp_err_t ws_result = httpd_register_uri_handler(camera_httpd, &ws_uri);
    #if DEBUG_ENABLED
      if (ws_result == ESP_OK) {
        Serial.println("[Server] WebSocket handler registered successfully at /ws");
      } else {
        Serial.printf("[Server] Failed to register WebSocket handler: %d\n", ws_result);
      }
    #endif
  } else {
    #if DEBUG_ENABLED
      Serial.println("[Server] Failed to start HTTP server");
    #endif
  }

  config.server_port = STREAM_PORT;
  config.ctrl_port = 32769;
  
  httpd_uri_t stream_uri = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = stream_handler,
    .user_ctx = NULL
  };

  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}
