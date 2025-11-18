// Version: 1.2.9
// ESP32-S3 DevKitC-1 N16R8 - GPS GT-U7 Tester
// Main Application File

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <TFT_eSPI.h>
#include <TinyGPSPlus.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "secrets.h"

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================
TFT_eSPI tft = TFT_eSPI();
TinyGPSPlus gps;
WiFiMulti wifiMulti;
AsyncWebServer server(WEB_SERVER_PORT);
AsyncWebSocket ws("/ws");
Adafruit_NeoPixel pixel(NEOPIXEL_NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);


// ============================================================================
// GLOBAL VARIABLES
// ============================================================================
HardwareSerial gpsSerial(2); // Using UART2 for GPS
uint8_t currentPage = PAGE_GPS_DATA;
bool lastButton1State = HIGH;
unsigned long lastButton1Press = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastGPSData = 0;
unsigned long gpsFixAcquiredTime = 0;
bool previousFixStatus = false;
bool wifiConnected = false;
String ipAddress = "";
bool webServerSetupDone = false;
int connectedClients = 0;

// NeoPixel status variables
enum LedState { OFF, SOLID, BLINKING };
LedState ledState = SOLID;
uint32_t ledColor = NEOPIXEL_COLOR_BLUE;
bool ledOn = true;
unsigned long lastBlinkTime = 0;


// GPS statistics
uint32_t totalSentences = 0;
uint32_t failedChecksums = 0;
uint32_t validSentences = 0;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================
void setupPins();
void setupSerial();
void setupDisplay();
void setupWiFi();
void setupWebServer();
void setupGPS();
void handleButton();
void updateGPS();
void updateDisplay();
void drawHeader();
void drawPageGPSData();
void drawPageDiagnostics();
void drawPageSatellites();
void setLedStatus(LedState state, uint32_t color);
void updateLed();
void playTone(int frequency, int duration);
void resetGPS();
String getGPSJson();
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len);

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  setLedStatus(SOLID, NEOPIXEL_COLOR_BLUE); // Blue during init

  setupPins();
  setupSerial();
  setupDisplay();
  setupGPS();
  setupWiFi();

  DEBUG_PRINTLN("=================================");
  DEBUG_PRINTLN(PROJECT_NAME);
  DEBUG_PRINT("Version: ");
  DEBUG_PRINTLN(PROJECT_VERSION);
  DEBUG_PRINTLN("=================================");
  DEBUG_PRINTLN("Setup complete. Starting main loop...");

}

// ============================================================================
// PIN SETUP
// ============================================================================
void setupPins() {
  pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  pinMode(PIN_BUTTON_2, INPUT_PULLUP);

  pixel.begin();
  pixel.setBrightness(NEOPIXEL_BRIGHTNESS);
  pixel.clear();
  pixel.show();

  // Initialize LEDC for the buzzer (tone function)
  // This prevents the "LEDC is not initialized" crash
  ledcSetup(BUZZER_LEDC_CHANNEL, BUZZER_FREQ_FIX, 8); // Setup channel with default freq, 8-bit resolution
  ledcAttachPin(PIN_BUZZER, BUZZER_LEDC_CHANNEL);

  pinMode(PIN_TFT_BL, OUTPUT);
  digitalWrite(PIN_TFT_BL, HIGH);

  pinMode(PIN_GPS_PPS, INPUT);
}

// ============================================================================
// SERIAL SETUP
// ============================================================================
void setupSerial() {
  Serial.begin(SERIAL_DEBUG_BAUD);
  delay(1000);
  DEBUG_PRINTLN("\n\nStarting GPS GT-U7 Tester...");
}

// ============================================================================
// DISPLAY SETUP
// ============================================================================
void setupDisplay() {
  updateLed(); // Show blue color
  DEBUG_PRINTLN("Initializing TFT display...");
  tft.init();
  tft.setRotation(TFT_ROTATION);
  tft.fillScreen(TFT_COLOR_BG);
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);

  tft.setTextDatum(MC_DATUM);
  tft.drawString("Initializing...", TFT_WIDTH/2, TFT_HEIGHT/2, 4);

  DEBUG_PRINTLN("TFT display initialized");
}

// ============================================================================
// GPS SETUP
// ============================================================================
void setupGPS() {
  updateLed(); // Show blue color
  DEBUG_PRINTLN("Initializing GPS...");
  gpsSerial.begin(GPS_BAUD_RATE, SERIAL_8N1, PIN_GPS_RXD, PIN_GPS_TXD);
  DEBUG_PRINTF("GPS Serial initialized on RX:%d TX:%d at %d baud\n",
               PIN_GPS_RXD, PIN_GPS_TXD, GPS_BAUD_RATE);
}

// ============================================================================
// WIFI SETUP
// ============================================================================
void setupWiFi() {
  updateLed(); // Show blue color
  DEBUG_PRINTLN("Connecting to WiFi...");

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID_1, WIFI_PASSWORD_1);
  wifiMulti.addAP(WIFI_SSID_2, WIFI_PASSWORD_2);

  // La connexion est maintenant gérée de manière non bloquante dans la loop()
  // On lance juste la première tentative ici.
  wifiMulti.run();
  ipAddress = "Connecting...";
  DEBUG_PRINTLN("WiFi connection process started...");
}

// ============================================================================
// WEB SERVER SETUP
// ============================================================================
void setupWebServer() {
  if (!wifiConnected) {
    DEBUG_PRINTLN("Skipping web server setup (no WiFi)");
    return;
  }

  DEBUG_PRINTLN("Setting up web server...");

  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    DEBUG_PRINTF("Web request received from: %s\n", request->client()->remoteIP().toString().c_str());
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GPS GT-U7 Tester</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: #fff;
            padding: 20px;
            min-height: 100vh;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        .header {
            text-align: center;
            padding: 30px 0;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 15px;
            margin-bottom: 30px;
            backdrop-filter: blur(10px);
        }
        .header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }
        .header .subtitle {
            font-size: 1.2em;
            opacity: 0.9;
        }
        .status-bar {
            display: flex;
            justify-content: space-around;
            flex-wrap: wrap;
            gap: 15px;
            margin-bottom: 30px;
        }
        .status-item {
            background: rgba(255, 255, 255, 0.15);
            padding: 15px 25px;
            border-radius: 10px;
            backdrop-filter: blur(10px);
            flex: 1;
            min-width: 150px;
            text-align: center;
        }
        .status-item .label {
            font-size: 0.9em;
            opacity: 0.8;
            margin-bottom: 5px;
        }
        .status-item .value {
            font-size: 1.5em;
            font-weight: bold;
        }
        .status-good { border-left: 4px solid #4CAF50; }
        .status-warning { border-left: 4px solid #FFC107; }
        .status-error { border-left: 4px solid #F44336; }
        .cards {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        .card {
            background: rgba(255, 255, 255, 0.15);
            border-radius: 15px;
            padding: 25px;
            backdrop-filter: blur(10px);
            box-shadow: 0 8px 32px 0 rgba(31, 38, 135, 0.37);
        }
        .card h2 {
            margin-bottom: 20px;
            font-size: 1.5em;
            border-bottom: 2px solid rgba(255, 255, 255, 0.3);
            padding-bottom: 10px;
        }
        .data-row {
            display: flex;
            justify-content: space-between;
            padding: 10px 0;
            border-bottom: 1px solid rgba(255, 255, 255, 0.1);
        }
        .data-row:last-child {
            border-bottom: none;
        }
        .data-label {
            opacity: 0.8;
        }
        .data-value {
            font-weight: bold;
            font-family: 'Courier New', monospace;
        }
        .map-container {
            background: rgba(255, 255, 255, 0.15);
            border-radius: 15px;
            padding: 25px;
            backdrop-filter: blur(10px);
            margin-bottom: 30px;
        }
        .map-container h2 {
            margin-bottom: 15px;
        }
        #map {
            width: 100%;
            height: 400px;
            border-radius: 10px;
            background: #fff;
        }
        .controls {
            display: flex;
            gap: 15px;
            justify-content: center;
            flex-wrap: wrap;
        }
        .btn {
            padding: 15px 30px;
            font-size: 1.1em;
            border: none;
            border-radius: 10px;
            cursor: pointer;
            transition: all 0.3s;
            font-weight: bold;
            text-transform: uppercase;
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
        }
        .btn-primary {
            background: #4CAF50;
            color: white;
        }
        .btn-primary:hover {
            background: #45a049;
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(0,0,0,0.3);
        }
        .btn-danger {
            background: #F44336;
            color: white;
        }
        .btn-danger:hover {
            background: #da190b;
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(0,0,0,0.3);
        }
        .satellite-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(80px, 1fr));
            gap: 10px;
            margin-top: 15px;
        }
        .satellite {
            background: rgba(255, 255, 255, 0.2);
            padding: 10px;
            border-radius: 8px;
            text-align: center;
        }
        .satellite .sat-id {
            font-weight: bold;
            margin-bottom: 5px;
        }
        .satellite .sat-snr {
            font-size: 0.9em;
            opacity: 0.9;
        }
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        .updating {
            animation: pulse 1s infinite;
        }
        @media (max-width: 768px) {
            .header h1 { font-size: 1.8em; }
            .cards { grid-template-columns: 1fr; }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🛰️ GPS GT-U7 Tester</h1>
            <div class="subtitle">ESP32-S3 DevKitC-1 N16R8 | v)rawliteral" + String(PROJECT_VERSION) + R"rawliteral(</div>
        </div>

        <div class="status-bar">
            <div class="status-item" id="fix-status">
                <div class="label">GPS Fix</div>
                <div class="value" id="fix-value">Searching...</div>
            </div>
            <div class="status-item" id="sat-status">
                <div class="label">Satellites</div>
                <div class="value" id="sat-value">0</div>
            </div>
            <div class="status-item" id="hdop-status">
                <div class="label">HDOP</div>
                <div class="value" id="hdop-value">--</div>
            </div>
            <div class="status-item status-good">
                <div class="label">Uptime</div>
                <div class="value" id="uptime-value">0s</div>
            </div>
        </div>

        <div class="cards">
            <div class="card">
                <h2>📍 Position</h2>
                <div class="data-row">
                    <span class="data-label">Latitude:</span>
                    <span class="data-value" id="lat">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Longitude:</span>
                    <span class="data-value" id="lng">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Altitude:</span>
                    <span class="data-value" id="alt">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Speed:</span>
                    <span class="data-value" id="speed">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Course:</span>
                    <span class="data-value" id="course">--</span>
                </div>
            </div>

            <div class="card">
                <h2>🕐 Date & Time</h2>
                <div class="data-row">
                    <span class="data-label">Date:</span>
                    <span class="data-value" id="date">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Time (UTC):</span>
                    <span class="data-value" id="time">--</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Age:</span>
                    <span class="data-value" id="age">--</span>
                </div>
            </div>

            <div class="card">
                <h2>📊 Diagnostics</h2>
                <div class="data-row">
                    <span class="data-label">Valid Sentences:</span>
                    <span class="data-value" id="valid">0</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Failed Checksums:</span>
                    <span class="data-value" id="failed">0</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Total Characters:</span>
                    <span class="data-value" id="chars">0</span>
                </div>
                <div class="data-row">
                    <span class="data-label">Success Rate:</span>
                    <span class="data-value" id="success-rate">100%</span>
                </div>
            </div>
        </div>

        <div class="controls">
            <button class="btn btn-danger" onclick="resetGPS()">🔄 Reset GPS</button>
        </div>
    </div>

    <script>
        let ws;
        let reconnectInterval;

        function connect() {
            ws = new WebSocket('ws://' + window.location.hostname + '/ws');

            ws.onopen = () => {
                console.log('WebSocket connected');
                clearInterval(reconnectInterval);
            };

            ws.onmessage = (event) => {
                const data = JSON.parse(event.data);
                updateDisplay(data);
            };

            ws.onclose = () => {
                console.log('WebSocket disconnected');
                reconnectInterval = setInterval(() => {
                    connect();
                }, 3000);
            };

            ws.onerror = (error) => {
                console.error('WebSocket error:', error);
            };
        }

        function updateDisplay(data) {
            document.getElementById('fix-value').textContent = data.fix ? 'LOCKED' : 'NO FIX';
            const fixStatus = document.getElementById('fix-status');
            fixStatus.className = 'status-item ' + (data.fix ? 'status-good' : 'status-error');

            document.getElementById('sat-value').textContent = data.satellites;
            const satStatus = document.getElementById('sat-status');
            satStatus.className = 'status-item ' + (data.satellites >= 4 ? 'status-good' : 'status-warning');

            document.getElementById('hdop-value').textContent = data.hdop;
            const hdopStatus = document.getElementById('hdop-status');
            const hdopVal = parseFloat(data.hdop);
            hdopStatus.className = 'status-item ' + (hdopVal < 2 ? 'status-good' : hdopVal < 5 ? 'status-warning' : 'status-error');

            document.getElementById('uptime-value').textContent = data.uptime;

            document.getElementById('lat').textContent = data.latitude;
            document.getElementById('lng').textContent = data.longitude;
            document.getElementById('alt').textContent = data.altitude;
            document.getElementById('speed').textContent = data.speed;
            document.getElementById('course').textContent = data.course;

            document.getElementById('date').textContent = data.date;
            document.getElementById('time').textContent = data.time;
            document.getElementById('age').textContent = data.age;

            document.getElementById('valid').textContent = data.validSentences;
            document.getElementById('failed').textContent = data.failedChecksums;
            document.getElementById('chars').textContent = data.totalChars;
            document.getElementById('success-rate').textContent = data.successRate;
        }

        function resetGPS() {
            if (confirm('Reset GPS module? This will restart the GPS.')) {
                fetch('/reset', { method: 'POST' })
                    .then(response => response.text())
                    .then(data => alert(data))
                    .catch(error => alert('Error: ' + error));
            }
        }

        connect();
    </script>
</body>
</html>
)rawliteral";
    request->send(200, "text/html", html);
  });

  server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    DEBUG_PRINTLN("GPS reset requested via web");
    resetGPS();
    request->send(200, "text/plain", "GPS module reset command sent");
  });

  server.begin();
  DEBUG_PRINTLN("Web server started");
  DEBUG_PRINT("Access at: http://");
  DEBUG_PRINTLN(ipAddress);
}

// ============================================================================
// WEBSOCKET EVENT HANDLER
// ============================================================================
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    DEBUG_PRINTF("WebSocket client #%u connected\n", client->id());
    connectedClients++;
    client->text(getGPSJson());
  } else if (type == WS_EVT_DISCONNECT) {
    DEBUG_PRINTF("WebSocket client #%u disconnected\n", client->id());
    connectedClients--;
  }
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
  static unsigned long wifiConnectStart = millis();

  // --- Gestion de la connexion WiFi (non-bloquant) ---
  if (!wifiConnected) {
    if (wifiMulti.run() == WL_CONNECTED) {
      DEBUG_PRINTLN(">>> WiFi connection successful!");
      wifiConnected = true;
      ipAddress = WiFi.localIP().toString();
      DEBUG_PRINTLN("\nWiFi connected!");
      DEBUG_PRINT("IP address: "); DEBUG_PRINTLN(ipAddress);
      DEBUG_PRINT("Connected to: "); DEBUG_PRINTLN(WiFi.SSID());
      setLedStatus(BLINKING, NEOPIXEL_COLOR_GREEN); // Start searching for GPS
      playTone(BUZZER_FREQ_FIX, BUZZER_DURATION);
    } else if (millis() - wifiConnectStart > WIFI_CONNECT_TIMEOUT) {
      // Timeout de connexion
      ipAddress = "No WiFi";
      DEBUG_PRINTLN("\nWiFi connection failed (timeout)!");
      setLedStatus(SOLID, NEOPIXEL_COLOR_RED); // Error state
      playTone(BUZZER_FREQ_LOST, BUZZER_DURATION * 2);
      wifiConnectStart = millis(); // Évite de retenter immédiatement
    }
  }

  // --- Initialisation du serveur web (une fois le WiFi connecté) ---
  if (wifiConnected && !webServerSetupDone) {
    DEBUG_PRINTLN(">>> WiFi is connected, proceeding to web server setup...");
    setupWebServer();
    webServerSetupDone = true;
  }

  // --- Tâches principales ---
  handleButton();
  updateGPS();
  updateLed();
  updateDisplay();

  // --- Mise à jour WebSocket ---
  // Le serveur doit être initialisé et des clients connectés
  if (webServerSetupDone && connectedClients > 0) {
    static unsigned long lastWebUpdate = 0;
    if (millis() - lastWebUpdate > WEB_UPDATE_INTERVAL) {
      ws.textAll(getGPSJson());
      lastWebUpdate = millis();
    }
  }

  if (webServerSetupDone) {
    ws.cleanupClients();
  }
}

// ============================================================================
// BUTTON HANDLER
// ============================================================================
void handleButton() {
  bool button1State = digitalRead(PIN_BUTTON_1);

  if (button1State == LOW && lastButton1State == HIGH) {
    if (millis() - lastButton1Press > BUTTON_DEBOUNCE_MS) {
      currentPage++;
      if (currentPage >= NUM_PAGES) {
        currentPage = 0;
      }
      DEBUG_PRINTF("Page changed to: %d\n", currentPage);
      lastDisplayUpdate = 0;
      playTone(2000, 50);
    }
    lastButton1Press = millis();
  }

  lastButton1State = button1State;
}

// ============================================================================
// GPS UPDATE
// ============================================================================
void updateGPS() {
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    if (gps.encode(c)) {
      validSentences++;
      lastGPSData = millis();
    }
  }

  totalSentences = gps.sentencesWithFix() + gps.failedChecksum();
  failedChecksums = gps.failedChecksum();

  bool currentFixStatus = gps.location.isValid() && gps.location.age() < GPS_TIMEOUT;

  if (currentFixStatus != previousFixStatus) {
    if (currentFixStatus) {
      DEBUG_PRINTLN("GPS FIX ACQUIRED!");
      gpsFixAcquiredTime = millis();
      if (BUZZER_ENABLED) {
        playTone(BUZZER_FREQ_FIX, BUZZER_DURATION);
      }
      setLedStatus(SOLID, NEOPIXEL_COLOR_GREEN); // Solid Green for GPS fix
    } else {
      DEBUG_PRINTLN("GPS FIX LOST!");
      if (BUZZER_ENABLED) {
        playTone(BUZZER_FREQ_LOST, BUZZER_DURATION * 2);
      }
      // If WiFi is connected, blink green for searching. Otherwise, it might be an error.
      if (wifiConnected) {
        setLedStatus(BLINKING, NEOPIXEL_COLOR_GREEN); // Blinking Green for searching
      }
    }
    previousFixStatus = currentFixStatus;
  }
}

// ============================================================================
// DISPLAY UPDATE
// ============================================================================
void updateDisplay() {
  if (millis() - lastDisplayUpdate < GPS_UPDATE_RATE) {
    return;
  }

  lastDisplayUpdate = millis();

  drawHeader();

  switch (currentPage) {
    case PAGE_GPS_DATA:
      drawPageGPSData();
      break;
    case PAGE_DIAGNOSTICS:
      drawPageDiagnostics();
      break;
    case PAGE_SATELLITES:
      drawPageSatellites();
      break;
  }
}

// ============================================================================
// DRAW HEADER
// ============================================================================
void drawHeader() {
  tft.fillRect(0, 0, TFT_WIDTH, 100, TFT_COLOR_HEADER);

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_HEADER);
  tft.setTextDatum(TC_DATUM);
  tft.drawString(PROJECT_NAME, TFT_WIDTH/2, 5, 2);
  tft.drawString("Model: " + String(GPS_MODEL), TFT_WIDTH/2, 25, 2);

  bool hasFix = gps.location.isValid() && gps.location.age() < GPS_TIMEOUT;
  String status = hasFix ? "FIX OK" : "NO FIX";
  uint16_t statusColor = hasFix ? TFT_COLOR_VALUE : TFT_COLOR_ERROR;
  tft.setTextColor(statusColor, TFT_COLOR_HEADER);
  tft.drawString("Status: " + status, TFT_WIDTH/2, 45, 2);

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_HEADER);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("IP:", 5, 70, 2);
  tft.setTextDatum(TR_DATUM);
  if (!wifiConnected && ipAddress == "Connecting...") {
      tft.setTextColor(TFT_COLOR_WARNING, TFT_COLOR_HEADER);
  } else {
      tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_HEADER);
  }
  tft.drawString(ipAddress, TFT_WIDTH - 5, 70, 2);

  tft.drawFastHLine(0, 100, TFT_WIDTH, TFT_COLOR_SEPARATOR);
}

// ============================================================================
// DRAW PAGE: GPS DATA
// ============================================================================
void drawPageGPSData() {
  int y = 110;
  int lineHeight = 22;

  tft.fillRect(0, 101, TFT_WIDTH, TFT_HEIGHT - 101, TFT_COLOR_BG);

  tft.setTextColor(TFT_COLOR_WARNING, TFT_COLOR_BG);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("GPS DATA", TFT_WIDTH/2, y, 2);
  y += lineHeight + 5;

  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);

  String lat = gps.location.isValid() ? String(gps.location.lat(), 6) : "--";
  tft.drawString("Lat: ", 10, y, 2);
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.drawString(lat, 60, y, 2);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  String lng = gps.location.isValid() ? String(gps.location.lng(), 6) : "--";
  tft.drawString("Lng: ", 10, y, 2);
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.drawString(lng, 60, y, 2);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  String alt = gps.altitude.isValid() ? String(gps.altitude.meters(), 1) + "m" : "--";
  tft.drawString("Alt: ", 10, y, 2);
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.drawString(alt, 60, y, 2);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  String spd = gps.speed.isValid() ? String(gps.speed.kmph(), 1) + "km/h" : "--";
  tft.drawString("Speed:", 10, y, 2);
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.drawString(spd, 80, y, 2);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  String crs = gps.course.isValid() ? String(gps.course.deg(), 1) + "°" : "--";
  tft.drawString("Course:", 10, y, 2);
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.drawString(crs, 80, y, 2);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  String sats = String(gps.satellites.value());
  tft.drawString("Sats:", 10, y, 2);
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.drawString(sats, 80, y, 2);
  y += lineHeight;

  if (gps.date.isValid() && gps.time.isValid()) {
    tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
    char dateStr[32];
    sprintf(dateStr, "%02d/%02d/%04d %02d:%02d:%02d",
            gps.date.day(), gps.date.month(), gps.date.year(),
            gps.time.hour(), gps.time.minute(), gps.time.second());
    tft.drawString("UTC:", 10, y, 2);
    tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
    tft.drawString(dateStr, 10, y + lineHeight, 2);
  }
}

// ============================================================================
// DRAW PAGE: DIAGNOSTICS
// ============================================================================
void drawPageDiagnostics() {
  int y = 110;
  int lineHeight = 22;

  tft.fillRect(0, 101, TFT_WIDTH, TFT_HEIGHT - 101, TFT_COLOR_BG);

  tft.setTextColor(TFT_COLOR_WARNING, TFT_COLOR_BG);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("DIAGNOSTICS", TFT_WIDTH/2, y, 2);
  y += lineHeight + 5;

  tft.setTextDatum(TL_DATUM);

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.drawString("Valid:", 10, y, 2);
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.drawString(String(validSentences), 120, y, 2);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.drawString("Failed:", 10, y, 2);
  tft.setTextColor(failedChecksums > 0 ? TFT_COLOR_ERROR : TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.drawString(String(failedChecksums), 120, y, 2);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.drawString("Chars:", 10, y, 2);
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.drawString(String(gps.charsProcessed()), 120, y, 2);
  y += lineHeight;

  float successRate = 0;
  if (totalSentences > 0) {
    successRate = ((totalSentences - failedChecksums) * 100.0) / totalSentences;
  }
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.drawString("Success:", 10, y, 2);
  tft.setTextColor(successRate > 95 ? TFT_COLOR_VALUE : TFT_COLOR_WARNING, TFT_COLOR_BG);
  tft.drawString(String(successRate, 1) + "%", 120, y, 2);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  String hdop = gps.hdop.isValid() ? String(gps.hdop.hdop(), 2) : "--";
  tft.drawString("HDOP:", 10, y, 2);
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.drawString(hdop, 120, y, 2);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  unsigned long age = gps.location.age();
  String ageStr = age < 1000 ? String(age) + "ms" : String(age / 1000) + "s";
  tft.drawString("Age:", 10, y, 2);
  tft.setTextColor(age < GPS_TIMEOUT ? TFT_COLOR_VALUE : TFT_COLOR_ERROR, TFT_COLOR_BG);
  tft.drawString(ageStr, 120, y, 2);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.drawString("Uptime:", 10, y, 2);
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  unsigned long uptime = millis() / 1000;
  char uptimeStr[32];
  sprintf(uptimeStr, "%luh %lum %lus", uptime / 3600, (uptime % 3600) / 60, uptime % 60);
  tft.drawString(uptimeStr, 10, y + lineHeight, 2);
}

// ============================================================================
// DRAW PAGE: SATELLITES
// ============================================================================
void drawPageSatellites() {
  int y = 110;
  int lineHeight = 22;

  tft.fillRect(0, 101, TFT_WIDTH, TFT_HEIGHT - 101, TFT_COLOR_BG);

  tft.setTextColor(TFT_COLOR_WARNING, TFT_COLOR_BG);
  tft.setTextDatum(TC_DATUM);
  tft.drawString("SATELLITES", TFT_WIDTH/2, y, 2);
  y += lineHeight + 5;

  tft.setTextDatum(TL_DATUM);

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.drawString("Satellites:", 10, y, 2);
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  uint32_t satCount = gps.satellites.value();
  tft.drawString(String(satCount), 140, y, 2);
  y += lineHeight;

  if (satCount > 0) {
    int barWidth = TFT_WIDTH - 40;
    int barHeight = 15;
    int barY = y + 10;

    tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
    tft.drawString("Signal Quality:", 10, y, 2);

    tft.drawRect(20, barY, barWidth, barHeight, TFT_COLOR_TEXT);

    uint16_t fillColor = satCount >= 4 ? TFT_COLOR_VALUE : TFT_COLOR_WARNING;
    int fillWidth = (satCount * barWidth) / 12;
    if (fillWidth > barWidth) fillWidth = barWidth;
    tft.fillRect(21, barY + 1, fillWidth, barHeight - 2, fillColor);

    y += lineHeight * 2;
  }

  String hdop = gps.hdop.isValid() ? String(gps.hdop.hdop(), 2) : "--";
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.drawString("HDOP:", 10, y, 2);
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.drawString(hdop, 140, y, 2);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  bool hasFix = gps.location.isValid() && gps.location.age() < GPS_TIMEOUT;
  if (hasFix) {
    unsigned long fixDuration = (millis() - gpsFixAcquiredTime) / 1000;
    char fixStr[32];
    sprintf(fixStr, "%lum %lus", fixDuration / 60, fixDuration % 60);
    tft.drawString("Fix Time:", 10, y, 2);
    tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
    tft.drawString(fixStr, 10, y + lineHeight, 2);
  } else {
    tft.setTextColor(TFT_COLOR_ERROR, TFT_COLOR_BG);
    tft.drawString("Searching for", 10, y, 2);
    tft.drawString("satellites...", 10, y + lineHeight, 2);
  }
}

// ============================================================================
// NEOPIXEL LED CONTROL
// ============================================================================
void setLedStatus(LedState state, uint32_t color) {
  ledState = state;
  ledColor = color;
  lastBlinkTime = millis(); // Reset blink timer on state change
  ledOn = true; // Ensure LED is on when state changes
  updateLed(); // Update immediately
}

void updateLed() {
  switch (ledState) {
    case SOLID:
      pixel.setPixelColor(0, ledColor);
      break;
    case BLINKING:
      if (millis() - lastBlinkTime > NEOPIXEL_BLINK_INTERVAL) {
        ledOn = !ledOn;
        lastBlinkTime = millis();
      }
      pixel.setPixelColor(0, ledOn ? ledColor : NEOPIXEL_COLOR_OFF);
      break;
    case OFF:
      pixel.setPixelColor(0, NEOPIXEL_COLOR_OFF);
      break;
  }
  pixel.show();
}

// ============================================================================
// BUZZER CONTROL
// ============================================================================
void playTone(int frequency, int duration) {
  if (!BUZZER_ENABLED) return;

  ledcWriteTone(BUZZER_LEDC_CHANNEL, frequency);
  delay(duration); // Keep the tone for the duration
  ledcWriteTone(BUZZER_LEDC_CHANNEL, 0); // Stop the tone
}

// ============================================================================
// GPS RESET
// ============================================================================
void resetGPS() {
  DEBUG_PRINTLN("Resetting GPS module...");

  gpsSerial.end();
  delay(100);

  gpsSerial.begin(GPS_BAUD_RATE, SERIAL_8N1, PIN_GPS_RXD, PIN_GPS_TXD);
  delay(100);

  validSentences = 0;
  failedChecksums = 0;
  totalSentences = 0;
  previousFixStatus = false;

  DEBUG_PRINTLN("GPS module reset complete");
}

// ============================================================================
// GET GPS DATA AS JSON
// ============================================================================
String getGPSJson() {
  JsonDocument doc;

  doc["fix"] = gps.location.isValid() && gps.location.age() < GPS_TIMEOUT;
  doc["satellites"] = gps.satellites.value();
  doc["hdop"] = gps.hdop.isValid() ? String(gps.hdop.hdop(), 2) : "--";

  unsigned long uptime = millis() / 1000;
  char uptimeStr[32];
  sprintf(uptimeStr, "%luh %lum %lus", uptime / 3600, (uptime % 3600) / 60, uptime % 60);
  doc["uptime"] = String(uptimeStr);

  doc["latitude"] = gps.location.isValid() ? String(gps.location.lat(), 6) : "--";
  doc["longitude"] = gps.location.isValid() ? String(gps.location.lng(), 6) : "--";
  doc["altitude"] = gps.altitude.isValid() ? String(gps.altitude.meters(), 1) + " m" : "--";
  doc["speed"] = gps.speed.isValid() ? String(gps.speed.kmph(), 1) + " km/h" : "--";
  doc["course"] = gps.course.isValid() ? String(gps.course.deg(), 1) + "°" : "--";

  if (gps.date.isValid()) {
    char dateStr[16];
    sprintf(dateStr, "%02d/%02d/%04d", gps.date.day(), gps.date.month(), gps.date.year());
    doc["date"] = String(dateStr);
  } else {
    doc["date"] = "--";
  }

  if (gps.time.isValid()) {
    char timeStr[16];
    sprintf(timeStr, "%02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
    doc["time"] = String(timeStr);
  } else {
    doc["time"] = "--";
  }

  unsigned long age = gps.location.age();
  String ageStr = age < 1000 ? String(age) + " ms" : String(age / 1000) + " s";
  doc["age"] = ageStr;

  doc["validSentences"] = validSentences;
  doc["failedChecksums"] = failedChecksums;
  doc["totalChars"] = gps.charsProcessed();

  float successRate = 0;
  if (totalSentences > 0) {
    successRate = ((totalSentences - failedChecksums) * 100.0) / totalSentences;
  }
  doc["successRate"] = String(successRate, 1) + "%";

  String output;
  serializeJson(doc, output);
  return output;
}