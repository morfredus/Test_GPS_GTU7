# User Manual - GPS GT-U7 Tester

**Document Version:** For software v1.2.9

## 1. Introduction

This document describes the operation of the `Test_GPS_GTU7` project on an ESP32-S3 DevKitC-1 N16R8 development board.

## 2. Features

- Displays GPS data (coordinates, speed, altitude, etc.) on a TFT screen.
- Status indication via an onboard NeoPixel RGB LED.
- Web interface to view data remotely.
- Buttons to navigate between different information screens.

## 3. Visual Indicators (NeoPixel LED)
- **Solid Blue:** Initializing.
- **Blinking Green:** Searching for a GPS fix.
- **Solid Green:** GPS fix acquired.
- **Red:** Error (e.g., GPS not detected, timeout).