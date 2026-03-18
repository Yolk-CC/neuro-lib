#ifndef BLUETOOTH_CONFIG_H
#define BLUETOOTH_CONFIG_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <BluetoothSerial.h>
#include <vector>

#define DEFAULT_AP_SSID "NeuroLib_Config"
#define DEFAULT_AP_PASSWORD "neurolib123"
#define DEFAULT_AP_IP "192.168.4.1"

struct BluetoothDevice {
    String name;
    String address;
    int rssi;
};

class BluetoothConfig {
public:
    BluetoothConfig();
    ~BluetoothConfig();
    
    bool begin(const String& apSsid = DEFAULT_AP_SSID, 
               const String& apPassword = DEFAULT_AP_PASSWORD);
    void end();
    
    void handleClient();
    
    bool isBluetoothConnected() const;
    String getConnectedBluetoothAddress() const;
    String getConnectedBluetoothName() const;
    
    bool connectToBluetooth(const String& address);
    void disconnectBluetooth();
    
    bool scanForBluetoothDevices(int scanTime = 5);
    size_t getScannedDeviceCount() const;
    BluetoothDevice getScannedDevice(size_t index) const;
    
    String getScannedDevicesJSON() const;
    String getConfigPageHTML() const;
    
    bool writeToBluetooth(const uint8_t* data, size_t size);
    size_t readFromBluetooth(uint8_t* buffer, size_t size);
    bool isBluetoothDataAvailable() const;
    
private:
    WebServer webServer_;
    BluetoothSerial serialPort_;
    
    bool apStarted_;
    bool bluetoothConnected_;
    String connectedBluetoothAddress_;
    String connectedBluetoothName_;
    
    std::vector<BluetoothDevice> scannedDevices_;
    
    String apSsid_;
    String apPassword_;
    
    void setupWebServer();
    void handleRoot();
    void handleScan();
    void handleConnect();
    void handleDisconnect();
    void handleStatus();
    void handleData();
    void handleNotFound();
    
    bool startAccessPoint();
    void stopAccessPoint();
};

#endif // BLUETOOTH_CONFIG_H
