# GPS GT-U7 Tester

![Version](https://img.shields.io/badge/version-1.0.00--dev-blue)
![Platform](https://img.shields.io/badge/platform-ESP32--S3-green)
![License](https://img.shields.io/badge/license-MIT-orange)

A comprehensive GPS module tester for the GT-U7 GPS module, built on the ESP32-S3 DevKitC-1 N16R8 platform using PlatformIO.

[🇫🇷 Version Française](README_FR.md)

## 📋 Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Pin Configuration](#pin-configuration)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [Web Interface](#web-interface)
- [TFT Display Pages](#tft-display-pages)
- [Troubleshooting](#troubleshooting)
- [Development](#development)
- [License](#license)

## ✨ Features

- **Real-time GPS Data Display**: View location, altitude, speed, course, and more
- **Modern Web Interface**: Professional web dashboard with WebSocket updates
- **TFT Display**: Multi-page display showing GPS data, diagnostics, and satellite info
- **WiFi Multi-Network Support**: Automatic failover between multiple WiFi networks
- **Diagnostic Tools**: Comprehensive GPS module diagnostics and statistics
- **Audio Feedback**: Buzzer notifications for GPS fix acquisition and loss
- **Visual Indicators**: RGB LED status indicators
- **Hot GPS Reset**: Reset GPS module via web interface or command
- **Satellite Tracking**: Display satellite count and signal quality
- **Low Memory Footprint**: Optimized for ESP32-S3 to prevent bootloop

## 🔧 Hardware Requirements

### Main Components

- **ESP32-S3 DevKitC-1 N16R8** (16MB Flash, 8MB OPI PSRAM)
- **GT-U7 GPS Module** with antenna
- **ST7789 TFT Display** (240x320 pixels, SPI interface)
- **RGB LED** (Common Cathode)
- **Buzzer** (Active or Passive)
- **2x Push Buttons**
- **Light Sensor** (Optional, ADC compatible)

### Optional Components

- **BME280 Sensor** (Temperature, Humidity, Pressure via I2C)
- **OLED Display** (Additional I2C display)

## 📌 Pin Configuration

### GPS GT-U7 (UART2)
- **GPS RX** → GPIO 16 (ESP32 TX)
- **GPS TX** → GPIO 17 (ESP32 RX)
- **GPS PPS** → GPIO 38 (Pulse Per Second)

### TFT ST7789 Display (SPI)
- **CS** → GPIO 5
- **DC** → GPIO 19
- **RST** → GPIO 4
- **BL** → GPIO 15 (Backlight)
- **SCL** → GPIO 18 (SPI Clock)
- **MOSI** → GPIO 12 (SPI Data)

### RGB LED (Common Cathode)
- **Red** → GPIO 14
- **Green** → GPIO 13
- **Blue** → GPIO 10

### Buttons
- **Button 1** → GPIO 1 (Page Switch)
- **Button 2** → GPIO 2 (Reserved)

### Other Peripherals
- **Buzzer** → GPIO 3
- **Light Sensor** → GPIO 6 (ADC)
- **I2C SDA** → GPIO 21
- **I2C SCL** → GPIO 20

## 💻 Software Requirements

- **PlatformIO IDE** or **PlatformIO Core**
- **Visual Studio Code** (recommended)
- **Python 3.x** (for PlatformIO)

## 📥 Installation

### 1. Clone the Repository

```bash
git clone <repository-url>
cd Test_GPS_GTU7
```

### 2. Install PlatformIO

If you haven't installed PlatformIO:

**Using VS Code:**
- Install the PlatformIO IDE extension from VS Code marketplace

**Using CLI:**
```bash
pip install platformio
```

### 3. Configure Secrets

Create the `include/secrets.h` file with your WiFi credentials:

```cpp
#ifndef SECRETS_H
#define SECRETS_H

#define WIFI_SSID_1         "your-ssid-1"
#define WIFI_PASSWORD_1     "your-password-1"

#define WIFI_SSID_2         "your-ssid-2"
#define WIFI_PASSWORD_2     "your-password-2"

#endif
```

### 4. Build and Upload

```bash
# Build the project
pio run

# Upload to ESP32-S3
pio run --target upload

# Monitor serial output
pio device monitor
```

## ⚙️ Configuration

All configuration is centralized in `include/config.h`:

- **Pin definitions**: All GPIO assignments
- **Display settings**: TFT resolution, colors, rotation
- **GPS settings**: Baud rate, update intervals, timeouts
- **WiFi settings**: Connection timeouts, retry delays
- **Buzzer settings**: Frequencies and durations
- **LED settings**: Blink patterns
- **Memory settings**: Buffer sizes, sprite configuration
- **Debug settings**: Serial baud rate, debug flags

## 🚀 Usage

### Initial Power-Up

1. Connect the ESP32-S3 to power
2. The device will attempt to connect to configured WiFi networks
3. Green LED indicates successful WiFi connection
4. GPS module starts searching for satellites
5. Blue LED blinks while searching for GPS fix
6. Buzzer beeps when GPS fix is acquired

### Button Controls

- **Button 1 (GPIO 1)**: Cycle through display pages
  - Page 1: GPS Data (location, speed, altitude)
  - Page 2: Diagnostics (statistics, HDOP, success rate)
  - Page 3: Satellites (satellite count, signal quality)

### LED Status Indicators

- **Red LED**: GPS fix lost / WiFi connection failed
- **Green LED**: GPS fix acquired / WiFi connected
- **Yellow (Red+Green)**: Searching for fix
- **Blinking Red**: No GPS fix (slow blink)

### Buzzer Feedback

- **2000 Hz beep**: GPS fix acquired
- **1000 Hz beep**: GPS fix lost
- Can be disabled in `config.h` by setting `BUZZER_ENABLED` to `false`

## 🌐 Web Interface

### Accessing the Interface

Once connected to WiFi, the IP address is displayed on the TFT screen.

Access the web interface at: `http://<IP-ADDRESS>/`

### Web Dashboard Features

- **Real-time Updates**: Live GPS data via WebSocket
- **GPS Status**: Fix status, satellite count, HDOP
- **Position Data**: Latitude, longitude, altitude, speed, course
- **Date & Time**: UTC date and time from GPS
- **Diagnostics**: Valid sentences, failed checksums, success rate
- **GPS Reset**: Hot reset button for GPS module
- **Responsive Design**: Works on mobile, tablet, and desktop
- **Modern UI**: Professional gradient design with animated updates

### WebSocket API

The device sends JSON data every second to all connected WebSocket clients:

```json
{
  "fix": true,
  "satellites": 8,
  "hdop": "1.2",
  "uptime": "0h 5m 32s",
  "latitude": "48.856614",
  "longitude": "2.352222",
  "altitude": "35.0 m",
  "speed": "0.0 km/h",
  "course": "0.0°",
  "date": "18/11/2025",
  "time": "14:23:45",
  "age": "120 ms",
  "validSentences": 1234,
  "failedChecksums": 2,
  "totalChars": 45678,
  "successRate": "99.8%"
}
```

## 📱 TFT Display Pages

### Page 1: GPS Data
- Latitude and Longitude (6 decimal places)
- Altitude (meters)
- Speed (km/h)
- Course (degrees)
- Satellite count
- UTC Date and Time

### Page 2: Diagnostics
- Valid NMEA sentences received
- Failed checksum count
- Characters processed
- Success rate percentage
- HDOP (Horizontal Dilution of Precision)
- Data age (time since last update)
- System uptime

### Page 3: Satellites
- Satellite count
- Signal quality bar graph
- HDOP value
- Time with fix / Searching status

## 🔍 Troubleshooting

### GPS Not Getting Fix

1. **Ensure clear sky view**: GPS requires line-of-sight to satellites
2. **Check antenna connection**: Verify GPS antenna is properly connected
3. **Wait longer**: Initial fix can take 30-60 seconds (cold start)
4. **Check wiring**: Verify TX/RX are correctly crossed
5. **Verify baud rate**: GT-U7 default is 9600 baud

### TFT Display Not Working

1. **Check SPI connections**: Verify all SPI pins are correctly wired
2. **Backlight**: Ensure GPIO 15 is HIGH (backlight on)
3. **Power supply**: Check 3.3V or 5V power (depending on display)
4. **TFT_eSPI configuration**: Verify platformio.ini build flags

### WiFi Connection Failed

1. **Check credentials**: Verify WiFi SSID and password in `secrets.h`
2. **Signal strength**: Ensure ESP32 is within WiFi range
3. **2.4GHz network**: ESP32 only supports 2.4GHz, not 5GHz
4. **Router settings**: Check if MAC filtering is enabled

### Bootloop Issues

1. **PSRAM configuration**: Verify OPI PSRAM settings in platformio.ini
2. **Pin conflicts**: Check for pin conflicts with strapping pins
3. **Power supply**: Ensure adequate power (500mA+)
4. **Reduce sprite usage**: Set `USE_TFT_SPRITE` to `false` in config.h
5. **Check serial monitor**: Look for crash messages during boot

### Web Interface Not Loading

1. **Check IP address**: Verify correct IP from TFT display or serial monitor
2. **Firewall**: Check if firewall is blocking port 80
3. **WiFi connection**: Ensure device is connected to same network
4. **Browser cache**: Clear browser cache or try incognito mode

## 🛠️ Development

### Project Structure

```
Test_GPS_GTU7/
├── include/
│   ├── config.h         # Hardware and software configuration
│   └── secrets.h        # WiFi credentials (not in Git)
├── src/
│   └── main.cpp         # Main application code
├── lib/                 # Custom libraries (if any)
├── test/                # Unit tests (if any)
├── platformio.ini       # PlatformIO configuration
├── .gitignore          # Git ignore rules
├── README.md           # This file (English)
├── README_FR.md        # French documentation
└── CHANGELOG.md        # Version history
```

### Adding New Features

1. Update version in `include/config.h`
2. Implement feature in `src/main.cpp`
3. Update documentation
4. Update CHANGELOG.md
5. Test thoroughly on hardware
6. Commit with semantic versioning

### Debugging

Enable debug output in `include/config.h`:

```cpp
#define DEBUG_ENABLED       true
#define SERIAL_DEBUG_BAUD   115200
```

View debug output:
```bash
pio device monitor -b 115200
```

### Memory Optimization

- The ESP32-S3 N16R8 has 8MB PSRAM
- TFT sprite buffers are disabled by default to prevent bootloop
- Full screen buffer would be 153,600 bytes (240×320×2)
- Current configuration uses direct drawing to minimize memory usage

### Building for Production

1. Set `DEBUG_ENABLED` to `false` in `config.h`
2. Change `build_type` to `release` in `platformio.ini`
3. Remove version `-dev` suffix
4. Build and test:
```bash
pio run --environment esp32-s3-devkitc-1
```

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🤝 Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Open a Pull Request

## 📞 Support

For issues, questions, or suggestions:

- Open an issue on GitHub
- Check existing documentation
- Review CHANGELOG for recent changes

## 🙏 Acknowledgments

- **TinyGPSPlus** by Mikal Hart - GPS parsing library
- **TFT_eSPI** by Bodmer - TFT display library
- **ESPAsyncWebServer** - Async web server for ESP32
- **PlatformIO** - Development platform

---

**Version:** 1.0.00-dev
**Platform:** ESP32-S3 DevKitC-1 N16R8
**GPS Module:** GT-U7
**Last Updated:** 2025-11-18