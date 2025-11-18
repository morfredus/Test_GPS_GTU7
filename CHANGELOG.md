# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.9] - 2025-11-19

### Fixed
- **Critical:** Resolved boot failure on ESP32-S3 caused by unsupported `USE_HSPI_PORT` flag.
- Disabled `USE_HSPI_PORT` in `platformio.ini` as ESP32-S3 uses SPI2 (FSPI) by default, not HSPI.
- ESP32-S3 does not have HSPI peripheral (only available on original ESP32).

## [1.2.8] - 2024-05-24

### Fixed
- **Critical:** Resolved a persistent SPI bus conflict by disabling the touch screen driver (`XPT2046_TOUCH`). The current display does not have a touch layer, and enabling it caused an SPI initialization failure, preventing the screen from working.

## [1.2.7] - 2024-05-24

### Fixed
- **Critical:** Corrected TFT driver configuration to match the hardware (1.54" ST7789).
- Changed driver from `ST7735_DRIVER` back to `ST7789_DRIVER`.
- Corrected screen dimensions from 240x320 to **240x240**. This mismatch was a likely cause of the boot crash.

## [1.2.6] - 2024-05-24

### Fixed
- **Critical:** Resolved a system freeze at boot caused by an SPI pin conflict.
- Moved `TFT_SCLK` from GPIO 18 to GPIO 11. GPIO 18 is reserved for Octal PSRAM communication on the ESP32-S3 N16R8 board and cannot be used for other peripherals.

## [1.2.5] - 2024-05-24

### Fixed
- Resolved compiler warnings about `TFT_WIDTH` and `TFT_HEIGHT` being redefined.
- Moved TFT dimension definitions from `config.h` to `platformio.ini` `build_flags` to ensure the TFT_eSPI library is compiled with the correct dimensions.

## [1.2.4] - 2024-05-24

### Fixed
- Resolved a program freeze caused by an uninitialized `LEDC` peripheral.
- Explicitly initialized a LEDC channel for the buzzer in `setupPins()` and switched `playTone()` to use `ledcWriteTone()` instead of the generic `tone()` function for better stability on ESP32.

## [1.2.3] - 2024-05-24

### Added
- Additional debug messages to the serial monitor to better trace WiFi connection status and web server requests.
- A debug message now prints the IP address of any client requesting the main web page.

## [1.2.2] - 2024-05-24

### Fixed
- Changed TFT driver from `ST7789` to `ST7735_BLACKTAB` to troubleshoot a partial display issue (header visible, content area blank). This is a common fix for displays with compatible but different controllers.

## [1.2.1] - 2024-05-24

### Changed
- Implemented NeoPixel control logic in `main.cpp`, replacing the old RGB LED functions.
- LED status now follows the logic defined in the user manual (Blue for init, Blinking Green for searching, Solid Green for fix, Red for error).
- Created a non-blocking LED update system (`setLedStatus`, `updateLed`).

## [1.2.0] - 2024-05-24

### Added
- Support for the onboard NeoPixel RGB LED (GPIO 48) for status indication.
- Added `Adafruit NeoPixel` library dependency.

### Changed
- **Version incremented to 1.2.0** due to new hardware feature support.
- Replaced previous standard RGB LED pin definitions with NeoPixel configuration in `config.h`.

## [1.1.1] - 2024-05-24

### Fixed
- Corrected a major display issue where the TFT screen would not initialize.
- Resolved conflicting SPI pin definitions between `platformio.ini` and `config.h`.
- Centralized TFT pin configuration into `platformio.ini` `build_flags` for clarity and reliability.

## [1.1.0] - 2024-05-24

### Initial Release
- Base project structure.