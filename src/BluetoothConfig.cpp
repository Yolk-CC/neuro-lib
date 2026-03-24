#include "BluetoothConfig.h"

static BluetoothConfig* btConfigInstance = nullptr;

BluetoothConfig::BluetoothConfig()
    : web_server_(80)
    , ap_started_(false)
    , bluetooth_connected_(false)
    , connected_bluetooth_address_("")
    , connected_bluetooth_name_("") {
}

BluetoothConfig::~BluetoothConfig() {
    end();
}

bool BluetoothConfig::begin(const String& apSsid, const String& apPassword) {
    ap_ssid_ = apSsid;
    ap_password_ = apPassword;
    
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
    web_server_.stop();
    if (bluetooth_connected_) {
        serial_port_.end();
        bluetooth_connected_ = false;
    }
    scanned_devices_.clear();
}

void BluetoothConfig::handleClient() {
    if (ap_started_) {
        web_server_.handleClient();
    }
}

bool BluetoothConfig::isBluetoothConnected() const {
    return bluetooth_connected_;
}

String BluetoothConfig::getConnectedBluetoothAddress() const {
    return connected_bluetooth_address_;
}

String BluetoothConfig::getConnectedBluetoothName() const {
    return connected_bluetooth_name_;
}

bool BluetoothConfig::connectToBluetooth(const String& address) {
    if (bluetooth_connected_) {
        serial_port_.end();
        bluetooth_connected_ = false;
    }
    
    if (serial_port_.begin("NeuroLib_BT", true)) {
        if (serial_port_.connect(address)) {
            connected_bluetooth_address_ = address;
            bluetooth_connected_ = true;
            
            connected_bluetooth_name_ = "";
            for (const auto& device : scanned_devices_) {
                if (device.address == address) {
                    connected_bluetooth_name_ = device.name;
                    break;
                }
            }
            
            Serial.print("Connected to Bluetooth: ");
            Serial.println(connected_bluetooth_name_.isEmpty() ? address : connected_bluetooth_name_);
            return true;
        }
    }
    
    Serial.println("Failed to connect to Bluetooth device");
    return false;
}

void BluetoothConfig::disconnectBluetooth() {
    if (bluetooth_connected_) {
        serial_port_.end();
        bluetooth_connected_ = false;
        connected_bluetooth_address_ = "";
        connected_bluetooth_name_ = "";
    }
}

bool BluetoothConfig::scanForBluetoothDevices(int scanTime) {
    scanned_devices_.clear();
    
    Serial.println("Starting Bluetooth scan...");
    Serial.println("Note: Manual address entry is recommended for BluetoothSerial");
    Serial.println("Please enter the Bluetooth MAC address manually via web interface");
    
    return scanned_devices_.size() > 0;
}

size_t BluetoothConfig::getScannedDeviceCount() const {
    return scanned_devices_.size();
}

BluetoothDevice BluetoothConfig::getScannedDevice(size_t index) const {
    if (index < scanned_devices_.size()) {
        return scanned_devices_[index];
    }
    return {"", "", -100};
}

String BluetoothConfig::getScannedDevicesJSON() const {
    String json = "[";
    for (size_t i = 0; i < scanned_devices_.size(); i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"name\":\"" + (scanned_devices_[i].name.isEmpty() ? "Unknown" : scanned_devices_[i].name) + "\",";
        json += "\"address\":\"" + scanned_devices_[i].address + "\",";
        json += "\"rssi\":" + String(scanned_devices_[i].rssi);
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
        .input-group {
            margin-bottom: 15px;
        }
        .input-group label {
            display: block;
            margin-bottom: 5px;
            font-weight: bold;
            color: #333;
        }
        .input-group input {
            width: 100%;
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 6px;
            font-size: 14px;
        }
        .info-box {
            background: #e7f3ff;
            border-left: 4px solid #2196F3;
            padding: 15px;
            margin-bottom: 20px;
            border-radius: 4px;
        }
        .info-box h3 { margin-bottom: 10px; color: #1976D2; }
        .info-box p { font-size: 13px; line-height: 1.6; color: #555; }
        .info-box code {
            background: #fff;
            padding: 2px 6px;
            border-radius: 3px;
            font-family: monospace;
        }
        .hidden { display: none; }
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
            
            <div class="info-box">
                <h3>How to Connect</h3>
                <p>
                    1. Find your device's Bluetooth MAC address<br>
                    2. Enter the MAC address below (format: <code>XX:XX:XX:XX:XX:XX</code>)<br>
                    3. Click "Connect" button<br>
                    4. Wait for connection to establish
                </p>
            </div>
            
            <div class="input-group">
                <label for="btAddress">Bluetooth MAC Address:</label>
                <input type="text" id="btAddress" placeholder="XX:XX:XX:XX:XX:XX" 
                       pattern="([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}">
            </div>
            
            <button id="connectBtn" class="btn" onclick="connectManual()">Connect</button>
            <button id="disconnectBtn" class="btn btn-danger hidden" onclick="disconnect()">Disconnect</button>
            
            <div id="loading" class="hidden" style="text-align:center;padding:20px;">
                <p>Connecting...</p>
            </div>
        </div>
    </div>
    
    <script>
        function updateStatus() {
            fetch('/status')
                .then(r => r.json())
                .then(data => {
                    const statusBox = document.getElementById('statusBox');
                    const statusText = document.getElementById('statusText');
                    const connectedDevice = document.getElementById('connectedDevice');
                    const connectBtn = document.getElementById('connectBtn');
                    const disconnectBtn = document.getElementById('disconnectBtn');
                    
                    if (data.connected) {
                        statusBox.className = 'status-box connected';
                        statusText.textContent = 'Connected';
                        connectedDevice.innerHTML = '<strong>' + data.name + '</strong><br>' + data.address;
                        connectBtn.classList.add('hidden');
                        disconnectBtn.classList.remove('hidden');
                    } else {
                        statusBox.className = 'status-box disconnected';
                        statusText.textContent = 'Not Connected';
                        connectedDevice.innerHTML = '';
                        connectBtn.classList.remove('hidden');
                        disconnectBtn.classList.add('hidden');
                    }
                });
        }
        
        function connectManual() {
            const address = document.getElementById('btAddress').value.trim();
            if (!address) {
                alert('Please enter a Bluetooth MAC address');
                return;
            }
            
            if (!/^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$/.test(address)) {
                alert('Invalid MAC address format. Use XX:XX:XX:XX:XX:XX');
                return;
            }
            
            document.getElementById('loading').classList.remove('hidden');
            document.getElementById('connectBtn').disabled = true;
            
            fetch('/connect', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ address: address })
            })
            .then(r => r.json())
            .then(data => {
                document.getElementById('loading').classList.add('hidden');
                document.getElementById('connectBtn').disabled = false;
                if (data.success) {
                    updateStatus();
                } else {
                    alert('Connection failed: ' + data.error);
                }
            })
            .catch(err => {
                document.getElementById('loading').classList.add('hidden');
                document.getElementById('connectBtn').disabled = false;
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
    if (!bluetooth_connected_) return false;
    return serial_port_.write(data, size) == size;
}

size_t BluetoothConfig::readFromBluetooth(uint8_t* buffer, size_t size) {
    if (!bluetooth_connected_) return 0;
    return serial_port_.readBytes(buffer, size);
}

bool BluetoothConfig::isBluetoothDataAvailable() {
    if (!bluetooth_connected_) return false;
    return serial_port_.available() > 0;
}

bool BluetoothConfig::startAccessPoint() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    
    bool result = WiFi.softAP(ap_ssid_.c_str(), ap_password_.c_str());
    ap_started_ = result;
    return result;
}

void BluetoothConfig::stopAccessPoint() {
    if (ap_started_) {
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_OFF);
        ap_started_ = false;
    }
}

void BluetoothConfig::setupWebServer() {
    web_server_.on("/", HTTP_GET, [this]() { handleRoot(); });
    web_server_.on("/scan", HTTP_POST, [this]() { handleScan(); });
    web_server_.on("/connect", HTTP_POST, [this]() { handleConnect(); });
    web_server_.on("/disconnect", HTTP_POST, [this]() { handleDisconnect(); });
    web_server_.on("/status", HTTP_GET, [this]() { handleStatus(); });
    web_server_.onNotFound([this]() { handleNotFound(); });
    web_server_.begin();
}

void BluetoothConfig::handleRoot() {
    web_server_.send(200, "text/html", getConfigPageHTML());
}

void BluetoothConfig::handleScan() {
    bool success = scanForBluetoothDevices(5);
    
    String json = "{\"success\":" + String(success ? "true" : "false") + 
                  ",\"devices\":" + getScannedDevicesJSON() + "}";
    web_server_.send(200, "application/json", json);
}

void BluetoothConfig::handleConnect() {
    if (!web_server_.hasArg("plain")) {
        web_server_.send(400, "application/json", "{\"success\":false,\"error\":\"No data\"}");
        return;
    }
    
    String body = web_server_.arg("plain");
    int addrStart = body.indexOf("\"address\":\"") + 11;
    int addrEnd = body.indexOf("\"", addrStart);
    String address = body.substring(addrStart, addrEnd);
    
    bool success = connectToBluetooth(address);
    
    if (success) {
        web_server_.send(200, "application/json", "{\"success\":true}");
    } else {
        web_server_.send(200, "application/json", "{\"success\":false,\"error\":\"Connection failed\"}");
    }
}

void BluetoothConfig::handleDisconnect() {
    disconnectBluetooth();
    web_server_.send(200, "application/json", "{\"success\":true}");
}

void BluetoothConfig::handleStatus() {
    String json = "{";
    json += "\"connected\":" + String(bluetooth_connected_ ? "true" : "false");
    json += ",\"address\":\"" + connected_bluetooth_address_ + "\"";
    json += ",\"name\":\"" + connected_bluetooth_name_ + "\"";
    json += ",\"deviceCount\":" + String(scanned_devices_.size());
    json += ",\"devices\":" + getScannedDevicesJSON();
    json += "}";
    web_server_.send(200, "application/json", json);
}

void BluetoothConfig::handleData() {
    web_server_.send(200, "application/json", "{\"available\":" + String(isBluetoothDataAvailable() ? "true" : "false") + "}");
}

void BluetoothConfig::handleNotFound() {
    web_server_.send(404, "text/plain", "Not Found");
}
