# GPS Tester for ESP32-S3

[![Version](https://img.shields.io/badge/version-1.7.3-blue)](CHANGELOG.md)
[![Platform](https://img.shields.io/badge/platform-ESP32--S3-green)](https://docs.platformio.org/en/latest/boards/espressif32/esp32-s3-devkitc-1.html)
[![License](https://img.shields.io/badge/license-MIT-orange)](LICENSE)

This project turns an ESP32-S3 board (specifically the N16R8 variant) into a comprehensive GPS testing tool. It can read data from a GPS module, display it on a local ST7789 TFT screen, and serve a detailed web interface for real-time monitoring.

[🇫🇷 Version Française](README_FR.md)

## ✨ Features

- **Real-time Data**: View location, altitude, speed, and more on a TFT display and a web dashboard.
- **Modern Web Interface**: A responsive web UI accessible via WiFi, with live updates using WebSockets.
- **TFT Display Support**: A multi-page interface on a 240x240 ST7789 display. The program also runs headless if the screen is not connected.
- **WiFi Multi-Network Support**: Automatic failover between multiple WiFi networks.
- **GPS Module Flexibility**: Easily configurable for different GPS modules like the GT-U7 or NEO-6M.
- **Status Indicators**: Uses the onboard NeoPixel RGB LED and an optional buzzer for clear status feedback (GPS fix, WiFi connection).
- **Robustness**: The program runs smoothly even if the display is not connected.
- **User-friendly Configuration**: All major settings are centralized in `include/config.h`.

## 🚀 Quick Start

1.  **Install**: Follow the **Installation Guide** to set up your hardware and software.
2.  **Configure**: Edit `include/secrets.h` and `include/config.h` as described in the **Configuration Guide**.
3.  **Use**: Power on the device and follow the **Usage Guide** to start monitoring GPS data.

## 📚 Documentation

- **INSTALL.md**: Hardware requirements and setup instructions.
- **CONFIG.md**: Detailed guide on all configuration options.
- **USAGE.md**: How to use the device, web interface, and display.
- **ARCHITECTURE.md**: Overview of the project structure and code flow.
- **TROUBLESHOOTING.md**: Solutions to common problems.
- **CONTRIBUTING.md**: Guidelines for contributing to the project.

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments

- **TinyGPSPlus** by Mikal Hart - GPS parsing library
- **Adafruit GFX & ST7789 Libraries** - For display control
- **ESPAsyncWebServer** - Async web server for ESP32
- **PlatformIO** - Development platform