/*
 * Port Scanner Implementation
 * Handles TCP port scanning for specific services
 */

#include "port_scanner.h"

PortScanner::PortScanner() {
    scanResults.reserve(MAX_DEVICES * TARGET_PORTS.size());
}

PortScanner::~PortScanner() {
    scanResults.clear();
}

void PortScanner::begin() {
    Serial.println("Initializing Port Scanner...");
    scanResults.clear();
    
    #if DEBUG_PORT_SCAN
    Serial.println("Port Scanner initialized successfully");
    #endif
}

bool PortScanner::testPort(IPAddress target, int port) {
    if (!isValidPort(port)) {
        #if DEBUG_PORT_SCAN
        Serial.printf("Invalid port number: %d\n", port);
        #endif
        return false;
    }
    
    unsigned long responseTime = 0;
    bool isOpen = tcpConnect(target, port, responseTime);
    
    // Add result to cache
    addResult(target, port, isOpen, responseTime);
    
    #if DEBUG_PORT_SCAN
    Serial.printf("Port scan: %s:%d - %s (Response: %lu ms)\n", 
                  target.toString().c_str(), 
                  port, 
                  isOpen ? "OPEN" : "CLOSED", 
                  responseTime);
    #endif
    
    return isOpen;
}

std::vector<PortScanResult> PortScanner::scanPorts(IPAddress target, const std::vector<int>& ports) {
    std::vector<PortScanResult> results;
    
    #if DEBUG_PORT_SCAN
    Serial.printf("Scanning %d ports on %s\n", ports.size(), target.toString().c_str());
    #endif
    
    for (int port : ports) {
        unsigned long responseTime = 0;
        bool isOpen = tcpConnect(target, port, responseTime);
        
        PortScanResult result;
        result.target = target;
        result.port = port;
        result.isOpen = isOpen;
        result.responseTime = responseTime;
        result.serviceName = getServiceName(port);
        
        results.push_back(result);
        addResult(target, port, isOpen, responseTime);
        
        // Small delay between port scans
        delay(50);
        yield();
    }
    
    return results;
}

std::vector<PortScanResult> PortScanner::getLastResults() {
    return scanResults;
}

void PortScanner::clearResults() {
    scanResults.clear();
}

bool PortScanner::tcpConnect(IPAddress target, int port, unsigned long& responseTime) {
    WiFiClient client;
    
    // Set timeout for connection attempt
    client.setTimeout(PORT_TIMEOUT);
    
    unsigned long startTime = millis();
    bool connected = false;
    
    // Attempt connection with retry logic
    for (int attempt = 0; attempt < MAX_RETRY_ATTEMPTS; attempt++) {
        connected = client.connect(target, port);
        
        if (connected) {
            break;
        }
        
        if (attempt < MAX_RETRY_ATTEMPTS - 1) {
            delay(RETRY_DELAY);
        }
    }
    
    responseTime = millis() - startTime;
    
    if (connected) {
        // Send a minimal request for specific protocols
        if (port == 80) {
            client.print("HEAD / HTTP/1.1\r\nHost: ");
            client.print(target.toString());
            client.print("\r\nConnection: close\r\n\r\n");
        } else if (port == 443) {
            // For HTTPS, just the connection attempt is enough
            // as SSL handshake would require more complex implementation
        } else if (port == 502) {
            // MODBUS TCP - send a simple query
            uint8_t modbusQuery[] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x01, 0x03, 0x00, 0x00, 0x00, 0x01};
            client.write(modbusQuery, sizeof(modbusQuery));
        } else if (port == 47808) {
            // BACnet - send a simple who-is request
            uint8_t bacnetQuery[] = {0x81, 0x0B, 0x00, 0x0C, 0x01, 0x20, 0xFF, 0xFF, 0x00, 0xFF, 0x10, 0x08};
            client.write(bacnetQuery, sizeof(bacnetQuery));
        }
        
        // Wait for response (brief)
        delay(100);
        
        // Check if we got any response
        bool hasResponse = client.available() > 0;
        
        client.stop();
        
        return true;
    }
    
    return false;
}

bool PortScanner::synScan(IPAddress target, int port) {
    // Simplified SYN scan - not implementing raw sockets
    // Fall back to TCP connect
    unsigned long responseTime;
    return tcpConnect(target, port, responseTime);
}

bool PortScanner::isCommonPort(int port) {
    // Check if port is in our target list
    for (int targetPort : TARGET_PORTS) {
        if (port == targetPort) {
            return true;
        }
    }
    return false;
}

String PortScanner::getServiceName(int port) {
    switch (port) {
        case 80: return "HTTP";
        case 443: return "HTTPS";
        case 502: return "MODBUS TCP";
        case 47808: return "BACnet";
        case 21: return "FTP";
        case 22: return "SSH";
        case 23: return "Telnet";
        case 25: return "SMTP";
        case 53: return "DNS";
        case 110: return "POP3";
        case 143: return "IMAP";
        case 993: return "IMAPS";
        case 995: return "POP3S";
        case 1883: return "MQTT";
        case 8080: return "HTTP-Alt";
        case 8443: return "HTTPS-Alt";
        default: return "Unknown";
    }
}

bool PortScanner::isValidPort(int port) {
    return (port > 0 && port <= 65535);
}

void PortScanner::addResult(IPAddress target, int port, bool isOpen, unsigned long responseTime) {
    // Check if we already have this result
    for (auto& result : scanResults) {
        if (result.target == target && result.port == port) {
            result.isOpen = isOpen;
            result.responseTime = responseTime;
            return;
        }
    }
    
    // Add new result
    PortScanResult result;
    result.target = target;
    result.port = port;
    result.isOpen = isOpen;
    result.responseTime = responseTime;
    result.serviceName = getServiceName(port);
    
    scanResults.push_back(result);
    
    // Limit cache size
    if (scanResults.size() > MAX_DEVICES * TARGET_PORTS.size()) {
        scanResults.erase(scanResults.begin());
    }
}
