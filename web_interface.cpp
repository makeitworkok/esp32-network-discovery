/*
 * Web Interface Implementation
 * Handles HTTP web server and configuration interface
 */

#include "web_interface.h"

WebInterface::WebInterface() {
    server = new WebServer(80);
    scanRunning = false;
    scanProgress = 0;
    scanStatus = "Ready";
    
    // Initialize default configurations
    networkConfig.useDHCP = true;
    networkConfig.staticIP = IPAddress(192, 168, 1, 100);
    networkConfig.gateway = IPAddress(192, 168, 1, 1);
    networkConfig.subnet = IPAddress(255, 255, 255, 0);
    networkConfig.dns1 = IPAddress(8, 8, 8, 8);
    networkConfig.dns2 = IPAddress(8, 8, 4, 4);
    
    scanConfig.startIP = IPAddress(192, 168, 1, 1);
    scanConfig.endIP = IPAddress(192, 168, 1, 254);
    scanConfig.targetPorts = {80, 443, 502, 47808};
    scanConfig.scanTimeout = 3000;
    scanConfig.autoScan = false;
    scanConfig.scanInterval = 300; // 5 minutes
}

WebInterface::~WebInterface() {
    if (server) {
        delete server;
    }
}

void WebInterface::begin() {
    // Initialize SPIFFS for file storage
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS initialization failed");
        return;
    }
    
    // Load saved configuration
    loadConfiguration();
    
    // Set up web server routes
    server->on("/", HTTP_GET, [this]() { handleRoot(); });
    server->on("/config", HTTP_GET, [this]() { handleConfig(); });
    server->on("/scan", HTTP_GET, [this]() { handleScan(); });
    server->on("/results", HTTP_GET, [this]() { handleResults(); });
    server->on("/download", HTTP_GET, [this]() { handleCSVDownload(); });
    server->on("/wifi", HTTP_GET, [this]() { handleWiFiConfig(); });
    server->on("/wifi", HTTP_POST, [this]() { handleWiFiConfig(); });
    server->on("/wifi-scan", HTTP_GET, [this]() { handleWiFiScan(); });
    server->on("/api", HTTP_GET, [this]() { handleAPI(); });
    server->on("/api", HTTP_POST, [this]() { handleAPI(); });
    server->onNotFound([this]() { handleNotFound(); });
    
    // Start web server
    server->begin();
    Serial.println("Web server started on port 80");
    Serial.printf("Access the interface at: http://%s\n", ETH.localIP().toString().c_str());
}

void WebInterface::handleClient() {
    server->handleClient();
}

void WebInterface::handleRoot() {
    String html = generateHTML("ESP32 Network Discovery", R"(
        <div class="container">
            <h1>ESP32 Network Discovery Tool</h1>
            <div class="status-panel">
                <h3>Current Status</h3>
                <p><strong>IP Address:</strong> )" + ETH.localIP().toString() + R"(</p>
                <p><strong>Network Mode:</strong> )" + (networkConfig.useDHCP ? "DHCP" : "Static") + R"(</p>
                <p><strong>Scan Status:</strong> <span id="scan-status">)" + scanStatus + R"(</span></p>
                <p><strong>Devices Found:</strong> <span id="device-count">)" + String(scanResults.size()) + R"(</span></p>
            </div>
            <div class="nav-buttons">
                <a href="/config" class="btn">Network Configuration</a>
                <a href="/wifi" class="btn">WiFi Settings</a>
                <a href="/scan" class="btn">Start Scan</a>
                <a href="/results" class="btn">View Results</a>
                <a href="/download" class="btn">Download CSV</a>
            </div>
        </div>
        <script>
            // Auto-refresh status every 5 seconds
            setInterval(function() {
                fetch('/api?action=status')
                    .then(response => response.json())
                    .then(data => {
                        document.getElementById('scan-status').textContent = data.status;
                        document.getElementById('device-count').textContent = data.deviceCount;
                    });
            }, 5000);
        </script>
    )");
    
    server->send(200, "text/html", html);
}

void WebInterface::handleConfig() {
    if (server->method() == HTTP_POST) {
        // Handle configuration update
        networkConfig.useDHCP = server->hasArg("dhcp");
        if (!networkConfig.useDHCP) {
            networkConfig.staticIP = stringToIP(server->arg("static_ip"));
            networkConfig.gateway = stringToIP(server->arg("gateway"));
            networkConfig.subnet = stringToIP(server->arg("subnet"));
            networkConfig.dns1 = stringToIP(server->arg("dns1"));
            networkConfig.dns2 = stringToIP(server->arg("dns2"));
        }
        
        saveConfiguration();
        applyNetworkConfig();
        
        server->send(200, "text/html", generateHTML("Configuration Updated", 
            "<p>Network configuration updated successfully. The ESP32 will restart to apply changes.</p>"
            "<a href='/'>Return to Home</a>"));
        
        delay(2000);
        ESP.restart();
    } else {
        server->send(200, "text/html", generateConfigPage());
    }
}

void WebInterface::handleScan() {
    if (server->method() == HTTP_POST) {
        // Handle scan configuration and start
        if (server->hasArg("start_ip") && server->hasArg("end_ip")) {
            scanConfig.startIP = stringToIP(server->arg("start_ip"));
            scanConfig.endIP = stringToIP(server->arg("end_ip"));
        }
        
        if (server->hasArg("ports")) {
            String portsStr = server->arg("ports");
            scanConfig.targetPorts.clear();
            
            // Parse comma-separated ports
            int start = 0;
            int end = portsStr.indexOf(',');
            while (end != -1) {
                scanConfig.targetPorts.push_back(portsStr.substring(start, end).toInt());
                start = end + 1;
                end = portsStr.indexOf(',', start);
            }
            scanConfig.targetPorts.push_back(portsStr.substring(start).toInt());
        }
        
        startScan();
        server->send(200, "text/html", generateHTML("Scan Started", 
            "<p>Network scan started successfully.</p>"
            "<a href='/results'>View Results</a> | <a href='/'>Return to Home</a>"));
    } else {
        server->send(200, "text/html", generateScanPage());
    }
}

void WebInterface::handleResults() {
    server->send(200, "text/html", generateResultsPage());
}

void WebInterface::handleCSVDownload() {
    String csv = generateCSV();
    server->sendHeader("Content-Disposition", "attachment; filename=network_scan_results.csv");
    server->send(200, "text/csv", csv);
}

void WebInterface::handleAPI() {
    String action = server->arg("action");
    
    if (action == "status") {
        DynamicJsonDocument doc(1024);
        doc["status"] = scanStatus;
        doc["progress"] = scanProgress;
        doc["deviceCount"] = scanResults.size();
        doc["scanRunning"] = scanRunning;
        
        String response;
        serializeJson(doc, response);
        server->send(200, "application/json", response);
    }
    else if (action == "start_scan") {
        startScan();
        server->send(200, "application/json", "{\"status\":\"started\"}");
    }
    else if (action == "stop_scan") {
        stopScan();
        server->send(200, "application/json", "{\"status\":\"stopped\"}");
    }
    else if (action == "clear_results") {
        clearScanResults();
        server->send(200, "application/json", "{\"status\":\"cleared\"}");
    }
    else {
        server->send(400, "application/json", "{\"error\":\"Invalid action\"}");
    }
}

void WebInterface::handleNotFound() {
    server->send(404, "text/html", generateHTML("Page Not Found", 
        "<h1>404 - Page Not Found</h1><a href='/'>Return to Home</a>"));
}

String WebInterface::generateHTML(const String& title, const String& content) {
    return R"(<!DOCTYPE html>
<html>
<head>
    <title>)" + title + R"(</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }
        .container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .btn { display: inline-block; padding: 10px 20px; margin: 5px; background: #007bff; color: white; text-decoration: none; border-radius: 4px; border: none; cursor: pointer; }
        .btn:hover { background: #0056b3; }
        .status-panel { background: #e9ecef; padding: 15px; border-radius: 4px; margin: 20px 0; }
        .nav-buttons { text-align: center; margin: 20px 0; }
        .form-group { margin: 15px 0; }
        .form-group label { display: block; margin-bottom: 5px; font-weight: bold; }
        .form-group input, .form-group select { width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 4px; }
        .results-table { width: 100%; border-collapse: collapse; margin: 20px 0; }
        .results-table th, .results-table td { padding: 10px; text-align: left; border-bottom: 1px solid #ddd; }
        .results-table th { background-color: #f8f9fa; font-weight: bold; }
        .port-open { color: #28a745; font-weight: bold; }
        .port-closed { color: #dc3545; }
        .progress-bar { width: 100%; height: 20px; background: #e9ecef; border-radius: 10px; overflow: hidden; margin: 10px 0; }
        .progress-fill { height: 100%; background: #007bff; transition: width 0.3s ease; }
    </style>
</head>
<body>
    )" + content + R"(
</body>
</html>)";
}

String WebInterface::generateConfigPage() {
    String checked = networkConfig.useDHCP ? "checked" : "";
    String staticStyle = networkConfig.useDHCP ? "style='display:none'" : "";
    
    return generateHTML("Network Configuration", R"(
        <div class="container">
            <h1>Network Configuration</h1>
            <form method="POST">
                <div class="form-group">
                    <label>
                        <input type="checkbox" name="dhcp" )" + checked + R"( onchange="toggleStatic()"> Use DHCP
                    </label>
                </div>
                <div id="static-config" )" + staticStyle + R"(>
                    <div class="form-group">
                        <label>Static IP Address:</label>
                        <input type="text" name="static_ip" value=")" + ipToString(networkConfig.staticIP) + R"(">
                    </div>
                    <div class="form-group">
                        <label>Gateway:</label>
                        <input type="text" name="gateway" value=")" + ipToString(networkConfig.gateway) + R"(">
                    </div>
                    <div class="form-group">
                        <label>Subnet Mask:</label>
                        <input type="text" name="subnet" value=")" + ipToString(networkConfig.subnet) + R"(">
                    </div>
                    <div class="form-group">
                        <label>DNS 1:</label>
                        <input type="text" name="dns1" value=")" + ipToString(networkConfig.dns1) + R"(">
                    </div>
                    <div class="form-group">
                        <label>DNS 2:</label>
                        <input type="text" name="dns2" value=")" + ipToString(networkConfig.dns2) + R"(">
                    </div>
                </div>
                <button type="submit" class="btn">Apply Configuration</button>
                <a href="/" class="btn">Cancel</a>
            </form>
        </div>
        <script>
            function toggleStatic() {
                const checkbox = document.querySelector('input[name="dhcp"]');
                const staticConfig = document.getElementById('static-config');
                staticConfig.style.display = checkbox.checked ? 'none' : 'block';
            }
        </script>
    )");
}

String WebInterface::generateScanPage() {
    String portsStr = "";
    for (size_t i = 0; i < scanConfig.targetPorts.size(); i++) {
        if (i > 0) portsStr += ",";
        portsStr += String(scanConfig.targetPorts[i]);
    }
    
    return generateHTML("Network Scan", R"(
        <div class="container">
            <h1>Network Scan Configuration</h1>
            <form method="POST">
                <div class="form-group">
                    <label>Start IP Address:</label>
                    <input type="text" name="start_ip" value=")" + ipToString(scanConfig.startIP) + R"(">
                </div>
                <div class="form-group">
                    <label>End IP Address:</label>
                    <input type="text" name="end_ip" value=")" + ipToString(scanConfig.endIP) + R"(">
                </div>
                <div class="form-group">
                    <label>Target Ports (comma-separated):</label>
                    <input type="text" name="ports" value=")" + portsStr + R"(">
                </div>
                <button type="submit" class="btn">Start Scan</button>
                <a href="/" class="btn">Cancel</a>
            </form>
            <div id="scan-progress" style="display:none;">
                <h3>Scan Progress</h3>
                <div class="progress-bar">
                    <div class="progress-fill" id="progress-fill"></div>
                </div>
                <p id="progress-text">Scanning...</p>
            </div>
        </div>
        <script>
            function updateProgress() {
                fetch('/api?action=status')
                    .then(response => response.json())
                    .then(data => {
                        if (data.scanRunning) {
                            document.getElementById('scan-progress').style.display = 'block';
                            document.getElementById('progress-fill').style.width = data.progress + '%';
                            document.getElementById('progress-text').textContent = data.status;
                        } else {
                            document.getElementById('scan-progress').style.display = 'none';
                        }
                    });
            }
            setInterval(updateProgress, 1000);
        </script>
    )");
}

String WebInterface::generateResultsPage() {
    String resultsHtml = R"(
        <div class="container">
            <h1>Scan Results</h1>
            <p>Found )" + String(scanResults.size()) + R"( devices</p>
            <div class="nav-buttons">
                <a href="/download" class="btn">Download CSV</a>
                <a href="/scan" class="btn">New Scan</a>
                <button onclick="clearResults()" class="btn">Clear Results</button>
            </div>
            <table class="results-table">
                <thead>
                    <tr>
                        <th>IP Address</th>
                        <th>Hostname</th>
                        <th>Open Ports</th>
                        <th>Closed Ports</th>
                        <th>Response Time</th>
                        <th>Timestamp</th>
                    </tr>
                </thead>
                <tbody>
    )";
    
    for (const auto& result : scanResults) {
        String openPorts = "";
        for (size_t i = 0; i < result.openPorts.size(); i++) {
            if (i > 0) openPorts += ", ";
            openPorts += String(result.openPorts[i]);
        }
        
        String closedPorts = "";
        for (size_t i = 0; i < result.closedPorts.size(); i++) {
            if (i > 0) closedPorts += ", ";
            closedPorts += String(result.closedPorts[i]);
        }
        
        resultsHtml += "<tr>";
        resultsHtml += "<td>" + ipToString(result.deviceIP) + "</td>";
        resultsHtml += "<td>" + result.hostname + "</td>";
        resultsHtml += "<td class='port-open'>" + openPorts + "</td>";
        resultsHtml += "<td class='port-closed'>" + closedPorts + "</td>";
        resultsHtml += "<td>" + String(result.responseTime) + " ms</td>";
        resultsHtml += "<td>" + formatTimestamp(result.timestamp) + "</td>";
        resultsHtml += "</tr>";
    }
    
    resultsHtml += R"(
                </tbody>
            </table>
        </div>
        <script>
            function clearResults() {
                if (confirm('Are you sure you want to clear all results?')) {
                    fetch('/api?action=clear_results', {method: 'POST'})
                        .then(() => location.reload());
                }
            }
        </script>
    )";
    
    return generateHTML("Scan Results", resultsHtml);
}

String WebInterface::generateCSV() {
    String csv = "IP Address,Hostname,Open Ports,Closed Ports,Response Time (ms),Timestamp\n";
    
    for (const auto& result : scanResults) {
        String openPorts = "";
        for (size_t i = 0; i < result.openPorts.size(); i++) {
            if (i > 0) openPorts += ";";
            openPorts += String(result.openPorts[i]);
        }
        
        String closedPorts = "";
        for (size_t i = 0; i < result.closedPorts.size(); i++) {
            if (i > 0) closedPorts += ";";
            closedPorts += String(result.closedPorts[i]);
        }
        
        csv += ipToString(result.deviceIP) + ",";
        csv += result.hostname + ",";
        csv += "\"" + openPorts + "\",";
        csv += "\"" + closedPorts + "\",";
        csv += String(result.responseTime) + ",";
        csv += formatTimestamp(result.timestamp) + "\n";
    }
    
    return csv;
}

void WebInterface::loadConfiguration() {
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, configFile);
        
        networkConfig.useDHCP = doc["network"]["dhcp"] | true;
        if (!networkConfig.useDHCP) {
            networkConfig.staticIP = stringToIP(doc["network"]["static_ip"] | "192.168.1.100");
            networkConfig.gateway = stringToIP(doc["network"]["gateway"] | "192.168.1.1");
            networkConfig.subnet = stringToIP(doc["network"]["subnet"] | "255.255.255.0");
            networkConfig.dns1 = stringToIP(doc["network"]["dns1"] | "8.8.8.8");
            networkConfig.dns2 = stringToIP(doc["network"]["dns2"] | "8.8.4.4");
        }
        
        configFile.close();
    }
}

void WebInterface::saveConfiguration() {
    DynamicJsonDocument doc(1024);
    doc["network"]["dhcp"] = networkConfig.useDHCP;
    if (!networkConfig.useDHCP) {
        doc["network"]["static_ip"] = ipToString(networkConfig.staticIP);
        doc["network"]["gateway"] = ipToString(networkConfig.gateway);
        doc["network"]["subnet"] = ipToString(networkConfig.subnet);
        doc["network"]["dns1"] = ipToString(networkConfig.dns1);
        doc["network"]["dns2"] = ipToString(networkConfig.dns2);
    }
    
    File configFile = SPIFFS.open("/config.json", "w");
    if (configFile) {
        serializeJson(doc, configFile);
        configFile.close();
    }
}

void WebInterface::startScan() {
    scanRunning = true;
    scanProgress = 0;
    scanStatus = "Starting scan...";
    clearScanResults();
}

void WebInterface::stopScan() {
    scanRunning = false;
    scanStatus = "Scan stopped";
}

bool WebInterface::isScanRunning() {
    return scanRunning;
}

void WebInterface::addScanResult(const ScanResult& result) {
    scanResults.push_back(result);
}

std::vector<ScanResult> WebInterface::getScanResults() {
    return scanResults;
}

void WebInterface::clearScanResults() {
    scanResults.clear();
}

void WebInterface::setScanProgress(int progress) {
    scanProgress = progress;
}

int WebInterface::getScanProgress() {
    return scanProgress;
}

void WebInterface::setScanStatus(const String& status) {
    scanStatus = status;
}

String WebInterface::getScanStatus() {
    return scanStatus;
}

String WebInterface::ipToString(IPAddress ip) {
    return ip.toString();
}

IPAddress WebInterface::stringToIP(const String& str) {
    IPAddress ip;
    ip.fromString(str);
    return ip;
}

void WebInterface::applyNetworkConfig() {
    // This would typically restart the network interface
    // Implementation depends on specific requirements
}

bool WebInterface::validateIPAddress(const String& ip) {
    IPAddress addr;
    return addr.fromString(ip);
}

String WebInterface::formatTimestamp(unsigned long timestamp) {
    unsigned long seconds = timestamp / 1000;
    unsigned long hours = seconds / 3600;
    unsigned long minutes = (seconds % 3600) / 60;
    seconds = seconds % 60;
    
    return String(hours) + ":" + 
           (minutes < 10 ? "0" : "") + String(minutes) + ":" + 
           (seconds < 10 ? "0" : "") + String(seconds);
}

NetworkConfig WebInterface::getNetworkConfig() {
    return networkConfig;
}

void WebInterface::setNetworkConfig(const NetworkConfig& config) {
    networkConfig = config;
}

ScanConfig WebInterface::getScanConfig() {
    return scanConfig;
}

void WebInterface::setScanConfig(const ScanConfig& config) {
    scanConfig = config;
}

void WebInterface::handleWiFiConfig() {
    if (server->method() == HTTP_POST) {
        // Handle WiFi configuration update
        String ssid = server->arg("ssid");
        String password = server->arg("password");
        bool enableBackup = server->hasArg("enable_backup");
        
        if (!ssid.isEmpty()) {
            WiFiCredentials creds;
            creds.ssid = ssid;
            creds.password = password;
            creds.useStaticIP = server->hasArg("use_static_ip");
            creds.priority = server->arg("priority").toInt();
            
            if (creds.useStaticIP) {
                creds.staticIP = stringToIP(server->arg("wifi_static_ip"));
                creds.gateway = stringToIP(server->arg("wifi_gateway"));
                creds.subnet = stringToIP(server->arg("wifi_subnet"));
                creds.dns1 = stringToIP(server->arg("wifi_dns1"));
                creds.dns2 = stringToIP(server->arg("wifi_dns2"));
            }
            
            // Add network to WiFi manager
            extern WiFiManager wifiManager;
            wifiManager.addNetwork(creds);
            
            if (enableBackup) {
                wifiManager.enableBackupMode();
            } else {
                wifiManager.disableBackupMode();
            }
            
            server->send(200, "text/html", generateHTML("WiFi Configuration Updated", 
                "<p>WiFi settings updated successfully.</p>"
                "<a href='/wifi'>Back to WiFi Settings</a> | <a href='/'>Return to Home</a>"));
        }
    } else {
        server->send(200, "text/html", generateWiFiConfigPage());
    }
}

void WebInterface::handleWiFiScan() {
    extern WiFiManager wifiManager;
    std::vector<WiFiNetwork> networks = wifiManager.scanNetworks();
    
    DynamicJsonDocument doc(2048);
    JsonArray networksArray = doc.createNestedArray("networks");
    
    for (const auto& network : networks) {
        JsonObject networkObj = networksArray.createNestedObject();
        networkObj["ssid"] = network.ssid;
        networkObj["rssi"] = network.rssi;
        networkObj["channel"] = network.channel;
        networkObj["encryption"] = wifiManager.encryptionTypeStr(network.encryption);
        networkObj["isKnown"] = network.isKnown;
    }
    
    String response;
    serializeJson(doc, response);
    server->send(200, "application/json", response);
}

String WebInterface::generateWiFiConfigPage() {
    extern WiFiManager wifiManager;
    std::vector<WiFiCredentials> knownNetworks = wifiManager.getKnownNetworks();
    
    String knownNetworksHtml = "";
    for (const auto& network : knownNetworks) {
        knownNetworksHtml += "<tr>";
        knownNetworksHtml += "<td>" + network.ssid + "</td>";
        knownNetworksHtml += "<td>" + String(network.priority) + "</td>";
        knownNetworksHtml += "<td>" + (network.useStaticIP ? "Static" : "DHCP") + "</td>";
        knownNetworksHtml += "<td><button onclick=\"removeNetwork('" + network.ssid + "')\">Remove</button></td>";
        knownNetworksHtml += "</tr>";
    }
    
    String backupChecked = wifiManager.isBackupModeEnabled() ? "checked" : "";
    
    return generateHTML("WiFi Configuration", R"(
        <div class="container">
            <h1>WiFi Configuration</h1>
            
            <div class="status-panel">
                <h3>Current WiFi Status</h3>
                <p><strong>Backup Mode:</strong> )" + (wifiManager.isBackupModeEnabled() ? "Enabled" : "Disabled") + R"(</p>
                <p><strong>Connection:</strong> )" + (wifiManager.isConnected() ? "Connected to " + wifiManager.getCurrentSSID() : "Disconnected") + R"(</p>
                <p><strong>Signal:</strong> )" + (wifiManager.isConnected() ? String(wifiManager.getRSSI()) + " dBm" : "N/A") + R"(</p>
            </div>
            
            <h2>Add New Network</h2>
            <form method="POST">
                <div class="form-group">
                    <label>Network (SSID):</label>
                    <input type="text" name="ssid" required>
                    <button type="button" onclick="scanNetworks()">Scan Networks</button>
                </div>
                <div class="form-group">
                    <label>Password:</label>
                    <input type="password" name="password">
                </div>
                <div class="form-group">
                    <label>Priority (1-10):</label>
                    <input type="number" name="priority" value="1" min="1" max="10">
                </div>
                <div class="form-group">
                    <label>
                        <input type="checkbox" name="enable_backup" )" + backupChecked + R"(> Enable WiFi Backup Mode
                    </label>
                </div>
                <div class="form-group">
                    <label>
                        <input type="checkbox" name="use_static_ip" onchange="toggleWiFiStatic()"> Use Static IP
                    </label>
                </div>
                <div id="wifi-static-config" style="display:none;">
                    <div class="form-group">
                        <label>IP Address:</label>
                        <input type="text" name="wifi_static_ip" value="192.168.1.100">
                    </div>
                    <div class="form-group">
                        <label>Gateway:</label>
                        <input type="text" name="wifi_gateway" value="192.168.1.1">
                    </div>
                    <div class="form-group">
                        <label>Subnet:</label>
                        <input type="text" name="wifi_subnet" value="255.255.255.0">
                    </div>
                    <div class="form-group">
                        <label>DNS 1:</label>
                        <input type="text" name="wifi_dns1" value="8.8.8.8">
                    </div>
                    <div class="form-group">
                        <label>DNS 2:</label>
                        <input type="text" name="wifi_dns2" value="8.8.4.4">
                    </div>
                </div>
                <button type="submit" class="btn">Add Network</button>
            </form>
            
            <h2>Known Networks</h2>
            <table class="results-table">
                <thead>
                    <tr>
                        <th>SSID</th>
                        <th>Priority</th>
                        <th>IP Mode</th>
                        <th>Action</th>
                    </tr>
                </thead>
                <tbody>
                    )" + knownNetworksHtml + R"(
                </tbody>
            </table>
            
            <div class="nav-buttons">
                <a href="/" class="btn">Back to Home</a>
            </div>
            
            <div id="scan-results" style="display:none;">
                <h3>Available Networks</h3>
                <div id="network-list"></div>
            </div>
        </div>
        
        <script>
            function toggleWiFiStatic() {
                const checkbox = document.querySelector('input[name="use_static_ip"]');
                const staticConfig = document.getElementById('wifi-static-config');
                staticConfig.style.display = checkbox.checked ? 'block' : 'none';
            }
            
            function scanNetworks() {
                fetch('/wifi-scan')
                    .then(response => response.json())
                    .then(data => {
                        let html = '<table class="results-table"><thead><tr><th>SSID</th><th>Signal</th><th>Encryption</th><th>Known</th><th>Action</th></tr></thead><tbody>';
                        data.networks.forEach(network => {
                            html += '<tr>';
                            html += '<td>' + network.ssid + '</td>';
                            html += '<td>' + network.rssi + ' dBm</td>';
                            html += '<td>' + network.encryption + '</td>';
                            html += '<td>' + (network.isKnown ? 'Yes' : 'No') + '</td>';
                            html += '<td><button onclick="selectNetwork(\'' + network.ssid + '\')">Select</button></td>';
                            html += '</tr>';
                        });
                        html += '</tbody></table>';
                        document.getElementById('network-list').innerHTML = html;
                        document.getElementById('scan-results').style.display = 'block';
                    });
            }
            
            function selectNetwork(ssid) {
                document.querySelector('input[name="ssid"]').value = ssid;
                document.getElementById('scan-results').style.display = 'none';
            }
            
            function removeNetwork(ssid) {
                if (confirm('Remove network: ' + ssid + '?')) {
                    // Implementation for removing network would go here
                    location.reload();
                }
            }
        </script>
    )");
}