// Version: 1.7.3
// ESP32-S3 DevKitC-1 N16R8 - GPS GT-U7 Tester - TFT Display Enhancements
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
#include "DrSugiyama_Regular28pt7b.h" // Custom font for startup
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

// Helper macro to access TFT (for easy migration back if needed)
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
bool webServerSetupDone = false;
int connectedClients = 0;

// --- Display Layout Constants (consider moving to config.h) ---
const int TFT_HEADER_HEIGHT = 60; // Reduced header height
const int TFT_PAGE_START_Y = TFT_HEADER_HEIGHT + 1; // Y-start for page content
const int TFT_LINE_HEIGHT = 20; // Line spacing for font size 2


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
// SERIAL SETUP (Now called from setup() directly)
// ============================================================================
void setupSerial() {
  // This function is no longer needed as Serial is initialized in setup()
  // Kept for backwards compatibility
}

// ============================================================================
// DRAW INITIALIZATION SCREEN
// ============================================================================
void drawInitScreen(const String& line1, const String& line2, const String& line3) {
  tft.fillScreen(TFT_COLOR_BG);

  // --- Draw Title ("morfredus") with custom font ---
  tft.setFont(&DrSugiyama_Regular28pt7b);
  tft.setTextColor(TFT_COLOR_WARNING);
  tft.setTextSize(1.8); // Augmentation de la taille de la police personnalisée
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds("morfredus", 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((TFT_WIDTH - w) / 2, 60);
  tft.print("morfredus");

  // --- Draw Subtitle ("GPS Tester") ---
  tft.setFont(); // Reset to default font
  tft.setTextSize(3);
  tft.setTextColor(TFT_COLOR_TEXT); // Changed to TEXT for consistency with other status messages
  tft.getTextBounds("GPS Tester", 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((TFT_WIDTH - w) / 2, 95);
  tft.print("GPS Tester");

  // --- Draw status lines with default font ---
  tft.setTextColor(TFT_COLOR_TEXT);
  tft.setTextSize(2); // Reverted to original size 2
  tft.getTextBounds(line1, 0, 0, &x1, &y1, &w, &h); tft.setCursor((TFT_WIDTH - w) / 2, 150); tft.print(line1);
  tft.getTextBounds(line2, 0, 0, &x1, &y1, &w, &h); tft.setCursor((TFT_WIDTH - w) / 2, 180); tft.print(line2);
  tft.getTextBounds(line3, 0, 0, &x1, &y1, &w, &h); tft.setCursor((TFT_WIDTH - w) / 2, 210); tft.print(line3);
}

// ============================================================================
// DISPLAY SETUP
// ============================================================================
void setupDisplay() {
  updateLed(); // Show blue color
  DEBUG_PRINTLN("Initializing TFT display...");
  
  // Création dynamique de l'objet TFT pour éviter les crashs d'initialisation précoce
  // Assurez-vous que TFT_CS, TFT_DC, TFT_RST sont définis dans config.h
  tftPtr = new Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
  if (tftPtr == nullptr) {
    DEBUG_PRINTLN("ERROR: Failed to allocate TFT object!");
    return;
  }

  tft.init(TFT_WIDTH, TFT_HEIGHT);
  tft.setRotation(TFT_ROTATION);
  tft.fillScreen(TFT_COLOR_BG);
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.setTextWrap(false);
  drawInitScreen("Initializing...");
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
  drawInitScreen("Searching" , "for WiFi...");
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
      drawInitScreen("Connected to:", WiFi.SSID(), "IP: " + ipAddress);
      delay(2000); // Pause for 2 seconds as requested
      setLedStatus(BLINKING, NEOPIXEL_COLOR_GREEN); // Start searching for GPS (green blinking)
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

  // --- Gestion de l'état de la LED en fonction du GPS ---
  if (wifiConnected && millis() - lastGPSData > GPS_TIMEOUT && lastGPSData != 0) {
    // Erreur : Aucune donnée GPS reçue depuis un certain temps
    setLedStatus(SOLID, NEOPIXEL_COLOR_RED);
    if (previousFixStatus) { // Si on vient de perdre le fix à cause du timeout
      DEBUG_PRINTLN("GPS FIX LOST (Timeout)!");
      if (BUZZER_ENABLED) playTone(BUZZER_FREQ_LOST, BUZZER_DURATION * 2);
      ws.textAll(getGPSJson());
    }
  } else if (currentFixStatus) {
    // Fix GPS acquis et valide
    setLedStatus(SOLID, NEOPIXEL_COLOR_GREEN);
    if (!previousFixStatus) {
      DEBUG_PRINTLN("GPS FIX ACQUIRED!");
      gpsFixAcquiredTime = millis();
      if (BUZZER_ENABLED) playTone(BUZZER_FREQ_FIX, BUZZER_DURATION);
      ws.textAll(getGPSJson());
    }
  } else {
    // Pas de fix, en recherche (si le WiFi est connecté)
    if (wifiConnected) setLedStatus(BLINKING, NEOPIXEL_COLOR_GREEN);
    if (previousFixStatus) {
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
  tft.fillRect(0, 0, TFT_WIDTH, TFT_HEADER_HEIGHT, TFT_COLOR_HEADER);

  int16_t x1, y1;
  uint16_t w, h;
  
  // Project Name (Title)
  tft.setTextSize(2); // Increased font size for header title
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_HEADER);
  tft.getTextBounds(PROJECT_NAME, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((TFT_WIDTH - w) / 2, 5); // Centered, 5px from top
  tft.print(PROJECT_NAME);

  // GPS Status (centered)
  tft.setTextSize(2);
  bool hasFix = gps.location.isValid() && gps.location.age() < GPS_TIMEOUT;
  String status = hasFix ? "FIX OK" : "NO FIX";
  uint16_t statusColor = hasFix ? TFT_COLOR_VALUE : TFT_COLOR_ERROR;
  tft.setTextColor(statusColor, TFT_COLOR_HEADER);
  String statusStr = "Status: " + status;
  tft.getTextBounds(statusStr, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((TFT_WIDTH - w) / 2, 28); // Centered, below title
  tft.print(statusStr);
  
  // IP Address
  tft.setTextSize(1);
  if (!wifiConnected && ipAddress == "Connecting...") {
      tft.setTextColor(TFT_COLOR_WARNING, TFT_COLOR_HEADER);
  } else {
      tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_HEADER);
  }
  tft.getTextBounds(ipAddress, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(TFT_WIDTH - 5 - w, 45); // Right-aligned, moved down
  tft.print(ipAddress);

  tft.drawFastHLine(0, TFT_HEADER_HEIGHT, TFT_WIDTH, TFT_COLOR_SEPARATOR);
}

// ============================================================================
// DRAW PAGE: GPS DATA
// ============================================================================
void drawPageGPSData() {
  int y = TFT_PAGE_START_Y + 5;
  
  tft.fillRect(0, TFT_PAGE_START_Y, TFT_WIDTH, TFT_HEIGHT - TFT_PAGE_START_Y, TFT_COLOR_BG);
  
  tft.setTextSize(2); // Increased font size for page title

  int16_t x1, y1;
  uint16_t w, h;
  String pageTitle = "GPS DATA";
  tft.setTextColor(TFT_COLOR_WARNING, TFT_COLOR_BG);
  tft.getTextBounds(pageTitle, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((TFT_WIDTH - w) / 2, y); // Centered
  tft.print(pageTitle);
  y += TFT_LINE_HEIGHT + 5;

  tft.setTextSize(2); // Increased font size for data
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);

  // Lat / Lng on same line
  String lat = gps.location.isValid() ? String(gps.location.lat(), 6) : "--";
  String lng = gps.location.isValid() ? String(gps.location.lng(), 6) : "--";
  tft.setCursor(5, y); tft.print("Lat:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(65, y); tft.print(lat.substring(0, 7)); // Truncate for space
  y += TFT_LINE_HEIGHT;
  tft.setCursor(5, y); tft.print("Lng:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(65, y); tft.print(lng.substring(0, 7)); // Truncate for space
  y += TFT_LINE_HEIGHT;

  // Alt / Sats on same line
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  String alt = gps.altitude.isValid() ? String(gps.altitude.meters(), 1) + "m" : "--";
  String sats = String(gps.satellites.value());
  tft.setCursor(5, y); tft.print("Alt:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(65, y); tft.print(alt);
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.setCursor(150, y); tft.print("Sats:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(210, y); tft.print(sats);
  y += TFT_LINE_HEIGHT;

  // Speed / Course on same line
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  String spd = gps.speed.isValid() ? String(gps.speed.kmph(), 1) + "km/h" : "--";
  String crs = gps.course.isValid() ? String(gps.course.deg(), 1) + "°" : "--";
  tft.setCursor(5, y); tft.print("Spd:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(65, y); tft.print(spd);
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.setCursor(150, y); tft.print("Crs:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(210, y); tft.print(crs);
  y += TFT_LINE_HEIGHT;

  // UTC Time and Date
  if (gps.date.isValid() && gps.time.isValid()) {
    tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
    char dateStr[32];
    sprintf(dateStr, "%02d/%02d/%04d %02d:%02d:%02d",
            gps.date.day(), gps.date.month(), gps.date.year(),
            gps.time.hour(), gps.time.minute(), gps.time.second());
    tft.setCursor(5, y);
    tft.print("UTC:");
    tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
    tft.setCursor(65, y);
    tft.print(String(dateStr).substring(11)); // Time only, skipping the space
  }
}

// ============================================================================
// DRAW PAGE: DIAGNOSTICS
// ============================================================================
void drawPageDiagnostics() {
  int y = TFT_PAGE_START_Y + 5;
  
  tft.fillRect(0, TFT_PAGE_START_Y, TFT_WIDTH, TFT_HEIGHT - TFT_PAGE_START_Y, TFT_COLOR_BG);
  
  tft.setTextSize(2); // Increased font size for page title

  int16_t x1, y1;
  uint16_t w, h;
  String pageTitle = "DIAGNOSTICS";
  tft.setTextColor(TFT_COLOR_WARNING, TFT_COLOR_BG);
  tft.getTextBounds(pageTitle, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((TFT_WIDTH - w) / 2, y); // Centered
  tft.print(pageTitle);
  y += TFT_LINE_HEIGHT + 5;

  tft.setTextSize(2); // Increased font size for data
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);

  // GPS Model
  tft.setCursor(5, y); tft.print("Model:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(100, y); tft.print(String(GPS_MODEL));
  y += TFT_LINE_HEIGHT;

  // Valid Sentences
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.setCursor(5, y); tft.print("Valid:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(100, y); tft.print(String(validSentences));
  y += TFT_LINE_HEIGHT;

  // Failed Checksums
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.setCursor(5, y); tft.print("Failed:");
  tft.setTextColor(failedChecksums > 0 ? TFT_COLOR_ERROR : TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(100, y); tft.print(String(failedChecksums));
  y += TFT_LINE_HEIGHT;

  // Success Rate
  float successRate = 0;
  if (totalSentences > 0) {
    successRate = ((totalSentences - failedChecksums) * 100.0) / totalSentences;
  }
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.setCursor(5, y); tft.print("Success:");
  tft.setTextColor(successRate > 95 ? TFT_COLOR_VALUE : TFT_COLOR_WARNING, TFT_COLOR_BG);
  tft.setCursor(120, y); tft.print(String(successRate, 1) + "%");
  y += TFT_LINE_HEIGHT;

  // HDOP
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  String hdop = gps.hdop.isValid() ? String(gps.hdop.hdop(), 2) : "--";
  tft.setCursor(5, y); tft.print("HDOP:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(80, y); tft.print(hdop);
  y += TFT_LINE_HEIGHT;

  // Age
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  unsigned long age = gps.location.age();
  String ageStr = age < 1000 ? String(age) + "ms" : String(age / 1000) + "s";
  tft.setCursor(5, y); tft.print("Age:");
  tft.setTextColor(age < GPS_TIMEOUT ? TFT_COLOR_VALUE : TFT_COLOR_ERROR, TFT_COLOR_BG);
  tft.setCursor(80, y); tft.print(ageStr);
  y += TFT_LINE_HEIGHT;

  // Uptime
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  tft.setCursor(5, y); tft.print("Uptime:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  unsigned long uptime = millis() / 1000;
  char uptimeStr[32];
  sprintf(uptimeStr, "%luh%lum%lus", uptime / 3600, (uptime % 3600) / 60, uptime % 60);
  tft.setCursor(100, y);
  tft.print(uptimeStr);
}

// ============================================================================
// DRAW PAGE: SATELLITES
// ============================================================================
void drawPageSatellites() {
  int y = TFT_PAGE_START_Y + 5;
  
  tft.fillRect(0, TFT_PAGE_START_Y, TFT_WIDTH, TFT_HEIGHT - TFT_PAGE_START_Y, TFT_COLOR_BG);
  
  tft.setTextSize(2); // Increased font size for page title

  int16_t x1, y1;
  uint16_t w, h;
  String pageTitle = "SATELLITES";
  tft.setTextColor(TFT_COLOR_WARNING, TFT_COLOR_BG);
  tft.getTextBounds(pageTitle, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor((TFT_WIDTH - w) / 2, y); // Centered
  tft.print(pageTitle);
  y += TFT_LINE_HEIGHT + 5;

  tft.setTextSize(2); // Increased font size for data
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);

  // Satellites
  tft.setCursor(5, y); tft.print("Sats:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  uint32_t satCount = gps.satellites.value();
  tft.setCursor(80, y); tft.print(String(satCount));
  y += TFT_LINE_HEIGHT;

  // HDOP
  tft.setTextColor(TFT_COLOR_TEXT, TFT_COLOR_BG);
  String hdop = gps.hdop.isValid() ? String(gps.hdop.hdop(), 2) : "--";
  tft.setCursor(5, y); tft.print("HDOP:");
  tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG);
  tft.setCursor(80, y); tft.print(hdop);
  y += TFT_LINE_HEIGHT;

  // Signal Quality Bar
  tft.setCursor(5, y);
  tft.print("Signal Quality:");
  y += TFT_LINE_HEIGHT;

  int barWidth = TFT_WIDTH - 20; // 10px padding on each side for the bar
  int barHeight = 20; // Height of the bar
  int barX = 10; // X position of the bar

  tft.drawRect(barX, y, barWidth, barHeight, TFT_COLOR_TEXT);
  uint16_t fillColor = satCount >= 4 ? TFT_COLOR_VALUE : TFT_COLOR_WARNING;
  int fillWidth = (satCount * (barWidth - 2)) / 12; // Max 12 sats for full bar
  if (fillWidth > barWidth - 2) fillWidth = barWidth - 2;
  tft.fillRect(barX + 1, y + 1, fillWidth, barHeight - 2, fillColor);
  y += barHeight + 5; // Move past the bar with some padding

  // Fix Time
  bool hasFix = gps.location.isValid() && gps.location.age() < GPS_TIMEOUT;
  if (hasFix) {
    unsigned long fixDuration = (millis() - gpsFixAcquiredTime) / 1000;
    char fixStr[32];
    sprintf(fixStr, "%lum %lus", fixDuration / 60, fixDuration % 60);
    tft.setCursor(5, y); // Label
    tft.print("Fix Time:");
    tft.setTextColor(TFT_COLOR_VALUE, TFT_COLOR_BG); // Value color
    tft.setCursor(5, y + TFT_LINE_HEIGHT); // Value on next line
    tft.print(fixStr);
  } else {
    tft.setTextColor(TFT_COLOR_ERROR, TFT_COLOR_BG); // Error color
    tft.setCursor(5, y);
    tft.print("Searching for fix...");
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