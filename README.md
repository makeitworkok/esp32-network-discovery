# ESP32 Network Discovery Tool

A comprehensive network discovery tool for ESP32 with ethernet connectivity that scans for devices and tests industrial protocol ports.

## Features

- **Network Device Discovery**: Automatically scans the local network for active devices
- **Industrial Protocol Support**: Tests for common industrial ports including:
  - Port 80 (HTTP)
  - Port 443 (HTTPS)
  - Port 502 (MODBUS TCP)
  - Port 47808 (BACnet)
- **Dual Connectivity**: Primary ethernet with WiFi backup for reliability
  - Automatic failover when ethernet is disconnected
  - Manual WiFi management and configuration
  - Support for multiple saved WiFi networks with priority
- **Web Interface**: Complete web-based control panel with:
  - Network configuration (DHCP/Static IP settings)
  - WiFi network management and scanning
  - Customizable scan ranges and target ports
  - Real-time scan progress monitoring
  - CSV export of scan results
  - Responsive design for mobile and desktop
- **Triple Interface**: Serial commands, web interface, and captive portal
- **Persistent Configuration**: Network and WiFi settings saved to SPIFFS filesystem

## Hardware Requirements

- **WT32-ETH01** (recommended) - ESP32-S3 board with built-in ethernet
- **Alternative**: ESP32-Ethernet-Kit or other ESP32 boards with ethernet capability
- Ethernet cable and network connection
- USB cable for programming and serial communication

### WT32-ETH01 Compatibility
This project is optimized for the WT32-ETH01 board, featuring:
- Built-in LAN8720 PHY chip for reliable ethernet
- ESP32-S3 dual-core processor
- Compact design suitable for industrial applications
- Pre-configured pin assignments for plug-and-play operation

See `WT32-ETH01_SETUP.md` for detailed hardware setup instructions.

## Installation

1. **Arduino IDE Setup**:
   - Install Arduino IDE 2.0 or later
   - Add ESP32 board support: Go to File > Preferences, add this URL to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to Tools > Board > Board Manager, search for "esp32" and install

2. **Upload Code**:
   - Open `NetworkDiscovery.ino` in Arduino IDE
   - Select your ESP32 board (Tools > Board > ESP32 Arduino > ESP32 Dev Module)
   - Select the correct COM port (Tools > Port)
   - Click Upload

## Configuration

The tool is configured through `config.h`:

```cpp
// Network timeouts
#define ETH_CONNECTION_TIMEOUT 10000  // 10 seconds
#define SCAN_TIMEOUT 1000            // 1 second per IP
#define PORT_TIMEOUT 3000            // 3 seconds per port

// Scan intervals
#define SCAN_INTERVAL 60000          // 1 minute between scans

// Target ports
const std::vector<int> TARGET_PORTS = {
    80,     // HTTP
    443,    // HTTPS
    502,    // MODBUS TCP
    47808   // BACnet
};
```

## Usage

### Web Interface (Recommended)
1. **Power on** the ESP32 and connect ethernet cable
2. **Find the IP address** from serial monitor (115200 baud)
3. **Open web browser** and navigate to the ESP32's IP address
4. **Configure network settings** (DHCP/Static IP) in the Configuration page
5. **Set scan parameters** (IP range, target ports) in the Scan page
6. **Start scanning** and monitor progress in real-time
7. **View and export results** as CSV from the Results page

### Serial Interface
1. **Open Serial Monitor** (115200 baud) to see output
2. **Manual Commands**:
   - `scan` - Start immediate network scan
   - `status` - Show full system status (ethernet + WiFi)
   - `wifi` - Show WiFi status only
   - `wifi scan` - Scan for available WiFi networks
   - `wifi connect <ssid>` - Connect to WiFi network (prompts for password)
   - `wifi disconnect` - Disconnect from WiFi
   - `wifi toggle` - Enable/disable WiFi backup mode
   - `ping <ip>` - Ping specific IP address
   - `port <ip> <port>` - Test specific port
   - `help` - Show all commands

## Output Example

```
ESP32 Network Discovery Tool
============================
Ethernet connected successfully!

Starting network discovery...
=============================
Local IP: 192.168.1.100
Network: 192.168.1.0
Subnet: 255.255.255.0

Found 3 active devices:

Scanning device: 192.168.1.1
  Port  Service      Status
  ----  -----------  ------
  80    HTTP         OPEN
  443   HTTPS        OPEN
  502   MODBUS TCP   CLOSED
  47808 BACnet       CLOSED

Scanning device: 192.168.1.50
  Port  Service      Status
  ----  -----------  ------
  80    HTTP         CLOSED
  443   HTTPS        CLOSED
  502   MODBUS TCP   OPEN
  47808 BACnet       CLOSED
```

## Code Structure

- `NetworkDiscovery.ino` - Main application file
- `config.h` - Configuration constants
- `network_scanner.h/cpp` - Network discovery implementation
- `port_scanner.h/cpp` - Port scanning implementation

## Industrial Protocol Details

### MODBUS TCP (Port 502)
- Tests connection and sends basic query packet
- Detects MODBUS-enabled devices like PLCs and HMIs

### BACnet (Port 47808)
- Sends Who-Is broadcast request
- Identifies Building Automation devices

### HTTP/HTTPS (Ports 80/443)
- Standard web services
- Often used for device web interfaces

### WiFi Backup Configuration
1. **Access WiFi Settings** via web interface at `/wifi`
2. **Scan for networks** or manually enter SSID
3. **Set network priority** (1-10, higher = preferred)
4. **Enable backup mode** to automatically failover
5. **Configure static IP** if needed for WiFi networks

## Troubleshooting

1. **No Network Connection**:
   - Check ethernet cable connections first
   - Verify WiFi backup is enabled in settings
   - ESP32 will start Access Point mode if no connections available
   - Connect to "ESP32-NetScanner" AP (password: netscanner123)

2. **WiFi Issues**:
   - Check WiFi credentials are correct
   - Verify network is in range (check RSSI)
   - Ensure WiFi backup mode is enabled
   - Use serial commands for troubleshooting: `wifi status`

3. **No Devices Found**:
   - Confirm ESP32 has network connectivity
   - Check firewall settings on target network
   - Verify target devices are powered on and connected

4. **Compilation Errors**:
   - Ensure ESP32 board package is installed
   - Install ArduinoJson library via Library Manager
   - Check Arduino IDE version (2.0+ recommended)
   - Verify all files are in same directory

## License

This project is open source and available under the MIT License.