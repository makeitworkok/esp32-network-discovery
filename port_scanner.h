/*
 * Port Scanner Header
 * Handles TCP port scanning for specific services
 */

#ifndef PORT_SCANNER_H
#define PORT_SCANNER_H

#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include <IPAddress.h>
#include "config.h"

struct PortScanResult {
    IPAddress target;
    int port;
    bool isOpen;
    unsigned long responseTime;
    String serviceName;
};

class PortScanner {
public:
    PortScanner();
    ~PortScanner();
    
    // Initialize the port scanner
    void begin();
    
    // Test a specific port on a target IP
    bool testPort(IPAddress target, int port);
    
    // Scan multiple ports on a target
    std::vector<PortScanResult> scanPorts(IPAddress target, const std::vector<int>& ports);
    
    // Get scan results
    std::vector<PortScanResult> getLastResults();
    
    // Clear scan results
    void clearResults();
    
private:
    std::vector<PortScanResult> scanResults;
    
    // Perform TCP connect scan
    bool tcpConnect(IPAddress target, int port, unsigned long& responseTime);
    
    // Perform SYN scan (simplified)
    bool synScan(IPAddress target, int port);
    
    // Check if port is commonly open
    bool isCommonPort(int port);
    
    // Get service name for port
    String getServiceName(int port);
    
    // Validate port number
    bool isValidPort(int port);
    
    // Add result to cache
    void addResult(IPAddress target, int port, bool isOpen, unsigned long responseTime);
};

#endif // PORT_SCANNER_H
