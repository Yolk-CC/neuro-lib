/*
 * EEG Signal Acquisition with Bluetooth Configuration
 * 
 * This example demonstrates:
 * 1. Starting a WiFi access point for configuration
 * 2. Web-based Bluetooth device scanning and connection
 * 3. Receiving EEG data via Bluetooth
 * 4. Real-time FFT analysis of EEG signals
 * 
 * Usage:
 * 1. Upload this sketch to your ESP32
 * 2. Connect to WiFi network "NeuroLib_Config" (password: neurolib123)
 * 3. Open browser and go to 192.168.4.1
 * 4. Scan for Bluetooth devices and connect to your EEG device
 * 5. View FFT analysis results in Serial Monitor
 * 
 * Author: NeuroLib
 * License: MIT
 */

#include <Arduino.h>
#include <BluetoothConfig.h>
#include <NeuroLib.h>


// Configuration
#define BUFFER_SIZE 64          // FFT buffer size (must be power of 2)
#define SAMPLE_RATE 250.0f      // Sample rate in Hz
#define NUM_CHANNELS 1          // Number of EEG channels

// Global objects
BluetoothConfig btConfig;
NeuroLib neuroLib;

// FFT and analysis buffers
float fftInput[BUFFER_SIZE];
FFTResult fftOutput[BUFFER_SIZE / 2];
BrainwavePower brainwavePower;

// Data buffer
float dataBuffer[BUFFER_SIZE];
int bufferIndex = 0;
bool bufferReady = false;

// Statistics
unsigned long totalSamples = 0;
unsigned long lastReportTime = 0;
int samplesInLastSecond = 0;

// Function prototypes
void processEEGData(float value);
void performFFTAnalysis();
void printBrainwaveAnalysis();
void printFFTResult();
void setupNeuroLib();

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    
    Serial.println();
    Serial.println("========================================");
    Serial.println("  NeuroLib EEG with Bluetooth Config");
    Serial.println("========================================");
    Serial.println();
    
    // Initialize Bluetooth configuration
    Serial.print("Starting Bluetooth configuration... ");
    if (!btConfig.begin()) {
        Serial.println("Failed to start Bluetooth config!");
        Serial.println("Will continue in standalone mode");
    } else {
        Serial.println("Success!");
        Serial.println();
        Serial.println("WiFi Access Point started");
        Serial.print("SSID: ");
        Serial.println(DEFAULT_AP_SSID);
        Serial.print("Password: ");
        Serial.println(DEFAULT_AP_PASSWORD);
        Serial.print("Web Interface: http://192.168.4.1");
        Serial.println();
    }
    
    // Initialize NeuroLib
    setupNeuroLib();
    
    Serial.println();
    Serial.println("System ready!");
    Serial.println("Connect to your EEG device via the web interface");
    Serial.println();
}

void loop() {
    // Handle Bluetooth configuration web server
    btConfig.handleClient();
    
    // Check if Bluetooth is connected and data is available
    if (btConfig.isBluetoothConnected() && btConfig.isBluetoothDataAvailable()) {
        // Read data from Bluetooth
        uint8_t buffer[256];
        size_t bytesRead = btConfig.readFromBluetooth(buffer, sizeof(buffer));
        
        // Process received data
        for (size_t i = 0; i < bytesRead; i++) {
            // Assuming EEG data comes as float values
            // You may need to adjust this based on your EEG device's data format
            static uint8_t dataBuffer[4];
            static int dataIndex = 0;
            
            dataBuffer[dataIndex++] = buffer[i];
            
            if (dataIndex == 4) {
                float value = *((float*)dataBuffer);
                processEEGData(value);
                dataIndex = 0;
            }
        }
    }
    
    // Check if FFT buffer is ready
    if (bufferReady) {
        bufferReady = false;
        performFFTAnalysis();
    }
    
    // Print statistics every second
    unsigned long currentTime = millis();
    if (currentTime - lastReportTime >= 1000) {
        Serial.print("Samples/sec: ");
        Serial.println(samplesInLastSecond);
        samplesInLastSecond = 0;
        lastReportTime = currentTime;
    }
    
    delay(1);
}

void setupNeuroLib() {
    // Configure NeuroLib parameters
    neuroLib.setSampleRate(SAMPLE_RATE);
    neuroLib.setNumSamples(BUFFER_SIZE);
    neuroLib.setNumChannels(NUM_CHANNELS);
    
    Serial.print("NeuroLib configured: ");
    Serial.print(SAMPLE_RATE);
    Serial.print(" Hz, ");
    Serial.print(BUFFER_SIZE);
    Serial.print(" samples, ");
    Serial.print(NUM_CHANNELS);
    Serial.println(" channel(s)");
}

void processEEGData(float value) {
    // Add value to data buffer
    dataBuffer[bufferIndex++] = value;
    totalSamples++;
    samplesInLastSecond++;
    
    // When buffer is full, perform FFT
    if (bufferIndex >= BUFFER_SIZE) {
        bufferIndex = 0;
        bufferReady = true;
    }
}

void performFFTAnalysis() {
    // Copy data to FFT input buffer
    for (int i = 0; i < BUFFER_SIZE; i++) {
        fftInput[i] = dataBuffer[i];
    }
    
    // Compute FFT using NeuroLib
    neuroLib.computeFFT(fftInput, fftOutput, BUFFER_SIZE);
    
    // Calculate brainwave power in different frequency bands
    brainwavePower = neuroLib.calculateBrainwavePower(fftOutput, BUFFER_SIZE);
    
    // Print analysis results
    printBrainwaveAnalysis();
    
    // Optionally print detailed FFT results
    // printFFTResult();
}

void printBrainwaveAnalysis() {
    Serial.println();
    Serial.println("=== Brainwave Power Analysis ===");
    Serial.print("Delta (0.5-4 Hz):   ");
    Serial.print(brainwavePower.delta, 4);
    Serial.println(" uV^2");
    Serial.print("Theta (4-8 Hz):     ");
    Serial.print(brainwavePower.theta, 4);
    Serial.println(" uV^2");
    Serial.print("Alpha (8-13 Hz):    ");
    Serial.print(brainwavePower.alpha, 4);
    Serial.println(" uV^2");
    Serial.print("Beta (13-30 Hz):    ");
    Serial.print(brainwavePower.beta, 4);
    Serial.println(" uV^2");
    Serial.print("Gamma (30-100 Hz):  ");
    Serial.print(brainwavePower.gamma, 4);
    Serial.println(" uV^2");
    
    // Calculate and print dominant frequency
    float dominantFreq = neuroLib.getDominantFrequency(fftOutput, BUFFER_SIZE);
    Serial.print("Dominant Frequency: ");
    Serial.print(dominantFreq, 2);
    Serial.println(" Hz");
    
    // Determine brain state
    String brainState = "Unknown";
    float maxPower = 0;
    
    if (brainwavePower.alpha > maxPower) {
        maxPower = brainwavePower.alpha;
        brainState = "Relaxed/Alert";
    }
    if (brainwavePower.beta > maxPower) {
        maxPower = brainwavePower.beta;
        brainState = "Active/Focused";
    }
    if (brainwavePower.theta > maxPower) {
        maxPower = brainwavePower.theta;
        brainState = "Drowsy/Meditative";
    }
    if (brainwavePower.delta > maxPower) {
        maxPower = brainwavePower.delta;
        brainState = "Deep Sleep";
    }
    if (brainwavePower.gamma > maxPower) {
        maxPower = brainwavePower.gamma;
        brainState = "High-level Processing";
    }
    
    Serial.print("Brain State: ");
    Serial.println(brainState);
    Serial.println("================================");
    Serial.println();
}

void printFFTResult() {
    Serial.println("=== FFT Detailed Results ===");
    Serial.println("Freq (Hz)\tMagnitude\tPhase");
    
    // Print first 20 frequency bins
    for (int i = 0; i < 20 && i < BUFFER_SIZE / 2; i++) {
        Serial.print(fftOutput[i].frequency, 2);
        Serial.print("\t\t");
        Serial.print(fftOutput[i].magnitude, 4);
        Serial.print("\t\t");
        Serial.println(fftOutput[i].phase, 4);
    }
    Serial.println("============================");
}
