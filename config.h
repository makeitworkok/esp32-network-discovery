/*
 * Configuration file for ESP32 Network Discovery Tool
 * Contains all configurable parameters and constants
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <IPAddress.h>

// Network configuration
#define ETH_CONNECTION_TIMEOUT 10000  // 10 seconds
#define SCAN_TIMEOUT 1000            // 1 second per IP
#define PORT_TIMEOUT 3000            // 3 seconds per port
#define MAX_CONCURRENT_SCANS 5       // Maximum concurrent scans
#define SCAN_INTERVAL 60000          // 1 minute between automatic scans

// Target ports for scanning
const std::vector<int> TARGET_PORTS = {
    80,     // HTTP
    443,    // HTTPS
    502,    // MODBUS TCP
    47808   // BACnet
};

// Network scanning configuration
#define PING_TIMEOUT 1000           // 1 second ping timeout
#define MAX_PING_ATTEMPTS 3         // Maximum ping attempts
#define SCAN_DELAY 10               // Delay between scans in ms

// Serial configuration
#define SERIAL_BAUD_RATE 115200

// Memory management
#define MAX_DEVICES 254             // Maximum devices to track
#define DEVICE_CACHE_SIZE 50        // Cache size for discovered devices

// Error handling
#define MAX_RETRY_ATTEMPTS 3        // Maximum retry attempts for failed operations
#define RETRY_DELAY 1000           // Delay between retry attempts

// Debug configuration
#define DEBUG_NETWORK 1             // Enable network debugging
#define DEBUG_PORT_SCAN 1           // Enable port scan debugging
#define DEBUG_MEMORY 0              // Enable memory debugging

// Web server configuration
#define WEB_SERVER_PORT 80          // Web server port
#define MAX_SCAN_RESULTS 100        // Maximum scan results to store
#define WEB_UPDATE_INTERVAL 1000    // Web interface update interval in ms
#define CSV_BUFFER_SIZE 8192        // CSV export buffer size

// Network configuration options
#define SUPPORT_STATIC_IP 1         // Enable static IP configuration
#define SUPPORT_DHCP 1              // Enable DHCP configuration
#define CONFIG_SAVE_INTERVAL 30000  // Save configuration every 30 seconds

// WT32-ETH01 Ethernet Configuration
// This board has built-in LAN8720 PHY with specific pin assignments
#define ETH_PHY_POWER 16            // Power pin for LAN8720 on WT32-ETH01
#define ETH_PHY_MDC 23              // MDC pin
#define ETH_PHY_MDIO 18             // MDIO pin
#define ETH_PHY_TYPE ETH_PHY_LAN8720
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN  // WT32-ETH01 uses GPIO0 for clock input
#define ETH_PHY_ADDR 1              // PHY address for WT32-ETH01

// WiFi backup configuration
#define WIFI_BACKUP_ENABLED 1       // Enable WiFi backup functionality
#define WIFI_CONNECTION_TIMEOUT 15000  // 15 seconds WiFi connection timeout
#define WIFI_RETRY_DELAY 5000       // 5 seconds between WiFi retry attempts
#define WIFI_MAX_RETRIES 3          // Maximum WiFi connection retries
#define WIFI_SCAN_TIMEOUT 10000     // 10 seconds for WiFi network scan
#define WIFI_AP_MODE_TIMEOUT 300000 // 5 minutes in AP mode before retry

// Access Point configuration (when no known networks available)
#define AP_SSID "ESP32-NetScanner"
#define AP_PASSWORD "netscanner123"
#define AP_CHANNEL 1
#define AP_MAX_CONNECTIONS 4
#define AP_HIDDEN false

#endif // CONFIG_H
