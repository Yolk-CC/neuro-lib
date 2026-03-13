#include <NeuroLib.h>

NeuroLib neuro;

#define NUM_SAMPLES 64
#define NUM_CHANNELS 2
float channelBuffers[NUM_CHANNELS][NUM_SAMPLES];
FFTResult fftResult[NUM_SAMPLES / 2];

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ;
    }
    
    Serial.println("NeuroLib - Multi-Channel EEG");
    Serial.println("============================");
    
    if (!neuro.begin(115200, Serial)) {
        Serial.println("Failed to initialize NeuroLib!");
        while (1) {
            delay(1000);
        }
    }
    
    neuro.setSampleRate(250.0);
    neuro.setNumSamples(NUM_SAMPLES);
    neuro.setNumChannels(NUM_CHANNELS);
    
    Serial.print("Sample Rate: ");
    Serial.print(neuro.getSampleRate());
    Serial.println(" Hz");
    Serial.print("Number of Channels: ");
    Serial.println(neuro.getNumChannels());
    Serial.println();
    Serial.println("Monitoring multiple EEG channels...");
}

void loop() {
    for (uint8_t ch = 0; ch < NUM_CHANNELS; ch++) {
        int samplesRead = neuro.readChannel(ch, channelBuffers[ch], NUM_SAMPLES);
        
        if (samplesRead >= NUM_SAMPLES) {
            Serial.print("\n=== Channel ");
            Serial.print(ch);
            Serial.println(" Analysis ===");
            
            neuro.computeFFT(channelBuffers[ch], fftResult, NUM_SAMPLES);
            
            BrainwavePower power = neuro.calculateBrainwavePower(fftResult, NUM_SAMPLES / 2);
            
            Serial.print("Alpha Power: ");
            Serial.print(power.alpha, 4);
            Serial.print(" | Beta Power: ");
            Serial.print(power.beta, 4);
            Serial.println(" uV^2");
            
            float alphaRatio = power.alpha / (power.alpha + power.beta + 0.0001);
            Serial.print("Alpha/(Alpha+Beta) Ratio: ");
            Serial.println(alphaRatio, 4);
        }
    }
    
    delay(500);
}
