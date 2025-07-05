/*
 * Network Scanner Implementation
 * Handles network device discovery and ping operations
 */

#include "network_scanner.h"

NetworkScanner::NetworkScanner() {
    lastScanTime = 0;
    activeDevices.reserve(MAX_DEVICES);
}

NetworkScanner::~NetworkScanner() {
    activeDevices.clear();
}

void NetworkScanner::begin() {
    Serial.println("Initializing Network Scanner...");
    activeDevices.clear();
    lastScanTime = 0;
    
    #if DEBUG_NETWORK
    Serial.println("Network Scanner initialized successfully");
    #endif
}

std::vector<IPAddress> NetworkScanner::scanNetwork(IPAddress networkAddr, IPAddress subnetMask) {
    #if DEBUG_NETWORK
    Serial.println("Starting network scan...");
    #endif
    
    activeDevices.clear();
    
    IPAddress startIP, endIP;
    calculateScanRange(networkAddr, subnetMask, startIP, endIP);
    
    #if DEBUG_NETWORK
    Serial.printf("Scanning range: %s to %s\n", 
                  startIP.toString().c_str(), 
                  endIP.toString().c_str());
    #endif
    
    // Scan each IP in the range
    for (uint32_t ip = startIP; ip <= endIP; ip++) {
        IPAddress currentIP = IPAddress(ip);
        
        // Skip network and broadcast addresses
        if (currentIP == networkAddr || 
            currentIP == IPAddress(ip | ~((uint32_t)subnetMask))) {
            continue;
        }
        
        // Skip our own IP
        if (currentIP == ETH.localIP()) {
            continue;
        }
        
        #if DEBUG_NETWORK
        Serial.printf("Scanning: %s\r", currentIP.toString().c_str());
        #endif
        
        if (pingDevice(currentIP)) {
            activeDevices.push_back(currentIP);
            updateDeviceCache(currentIP);
            
            #if DEBUG_NETWORK
            Serial.printf("Found device: %s\n", currentIP.toString().c_str());
            #endif
        }
        
        delay(SCAN_DELAY);
        
        // Watchdog reset to prevent timeout
        yield();
    }
    
    lastScanTime = millis();
    
    #if DEBUG_NETWORK
    Serial.printf("Network scan completed. Found %d devices.\n", activeDevices.size());
    #endif
    
    return activeDevices;
}

bool NetworkScanner::pingDevice(IPAddress target) {
    if (!isValidIP(target)) {
        return false;
    }
    
    // Try ARP ping first (faster)
    if (arpPing(target)) {
        return true;
    }
    
    // Fallback to TCP ping
    return tcpPing(target);
}

std::vector<IPAddress> NetworkScanner::getActiveDevices() {
    return activeDevices;
}

void NetworkScanner::clearCache() {
    activeDevices.clear();
    lastScanTime = 0;
}

void NetworkScanner::calculateScanRange(IPAddress networkAddr, IPAddress subnetMask, 
                                       IPAddress& startIP, IPAddress& endIP) {
    uint32_t network = (uint32_t)networkAddr;
    uint32_t mask = (uint32_t)subnetMask;
    uint32_t broadcast = network | ~mask;
    
    startIP = IPAddress(network + 1);
    endIP = IPAddress(broadcast - 1);
}

bool NetworkScanner::arpPing(IPAddress target) {
    // Simple ARP-style ping using UDP
    WiFiUDP udp;
    
    if (!udp.begin(0)) {
        return false;
    }
    
    // Send a small UDP packet to trigger ARP resolution
    udp.beginPacket(target, 7); // Echo port
    udp.write((const uint8_t*)"ping", 4);
    bool result = udp.endPacket();
    
    udp.stop();
    
    return result;
}

bool NetworkScanner::tcpPing(IPAddress target) {
    WiFiClient client;
    client.setTimeout(PING_TIMEOUT);
    
    // Try to connect to a common port (80)
    bool connected = client.connect(target, 80);
    
    if (connected) {
        client.stop();
        return true;
    }
    
    // If port 80 fails, try port 443
    connected = client.connect(target, 443);
    
    if (connected) {
        client.stop();
        return true;
    }
    
    return false;
}

bool NetworkScanner::isValidIP(IPAddress ip) {
    // Check for valid IP address ranges
    uint8_t first = ip[0];
    
    // Exclude invalid ranges
    if (first == 0 || first == 127 || first >= 224) {
        return false;
    }
    
    // Check for multicast and broadcast
    if (ip == IPAddress(0, 0, 0, 0) || 
        ip == IPAddress(255, 255, 255, 255)) {
        return false;
    }
    
    return true;
}

void NetworkScanner::updateDeviceCache(IPAddress device) {
    // Check if device already exists in cache
    for (auto it = activeDevices.begin(); it != activeDevices.end(); ++it) {
        if (*it == device) {
            return; // Already in cache
        }
    }
    
    // Add to cache if not full
    if (activeDevices.size() < DEVICE_CACHE_SIZE) {
        activeDevices.push_back(device);
    }
}

void NetworkScanner::cleanupCache() {
    // Remove devices that haven't been seen recently
    // This is a simple implementation - in practice, you might want
    // to track last seen timestamps
    
    if (millis() - lastScanTime > SCAN_INTERVAL * 5) {
        activeDevices.clear();
    }
}
