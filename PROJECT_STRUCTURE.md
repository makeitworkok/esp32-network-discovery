# ESP32 Network Discovery Tool - Project Structure

## File Organization

```
esp32-network-discovery/
├── NetworkDiscovery.ino          # Main Arduino application
├── config.h                      # Configuration constants
├── network_scanner.h/.cpp        # Network device discovery
├── port_scanner.h/.cpp          # Industrial protocol port scanning
├── web_interface.h/.cpp         # Web server and HTML interface
├── wifi_manager.h/.cpp          # WiFi backup connectivity
├── README.md                    # Complete project documentation
├── replit.md                    # Technical architecture guide
├── validate_code.py             # Code validation script
├── arduino_cli.yaml             # Arduino CLI configuration
├── .gitignore                   # Git ignore rules
└── GITHUB_SETUP.md              # GitHub setup instructions
```

## Key Components

### Core Functionality
- **NetworkDiscovery.ino**: Main application loop, serial interface, dual connectivity management
- **config.h**: Network settings, target ports (80, 443, 502, 47808), WiFi credentials structure
- **network_scanner**: ARP-style device discovery, ping functionality, IP range calculation
- **port_scanner**: TCP connect tests, protocol-specific packets, industrial device detection

### User Interfaces
- **web_interface**: HTML control panel, REST API, configuration management, CSV export
- **wifi_manager**: WiFi credential storage, network scanning, captive portal mode

### Support Files
- **validate_code.py**: Syntax validation, compilation check, library verification
- **README.md**: Installation guide, usage instructions, troubleshooting
- **replit.md**: Architecture documentation, changelog, development context

## Hardware Requirements
- ESP32 development board with ethernet capability
- Ethernet cable and network connection
- USB cable for programming

## Software Dependencies
- Arduino IDE 2.0+
- ESP32 board support package
- ArduinoJson library
- Standard ESP32 libraries (ETH.h, WiFi.h, WebServer.h, SPIFFS.h)

## Network Protocols Supported
- HTTP (Port 80) - Web services
- HTTPS (Port 443) - Secure web services  
- MODBUS TCP (Port 502) - Industrial automation
- BACnet (Port 47808) - Building automation

## Repository Features
- Complete Arduino project structure
- Comprehensive documentation
- Code validation tools
- Professional Git configuration
- Ready for collaborative development