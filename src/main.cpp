// Version: 1.4.2
// ESP32-S3 DevKitC-1 N16R8 - GPS GT-U7 Tester
// Main Application File

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <TinyGPSPlus.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "webpage.h" // Externalized web page content
#include "DrSugiyama_Regular28pt7b.h" // Police personnalisée pour le démarrage
#include "secrets.h"

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================
// Utilisation de pointeurs pour les objets matériels afin d'éviter une initialisation précoce
Adafruit_ST7789 *tftPtr = nullptr;
TinyGPSPlus gps;
WiFiMulti wifiMulti;
AsyncWebServer server(WEB_SERVER_PORT);
AsyncWebSocket ws("/ws");
Adafruit_NeoPixel *pixelPtr = nullptr;

// Macro globale pour un accès simplifié à l'objet TFT via son pointeur
#define tft (*tftPtr)

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
bool displayAvailable = false;
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
void drawInitScreen(const String& line1, const String& line2 = "", const String& line3 = "");
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len);

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  // Initialize serial FIRST for debugging
  Serial.begin(SERIAL_DEBUG_BAUD);
  delay(1000);
  DEBUG_PRINTLN("\n\n=== BOOT STARTING ===");

  DEBUG_PRINTLN("Setting up pins...");
  setupPins();

  DEBUG_PRINTLN("Setting LED status...");
  setLedStatus(SOLID, NEOPIXEL_COLOR_BLUE); // Blue during init

  DEBUG_PRINTLN("Setting up display...");
  setupDisplay();

  DEBUG_PRINTLN("Setting up GPS...");
  setupGPS();

  DEBUG_PRINTLN("Setting up WiFi...");
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
  DEBUG_PRINTLN("  - Setting up buttons...");
  pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  pinMode(PIN_BUTTON_2, INPUT_PULLUP);

  DEBUG_PRINTLN("  - Initializing NeoPixel...");
  // Create NeoPixel object dynamically to avoid early initialization crash
  pixelPtr = new Adafruit_NeoPixel(NEOPIXEL_NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
  if (pixelPtr != nullptr) {
    pixelPtr->begin();
    pixelPtr->setBrightness(NEOPIXEL_BRIGHTNESS);
    pixelPtr->clear();
    pixelPtr->show();
    DEBUG_PRINTLN("  - NeoPixel OK");
  } else {
    DEBUG_PRINTLN("  - NeoPixel FAILED to allocate!");
  }

  DEBUG_PRINTLN("  - Setting up buzzer (LEDC)...");
  // Initialize LEDC for the buzzer (tone function)
  // This prevents the "LEDC is not initialized" crash
  ledcSetup(BUZZER_LEDC_CHANNEL, BUZZER_FREQ_FIX, 8); // Setup channel with default freq, 8-bit resolution
  ledcAttachPin(PIN_BUZZER, BUZZER_LEDC_CHANNEL);
  DEBUG_PRINTLN("  - Buzzer OK");

  DEBUG_PRINTLN("  - Setting up TFT backlight...");
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  DEBUG_PRINTLN("  - Setting up GPS PPS pin...");
  pinMode(PIN_GPS_PPS, INPUT);
  DEBUG_PRINTLN("  - Pin setup complete");
}

// ============================================================================
// DRAW INITIALIZATION SCREEN
// ============================================================================
void drawInitScreen(const String& line1, const String& line2, const String& line3) {
  if (!displayAvailable) return;

  tft.fillScreen(TFT_COLOR_BG);

  // --- Draw Title ("morfredus") with custom font ---
  tft.setFont(&DrSugiyama_Regular28pt7b);
  tft.setTextColor(TFT_COLOR_WARNING);
  tft.setTextSize(1); // Augmentation de la taille de la police personnalisée
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds("morfredus", 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((TFT_WIDTH - w) / 2, 60);
  tft.print("morfredus");

  // --- Draw Subtitle ("GPS Tester") ---
  tft.setFont(); // Reset to default font
  tft.setTextSize(3);
  tft.setTextColor(TFT_COLOR_HEADER);
  tft.getTextBounds("GPS Tester", 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((TFT_WIDTH - w) / 2, 95);
  tft.print("GPS Tester");

  // --- Draw status lines with default font ---
  tft.setTextSize(2);
  tft.setTextColor(TFT_COLOR_TEXT);
  // Centrer les lignes de statut pour un meilleur look
  tft.getTextBounds(line1, 0, 0, &x1, &y1, &w, &h); tft.setCursor((TFT_WIDTH - w) / 2, 150); tft.print(line1);
  tft.getTextBounds(line2, 0, 0, &x1, &y1, &w, &h); tft.setCursor((TFT_WIDTH - w) / 2, 180); tft.print(line2);
  tft.getTextBounds(line3, 0, 0, &x1, &y1, &w, &h); tft.setCursor((TFT_WIDTH - w) / 2, 210); tft.print(line3);
}

// ============================================================================
// DISPLAY SETUP
// ============================================================================
void setupDisplay() {
  updateLed(); // Show blue color

  DEBUG_PRINTLN("Initializing TFT ST7789 display...");
  tftPtr = new Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
  if (tftPtr != nullptr) {
    tftPtr->init(TFT_WIDTH, TFT_HEIGHT);
    tftPtr->setRotation(TFT_ROTATION);
    tftPtr->fillScreen(TFT_COLOR_BG);
    tftPtr->setTextWrap(false);
    displayAvailable = true;
    drawInitScreen("Initializing...");
    DEBUG_PRINTLN("TFT display initialized");
  } else {
    DEBUG_PRINTLN("ERROR: Failed to allocate TFT object!");
    displayAvailable = false;
  }
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
  if (displayAvailable) drawInitScreen("Searching for", "WiFi..."); // Texte sur deux lignes
  DEBUG_PRINTLN("Connecting to WiFi...");

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID_1, WIFI_PASSWORD_1);
  wifiMulti.addAP(WIFI_SSID_2, WIFI_PASSWORD_2);

  ipAddress = "Connecting..."; // Statut initial
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
    String html(HTML_CONTENT);
    html.replace("%PROJECT_VERSION%", String(PROJECT_VERSION));
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
  static unsigned long lastWifiCheck = 0;

  // --- Gestion de la connexion WiFi (non-bloquant) ---
  if (!wifiConnected && millis() - lastWifiCheck > WIFI_RETRY_DELAY) {
    if (wifiMulti.run() == WL_CONNECTED) { // Tentative de connexion
        DEBUG_PRINTLN(">>> WiFi connection successful!");
        wifiConnected = true;
        ipAddress = WiFi.localIP().toString();
        DEBUG_PRINTLN("\nWiFi connected!");
        DEBUG_PRINT("IP address: "); DEBUG_PRINTLN(ipAddress);
        DEBUG_PRINT("Connected to: "); DEBUG_PRINTLN(WiFi.SSID());
        if (displayAvailable) {
            drawInitScreen("Connected to:", WiFi.SSID(), "IP: " + ipAddress);
            delay(2000); // Pause pour montrer l'IP
        }
        setLedStatus(BLINKING, NEOPIXEL_COLOR_RED); // Commence la recherche GPS
        playTone(BUZZER_FREQ_FIX, BUZZER_DURATION);

        // Initialisation du serveur web (une seule fois)
        if (!webServerSetupDone) {
            DEBUG_PRINTLN(">>> WiFi is connected, proceeding to web server setup...");
            setupWebServer();
            webServerSetupDone = true;
        }
    }
    lastWifiCheck = millis();
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

  if (currentFixStatus) {
    // --- We have a GPS fix ---
    if (!previousFixStatus) {
      // This is the moment we just acquired the fix
      DEBUG_PRINTLN("GPS FIX ACQUIRED!");
      gpsFixAcquiredTime = millis();
      if (BUZZER_ENABLED) {
        playTone(BUZZER_FREQ_FIX, BUZZER_DURATION);
      }
      ws.textAll(getGPSJson()); // Send immediate update on fix acquired
    }
    // Check precision for LED color
    if (gps.hdop.isValid() && gps.hdop.hdop() < HDOP_GOOD_THRESHOLD) {
      setLedStatus(SOLID, NEOPIXEL_COLOR_GREEN); // Good precision
    } else {
      setLedStatus(SOLID, NEOPIXEL_COLOR_BLUE); // Fix OK, but precision is not optimal
    }
  } else {
    // --- No GPS fix ---
    if (previousFixStatus) {
      // This is the moment we just lost the fix
      DEBUG_PRINTLN("GPS FIX LOST!");
      if (BUZZER_ENABLED) {
        playTone(BUZZER_FREQ_LOST, BUZZER_DURATION * 2);
      }
      ws.textAll(getGPSJson()); // Send immediate update on fix lost
    }
  }
  previousFixStatus = currentFixStatus;
}

// ============================================================================
// DISPLAY UPDATE
// ============================================================================
void updateDisplay() {
  if (!displayAvailable) return;

  if (millis() - lastDisplayUpdate < GPS_UPDATE_RATE) {
    return;
  }
  lastDisplayUpdate = millis();

  drawHeader(); // Common header for all pages
  switch (currentPage) {
    case PAGE_GPS_DATA:      drawPageGPSData();      break;
    case PAGE_DIAGNOSTICS:   drawPageDiagnostics();  break;
    case PAGE_SATELLITES:    drawPageSatellites();   break;
  }
}

// ============================================================================
// DRAW HEADER
// ============================================================================
void drawHeader() {
  if (!displayAvailable) return;
  tft.fillRect(0, 0, TFT_WIDTH, 100, TFT_COLOR_HEADER);
  tft.setTextSize(FONT_SIZE_HEADER);

  int16_t x1, y1;
  uint16_t w, h;
  
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_HEADER);
  tft.getTextBounds(PROJECT_NAME, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((TFT_WIDTH - w) / 2, 5);
  tft.print(PROJECT_NAME);

  String modelStr = "Model: " + String(GPS_MODEL);
  tft.getTextBounds(modelStr, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((TFT_WIDTH - w) / 2, 25);
  tft.print(modelStr);

  bool hasFix = gps.location.isValid() && gps.location.age() < GPS_TIMEOUT;
  String status = hasFix ? "FIX OK" : "NO FIX";
  uint16_t statusColor = hasFix ? TFT_COLOR_VALUE : TFT_COLOR_ERROR;
  tft.setTextColor(statusColor, TFT_COLOR_HEADER);
  String statusStr = "Status: " + status;
  tft.getTextBounds(statusStr, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((TFT_WIDTH - w) / 2, 45);
  tft.print(statusStr);
  
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_HEADER);
  tft.setCursor(5, 70);
  tft.print("IP:");

  if (!wifiConnected && ipAddress == "Connecting...") {
      tft.setTextColor(TFT_COLOR_WARNING, TFT_COLOR_HEADER);
  } else {
      tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_HEADER);
  }
  tft.getTextBounds(ipAddress, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(TFT_WIDTH - 5 - w, 70);
  tft.print(ipAddress);

  tft.drawFastHLine(0, 100, TFT_WIDTH, TFT_COLOR_SEPARATOR);
}

// ============================================================================
// DRAW PAGE: GPS DATA
// ============================================================================
void drawPageGPSData() {
  int y = 110;
  int lineHeight = 22;

  tft.fillRect(0, 101, TFT_WIDTH, TFT_HEIGHT - 101, TFT_COLOR_BG);
  tft.setTextSize(FONT_SIZE_DEFAULT);

  int16_t x1, y1;
  uint16_t w, h;
  String pageTitle = "GPS DATA";
  tft.setTextColor(TFT_COLOR_WARNING, TFT_COLOR_BG);
  tft.getTextBounds(pageTitle, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((TFT_WIDTH - w) / 2, y);
  tft.print(pageTitle);
  y += lineHeight + 5;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);

  String lat = gps.location.isValid() ? String(gps.location.lat(), 6) : "--";
  tft.setCursor(10, y);
  tft.print("Lat: ");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(60, y);
  tft.print(lat);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  String lng = gps.location.isValid() ? String(gps.location.lng(), 6) : "--";
  tft.setCursor(10, y);
  tft.print("Lng: ");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(60, y);
  tft.print(lng);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  String alt = gps.altitude.isValid() ? String(gps.altitude.meters(), 1) + "m" : "--";
  tft.setCursor(10, y);
  tft.print("Alt: ");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(60, y);
  tft.print(alt);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  String spd = gps.speed.isValid() ? String(gps.speed.kmph(), 1) + "km/h" : "--";
  tft.setCursor(10, y);
  tft.print("Speed:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(80, y);
  tft.print(spd);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  String crs = gps.course.isValid() ? String(gps.course.deg(), 1) + "°" : "--";
  tft.setCursor(10, y);
  tft.print("Course:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(80, y);
  tft.print(crs);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  String sats = String(gps.satellites.value());
  tft.setCursor(10, y);
  tft.print("Sats:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(80, y);
  tft.print(sats);
  y += lineHeight;

  if (gps.date.isValid() && gps.time.isValid()) {
    tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
    char dateStr[32];
    sprintf(dateStr, "%02d/%02d/%04d %02d:%02d:%02d",
            gps.date.day(), gps.date.month(), gps.date.year(),
            gps.time.hour(), gps.time.minute(), gps.time.second());
    tft.setCursor(10, y);
    tft.print("UTC:");
    tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
    tft.setCursor(10, y + lineHeight);
    tft.print(dateStr);
  }
}

// ============================================================================
// DRAW PAGE: DIAGNOSTICS
// ============================================================================
void drawPageDiagnostics() {
  int y = 110;
  int lineHeight = 22;

  tft.fillRect(0, 101, TFT_WIDTH, TFT_HEIGHT - 101, TFT_COLOR_BG);
  tft.setTextSize(FONT_SIZE_DEFAULT);

  int16_t x1, y1;
  uint16_t w, h;
  String pageTitle = "DIAGNOSTICS";
  tft.setTextColor(TFT_COLOR_WARNING, TFT_COLOR_BG);
  tft.getTextBounds(pageTitle, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((TFT_WIDTH - w) / 2, y);
  tft.print(pageTitle);
  y += lineHeight + 5;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.setCursor(10, y);
  tft.print("Valid:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(120, y);
  tft.print(String(validSentences));
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.setCursor(10, y);
  tft.print("Failed:");
  tft.setTextColor(failedChecksums > 0 ? TFT_COLOR_ERROR : TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(120, y);
  tft.print(String(failedChecksums));
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.setCursor(10, y);
  tft.print("Chars:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(120, y);
  tft.print(String(gps.charsProcessed()));
  y += lineHeight;

  float successRate = 0;
  if (totalSentences > 0) {
    successRate = ((totalSentences - failedChecksums) * 100.0) / totalSentences;
  }
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.setCursor(10, y);
  tft.print("Success:");
  tft.setTextColor(successRate > 95 ? TFT_COLOR_VALUE : TFT_COLOR_WARNING, TFT_COLOR_BG);
  tft.setCursor(120, y);
  tft.print(String(successRate, 1) + "%");
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  String hdop = gps.hdop.isValid() ? String(gps.hdop.hdop(), 2) : "--";
  tft.setCursor(10, y);
  tft.print("HDOP:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(120, y);
  tft.print(hdop);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  unsigned long age = gps.location.age();
  String ageStr = age < 1000 ? String(age) + "ms" : String(age / 1000) + "s";
  tft.setCursor(10, y);
  tft.print("Age:");
  tft.setTextColor(age < GPS_TIMEOUT ? TFT_COLOR_VALUE : TFT_COLOR_ERROR, TFT_COLOR_BG);
  tft.setCursor(120, y);
  tft.print(ageStr);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.setCursor(10, y);
  tft.print("Uptime:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  unsigned long uptime = millis() / 1000;
  char uptimeStr[32];
  sprintf(uptimeStr, "%luh %lum %lus", uptime / 3600, (uptime % 3600) / 60, uptime % 60);
  tft.setCursor(10, y + lineHeight);
  tft.print(uptimeStr);
}

// ============================================================================
// DRAW PAGE: SATELLITES
// ============================================================================
void drawPageSatellites() {
  int y = 110;
  int lineHeight = 22;

  tft.fillRect(0, 101, TFT_WIDTH, TFT_HEIGHT - 101, TFT_COLOR_BG);
  tft.setTextSize(FONT_SIZE_DEFAULT);

  int16_t x1, y1;
  uint16_t w, h;
  String pageTitle = "SATELLITES";
  tft.setTextColor(TFT_COLOR_WARNING, TFT_COLOR_BG);
  tft.getTextBounds(pageTitle, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((TFT_WIDTH - w) / 2, y);
  tft.print(pageTitle);
  y += lineHeight + 5;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.setCursor(10, y);
  tft.print("Satellites:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  uint32_t satCount = gps.satellites.value();
  tft.setCursor(140, y);
  tft.print(String(satCount));
  y += lineHeight;

  if (satCount > 0) {
    int barWidth = TFT_WIDTH - 40;
    int barHeight = 15;
    int barY = y + 10;

    tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
    tft.setCursor(10, y);
    tft.print("Signal Quality:");

    tft.drawRect(20, barY, barWidth, barHeight, TFT_COLOR_TEXT);

    uint16_t fillColor = satCount >= 4 ? TFT_COLOR_VALUE : TFT_COLOR_WARNING;
    int fillWidth = (satCount * barWidth) / 12;
    if (fillWidth > barWidth) fillWidth = barWidth;
    tft.fillRect(21, barY + 1, fillWidth, barHeight - 2, fillColor);

    y += lineHeight * 2;
  }

  String hdop = gps.hdop.isValid() ? String(gps.hdop.hdop(), 2) : "--";
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.setCursor(10, y);
  tft.print("HDOP:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(140, y);
  tft.print(hdop);
  y += lineHeight;

  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  bool hasFix = gps.location.isValid() && gps.location.age() < GPS_TIMEOUT;
  if (hasFix) {
    unsigned long fixDuration = (millis() - gpsFixAcquiredTime) / 1000;
    char fixStr[32];
    sprintf(fixStr, "%lum %lus", fixDuration / 60, fixDuration % 60);
    tft.setCursor(10, y);
    tft.print("Fix Time:");
    tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
    tft.setCursor(10, y + lineHeight);
    tft.print(fixStr);
  } else {
    tft.setTextColor(TFT_COLOR_ERROR, TFT_COLOR_BG);
    tft.setCursor(10, y);
    tft.print("Searching for");
    tft.setCursor(10, y + lineHeight);
    tft.print("satellites...");
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
  if (pixelPtr == nullptr) return; // Safety check

  switch (ledState) {
    case SOLID:
      pixelPtr->setPixelColor(0, ledColor);
      break;
    case BLINKING:
      if (millis() - lastBlinkTime > NEOPIXEL_BLINK_INTERVAL) {
        ledOn = !ledOn;
        lastBlinkTime = millis();
      }
      pixelPtr->setPixelColor(0, ledOn ? ledColor : NEOPIXEL_COLOR_OFF);
      break;
    case OFF:
      pixelPtr->setPixelColor(0, NEOPIXEL_COLOR_OFF);
      break;
  }
  pixelPtr->show();
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

  // --- GPS Module Information ---
  doc["gpsModel"] = String(GPS_MODEL);
  doc["gpsBaud"] = String(GPS_BAUD_RATE) + " bps";
  doc["gpsRate"] = String(1000.0 / GPS_UPDATE_RATE, 1) + " Hz";

  // --- Board Information ---
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  doc["chipModel"] = chip_info.model == CHIP_ESP32S3 ? "ESP32-S3" : "Unknown";
  doc["chipCores"] = chip_info.cores;
  doc["chipFreq"] = String(ESP.getCpuFreqMHz()) + " MHz";
  doc["chipMemory"] = String(ESP.getFlashChipSize() / (1024 * 1024)) + "MB Flash / " +
                     String(ESP.getPsramSize() / (1024 * 1024)) + "MB PSRAM";

  String output;
  serializeJson(doc, output);
  return output;
}