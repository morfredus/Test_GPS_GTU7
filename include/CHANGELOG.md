# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.5.0] - 2024-07-25
### Added
- Creation of a detailed and modular documentation structure (`INSTALL`, `USAGE`, `CONFIG`, etc.).
- Added `CHANGELOG.md` to track project history.

### Fixed
- **Major Bug**: Fixed a blocking issue in the `loop()` function during WiFi connection attempts, which made the application unresponsive.
- **Compilation Error**: Corrected a scope issue (`'tft' was not declared`) for the screen macro, ensuring stable compilation.

### Improved
- **Stability**: Reverted to a stable board configuration in `platformio.ini` to prevent bootloops related to incorrect memory settings.
- **Responsiveness**: The WiFi connection process is now non-blocking, ensuring the UI (button, screen) remains responsive at all times.
- **Code Cleanup**: Removed unused dependencies and streamlined the main loop logic.