/*
 * Network Scanner Header
 * Handles network device discovery and ping operations
 */

#ifndef NETWORK_SCANNER_H
#define NETWORK_SCANNER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// Include config.h FIRST to define PHY constants before ETH.h
#include "config.h"

// Fix for ETH library compatibility across ESP32 board package versions
#if defined(ARDUINO_ARCH_ESP32) && defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  #include "ETH.h"
#elif defined(ESP_IDF_VERSION_MAJOR) && ESP_IDF_VERSION_MAJOR >= 4
  #include <ETH.h>
#else
  #include <ETH.h>
#endif

#include <vector>
#include <IPAddress.h>
#include "config.h"

class NetworkScanner {
public:
    NetworkScanner();
    ~NetworkScanner();
    
    // Initialize the scanner
    void begin();
    
    // Scan entire network for active devices
    std::vector<IPAddress> scanNetwork(IPAddress networkAddr, IPAddress subnetMask);
    
    // Ping a specific device
    bool pingDevice(IPAddress target);
    
    // Get list of recently discovered devices
    std::vector<IPAddress> getActiveDevices();
    
    // Clear device cache
    void clearCache();
    
private:
    std::vector<IPAddress> activeDevices;
    unsigned long lastScanTime;
    
    // Calculate network range
    void calculateScanRange(IPAddress networkAddr, IPAddress subnetMask, 
                           IPAddress& startIP, IPAddress& endIP);
    
    // Perform ARP scan
    bool arpPing(IPAddress target);
    
    // Perform TCP connect scan
    bool tcpPing(IPAddress target);
    
    // Check if IP is in valid range
    bool isValidIP(IPAddress ip);
    
    // Update device cache
    void updateDeviceCache(IPAddress device);
    
    // Remove stale devices from cache
    void cleanupCache();
};

#endif // NETWORK_SCANNER_H
