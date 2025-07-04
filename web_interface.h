/*
 * Web Interface Header
 * Handles HTTP web server and configuration interface
 */

#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <vector>
#include <IPAddress.h>
#include "config.h"

struct NetworkConfig {
    bool useDHCP;
    IPAddress staticIP;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns1;
    IPAddress dns2;
};

struct ScanConfig {
    IPAddress startIP;
    IPAddress endIP;
    std::vector<int> targetPorts;
    int scanTimeout;
    bool autoScan;
    int scanInterval;
};

struct ScanResult {
    IPAddress deviceIP;
    String hostname;
    std::vector<int> openPorts;
    std::vector<int> closedPorts;
    unsigned long responseTime;
    unsigned long timestamp;
    String status;
};

class WebInterface {
public:
    WebInterface();
    ~WebInterface();
    
    // Initialize web server
    void begin();
    
    // Handle web server requests
    void handleClient();
    
    // Configuration management
    void loadConfiguration();
    void saveConfiguration();
    NetworkConfig getNetworkConfig();
    void setNetworkConfig(const NetworkConfig& config);
    ScanConfig getScanConfig();
    void setScanConfig(const ScanConfig& config);
    
    // Scan management
    void startScan();
    void stopScan();
    bool isScanRunning();
    void addScanResult(const ScanResult& result);
    std::vector<ScanResult> getScanResults();
    void clearScanResults();
    
    // CSV export
    String generateCSV();
    
    // Status and progress
    void setScanProgress(int progress);
    int getScanProgress();
    void setScanStatus(const String& status);
    String getScanStatus();
    
private:
    WebServer* server;
    NetworkConfig networkConfig;
    ScanConfig scanConfig;
    std::vector<ScanResult> scanResults;
    bool scanRunning;
    int scanProgress;
    String scanStatus;
    
    // Web page handlers
    void handleRoot();
    void handleConfig();
    void handleScan();
    void handleResults();
    void handleCSVDownload();
    void handleWiFiConfig();
    void handleWiFiScan();
    void handleAPI();
    void handleNotFound();
    
    // API endpoints
    void handleGetConfig();
    void handleSetConfig();
    void handleStartScan();
    void handleStopScan();
    void handleGetResults();
    void handleGetStatus();
    void handleClearResults();
    
    // HTML generation
    String generateHTML(const String& title, const String& content);
    String generateConfigPage();
    String generateScanPage();
    String generateResultsPage();
    
    // Utility functions
    String ipToString(IPAddress ip);
    IPAddress stringToIP(const String& str);
    void applyNetworkConfig();
    bool validateIPAddress(const String& ip);
    String formatTimestamp(unsigned long timestamp);
};

#endif // WEB_INTERFACE_H