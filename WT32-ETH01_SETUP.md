# WT32-ETH01 Setup Guide

## About the WT32-ETH01

The WT32-ETH01 is an ESP32-based development board with built-in Ethernet connectivity. It's perfect for this network discovery project because:

- **Built-in Ethernet**: LAN8720 PHY chip with RJ45 connector
- **ESP32-S3 Core**: Powerful dual-core processor
- **Compact Design**: Small form factor suitable for industrial applications
- **GPIO Access**: Available pins for expansion
- **WiFi Capability**: Built-in WiFi for backup connectivity

## Hardware Specifications

- **MCU**: ESP32-S3-WROOM-1 module
- **Ethernet PHY**: LAN8720A
- **Power**: 5V via micro-USB or external power
- **Dimensions**: 55mm × 26mm
- **Operating Temperature**: -40°C to +85°C

## Pin Configuration

The code has been optimized for WT32-ETH01 with these pin assignments:

```cpp
// Ethernet Configuration (Pre-configured)
ETH_PHY_POWER = 16     // Power control pin
ETH_PHY_MDC = 23       // Management Data Clock
ETH_PHY_MDIO = 18      // Management Data Input/Output
ETH_PHY_TYPE = LAN8720 // PHY chip type
ETH_CLK_MODE = GPIO0_IN // Clock input mode
ETH_PHY_ADDR = 1       // PHY address
```

## Arduino IDE Setup

1. **Install ESP32 Board Package**:
   - Open Arduino IDE
   - Go to File → Preferences
   - Add this URL to Additional Board Manager URLs:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to Tools → Board → Board Manager
   - Search for "esp32" and install the latest version

2. **Select Board**:
   - Go to Tools → Board → ESP32 Arduino
   - Select "ESP32S3 Dev Module"

3. **Configure Board Settings**:
   - **CPU Frequency**: 240MHz
   - **Flash Size**: 4MB (32Mb)
   - **Partition Scheme**: Default 4MB with spiffs
   - **Upload Speed**: 921600
   - **Arduino Runs On**: Core 1
   - **Events Run On**: Core 1

4. **Install Required Libraries**:
   - Go to Tools → Manage Libraries
   - Install "ArduinoJson" library (latest version)

## Wiring and Connections

### Ethernet Connection
- Connect ethernet cable directly to the RJ45 connector
- The board handles all ethernet connections internally
- No external components required

### Power Supply
- **USB Power**: Connect micro-USB cable (5V, min 500mA)
- **External Power**: 5V DC via VIN pin (recommended for production)

### Serial Monitor
- Connect via USB cable
- Set baud rate to 115200
- Use for debugging and manual commands

## Programming the Board

1. **Download Project Files**:
   - Download all `.ino`, `.h`, and `.cpp` files
   - Place all files in the same directory

2. **Open in Arduino IDE**:
   - Open `NetworkDiscovery.ino`
   - Verify all other files are in the same folder

3. **Upload Code**:
   - Select the correct COM port
   - Click Upload button
   - Monitor serial output during upload

## Network Configuration

### Ethernet (Primary)
- Connects automatically via DHCP
- Static IP configuration available through web interface
- Automatic cable detection and connection

### WiFi (Backup)
- Automatically enabled as backup
- Configure networks through web interface at `/wifi`
- Supports multiple saved networks with priority

## Troubleshooting

### Common Issues:

1. **Board Not Recognized**:
   - Install CP2102 USB driver
   - Check USB cable connection
   - Try different USB port

2. **Upload Fails**:
   - Hold BOOT button during upload
   - Check board and port selection
   - Reduce upload speed to 115200

3. **Ethernet Not Working**:
   - Verify ethernet cable connection
   - Check network switch/router
   - Try different ethernet cable
   - Check power supply (minimum 500mA)

4. **No Serial Output**:
   - Verify baud rate is 115200
   - Check COM port selection
   - Press Reset button on board

### LED Indicators:
- **Power LED**: Solid when powered
- **Ethernet LED**: Blinks when network activity
- **WiFi LED**: Indicates WiFi connection status

## Performance Optimization

For best performance with WT32-ETH01:

1. **Power Supply**: Use external 5V supply for stable operation
2. **Ethernet Cable**: Use Cat5e or better cable
3. **Network Switch**: Use managed switch for better performance
4. **Cooling**: Ensure adequate ventilation in enclosure

## Production Deployment

When deploying in industrial environments:

1. **Enclosure**: Use IP65 rated enclosure
2. **Power**: Use industrial power supply
3. **Ethernet**: Use shielded ethernet cable
4. **Mounting**: Secure mounting with vibration isolation
5. **Grounding**: Proper grounding for EMC compliance

## Support

- **WT32-ETH01 Documentation**: Available from manufacturer
- **ESP32 Resources**: ESP32 Arduino Core documentation
- **Community**: ESP32 forums and Arduino communities

The WT32-ETH01 is an excellent choice for this network discovery tool, providing reliable ethernet connectivity with WiFi backup capability.