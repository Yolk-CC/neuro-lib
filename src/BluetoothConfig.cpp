#include "BluetoothConfig.h"

static BluetoothConfig* btConfigInstance = nullptr;

BluetoothConfig::BluetoothConfig()
    : webServer_(80)
    , apStarted_(false)
    , bluetoothConnected_(false)
    , connectedBluetoothAddress_("")
    , connectedBluetoothName_("") {
}

BluetoothConfig::~BluetoothConfig() {
    end();
}

bool BluetoothConfig::begin(const String& apSsid, const String& apPassword) {
    apSsid_ = apSsid;
    apPassword_ = apPassword;
    
    btConfigInstance = this;
    
    if (!startAccessPoint()) {
        Serial.println("Failed to start access point");
        return false;
    }
    
    setupWebServer();
    
    Serial.println("Bluetooth Config started");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    
    return true;
}

void BluetoothConfig::end() {
    stopAccessPoint();
    webServer_.stop();
    serialPort_.end();
    bluetoothConnected_ = false;
    scannedDevices_.clear();
}

void BluetoothConfig::handleClient() {
    if (apStarted_) {
        webServer_.handleClient();
    }
}

bool BluetoothConfig::isBluetoothConnected() const {
    return bluetoothConnected_;
}

String BluetoothConfig::getConnectedBluetoothAddress() const {
    return connectedBluetoothAddress_;
}

String BluetoothConfig::getConnectedBluetoothName() const {
    return connectedBluetoothName_;
}

bool BluetoothConfig::connectToBluetooth(const String& address) {
    if (bluetoothConnected_) {
        serialPort_.end();
        bluetoothConnected_ = false;
    }
    
    if (serialPort_.begin("NeuroLib_BT", true)) {
        if (serialPort_.connect(address)) {
            connectedBluetoothAddress_ = address;
            bluetoothConnected_ = true;
            
            connectedBluetoothName_ = "";
            for (const auto& device : scannedDevices_) {
                if (device.address == address) {
                    connectedBluetoothName_ = device.name;
                    break;
                }
            }
            
            Serial.print("Connected to Bluetooth: ");
            Serial.println(connectedBluetoothName_.isEmpty() ? address : connectedBluetoothName_);
            return true;
        }
    }
    
    Serial.println("Failed to connect to Bluetooth device");
    return false;
}

void BluetoothConfig::disconnectBluetooth() {
    if (bluetoothConnected_) {
        serialPort_.end();
        bluetoothConnected_ = false;
        connectedBluetoothAddress_ = "";
        connectedBluetoothName_ = "";
    }
}

bool BluetoothConfig::scanForBluetoothDevices(int scanTime) {
    scannedDevices_.clear();
    
    Serial.println("Starting Bluetooth scan...");
    
    // Use the serialPort_ instance which is BluetoothSerial
    int n = serialPort_.scanDevices(scanTime * 1000);
    
    for (int i = 0; i < n; i++) {
        BluetoothDevice device;
        device.name = serialPort_.getDeviceName(i, "");
        device.address = serialPort_.getDeviceAddress(i, "");
        device.rssi = serialPort_.getDeviceRSSI(i, -100);
        
        if (!device.address.isEmpty()) {
            scannedDevices_.push_back(device);
            Serial.print("Found: ");
            Serial.print(device.name.isEmpty() ? "Unknown" : device.name);
            Serial.print(" - ");
            Serial.println(device.address);
        }
    }
    
    Serial.print("Scan complete. Found ");
    Serial.print(scannedDevices_.size());
    Serial.println(" devices");
    
    return scannedDevices_.size() > 0;
}

size_t BluetoothConfig::getScannedDeviceCount() const {
    return scannedDevices_.size();
}

BluetoothDevice BluetoothConfig::getScannedDevice(size_t index) const {
    if (index < scannedDevices_.size()) {
        return scannedDevices_[index];
    }
    return {"", "", -100};
}

String BluetoothConfig::getScannedDevicesJSON() const {
    String json = "[";
    for (size_t i = 0; i < scannedDevices_.size(); i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"name\":\"" + (scannedDevices_[i].name.isEmpty() ? "Unknown" : scannedDevices_[i].name) + "\",";
        json += "\"address\":\"" + scannedDevices_[i].address + "\",";
        json += "\"rssi\":" + String(scannedDevices_[i].rssi);
        json += "}";
    }
    json += "]";
    return json;
}

String BluetoothConfig::getConfigPageHTML() const {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>NeuroLib Bluetooth Config</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
            background: white;
            border-radius: 16px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.2);
            overflow: hidden;
        }
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            text-align: center;
        }
        .header h1 { font-size: 24px; margin-bottom: 10px; }
        .header p { font-size: 14px; opacity: 0.9; }
        .content { padding: 30px; }
        .status-box {
            background: #f5f5f5;
            border-radius: 12px;
            padding: 20px;
            margin-bottom: 20px;
        }
        .status-box.connected { background: #d4edda; border: 2px solid #28a745; }
        .status-box.disconnected { background: #f8d7da; border: 2px solid #dc3545; }
        .status-title { font-weight: bold; margin-bottom: 10px; display: flex; align-items: center; gap: 10px; }
        .status-dot {
            width: 12px; height: 12px; border-radius: 50%;
            background: #dc3545;
        }
        .connected .status-dot { background: #28a745; }
        .btn {
            display: inline-block;
            padding: 12px 24px;
            background: #667eea;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            cursor: pointer;
            text-decoration: none;
            transition: all 0.3s;
            width: 100%;
            text-align: center;
            margin-bottom: 10px;
        }
        .btn:hover { background: #5568d3; transform: translateY(-2px); }
        .btn:disabled { background: #ccc; cursor: not-allowed; transform: none; }
        .btn-danger { background: #dc3545; }
        .btn-danger:hover { background: #c82333; }
        .btn-success { background: #28a745; }
        .btn-success:hover { background: #218838; }
        .device-list { margin-top: 20px; }
        .device-item {
            background: #f8f9fa;
            border: 1px solid #dee2e6;
            border-radius: 8px;
            padding: 15px;
            margin-bottom: 10px;
            cursor: pointer;
            transition: all 0.3s;
        }
        .device-item:hover { background: #e9ecef; border-color: #667eea; }
        .device-name { font-weight: bold; color: #333; }
        .device-info { font-size: 12px; color: #666; margin-top: 5px; }
        .rssi { 
            float: right; 
            padding: 4px 8px; 
            border-radius: 4px; 
            font-size: 12px;
        }
        .rssi.strong { background: #d4edda; color: #28a745; }
        .rssi.medium { background: #fff3cd; color: #856404; }
        .rssi.weak { background: #f8d7da; color: #dc3545; }
        .loading {
            text-align: center;
            padding: 20px;
            color: #666;
        }
        .spinner {
            border: 3px solid #f3f3f3;
            border-top: 3px solid #667eea;
            border-radius: 50%;
            width: 30px;
            height: 30px;
            animation: spin 1s linear infinite;
            margin: 0 auto 10px;
        }
        @keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }
        .hidden { display: none; }
        .info-text { font-size: 12px; color: #666; margin-top: 10px; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>NeuroLib Bluetooth Config</h1>
            <p>Connect to your EEG device via Bluetooth</p>
        </div>
        <div class="content">
            <div id="statusBox" class="status-box disconnected">
                <div class="status-title">
                    <span class="status-dot"></span>
                    <span id="statusText">Not Connected</span>
                </div>
                <div id="connectedDevice"></div>
            </div>
            
            <button id="scanBtn" class="btn" onclick="startScan()">Scan for Devices</button>
            <button id="disconnectBtn" class="btn btn-danger hidden" onclick="disconnect()">Disconnect</button>
            
            <div id="loading" class="loading hidden">
                <div class="spinner"></div>
                <p>Scanning for devices...</p>
            </div>
            
            <div id="deviceList" class="device-list"></div>
            
            <p class="info-text">1. Click "Scan for Devices" to find available Bluetooth devices<br>
            2. Click on a device to connect<br>
            3. Once connected, data will be received automatically</p>
        </div>
    </div>
    
    <script>
        let isConnected = false;
        
        function updateStatus() {
            fetch('/status')
                .then(r => r.json())
                .then(data => {
                    const statusBox = document.getElementById('statusBox');
                    const statusText = document.getElementById('statusText');
                    const connectedDevice = document.getElementById('connectedDevice');
                    const scanBtn = document.getElementById('scanBtn');
                    const disconnectBtn = document.getElementById('disconnectBtn');
                    
                    isConnected = data.connected;
                    
                    if (data.connected) {
                        statusBox.className = 'status-box connected';
                        statusText.textContent = 'Connected';
                        connectedDevice.innerHTML = '<strong>' + data.name + '</strong><br>' + data.address;
                        scanBtn.classList.add('hidden');
                        disconnectBtn.classList.remove('hidden');
                    } else {
                        statusBox.className = 'status-box disconnected';
                        statusText.textContent = 'Not Connected';
                        connectedDevice.innerHTML = '';
                        scanBtn.classList.remove('hidden');
                        disconnectBtn.classList.add('hidden');
                    }
                });
        }
        
        function startScan() {
            document.getElementById('loading').classList.remove('hidden');
            document.getElementById('deviceList').classList.add('hidden');
            document.getElementById('scanBtn').disabled = true;
            
            fetch('/scan', { method: 'POST' })
                .then(r => r.json())
                .then(data => {
                    document.getElementById('loading').classList.add('hidden');
                    document.getElementById('scanBtn').disabled = false;
                    
                    const deviceList = document.getElementById('deviceList');
                    deviceList.innerHTML = '';
                    deviceList.classList.remove('hidden');
                    
                    if (data.devices.length === 0) {
                        deviceList.innerHTML = '<p style="text-align:center;color:#666;">No devices found</p>';
                        return;
                    }
                    
                    data.devices.forEach(device => {
                        const rssiClass = device.rssi > -50 ? 'strong' : (device.rssi > -70 ? 'medium' : 'weak');
                        const item = document.createElement('div');
                        item.className = 'device-item';
                        item.onclick = () => connect(device.address);
                        item.innerHTML = `
                            <span class="rssi ${rssiClass}">${device.rssi} dBm</span>
                            <div class="device-name">${device.name}</div>
                            <div class="device-info">${device.address}</div>
                        `;
                        deviceList.appendChild(item);
                    });
                })
                .catch(err => {
                    document.getElementById('loading').classList.add('hidden');
                    document.getElementById('scanBtn').disabled = false;
                    alert('Scan failed: ' + err);
                });
        }
        
        function connect(address) {
            if (!confirm('Connect to this device?')) return;
            
            document.getElementById('loading').classList.remove('hidden');
            document.getElementById('deviceList').classList.add('hidden');
            
            fetch('/connect', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ address: address })
            })
            .then(r => r.json())
            .then(data => {
                document.getElementById('loading').classList.add('hidden');
                if (data.success) {
                    updateStatus();
                } else {
                    alert('Connection failed: ' + data.error);
                }
            })
            .catch(err => {
                document.getElementById('loading').classList.add('hidden');
                alert('Connection failed: ' + err);
            });
        }
        
        function disconnect() {
            fetch('/disconnect', { method: 'POST' })
                .then(r => r.json())
                .then(data => {
                    updateStatus();
                });
        }
        
        setInterval(updateStatus, 2000);
        updateStatus();
    </script>
</body>
</html>
)rawliteral";
    return html;
}

bool BluetoothConfig::writeToBluetooth(const uint8_t* data, size_t size) {
    if (!bluetoothConnected_) return false;
    return serialPort_.write(data, size) == size;
}

size_t BluetoothConfig::readFromBluetooth(uint8_t* buffer, size_t size) {
    if (!bluetoothConnected_) return 0;
    return serialPort_.readBytes(buffer, size);
}

bool BluetoothConfig::isBluetoothDataAvailable() const {
    if (!bluetoothConnected_) return false;
    return serialPort_.available() > 0;
}

bool BluetoothConfig::startAccessPoint() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    
    bool result = WiFi.softAP(apSsid_.c_str(), apPassword_.c_str());
    apStarted_ = result;
    return result;
}

void BluetoothConfig::stopAccessPoint() {
    if (apStarted_) {
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_OFF);
        apStarted_ = false;
    }
}

void BluetoothConfig::setupWebServer() {
    webServer_.on("/", HTTP_GET, [this]() { handleRoot(); });
    webServer_.on("/scan", HTTP_POST, [this]() { handleScan(); });
    webServer_.on("/connect", HTTP_POST, [this]() { handleConnect(); });
    webServer_.on("/disconnect", HTTP_POST, [this]() { handleDisconnect(); });
    webServer_.on("/status", HTTP_GET, [this]() { handleStatus(); });
    webServer_.onNotFound([this]() { handleNotFound(); });
    webServer_.begin();
}

void BluetoothConfig::handleRoot() {
    webServer_.send(200, "text/html", getConfigPageHTML());
}

void BluetoothConfig::handleScan() {
    bool success = scanForBluetoothDevices(5);
    
    String json = "{\"success\":" + String(success ? "true" : "false") + 
                  ",\"devices\":" + getScannedDevicesJSON() + "}";
    webServer_.send(200, "application/json", json);
}

void BluetoothConfig::handleConnect() {
    if (!webServer_.hasArg("plain")) {
        webServer_.send(400, "application/json", "{\"success\":false,\"error\":\"No data\"}");
        return;
    }
    
    String body = webServer_.arg("plain");
    int addrStart = body.indexOf("\"address\":\"") + 11;
    int addrEnd = body.indexOf("\"", addrStart);
    String address = body.substring(addrStart, addrEnd);
    
    bool success = connectToBluetooth(address);
    
    if (success) {
        webServer_.send(200, "application/json", "{\"success\":true}");
    } else {
        webServer_.send(200, "application/json", "{\"success\":false,\"error\":\"Connection failed\"}");
    }
}

void BluetoothConfig::handleDisconnect() {
    disconnectBluetooth();
    webServer_.send(200, "application/json", "{\"success\":true}");
}

void BluetoothConfig::handleStatus() {
    String json = "{";
    json += "\"connected\":" + String(bluetoothConnected_ ? "true" : "false");
    json += ",\"address\":\"" + connectedBluetoothAddress_ + "\"";
    json += ",\"name\":\"" + connectedBluetoothName_ + "\"";
    json += ",\"deviceCount\":" + String(scannedDevices_.size());
    json += ",\"devices\":" + getScannedDevicesJSON();
    json += "}";
    webServer_.send(200, "application/json", json);
}

void BluetoothConfig::handleData() {
    webServer_.send(200, "application/json", "{\"available\":" + String(isBluetoothDataAvailable() ? "true" : "false") + "}");
}

void BluetoothConfig::handleNotFound() {
    webServer_.send(404, "text/plain", "Not Found");
}
