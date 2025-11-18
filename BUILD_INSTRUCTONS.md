# Build Instructions - GPS GT-U7 Tester

## Prerequisites

Before building this project, ensure you have the following installed:

1. **Visual Studio Code** - https://code.visualstudio.com/
2. **PlatformIO Extension** for VS Code
3. **Git** (for version control)

## Installation Steps

### 1. Install PlatformIO

#### Option A: Via VS Code (Recommended)
1. Open Visual Studio Code
2. Go to Extensions (Ctrl+Shift+X or Cmd+Shift+X)
3. Search for "PlatformIO IDE"
4. Click Install
5. Restart VS Code

#### Option B: Via Command Line
```bash
pip install platformio
```

### 2. Clone and Open Project

```bash
git clone <repository-url>
cd Test_GPS_GTU7
code .
```

### 3. Configure WiFi Credentials

**IMPORTANT**: The `include/secrets.h` file contains your WiFi credentials and is already configured. However, if you need to modify them:

```cpp
// include/secrets.h
#define WIFI_SSID_1         "your-ssid"
#define WIFI_PASSWORD_1     "your-password"
```

This file is excluded from Git for security.

## Building the Project

### Using VS Code with PlatformIO

1. Open the project folder in VS Code
2. PlatformIO will automatically detect the project
3. Click on the PlatformIO icon in the sidebar
4. Under "Project Tasks", select:
   - **Build** - To compile the project
   - **Upload** - To upload to the ESP32-S3
   - **Monitor** - To view serial output
   - **Upload and Monitor** - To upload and monitor in one step

### Using Command Line

```bash
# Install dependencies and build
pio run

# Build only
pio run --target build

# Upload to device
pio run --target upload

# Monitor serial output
pio device monitor -b 115200

# Clean build files
pio run --target clean
```

## First Time Build

The first time you build the project, PlatformIO will:
1. Download the ESP32 platform (~500MB)
2. Download all required libraries
3. Compile the code

This can take 5-10 minutes depending on your internet connection.

## Verification Checklist

Before building, verify:

- [x] `platformio.ini` exists and is properly configured
- [x] `include/config.h` contains correct pin definitions
- [x] `include/secrets.h` contains your WiFi credentials
- [x] `src/main.cpp` exists
- [x] ESP32-S3 board is connected via USB
- [x] Correct USB port is selected in PlatformIO

## Expected Build Output

A successful build should show:

```
Processing esp32-s3-devkitc-1 (platform: espressif32; board: esp32-s3-devkitc-1; framework: arduino)
...
Building .pio/build/esp32-s3-devkitc-1/firmware.bin
Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
RAM:   [==        ]  XX.X% (used XXXXX bytes from 524288 bytes)
Flash: [====      ]  XX.X% (used XXXXXX bytes from 16777216 bytes)
========================= [SUCCESS] Took XX.XX seconds =========================
```

## Upload to ESP32-S3

### Connection
1. Connect ESP32-S3 to computer via USB-C cable
2. Ensure the USB cable supports data transfer (not just charging)
3. The board should appear as a serial port

### Upload Process
1. Hold BOOT button on ESP32-S3
2. Click Upload in PlatformIO
3. Release BOOT button when "Connecting..." appears
4. Wait for upload to complete

### Modern ESP32-S3 Boards
Most ESP32-S3 DevKitC-1 boards auto-enter bootloader mode, so you may not need to hold BOOT.

## Troubleshooting Build Issues

### "Port not found" or "Serial port not detected"
- **Windows**: Install CH340/CP2102 USB drivers
- **Linux**: Add user to dialout group: `sudo usermod -a -G dialout $USER`
- **Mac**: Install CH340 drivers if needed

### "Failed to connect to ESP32"
1. Hold BOOT button during upload
2. Try a different USB cable
3. Try a different USB port
4. Check Device Manager (Windows) or `ls /dev/tty*` (Linux/Mac)

### "Out of memory" during compilation
1. Close other applications
2. Increase available RAM
3. Reduce optimization level in platformio.ini

### Library Download Failures
```bash
# Clear PlatformIO cache
pio system prune -f

# Reinstall libraries
pio pkg install
```

### Build Errors Related to TFT_eSPI
The TFT_eSPI configuration is handled via build flags in `platformio.ini`. Do not modify the TFT_eSPI User_Setup.h file.

## Monitoring Serial Output

```bash
# Using PlatformIO
pio device monitor -b 115200

# With filters
pio device monitor -b 115200 --filter esp32_exception_decoder
```

Expected serial output on startup:
```
Starting GPS GT-U7 Tester...
Initializing TFT display...
TFT display initialized
Initializing GPS...
GPS Serial initialized on RX:16 TX:17 at 9600 baud
Connecting to WiFi...
WiFi connected!
IP address: 192.168.X.X
Connected to: YourSSID
Web server started
Access at: http://192.168.X.X
Setup complete. Starting main loop...
```

## Post-Build Testing

After successful upload:

1. **Check Serial Monitor**: Verify startup messages
2. **Check TFT Display**: Should show project name and status
3. **Check LED**: Should be green if WiFi connected
4. **Access Web Interface**: Navigate to IP shown on display
5. **Test GPS**: Wait for satellites (can take 30-60s outdoors)

## Common First-Time Issues

### Bootloop After Upload
- **Cause**: Memory configuration issue
- **Solution**: Verify PSRAM settings in platformio.ini
- **Quick Fix**: Set `USE_TFT_SPRITE = false` in config.h (already default)

### GPS Not Working
- **Cause**: TX/RX pins swapped
- **Solution**: GPS TX connects to ESP32 RX (GPIO 16)
              GPS RX connects to ESP32 TX (GPIO 17)

### TFT Display Blank
- **Cause**: Backlight not enabled or SPI pins incorrect
- **Solution**: Verify all pins in config.h match your wiring
- **Check**: GPIO 15 (backlight) should be HIGH

### WiFi Not Connecting
- **Cause**: Incorrect credentials or 5GHz network
- **Solution**: Verify credentials in secrets.h
- **Note**: ESP32 only supports 2.4GHz WiFi

## Build Optimization

### Release Build
For production deployment, modify `platformio.ini`:

```ini
build_type = release
```

And in `config.h`:
```cpp
#define DEBUG_ENABLED  false
```

### Memory Optimization
Monitor memory usage:
```bash
pio run --target size
```

Current expected usage:
- RAM: ~30-40% (with all features enabled)
- Flash: ~40-50% (with libraries)

## Platform Compatibility

This project has been configured to be compatible with:
- **AMD-based systems**
- **Intel-based systems**

All libraries are cross-platform compatible.

## Version Information

- **Project Version**: 1.0.00-dev
- **Platform**: ESP32-S3
- **Framework**: Arduino
- **Build System**: PlatformIO

For more information, see:
- [README.md](README.md) - Full documentation
- [README_FR.md](README_FR.md) - Documentation en français
- [CHANGELOG.md](CHANGELOG.md) - Version history

## Support

If you encounter issues:
1. Check this document first
2. Review README.md troubleshooting section
3. Check CHANGELOG.md for known issues
4. Verify all hardware connections
5. Test with minimal configuration

## Next Steps

After successful build and upload:
1. Test all three TFT display pages
2. Verify web interface functionality
3. Test GPS fix acquisition outdoors
4. Verify button functionality
5. Test GPS reset feature
6. Monitor for any stability issues

---

**Last Updated**: 2025-11-18
**Version**: 1.0.00-dev