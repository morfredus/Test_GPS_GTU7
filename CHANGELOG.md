# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.0] - YYYY-MM-DD

### Added
- Support for the onboard NeoPixel RGB LED (GPIO 48) for status indication.
- Added `Adafruit NeoPixel` library dependency.

### Changed
- **Version incremented to 1.2.0** due to new hardware feature support.
- Replaced previous standard RGB LED pin definitions with NeoPixel configuration in `config.h`.

## [1.1.1] - YYYY-MM-DD

### Fixed
- Corrected a major display issue where the TFT screen would not initialize.
- Resolved conflicting SPI pin definitions between `platformio.ini` and `config.h`.
- Centralized TFT pin configuration into `platformio.ini` `build_flags` for clarity and reliability.

## [1.1.0] - YYYY-MM-DD

### Initial Release
- Base project structure.