# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.0.0-dev] - 2025-11-18

### Added
- Initial project setup with PlatformIO for ESP32-S3 DevKitC-1 N16R8
- GPS GT-U7 module integration via UART2 using TinyGPSPlus library
- ST7789 TFT display support (240x320 pixels) with SPI interface
- Multi-page TFT display system with three pages:
  - Page 1: GPS Data (position, speed, altitude, course, satellites, UTC time)
  - Page 2: Diagnostics (statistics, HDOP, success rate, uptime)
  - Page 3: Satellites (satellite count, signal quality, fix time)
- Modern web interface with real-time updates via WebSocket
- Professional gradient-based web design with responsive layout
- WiFi multi-network support with automatic failover using WiFiMulti
- RGB LED status indicators (fix status, WiFi status, searching)
- Buzzer audio feedback for GPS fix acquisition and loss
- Button control for cycling through TFT display pages
- Hot GPS reset functionality via web interface
- Comprehensive GPS diagnostics and statistics tracking
- Configuration management system with centralized config.h
- Secrets management for WiFi credentials (excluded from Git)
- Memory-optimized design to prevent ESP32-S3 bootloop
- Debug output system with enable/disable flag
- JSON-based WebSocket data transmission
- Automatic LED blinking patterns for different states
- GPS timeout detection and handling
- Data age tracking for GPS information
- Success rate calculation for GPS sentences
- HDOP (Horizontal Dilution of Precision) monitoring
- Satellite count visualization
- UTC date and time display from GPS
- Speed and course tracking
- WebSocket client management with auto-reconnection
- Asynchronous web server using ESPAsyncWebServer
- Button debouncing for reliable input
- Light sensor pin definition (for future expansion)
- I2C sensor support (BME280/OLED ready)
- GPS PPS (Pulse Per Second) pin support
- Comprehensive documentation in English and French
- Project README with detailed setup and troubleshooting guides
- CHANGELOG following semantic versioning

### Configuration
- ESP32-S3 with 16MB Flash and 8MB OPI PSRAM support
- TFT_eSPI library configured for ST7789 driver
- Build flags optimized for ESP32-S3 to avoid bootloop
- Partition scheme: default_16MB.csv
- Serial debug output at 115200 baud
- GPS serial at 9600 baud (GT-U7 default)
- WebSocket update interval: 1000ms
- GPS update rate: 1000ms (1 Hz)
- WiFi connection timeout: 10 seconds
- GPS fix timeout: 60 seconds
- TFT SPI frequency: 40MHz

### Pin Assignments
- GPS UART2: RX=16, TX=17, PPS=38
- TFT SPI: CS=5, DC=19, RST=4, BL=15, SCL=18, MOSI=12
- RGB LED: Red=14, Green=13, Blue=10
- Buttons: Button1=1, Button2=2
- Buzzer: GPIO 3
- Light Sensor: GPIO 6 (ADC)
- I2C: SDA=21, SCL=20

### Libraries
- TFT_eSPI v2.5.43 - TFT display driver
- TinyGPSPlus v1.1.0 - GPS parsing
- ESPAsyncWebServer v1.2.4 - Async web server
- AsyncTCP v1.1.4 - Async TCP library
- ArduinoJson v7.2.1 - JSON serialization

### Notes
- First development release
- Tested on ESP32-S3 DevKitC-1 N16R8
- GPS module: GT-U7 with external antenna
- TFT sprite buffers disabled by default to save memory
- secrets.h file must be created manually with WiFi credentials
- Build type set to debug for development

### Known Issues
- None reported in initial release

### Security
- WiFi credentials stored in separate secrets.h file
- secrets.h excluded from version control via .gitignore

---

## Version History

### Versioning Scheme
This project uses [Semantic Versioning](https://semver.org/):
- **MAJOR** version: Incompatible API changes
- **MINOR** version: Add functionality in a backward compatible manner
- **PATCH** version: Backward compatible bug fixes
- **-dev** suffix: Development/pre-release version

### Version Format
- Development: X.Y.ZZ-dev
- Release: X.Y.ZZ

Example: 1.0.00-dev → 1.0.00 (release) → 1.0.01 (patch) → 1.1.00 (minor update) → 2.0.00 (major update)

[Unreleased]: https://github.com/username/Test_GPS_GTU7/compare/v1.0.0-dev...HEAD
[1.0.0-dev]: https://github.com/username/Test_GPS_GTU7/releases/tag/v1.0.0-dev