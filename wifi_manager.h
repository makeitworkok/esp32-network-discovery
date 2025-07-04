/*
 * WiFi Manager Header
 * Handles WiFi connectivity as backup to ethernet
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <vector>
#include "config.h"

enum WiFiMode {
    WIFI_OFF,
    WIFI_STATION,
    WIFI_ACCESS_POINT,
    WIFI_STATION_AP
};

enum ConnectionState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    FAILED,
    AP_MODE
};

struct WiFiCredentials {
    String ssid;
    String password;
    bool useStaticIP;
    IPAddress staticIP;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns1;
    IPAddress dns2;
    int priority;  // Higher number = higher priority
};

struct WiFiNetwork {
    String ssid;
    int rssi;
    int channel;
    wifi_auth_mode_t encryption;
    bool isKnown;
};

class WiFiManager {
public:
    WiFiManager();
    ~WiFiManager();
    
    // Initialize WiFi manager
    void begin();
    
    // Connection management
    bool connectToWiFi();
    bool connectToKnownNetwork();
    bool connectToNetwork(const WiFiCredentials& creds);
    void startAccessPoint();
    void stopWiFi();
    
    // Network scanning
    std::vector<WiFiNetwork> scanNetworks();
    std::vector<WiFiCredentials> getKnownNetworks();
    
    // Credential management
    void addNetwork(const WiFiCredentials& creds);
    void removeNetwork(const String& ssid);
    void updateNetwork(const WiFiCredentials& creds);
    void clearAllNetworks();
    
    // Status and monitoring
    ConnectionState getConnectionState();
    String getCurrentSSID();
    IPAddress getCurrentIP();
    int getRSSI();
    bool isConnected();
    bool isAPMode();
    
    // Configuration
    void saveCredentials();
    void loadCredentials();
    void setMode(WiFiMode mode);
    WiFiMode getMode();
    
    // Backup mode control
    void enableBackupMode();
    void disableBackupMode();
    bool isBackupModeEnabled();
    void checkEthernetAndSwitch();
    
    // Captive portal for AP mode
    void startCaptivePortal();
    void stopCaptivePortal();
    void handleCaptivePortal();
    
private:
    std::vector<WiFiCredentials> knownNetworks;
    ConnectionState connectionState;
    WiFiMode currentMode;
    bool backupModeEnabled;
    unsigned long lastConnectionAttempt;
    int connectionRetries;
    DNSServer* dnsServer;
    
    // Internal connection methods
    bool attemptConnection(const WiFiCredentials& creds);
    void sortNetworksByPriority();
    WiFiCredentials* findKnownNetwork(const String& ssid);
    bool isNetworkInRange(const String& ssid);
    
    // Configuration file management
    bool saveToFile();
    bool loadFromFile();
    
public:
    // Utility functions (public for external access)
    String encryptionTypeStr(wifi_auth_mode_t encryptionType);
    wifi_auth_mode_t stringToEncryptionType(const String& str);
    
private:
    // Internal utility functions
    void printWiFiStatus();
    void resetConnectionAttempts();
    
    // Event handlers
    void onWiFiEvent(WiFiEvent_t event);
    static void WiFiEventCallback(WiFiEvent_t event);
    
    // Captive portal pages
    String generateCaptivePortalPage();
    String generateNetworkConfigPage();
    void handleCaptiveRoot();
    void handleCaptiveConfig();
    void handleCaptiveScan();
};

// Global instance declaration
extern WiFiManager wifiManager;

#endif // WIFI_MANAGER_H