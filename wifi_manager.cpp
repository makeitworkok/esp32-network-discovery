/*
 * WiFi Manager Implementation
 * Handles WiFi connectivity as backup to ethernet
 */

#include "wifi_manager.h"

// Global instance
WiFiManager wifiManager;

WiFiManager::WiFiManager() {
    connectionState = DISCONNECTED;
    currentMode = WIFI_OFF;
    backupModeEnabled = false;
    lastConnectionAttempt = 0;
    connectionRetries = 0;
    dnsServer = nullptr;
}

WiFiManager::~WiFiManager() {
    if (dnsServer) {
        delete dnsServer;
    }
}

void WiFiManager::begin() {
    Serial.println("Initializing WiFi Manager...");
    
    // Set up WiFi event handler
    WiFi.onEvent([this](WiFiEvent_t event) { this->onWiFiEvent(event); });
    
    // Load saved credentials
    loadCredentials();
    
    // Initialize in station mode
    WiFi.mode(WIFI_STA);
    
    Serial.println("WiFi Manager initialized");
    
    #if DEBUG_NETWORK
    Serial.printf("Found %d known networks\n", knownNetworks.size());
    #endif
}

bool WiFiManager::connectToWiFi() {
    if (!backupModeEnabled) {
        return false;
    }
    
    Serial.println("Attempting WiFi connection...");
    connectionState = CONNECTING;
    
    // Try to connect to known networks
    if (connectToKnownNetwork()) {
        return true;
    }
    
    // If no known networks available, start AP mode
    Serial.println("No known networks found, starting Access Point mode");
    startAccessPoint();
    return false;
}

bool WiFiManager::connectToKnownNetwork() {
    // Scan for available networks
    std::vector<WiFiNetwork> availableNetworks = scanNetworks();
    
    // Sort known networks by priority
    sortNetworksByPriority();
    
    // Try to connect to highest priority available network
    for (const auto& knownNet : knownNetworks) {
        for (const auto& availNet : availableNetworks) {
            if (knownNet.ssid == availNet.ssid) {
                Serial.printf("Attempting connection to: %s\n", knownNet.ssid.c_str());
                
                if (attemptConnection(knownNet)) {
                    connectionState = CONNECTED;
                    currentMode = WIFI_STATION;
                    resetConnectionAttempts();
                    printWiFiStatus();
                    return true;
                }
                break;
            }
        }
    }
    
    return false;
}

bool WiFiManager::connectToNetwork(const WiFiCredentials& creds) {
    Serial.printf("Connecting to network: %s\n", creds.ssid.c_str());
    
    connectionState = CONNECTING;
    
    if (attemptConnection(creds)) {
        connectionState = CONNECTED;
        currentMode = WIFI_STATION;
        
        // Add to known networks if not already present
        if (!findKnownNetwork(creds.ssid)) {
            addNetwork(creds);
        }
        
        printWiFiStatus();
        return true;
    }
    
    connectionState = FAILED;
    return false;
}

void WiFiManager::startAccessPoint() {
    Serial.println("Starting Access Point mode...");
    
    WiFi.mode(WIFI_AP);
    
    bool success = WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, AP_HIDDEN, AP_MAX_CONNECTIONS);
    
    if (success) {
        connectionState = AP_MODE;
        currentMode = WIFI_ACCESS_POINT;
        
        Serial.printf("Access Point started: %s\n", AP_SSID);
        Serial.printf("AP IP address: %s\n", WiFi.softAPIP().toString().c_str());
        Serial.printf("AP Password: %s\n", AP_PASSWORD);
        
        startCaptivePortal();
    } else {
        Serial.println("Failed to start Access Point");
        connectionState = FAILED;
    }
}

void WiFiManager::stopWiFi() {
    Serial.println("Stopping WiFi...");
    
    stopCaptivePortal();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    
    connectionState = DISCONNECTED;
    currentMode = WIFI_OFF;
}

std::vector<WiFiNetwork> WiFiManager::scanNetworks() {
    std::vector<WiFiNetwork> networks;
    
    Serial.println("Scanning for WiFi networks...");
    
    int networkCount = WiFi.scanNetworks();
    
    if (networkCount == 0) {
        Serial.println("No networks found");
        return networks;
    }
    
    for (int i = 0; i < networkCount; i++) {
        WiFiNetwork network;
        network.ssid = WiFi.SSID(i);
        network.rssi = WiFi.RSSI(i);
        network.channel = WiFi.channel(i);
        network.encryption = WiFi.encryptionType(i);
        network.isKnown = (findKnownNetwork(network.ssid) != nullptr);
        
        networks.push_back(network);
        
        #if DEBUG_NETWORK
        Serial.printf("Found: %s (RSSI: %d, Ch: %d, Enc: %s, Known: %s)\n",
                     network.ssid.c_str(),
                     network.rssi,
                     network.channel,
                     encryptionTypeStr(network.encryption).c_str(),
                     network.isKnown ? "Yes" : "No");
        #endif
    }
    
    return networks;
}

std::vector<WiFiCredentials> WiFiManager::getKnownNetworks() {
    return knownNetworks;
}

void WiFiManager::addNetwork(const WiFiCredentials& creds) {
    // Check if network already exists
    for (auto& existing : knownNetworks) {
        if (existing.ssid == creds.ssid) {
            // Update existing network
            existing = creds;
            saveCredentials();
            return;
        }
    }
    
    // Add new network
    knownNetworks.push_back(creds);
    saveCredentials();
    
    Serial.printf("Added network: %s\n", creds.ssid.c_str());
}

void WiFiManager::removeNetwork(const String& ssid) {
    for (auto it = knownNetworks.begin(); it != knownNetworks.end(); ++it) {
        if (it->ssid == ssid) {
            knownNetworks.erase(it);
            saveCredentials();
            Serial.printf("Removed network: %s\n", ssid.c_str());
            return;
        }
    }
}

void WiFiManager::updateNetwork(const WiFiCredentials& creds) {
    for (auto& network : knownNetworks) {
        if (network.ssid == creds.ssid) {
            network = creds;
            saveCredentials();
            Serial.printf("Updated network: %s\n", creds.ssid.c_str());
            return;
        }
    }
    
    // If not found, add as new
    addNetwork(creds);
}

void WiFiManager::clearAllNetworks() {
    knownNetworks.clear();
    saveCredentials();
    Serial.println("Cleared all WiFi networks");
}

ConnectionState WiFiManager::getConnectionState() {
    return connectionState;
}

String WiFiManager::getCurrentSSID() {
    if (connectionState == CONNECTED) {
        return WiFi.SSID();
    } else if (connectionState == AP_MODE) {
        return AP_SSID;
    }
    return "";
}

IPAddress WiFiManager::getCurrentIP() {
    if (connectionState == CONNECTED) {
        return WiFi.localIP();
    } else if (connectionState == AP_MODE) {
        return WiFi.softAPIP();
    }
    return IPAddress(0, 0, 0, 0);
}

int WiFiManager::getRSSI() {
    if (connectionState == CONNECTED) {
        return WiFi.RSSI();
    }
    return 0;
}

bool WiFiManager::isConnected() {
    return connectionState == CONNECTED;
}

bool WiFiManager::isAPMode() {
    return connectionState == AP_MODE;
}

void WiFiManager::setMode(WiFiMode mode) {
    currentMode = mode;
}

WiFiMode WiFiManager::getMode() {
    return currentMode;
}

void WiFiManager::enableBackupMode() {
    backupModeEnabled = true;
    Serial.println("WiFi backup mode enabled");
}

void WiFiManager::disableBackupMode() {
    backupModeEnabled = false;
    Serial.println("WiFi backup mode disabled");
}

bool WiFiManager::isBackupModeEnabled() {
    return backupModeEnabled;
}

void WiFiManager::checkEthernetAndSwitch() {
    if (!backupModeEnabled) {
        return;
    }
    
    // This function should be called from main loop to check ethernet status
    // and switch to WiFi if ethernet is down
    
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck < 5000) { // Check every 5 seconds
        return;
    }
    lastCheck = millis();
    
    // Check if ethernet is connected (this should be implemented in main application)
    // For now, we'll assume this is called when ethernet is down
    
    if (connectionState == DISCONNECTED && backupModeEnabled) {
        Serial.println("Ethernet down, attempting WiFi backup connection...");
        connectToWiFi();
    }
}

void WiFiManager::startCaptivePortal() {
    if (!dnsServer) {
        dnsServer = new DNSServer();
    }
    
    // Start DNS server for captive portal
    dnsServer->start(53, "*", WiFi.softAPIP());
    
    Serial.println("Captive portal started");
}

void WiFiManager::stopCaptivePortal() {
    if (dnsServer) {
        dnsServer->stop();
    }
}

void WiFiManager::handleCaptivePortal() {
    if (dnsServer && connectionState == AP_MODE) {
        dnsServer->processNextRequest();
    }
}

bool WiFiManager::attemptConnection(const WiFiCredentials& creds) {
    WiFi.mode(WIFI_STA);
    
    // Configure static IP if required
    if (creds.useStaticIP) {
        if (!WiFi.config(creds.staticIP, creds.gateway, creds.subnet, creds.dns1, creds.dns2)) {
            Serial.println("Failed to configure static IP");
            return false;
        }
    }
    
    // Start connection
    WiFi.begin(creds.ssid.c_str(), creds.password.c_str());
    
    // Wait for connection
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < WIFI_CONNECTION_TIMEOUT) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("Connected to %s\n", creds.ssid.c_str());
        return true;
    } else {
        Serial.printf("Failed to connect to %s\n", creds.ssid.c_str());
        return false;
    }
}

void WiFiManager::sortNetworksByPriority() {
    std::sort(knownNetworks.begin(), knownNetworks.end(), 
              [](const WiFiCredentials& a, const WiFiCredentials& b) {
                  return a.priority > b.priority;
              });
}

WiFiCredentials* WiFiManager::findKnownNetwork(const String& ssid) {
    for (auto& network : knownNetworks) {
        if (network.ssid == ssid) {
            return &network;
        }
    }
    return nullptr;
}

bool WiFiManager::isNetworkInRange(const String& ssid) {
    std::vector<WiFiNetwork> networks = scanNetworks();
    for (const auto& network : networks) {
        if (network.ssid == ssid) {
            return true;
        }
    }
    return false;
}

void WiFiManager::saveCredentials() {
    saveToFile();
}

void WiFiManager::loadCredentials() {
    loadFromFile();
}

bool WiFiManager::saveToFile() {
    File configFile = FILESYSTEM.open("/wifi_config.json", "w");
    if (!configFile) {
        Serial.println("Failed to open WiFi config file for writing");
        return false;
    }
    
    DynamicJsonDocument doc(2048);
    JsonArray networks = doc.createNestedArray("networks");
    
    for (const auto& creds : knownNetworks) {
        JsonObject network = networks.createNestedObject();
        network["ssid"] = creds.ssid;
        network["password"] = creds.password;
        network["useStaticIP"] = creds.useStaticIP;
        network["staticIP"] = creds.staticIP.toString();
        network["gateway"] = creds.gateway.toString();
        network["subnet"] = creds.subnet.toString();
        network["dns1"] = creds.dns1.toString();
        network["dns2"] = creds.dns2.toString();
        network["priority"] = creds.priority;
    }
    
    doc["backupEnabled"] = backupModeEnabled;
    
    serializeJson(doc, configFile);
    configFile.close();
    
    #if DEBUG_NETWORK
    Serial.println("WiFi configuration saved");
    #endif
    
    return true;
}

bool WiFiManager::loadFromFile() {
    File configFile = FILESYSTEM.open("/wifi_config.json", "r");
    if (!configFile) {
        Serial.println("WiFi config file not found, using defaults");
        return false;
    }
    
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();
    
    if (error) {
        Serial.println("Failed to parse WiFi config file");
        return false;
    }
    
    knownNetworks.clear();
    
    JsonArray networks = doc["networks"];
    for (JsonObject network : networks) {
        WiFiCredentials creds;
        creds.ssid = network["ssid"].as<String>();
        creds.password = network["password"].as<String>();
        creds.useStaticIP = network["useStaticIP"] | false;
        String staticIP = network["staticIP"].as<String>();
        if (staticIP.isEmpty()) staticIP = "192.168.1.100";
        creds.staticIP.fromString(staticIP);
        
        String gateway = network["gateway"].as<String>();
        if (gateway.isEmpty()) gateway = "192.168.1.1";
        creds.gateway.fromString(gateway);
        
        String subnet = network["subnet"].as<String>();
        if (subnet.isEmpty()) subnet = "255.255.255.0";
        creds.subnet.fromString(subnet);
        
        String dns1 = network["dns1"].as<String>();
        if (dns1.isEmpty()) dns1 = "8.8.8.8";
        creds.dns1.fromString(dns1);
        
        String dns2 = network["dns2"].as<String>();
        if (dns2.isEmpty()) dns2 = "8.8.4.4";
        creds.dns2.fromString(dns2);
        creds.priority = network["priority"] | 1;
        
        knownNetworks.push_back(creds);
    }
    
    backupModeEnabled = doc["backupEnabled"] | false;
    
    #if DEBUG_NETWORK
    Serial.printf("Loaded %d WiFi networks from config\n", knownNetworks.size());
    #endif
    
    return true;
}

String WiFiManager::encryptionTypeStr(wifi_auth_mode_t encryptionType) {
    switch (encryptionType) {
        case WIFI_AUTH_OPEN: return "Open";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA";
        case WIFI_AUTH_WPA2_PSK: return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-EAP";
        case WIFI_AUTH_WPA3_PSK: return "WPA3";
        case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/WPA3";
        default: return "Unknown";
    }
}

wifi_auth_mode_t WiFiManager::stringToEncryptionType(const String& str) {
    if (str == "Open") return WIFI_AUTH_OPEN;
    if (str == "WEP") return WIFI_AUTH_WEP;
    if (str == "WPA") return WIFI_AUTH_WPA_PSK;
    if (str == "WPA2") return WIFI_AUTH_WPA2_PSK;
    if (str == "WPA/WPA2") return WIFI_AUTH_WPA_WPA2_PSK;
    if (str == "WPA2-EAP") return WIFI_AUTH_WPA2_ENTERPRISE;
    if (str == "WPA3") return WIFI_AUTH_WPA3_PSK;
    if (str == "WPA2/WPA3") return WIFI_AUTH_WPA2_WPA3_PSK;
    return WIFI_AUTH_OPEN;
}

void WiFiManager::printWiFiStatus() {
    Serial.println("WiFi Status:");
    Serial.printf("- SSID: %s\n", WiFi.SSID().c_str());
    Serial.printf("- IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("- Signal Strength: %d dBm\n", WiFi.RSSI());
    Serial.printf("- Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("- DNS: %s\n", WiFi.dnsIP().toString().c_str());
}

void WiFiManager::resetConnectionAttempts() {
    connectionRetries = 0;
    lastConnectionAttempt = 0;
}

void WiFiManager::onWiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("WiFi Station started");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("WiFi connected");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.printf("WiFi got IP: %s\n", WiFi.localIP().toString().c_str());
            connectionState = CONNECTED;
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("WiFi disconnected");
            connectionState = DISCONNECTED;
            break;
        case ARDUINO_EVENT_WIFI_AP_START:
            Serial.println("WiFi AP started");
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            Serial.println("WiFi AP stopped");
            break;
        default:
            break;
    }
}

String WiFiManager::generateCaptivePortalPage() {
    return R"(<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Network Scanner - WiFi Setup</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }
        .container { max-width: 500px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .btn { display: inline-block; padding: 10px 20px; margin: 5px; background: #007bff; color: white; text-decoration: none; border-radius: 4px; border: none; cursor: pointer; }
        .btn:hover { background: #0056b3; }
        .form-group { margin: 15px 0; }
        .form-group label { display: block; margin-bottom: 5px; font-weight: bold; }
        .form-group input, .form-group select { width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 Network Scanner</h1>
        <h2>WiFi Configuration</h2>
        <p>Configure WiFi connection to access the network scanner interface.</p>
        <form action="/wifi-config" method="POST">
            <div class="form-group">
                <label>Network (SSID):</label>
                <input type="text" name="ssid" required>
            </div>
            <div class="form-group">
                <label>Password:</label>
                <input type="password" name="password">
            </div>
            <div class="form-group">
                <label>
                    <input type="checkbox" name="static_ip"> Use Static IP
                </label>
            </div>
            <button type="submit" class="btn">Connect</button>
        </form>
        <p><a href="/wifi-scan" class="btn">Scan Networks</a></p>
    </div>
</body>
</html>)";
}