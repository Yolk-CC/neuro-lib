#include <NeuroLib.h>

NeuroLib neuro;

#define NUM_SAMPLES 64
float eegBuffer[NUM_SAMPLES];
FFTResult fftResult[NUM_SAMPLES / 2];

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ;
    }
    
    Serial.println("NeuroLib - Basic EEG Reader");
    Serial.println("==========================");
    
    if (!neuro.begin(115200, Serial)) {
        Serial.println("Failed to initialize NeuroLib!");
        while (1) {
            delay(1000);
        }
    }
    
    neuro.setSampleRate(250.0);
    neuro.setNumSamples(NUM_SAMPLES);
    neuro.setNumChannels(1);
    
    Serial.print("Sample Rate: ");
    Serial.print(neuro.getSampleRate());
    Serial.println(" Hz");
    Serial.print("Number of Samples: ");
    Serial.println(neuro.getNumSamples());
    Serial.println("Ready to receive EEG data...");
    Serial.println("Expected format: value,channel,timestamp");
}

void loop() {
    if (neuro.dataAvailable()) {
        EEGData data = neuro.readData();
        
        Serial.print("Value: ");
        Serial.print(data.value, 4);
        Serial.print(" | Channel: ");
        Serial.print(data.channel);
        Serial.print(" | Time: ");
        Serial.println(data.timestamp);
    }
    
    delay(10);
}
