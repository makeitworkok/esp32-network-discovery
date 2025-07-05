/*
 * ESP32 Network Discovery Tool
 * Scans for devices on the network and tests specific industrial protocol ports
 * Supports Ethernet connectivity with device discovery and port scanning
 */

// Disable Bluetooth to avoid A2DP compilation errors
#define CONFIG_BT_ENABLED 0

#include <ETH.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "config.h"
#include "network_scanner.h"
#include "port_scanner.h"
#include "web_interface.h"
#include "wifi_manager.h"

// Network configuration
bool eth_connected = false;
bool wifi_connected = false;
bool using_wifi_backup = false;
NetworkScanner scanner;
PortScanner portScanner;
WebInterface webInterface;

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      ETH.setHostname("ESP32-NetScanner");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      Serial.print(", Gateway: ");
      Serial.print(ETH.gatewayIP());
      Serial.print(", Subnet: ");
      Serial.println(ETH.subnetMask());
      eth_connected = true;
      using_wifi_backup = false;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      // Trigger WiFi backup if enabled
      if (WIFI_BACKUP_ENABLED && !using_wifi_backup) {
        Serial.println("Attempting WiFi backup connection...");
        wifiManager.enableBackupMode();
        if (wifiManager.connectToWiFi()) {
          using_wifi_backup = true;
          wifi_connected = true;
        }
      }
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("WiFi connected - IP: ");
      Serial.println(WiFi.localIP());
      wifi_connected = true;
      if (!eth_connected) {
        using_wifi_backup = true;
      }
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("WiFi disconnected");
      wifi_connected = false;
      if (using_wifi_backup && !eth_connected) {
        Serial.println("WiFi backup lost, attempting reconnection...");
        wifiManager.connectToWiFi();
      }
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("ESP32 Network Discovery Tool");
  Serial.println("============================");
  
  // Initialize Ethernet for WT32-ETH01
  WiFi.onEvent(WiFiEvent);
  ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_TYPE, ETH_CLK_MODE);
  
  // Wait for connection
  Serial.println("Waiting for Ethernet connection...");
  unsigned long startTime = millis();
  while (!eth_connected && (millis() - startTime) < ETH_CONNECTION_TIMEOUT) {
    delay(100);
  }
  
  if (!eth_connected) {
    Serial.println("Failed to connect to Ethernet!");
    Serial.println("Please check cable connection and try again.");
    return;
  }
  
  Serial.println("Ethernet connected successfully!");
  Serial.println();
  
  // Initialize WiFi manager
  wifiManager.begin();
  wifiManager.enableBackupMode();  // Enable WiFi backup by default
  
  // Initialize scanner components
  scanner.begin();
  portScanner.begin();
  
  // Initialize web interface
  webInterface.begin();
  
  // Display configuration
  Serial.println("Configuration:");
  Serial.printf("- Scan timeout: %d ms\n", SCAN_TIMEOUT);
  Serial.printf("- Port timeout: %d ms\n", PORT_TIMEOUT);
  Serial.printf("- Max concurrent scans: %d\n", MAX_CONCURRENT_SCANS);
  Serial.printf("- WiFi backup: %s\n", WIFI_BACKUP_ENABLED ? "Enabled" : "Disabled");
  Serial.println();
  
  Serial.println("Network interface ready!");
  Serial.println("Primary: Ethernet, Backup: WiFi");
  Serial.println("Commands: 'scan', 'status', 'wifi', 'help'");
}

void loop() {
  // Check network connectivity and handle failover
  if (!eth_connected && !wifi_connected) {
    static unsigned long lastReconnectAttempt = 0;
    if (millis() - lastReconnectAttempt > 10000) { // Try every 10 seconds
      Serial.println("No network connection. Attempting to reconnect...");
      
      // Try ethernet first
      if (!eth_connected) {
        Serial.println("Checking ethernet connection...");
        // Ethernet connection check happens automatically via events
      }
      
      // Try WiFi backup if ethernet fails
      if (!eth_connected && WIFI_BACKUP_ENABLED) {
        Serial.println("Attempting WiFi backup connection...");
        wifiManager.connectToWiFi();
      }
      
      lastReconnectAttempt = millis();
    }
    delay(1000);
    return;
  }
  
  // Handle WiFi manager operations
  wifiManager.handleCaptivePortal();
  wifiManager.checkEthernetAndSwitch();
  
  // Handle web interface requests
  webInterface.handleClient();
  
  // Handle web-initiated scans
  if (webInterface.isScanRunning()) {
    performWebScan();
  }
  
  // Check for serial commands
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    handleSerialCommand(command);
  }
  
  delay(50);
}

void performNetworkScan() {
  Serial.println("Starting network discovery...");
  Serial.println("=============================");
  
  IPAddress localIP = ETH.localIP();
  IPAddress subnet = ETH.subnetMask();
  
  // Calculate network range
  IPAddress networkAddr = IPAddress(
    localIP[0] & subnet[0],
    localIP[1] & subnet[1], 
    localIP[2] & subnet[2],
    localIP[3] & subnet[3]
  );
  
  Serial.printf("Local IP: %s\n", localIP.toString().c_str());
  Serial.printf("Network: %s\n", networkAddr.toString().c_str());
  Serial.printf("Subnet: %s\n", subnet.toString().c_str());
  Serial.println();
  
  // Scan for active devices
  std::vector<IPAddress> activeDevices = scanner.scanNetwork(networkAddr, subnet);
  
  if (activeDevices.empty()) {
    Serial.println("No devices found on the network.");
    return;
  }
  
  Serial.printf("Found %d active devices:\n", activeDevices.size());
  Serial.println();
  
  // Test ports for each device
  for (const auto& device : activeDevices) {
    Serial.printf("Scanning device: %s\n", device.toString().c_str());
    Serial.println("  Port  Service      Status");
    Serial.println("  ----  -----------  ------");
    
    // Test each target port
    for (int port : TARGET_PORTS) {
      String serviceName = getServiceName(port);
      bool isOpen = portScanner.testPort(device, port);
      
      Serial.printf("  %-4d  %-11s  %s\n", 
                   port, 
                   serviceName.c_str(), 
                   isOpen ? "OPEN" : "CLOSED");
    }
    
    Serial.println();
  }
  
  Serial.println("Network scan completed.");
  Serial.println("=======================");
  Serial.println();
}

void performWebScan() {
  static unsigned long lastUpdate = 0;
  static int currentIP = 0;
  static bool scanInitialized = false;
  
  ScanConfig config = webInterface.getScanConfig();
  
  if (!scanInitialized) {
    Serial.println("Starting web-initiated scan...");
    webInterface.setScanStatus("Initializing scan...");
    webInterface.clearScanResults();
    currentIP = (uint32_t)config.startIP;
    scanInitialized = true;
    lastUpdate = millis();
    return;
  }
  
  // Update progress periodically
  if (millis() - lastUpdate > 100) {
    uint32_t startIP = (uint32_t)config.startIP;
    uint32_t endIP = (uint32_t)config.endIP;
    uint32_t totalIPs = endIP - startIP + 1;
    uint32_t currentProgress = currentIP - startIP;
    
    int progress = (currentProgress * 100) / totalIPs;
    webInterface.setScanProgress(progress);
    webInterface.setScanStatus("Scanning " + IPAddress(currentIP).toString() + "...");
    
    lastUpdate = millis();
  }
  
  // Scan current IP
  IPAddress targetIP = IPAddress(currentIP);
  
  if (scanner.pingDevice(targetIP)) {
    Serial.printf("Found device: %s\n", targetIP.toString().c_str());
    
    // Create scan result
    ScanResult result;
    result.deviceIP = targetIP;
    result.hostname = "Unknown";
    result.timestamp = millis();
    
    unsigned long scanStart = millis();
    
    // Test each target port
    for (int port : config.targetPorts) {
      bool isOpen = portScanner.testPort(targetIP, port);
      if (isOpen) {
        result.openPorts.push_back(port);
      } else {
        result.closedPorts.push_back(port);
      }
    }
    
    result.responseTime = millis() - scanStart;
    result.status = "Complete";
    
    webInterface.addScanResult(result);
  }
  
  currentIP++;
  
  // Check if scan is complete
  if (currentIP > (uint32_t)config.endIP) {
    webInterface.setScanStatus("Scan completed - found " + String(webInterface.getScanResults().size()) + " devices");
    webInterface.setScanProgress(100);
    webInterface.stopScan();
    scanInitialized = false;
    Serial.println("Web scan completed.");
  }
}

void handleSerialCommand(String command) {
  command.toLowerCase();
  
  if (command == "scan") {
    performNetworkScan();
  } else if (command == "status") {
    printStatus();
  } else if (command == "wifi") {
    printWiFiStatus();
  } else if (command == "wifi scan") {
    scanWiFiNetworks();
  } else if (command.startsWith("wifi connect ")) {
    String ssid = command.substring(13);
    connectToWiFi(ssid);
  } else if (command == "wifi disconnect") {
    disconnectWiFi();
  } else if (command == "wifi toggle") {
    toggleWiFiBackup();
  } else if (command == "help") {
    printHelp();
  } else if (command.startsWith("ping ")) {
    String ip = command.substring(5);
    pingDevice(ip);
  } else if (command.startsWith("port ")) {
    handlePortCommand(command);
  } else {
    Serial.println("Unknown command. Type 'help' for available commands.");
  }
}

void printStatus() {
  Serial.println("System Status:");
  Serial.println("==============");
  
  // Ethernet status
  Serial.printf("Ethernet: %s\n", eth_connected ? "Connected" : "Disconnected");
  if (eth_connected) {
    Serial.printf("  IP Address: %s\n", ETH.localIP().toString().c_str());
    Serial.printf("  MAC Address: %s\n", ETH.macAddress().c_str());
    Serial.printf("  Gateway: %s\n", ETH.gatewayIP().toString().c_str());
  }
  
  // WiFi status
  Serial.printf("WiFi: %s\n", wifi_connected ? "Connected" : "Disconnected");
  Serial.printf("WiFi Backup: %s\n", wifiManager.isBackupModeEnabled() ? "Enabled" : "Disabled");
  if (wifi_connected) {
    Serial.printf("  SSID: %s\n", wifiManager.getCurrentSSID().c_str());
    Serial.printf("  IP Address: %s\n", wifiManager.getCurrentIP().toString().c_str());
    Serial.printf("  Signal: %d dBm\n", wifiManager.getRSSI());
  }
  
  // Connection mode
  Serial.printf("Active Connection: %s\n", 
                eth_connected ? "Ethernet" : 
                (wifi_connected ? "WiFi (Backup)" : "None"));
  
  // System info
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Uptime: %lu seconds\n", millis() / 1000);
  Serial.println();
}

void printHelp() {
  Serial.println("Available Commands:");
  Serial.println("===================");
  Serial.println("Network Scanning:");
  Serial.println("  scan              - Perform network scan");
  Serial.println("  ping <ip>         - Ping specific IP address");
  Serial.println("  port <ip> <port>  - Test specific port on IP");
  Serial.println();
  Serial.println("System Status:");
  Serial.println("  status            - Show full system status");
  Serial.println("  wifi              - Show WiFi status only");
  Serial.println();
  Serial.println("WiFi Management:");
  Serial.println("  wifi scan         - Scan for WiFi networks");
  Serial.println("  wifi connect <ssid> - Connect to WiFi network");
  Serial.println("  wifi disconnect   - Disconnect from WiFi");
  Serial.println("  wifi toggle       - Toggle WiFi backup mode");
  Serial.println();
  Serial.println("General:");
  Serial.println("  help              - Show this help message");
  Serial.println();
}

void pingDevice(String ipStr) {
  IPAddress ip;
  if (!ip.fromString(ipStr)) {
    Serial.println("Invalid IP address format.");
    return;
  }
  
  bool result = scanner.pingDevice(ip);
  Serial.printf("Ping %s: %s\n", ipStr.c_str(), result ? "Success" : "Failed");
}

void handlePortCommand(String command) {
  // Parse "port <ip> <port>"
  int firstSpace = command.indexOf(' ');
  int secondSpace = command.indexOf(' ', firstSpace + 1);
  
  if (firstSpace == -1 || secondSpace == -1) {
    Serial.println("Usage: port <ip> <port>");
    return;
  }
  
  String ipStr = command.substring(firstSpace + 1, secondSpace);
  String portStr = command.substring(secondSpace + 1);
  
  IPAddress ip;
  if (!ip.fromString(ipStr)) {
    Serial.println("Invalid IP address format.");
    return;
  }
  
  int port = portStr.toInt();
  if (port <= 0 || port > 65535) {
    Serial.println("Invalid port number.");
    return;
  }
  
  bool isOpen = portScanner.testPort(ip, port);
  String serviceName = getServiceName(port);
  
  Serial.printf("Port %d (%s) on %s: %s\n", 
               port, 
               serviceName.c_str(), 
               ipStr.c_str(), 
               isOpen ? "OPEN" : "CLOSED");
}

void printWiFiStatus() {
  Serial.println("WiFi Status:");
  Serial.println("============");
  Serial.printf("Backup Mode: %s\n", wifiManager.isBackupModeEnabled() ? "Enabled" : "Disabled");
  Serial.printf("Connection State: %s\n", wifi_connected ? "Connected" : "Disconnected");
  
  if (wifi_connected) {
    Serial.printf("SSID: %s\n", wifiManager.getCurrentSSID().c_str());
    Serial.printf("IP Address: %s\n", wifiManager.getCurrentIP().toString().c_str());
    Serial.printf("Signal Strength: %d dBm\n", wifiManager.getRSSI());
  } else if (wifiManager.isAPMode()) {
    Serial.println("Mode: Access Point");
    Serial.printf("AP SSID: %s\n", AP_SSID);
    Serial.printf("AP IP: %s\n", wifiManager.getCurrentIP().toString().c_str());
  }
  
  std::vector<WiFiCredentials> knownNetworks = wifiManager.getKnownNetworks();
  Serial.printf("Known Networks: %d\n", knownNetworks.size());
  
  for (const auto& network : knownNetworks) {
    Serial.printf("  - %s (Priority: %d)\n", network.ssid.c_str(), network.priority);
  }
  Serial.println();
}

void scanWiFiNetworks() {
  Serial.println("Scanning for WiFi networks...");
  
  std::vector<WiFiNetwork> networks = wifiManager.scanNetworks();
  
  if (networks.empty()) {
    Serial.println("No networks found.");
    return;
  }
  
  Serial.println("Available Networks:");
  Serial.println("SSID                          | RSSI | Ch | Encryption | Known");
  Serial.println("------------------------------|------|----|-----------|---------");
  
  for (const auto& network : networks) {
    Serial.printf("%-30s| %4d | %2d | %-9s | %s\n",
                 network.ssid.c_str(),
                 network.rssi,
                 network.channel,
                 wifiManager.encryptionTypeStr(network.encryption).c_str(),
                 network.isKnown ? "Yes" : "No");
  }
  Serial.println();
}

void connectToWiFi(String ssid) {
  Serial.printf("Enter password for '%s': ", ssid.c_str());
  
  // Wait for password input
  while (!Serial.available()) {
    delay(100);
  }
  
  String password = Serial.readStringUntil('\n');
  password.trim();
  
  WiFiCredentials creds;
  creds.ssid = ssid;
  creds.password = password;
  creds.useStaticIP = false;
  creds.priority = 1;
  
  Serial.printf("Connecting to %s...\n", ssid.c_str());
  
  if (wifiManager.connectToNetwork(creds)) {
    Serial.println("WiFi connection successful!");
    wifi_connected = true;
  } else {
    Serial.println("WiFi connection failed!");
  }
}

void disconnectWiFi() {
  wifiManager.stopWiFi();
  wifi_connected = false;
  using_wifi_backup = false;
  Serial.println("WiFi disconnected");
}

void toggleWiFiBackup() {
  if (wifiManager.isBackupModeEnabled()) {
    wifiManager.disableBackupMode();
    Serial.println("WiFi backup mode disabled");
  } else {
    wifiManager.enableBackupMode();
    Serial.println("WiFi backup mode enabled");
  }
}

String getServiceName(int port) {
  switch (port) {
    case 80: return "HTTP";
    case 443: return "HTTPS";
    case 502: return "MODBUS TCP";
    case 47808: return "BACnet";
    default: return "Unknown";
  }
}
