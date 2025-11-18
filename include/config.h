// Version: 1.2.10
// ESP32-S3 DevKitC-1 N16R8 - GPS GT-U7 Tester Configuration File

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// PROJECT VERSION (Semantic Versioning)
// ============================================================================
#define PROJECT_NAME "GPS GT-U7 Tester"
#define GPS_MODEL "GT-U7"

// ============================================================================
// HARDWARE PIN DEFINITIONS - ESP32-S3 DevKitC-1 N16R8
// ============================================================================

// Light Sensor (ADC)
#define PIN_LIGHT_SENSOR    6     // S3 safe ADC pin

// TFT ST7789 Display (SPI) - Pins are now defined in platformio.ini
// PIN_TFT_CS: 5, PIN_TFT_DC: 19, PIN_TFT_RST: 4
// PIN_TFT_SCL: 11, PIN_TFT_MOSI: 12
#define PIN_TFT_BL          15    // Backlight pin, must match platformio.ini

// NeoPixel RGB LED
#define PIN_NEOPIXEL        48    // Onboard RGB LED on ESP32-S3 DevKitC-1
#define NEOPIXEL_NUM_PIXELS 1
#define NEOPIXEL_BRIGHTNESS 50    // 0-255, a lower value is usually enough

// Buttons (Input with pull-up)
#define PIN_BUTTON_1        1     // Page switch button
#define PIN_BUTTON_2        2     // Reserved for future use

// Buzzer
#define PIN_BUZZER          3
#define BUZZER_LEDC_CHANNEL 0     // LEDC channel for the buzzer (0-7)

// GPS GT-U7 (UART 2)
#define PIN_GPS_RXD         8     // Connects to GPS TX (Moved from 16 to avoid conflict with TOUCH_CS)
#define PIN_GPS_TXD         7     // Connects to GPS RX (Moved from 17)
#define PIN_GPS_PPS         38    // Pulse Per Second
#define GPS_BAUD_RATE       9600  // GT-U7 default baud rate

// I2C Sensors (BME280/OLED)
#define PIN_I2C_SDA         21    // I2C Data
#define PIN_I2C_SCL         20    // I2C Clock

// ============================================================================
// TFT DISPLAY SETTINGS
// ============================================================================
// TFT_WIDTH and TFT_HEIGHT are now defined in platformio.ini build_flags
// to properly configure the TFT_eSPI library during compilation.
#define TFT_ROTATION        0     // 0=Portrait, 1=Landscape, 2=Portrait inverted, 3=Landscape inverted
#define TFT_BACKLIGHT_PWM   255   // 0-255, 255=full brightness

// TFT Colors (RGB565 format)
#define TFT_COLOR_BG        0x0000  // Black
#define TFT_COLOR_HEADER    0x001F  // Blue
#define TFT_COLOR_TEXT      0xFFFF  // White
#define TFT_COLOR_VALUE     0x07E0  // Green
#define TFT_COLOR_WARNING   0xFFE0  // Yellow
#define TFT_COLOR_ERROR     0xF800  // Red
#define TFT_COLOR_SEPARATOR 0x4208  // Dark Gray

// Display pages
#define NUM_PAGES           3
#define PAGE_GPS_DATA       0
#define PAGE_DIAGNOSTICS    1
#define PAGE_SATELLITES     2

// ============================================================================
// GPS SETTINGS
// ============================================================================
#define GPS_UPDATE_RATE     1000  // Update display every 1000ms (1 Hz)
#define GPS_TIMEOUT         5000  // GPS data timeout in ms
#define GPS_FIX_TIMEOUT     60000 // Time to wait for fix before warning (60s)

// ============================================================================
// WIFI SETTINGS
// ============================================================================
#define WIFI_CONNECT_TIMEOUT 10000 // WiFi connection timeout in ms
#define WIFI_RETRY_DELAY     1000  // Delay between connection attempts

// ============================================================================
// WEB SERVER SETTINGS
// ============================================================================
#define WEB_SERVER_PORT     80
#define WEB_UPDATE_INTERVAL 1000   // WebSocket update interval in ms

// ============================================================================
// BUZZER SETTINGS
// ============================================================================
#define BUZZER_FREQ_FIX     2000   // Frequency for GPS fix acquired (Hz)
#define BUZZER_FREQ_LOST    1000   // Frequency for GPS fix lost (Hz)
#define BUZZER_DURATION     200    // Buzzer duration in ms
#define BUZZER_ENABLED      true   // Set to false to disable buzzer

// ============================================================================
// LED SETTINGS
// ============================================================================
// NeoPixel status colors (used in main.cpp)
#define NEOPIXEL_COLOR_OFF      0
#define NEOPIXEL_COLOR_BLUE     Adafruit_NeoPixel::Color(0, 0, 255)
#define NEOPIXEL_COLOR_GREEN    Adafruit_NeoPixel::Color(0, 255, 0)
#define NEOPIXEL_COLOR_RED      Adafruit_NeoPixel::Color(255, 0, 0)
#define NEOPIXEL_BLINK_INTERVAL 500 // ms for blinking

// ============================================================================
// MEMORY ALLOCATION SETTINGS
// ============================================================================
// TFT sprite buffer size - Careful with PSRAM allocation
// ESP32-S3 N16R8 has 8MB PSRAM, but be conservative
// Full screen sprite would be 240*320*2 = 153,600 bytes
// We'll use partial screen sprites to avoid bootloop
#define USE_TFT_SPRITE      false  // Set to true to use sprites (needs testing)
#define SPRITE_HEIGHT       80     // Height of sprite buffer (if used)

// JSON buffer size for web data
#define JSON_BUFFER_SIZE    2048

// ============================================================================
// BUTTON DEBOUNCE
// ============================================================================
#define BUTTON_DEBOUNCE_MS  50     // Debounce delay in milliseconds

// ============================================================================
// DEBUG SETTINGS
// ============================================================================
#define SERIAL_DEBUG_BAUD   115200
#define DEBUG_ENABLED       true   // Set to false to disable debug output

#if DEBUG_ENABLED
  #define DEBUG_PRINT(x)    Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
#endif

// ============================================================================
// SAFE BOOT PINS - DO NOT USE
// ============================================================================
// GPIO 0: Boot button (strapping pin)
// GPIO 35, 36, 37: Reserved for OPI PSRAM
// GPIO 45, 46: Strapping pins
// These pins are intentionally avoided in the design

#endif // CONFIG_H