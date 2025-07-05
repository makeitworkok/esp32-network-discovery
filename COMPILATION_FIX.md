# Compilation Fix for A2DP Error

## Problem
You're getting an A2DP (Advanced Audio Distribution Profile) error when compiling. This happens when the wrong ESP32 board is selected in Arduino IDE.

## Solution

### 1. Correct Board Selection for WT32-ETH01

In Arduino IDE:
1. Go to **Tools → Board → ESP32 Arduino**
2. Select **"ESP32S3 Dev Module"** (NOT "ESP32 Dev Module")
3. Configure the following settings:

**Board Settings:**
- **Board**: ESP32S3 Dev Module
- **Upload Speed**: 921600
- **USB Mode**: Hardware CDC and JTAG
- **USB CDC On Boot**: Enabled
- **USB Firmware MSC On Boot**: Disabled
- **USB DFU On Boot**: Disabled
- **Upload Mode**: UART0 / Hardware CDC
- **CPU Frequency**: 240MHz (WiFi)
- **Flash Mode**: QIO 80MHz
- **Flash Size**: 4MB (32Mb)
- **Partition Scheme**: Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)
- **Core Debug Level**: None
- **PSRAM**: Disabled
- **Arduino Runs On**: Core 1
- **Events Run On**: Core 1

### 2. Alternative: Use ESP32 Dev Module (Classic)

If you want to use the classic ESP32 settings:
1. Select **"ESP32 Dev Module"**
2. Make sure **Bluetooth** is **Disabled** in the board configuration
3. Configure these settings:

**Board Settings:**
- **Board**: ESP32 Dev Module
- **Upload Speed**: 921600
- **CPU Frequency**: 240MHz (WiFi/BT)
- **Flash Frequency**: 80MHz
- **Flash Mode**: QIO
- **Flash Size**: 4MB (32Mb)
- **Partition Scheme**: Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)
- **Core Debug Level**: None
- **PSRAM**: Disabled

### 3. Verify Libraries

Make sure you have the correct libraries installed:
1. Go to **Tools → Manage Libraries**
2. Search and install:
   - **ArduinoJson** (latest version)
   - **ESP32** board package (version 2.0.11 or later)

### 4. Clean Compilation

If errors persist:
1. Close Arduino IDE
2. Delete the build cache:
   - Windows: `%LOCALAPPDATA%\Arduino15\packages\esp32\hardware\esp32\*\tools\`
   - Mac: `~/Library/Arduino15/packages/esp32/hardware/esp32/*/tools/`
   - Linux: `~/.arduino15/packages/esp32/hardware/esp32/*/tools/`
3. Reopen Arduino IDE
4. Recompile the project

### 5. Code Modifications (Already Applied)

The following fixes have been applied to resolve compilation errors:

**A2DP Error Fix:**
```cpp
// Disable Bluetooth to avoid A2DP conflicts
#define CONFIG_BT_ENABLED 0
```

**Type Conversion and Include Fixes:**
- **Line 129 network_scanner.cpp**: Fixed `udp.write("ping")` to `udp.write((const uint8_t*)"ping", 4)`
- **ArduinoJson compatibility**: Updated JSON parsing in `web_interface.cpp` and `wifi_manager.cpp`
- **Missing includes**: Added `#include <WiFiUdp.h>` to `network_scanner.h`
- **Missing includes**: Added `#include <WiFiClient.h>` to `port_scanner.h`
- Fixed all const char to uint8_t conversion errors

### 6. Troubleshooting Steps

1. **Check ESP32 Board Package Version**:
   - Go to Tools → Board → Board Manager
   - Search "esp32"
   - Make sure version 2.0.11 or later is installed

2. **Verify Board Selection**:
   - The board name should show "ESP32S3 Dev Module" in the status bar
   - Port should be detected automatically

3. **Check Compilation Output**:
   - Look for specific error messages
   - Common issues: missing libraries, wrong board selection

### 7. Working Configuration Summary

For WT32-ETH01, use these exact settings:
- **Board**: ESP32S3 Dev Module
- **Partition Scheme**: Default 4MB with spiffs
- **Flash Size**: 4MB
- **Upload Speed**: 921600
- **CPU Frequency**: 240MHz

This configuration disables Bluetooth features and focuses on WiFi/Ethernet connectivity, which is perfect for the network discovery tool.

### 8. Alternative Board Packages

If issues persist, try these ESP32 board package versions:
- **Recommended**: 2.0.11 (stable)
- **Alternative**: 2.0.14 (latest)
- **Fallback**: 2.0.9 (older but stable)

The A2DP error should be completely resolved with the correct board selection and configuration.