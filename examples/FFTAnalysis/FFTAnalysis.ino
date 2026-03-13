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
    
    Serial.println("NeuroLib - FFT Analysis Example");
    Serial.println("==============================");
    
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
    Serial.print("FFT Size: ");
    Serial.println(neuro.getNumSamples());
    Serial.print("Frequency Resolution: ");
    Serial.print(neuro.getSampleRate() / neuro.getNumSamples(), 2);
    Serial.println(" Hz");
    Serial.println();
    Serial.println("Collecting EEG data for FFT analysis...");
}

void loop() {
    static int sampleCount = 0;
    
    if (neuro.dataAvailable()) {
        EEGData data = neuro.readData();
        eegBuffer[sampleCount++] = data.value;
        
        if (sampleCount >= NUM_SAMPLES) {
            Serial.println("\n=== FFT Analysis Results ===");
            
            neuro.computeFFT(eegBuffer, fftResult, NUM_SAMPLES);
            
            BrainwavePower power = neuro.calculateBrainwavePower(fftResult, NUM_SAMPLES / 2);
            
            Serial.print("Delta (0.5-4 Hz):   ");
            Serial.print(power.delta, 4);
            Serial.println(" uV^2");
            
            Serial.print("Theta (4-8 Hz):     ");
            Serial.print(power.theta, 4);
            Serial.println(" uV^2");
            
            Serial.print("Alpha (8-13 Hz):    ");
            Serial.print(power.alpha, 4);
            Serial.println(" uV^2");
            
            Serial.print("Beta (13-30 Hz):    ");
            Serial.print(power.beta, 4);
            Serial.println(" uV^2");
            
            Serial.print("Gamma (30-100 Hz):  ");
            Serial.print(power.gamma, 4);
            Serial.println(" uV^2");
            
            float dominantFreq = neuro.getDominantFrequency(fftResult, NUM_SAMPLES / 2);
            Serial.print("\nDominant Frequency: ");
            Serial.print(dominantFreq, 2);
            Serial.println(" Hz");
            
            Serial.println("\nTop 5 Frequency Components:");
            Serial.println("Freq (Hz)\tMagnitude");
            
            int count = 0;
            for (int i = 1; i < NUM_SAMPLES / 2 && count < 5; i++) {
                if (fftResult[i].magnitude > 0.1) {
                    Serial.print(fftResult[i].frequency, 2);
                    Serial.print("\t\t");
                    Serial.println(fftResult[i].magnitude, 4);
                    count++;
                }
            }
            
            Serial.println("============================\n");
            
            sampleCount = 0;
            delay(1000);
        }
    }
}
