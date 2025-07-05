# Replit Project Guide

## Overview

This is a comprehensive ESP32-based network discovery tool that scans for devices on ethernet networks and tests specific industrial protocol ports. The tool is designed for network administrators and industrial automation engineers who need to discover and assess devices on their networks.

## System Architecture

The project is structured as an Arduino-based ESP32 application with the following components:

### Core Files:
- `NetworkDiscovery.ino` - Main application with dual network connectivity and command handling
- `config.h` - Configuration constants, target ports, and WiFi backup settings
- `network_scanner.h/cpp` - Network device discovery and ping functionality
- `port_scanner.h/cpp` - TCP port scanning for industrial protocols
- `web_interface.h/cpp` - Web server and HTML interface for remote control
- `wifi_manager.h/cpp` - WiFi connectivity management and backup functionality

### Supporting Files:
- `README.md` - Complete documentation and usage instructions
- `validate_code.py` - Code validation script for syntax and structure checking

## Key Components

### Network Scanner
- Automatically discovers active devices on the local network
- Performs ARP-style pings and TCP connect tests
- Calculates network ranges based on subnet masks
- Maintains device cache for efficient scanning

### Port Scanner
- Tests specific industrial protocol ports on discovered devices
- Supports HTTP (80), HTTPS (443), MODBUS TCP (502), and BACnet (47808)
- Implements protocol-specific test packets for accurate detection
- Tracks response times and maintains scan results

### Serial Interface
- Interactive command system for manual control
- Commands: scan, status, ping, port testing, help
- Real-time monitoring and debugging output

### Web Interface
- Complete HTML-based control panel hosted on ESP32
- Network configuration management (DHCP/Static IP)
- WiFi network management with scanning and credential storage
- Customizable scan parameters (IP ranges, target ports)
- Real-time scan progress monitoring with visual indicators
- CSV export functionality for scan results
- Responsive design for mobile and desktop access
- RESTful API endpoints for programmatic control

### WiFi Manager
- Automatic failover from ethernet to WiFi backup
- Multiple network credential storage with priority system
- Network scanning and signal strength monitoring
- Captive portal for initial configuration when no networks available
- Support for both DHCP and static IP on WiFi connections
- Persistent configuration storage in SPIFFS filesystem

## Data Flow

1. **Initialization**: ESP32 connects to ethernet and obtains IP configuration
2. **Web Server Start**: Launches HTTP server on port 80 for web interface
3. **Configuration Loading**: Reads saved network and scan settings from SPIFFS
4. **Network Discovery**: Scans network range for active devices using ping tests
5. **Port Scanning**: Tests target ports on each discovered device
6. **Result Storage**: Saves scan results to memory and SPIFFS for persistence
7. **Web Interface**: Serves HTML pages and handles API requests for remote control
8. **CSV Export**: Generates downloadable CSV files with scan results
9. **Continuous Operation**: Handles web requests and performs user-initiated scans

## External Dependencies

### Hardware Requirements:
- **WT32-ETH01** (recommended) - ESP32-S3 board with built-in ethernet
- Alternative: ESP32-Ethernet-Kit or other ESP32 boards with ethernet capability
- Ethernet cable and network connection
- USB cable for programming and serial communication

### Software Requirements:
- Arduino IDE 2.0 or later
- ESP32 board support package
- Standard ESP32 libraries (ETH.h, WiFi.h, WebServer.h, SPIFFS.h)
- ArduinoJson library (installable via Library Manager)

## Deployment Strategy

The tool is deployed directly to ESP32 hardware using Arduino IDE:

1. **Development Environment**: Arduino IDE with ESP32 board support
2. **Hardware Setup**: Connect ESP32 to ethernet and programming interface
3. **Upload Process**: Compile and upload via Arduino IDE
4. **Operation**: Monitor via serial interface at 115200 baud
5. **Network Integration**: Automatic ethernet connection and network discovery

## Changelog

```
Changelog:
- July 04, 2025: Initial setup
- July 04, 2025: Created complete ESP32 network discovery tool with:
  * Main application (NetworkDiscovery.ino)
  * Network scanner module for device discovery
  * Port scanner module for industrial protocol testing
  * Configuration system with target ports (80, 443, 502, 47808)
  * Serial interface with interactive commands
  * Code validation script and comprehensive documentation
  * Supports ethernet connectivity and automatic network scanning
- July 04, 2025: Enhanced with comprehensive web interface:
  * Complete HTML-based control panel hosted on ESP32
  * Network configuration management (DHCP/Static IP settings)
  * Customizable scan parameters (IP ranges, target ports)
  * Real-time scan progress monitoring with visual indicators
  * CSV export functionality for scan results
  * Responsive design for mobile and desktop access
  * RESTful API endpoints for programmatic control
  * SPIFFS-based persistent configuration storage
- July 04, 2025: Implemented WiFi backup connectivity:
  * Automatic failover from ethernet to WiFi when connection lost
  * Complete WiFi manager with credential storage and priority system
  * WiFi network scanning and signal strength monitoring
  * Captive portal mode when no known networks available
  * Web interface for WiFi configuration and management
  * Serial commands for WiFi control and troubleshooting
  * Support for static IP configuration on WiFi connections
  * Persistent WiFi configuration storage in SPIFFS
- July 05, 2025: Enhanced ESP32 3.2.0 compatibility:
  * Added intelligent ETH library include detection for board package 3.2.0
  * Implemented LittleFS filesystem support with automatic SPIFFS fallback
  * Updated all filesystem operations to use FILESYSTEM macro for compatibility
  * Verified all WiFi event constants for ESP32 3.2.0 compatibility
  * Comprehensive validation script confirms full 3.2.0 compatibility
  * Code now supports ESP32 board packages 2.0.9, 2.0.11, 2.0.14, and 3.2.0
```

## User Preferences

```
Preferred communication style: Simple, everyday language.
```

## Next Steps for Development

Since this is an empty repository, the following initial steps may be needed:

1. **Project Initialization**: Set up the basic project structure and configuration files
2. **Technology Stack Selection**: Choose appropriate frameworks, libraries, and tools
3. **Database Setup**: If data persistence is needed, configure database connections and schemas
4. **API Structure**: Define endpoints and routing if building a web service
5. **Frontend Framework**: Set up client-side framework if building a web application
6. **Authentication**: Implement user authentication and authorization if required
7. **External Services**: Configure any third-party integrations
8. **Deployment Configuration**: Set up deployment scripts and environment configurations

The code agent should be prepared to help establish the initial project structure based on the specific requirements and use case for this application.