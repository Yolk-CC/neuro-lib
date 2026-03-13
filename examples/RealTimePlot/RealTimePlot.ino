#include <NeuroLib.h>

NeuroLib neuro;

#define NUM_SAMPLES 64
float eegBuffer[NUM_SAMPLES];
FFTResult fftResult[NUM_SAMPLES / 2];

unsigned long lastPlotTime = 0;
const unsigned long plotInterval = 100;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ;
    }
    
    Serial.println("NeuroLib - Real-time Plotting");
    Serial.println("=============================");
    
    if (!neuro.begin(115200, Serial)) {
        Serial.println("Failed to initialize NeuroLib!");
        while (1) {
            delay(1000);
        }
    }
    
    neuro.setSampleRate(250.0);
    neuro.setNumSamples(NUM_SAMPLES);
    neuro.setNumChannels(1);
    
    Serial.println("READY");
    Serial.flush();
}

void loop() {
    static int sampleCount = 0;
    
    if (neuro.dataAvailable()) {
        EEGData data = neuro.readData();
        eegBuffer[sampleCount++] = data.value;
        
        if (millis() - lastPlotTime >= plotInterval) {
            Serial.print("RAW:");
            Serial.print(data.value, 4);
            Serial.print(",");
            Serial.println(data.timestamp);
            
            lastPlotTime = millis();
        }
        
        if (sampleCount >= NUM_SAMPLES) {
            neuro.computeFFT(eegBuffer, fftResult, NUM_SAMPLES);
            
            BrainwavePower power = neuro.calculateBrainwavePower(fftResult, NUM_SAMPLES / 2);
            
            Serial.print("FFT:");
            Serial.print(power.delta, 4);
            Serial.print(",");
            Serial.print(power.theta, 4);
            Serial.print(",");
            Serial.print(power.alpha, 4);
            Serial.print(",");
            Serial.print(power.beta, 4);
            Serial.print(",");
            Serial.println(power.gamma, 4);
            
            sampleCount = 0;
        }
    }
}
