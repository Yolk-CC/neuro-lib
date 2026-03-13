#include <Arduino.h>
#include <unity.h>
#include <NeuroLib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NUM_SAMPLES 64
#define SAMPLE_RATE 250.0
#define EPSILON 0.01

NeuroLib neuro;

float testBuffer[NUM_SAMPLES];
FFTResult fftResult[NUM_SAMPLES / 2];

void setUp(void) {
    neuro.begin(115200, Serial);
    neuro.setSampleRate(SAMPLE_RATE);
    neuro.setNumSamples(NUM_SAMPLES);
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = 0.0f;
    }
}

void tearDown(void) {
    neuro.end();
}

void test_fft_single_frequency() {
    float frequency = 10.0f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = sinf(2.0f * M_PI * frequency * i / SAMPLE_RATE);
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    float dominantFreq = neuro.getDominantFrequency(fftResult, NUM_SAMPLES / 2);
    
    TEST_ASSERT_FLOAT_WITHIN(2.0f, frequency, dominantFreq);
}

void test_fft_amplitude_accuracy() {
    float frequency = 8.0f;
    float amplitude = 5.0f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = amplitude * sinf(2.0f * M_PI * frequency * i / SAMPLE_RATE);
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    int peakIndex = 0;
    float maxMagnitude = 0.0f;
    for (int i = 1; i < NUM_SAMPLES / 2; i++) {
        if (fftResult[i].magnitude > maxMagnitude) {
            maxMagnitude = fftResult[i].magnitude;
            peakIndex = i;
        }
    }
    
    float expectedFreq = frequency;
    float actualFreq = fftResult[peakIndex].frequency;
    
    TEST_ASSERT_FLOAT_WITHIN(2.0f, expectedFreq, actualFreq);
    TEST_ASSERT_GREATER_THAN(amplitude * 0.3, maxMagnitude);
}

void test_fft_multiple_frequencies() {
    float freq1 = 5.0f;
    float freq2 = 15.0f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = sinf(2.0f * M_PI * freq1 * i / SAMPLE_RATE) +
                       0.5f * sinf(2.0f * M_PI * freq2 * i / SAMPLE_RATE);
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    BrainwavePower power = neuro.calculateBrainwavePower(fftResult, NUM_SAMPLES / 2);
    
    TEST_ASSERT_GREATER_THAN(0.0f, power.delta);
    TEST_ASSERT_GREATER_THAN(0.0f, power.theta);
    TEST_ASSERT_GREATER_THAN(0.0f, power.beta);
}

void test_fft_frequency_resolution() {
    float resolution = SAMPLE_RATE / NUM_SAMPLES;
    
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 3.90625f, resolution);
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = sinf(2.0f * M_PI * resolution * i / SAMPLE_RATE);
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    float dominantFreq = neuro.getDominantFrequency(fftResult, NUM_SAMPLES / 2);
    
    TEST_ASSERT_FLOAT_WITHIN(1.0f, resolution, dominantFreq);
}

void test_fft_dc_component() {
    float dcOffset = 2.5f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = dcOffset;
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    TEST_ASSERT_FLOAT_WITHIN(0.5f, dcOffset * NUM_SAMPLES / 2, fftResult[0].magnitude);
    
    for (int i = 1; i < NUM_SAMPLES / 2; i++) {
        TEST_ASSERT_FLOAT_WITHIN(0.1f, 0.0f, fftResult[i].magnitude);
    }
}

void test_fft_phase_shift() {
    float frequency = 10.0f;
    float phaseShift = M_PI / 4.0f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = sinf(2.0f * M_PI * frequency * i / SAMPLE_RATE + phaseShift);
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    int peakIndex = 0;
    float maxMagnitude = 0.0f;
    for (int i = 1; i < NUM_SAMPLES / 2; i++) {
        if (fftResult[i].magnitude > maxMagnitude) {
            maxMagnitude = fftResult[i].magnitude;
            peakIndex = i;
        }
    }
    
    TEST_ASSERT_FLOAT_WITHIN(2.0f, frequency, fftResult[peakIndex].frequency);
}

void test_fft_noise_tolerance() {
    float frequency = 12.0f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        float noise = ((float)rand() / RAND_MAX - 0.5f) * 0.2f;
        testBuffer[i] = sinf(2.0f * M_PI * frequency * i / SAMPLE_RATE) + noise;
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    float dominantFreq = neuro.getDominantFrequency(fftResult, NUM_SAMPLES / 2);
    
    TEST_ASSERT_FLOAT_WITHIN(3.0f, frequency, dominantFreq);
}

void test_fft_different_sample_sizes() {
    uint16_t sizes[] = {16, 32, 64, 128};
    float frequency = 10.0f;
    
    for (int s = 0; s < 4; s++) {
        neuro.setNumSamples(sizes[s]);
        
        for (int i = 0; i < sizes[s]; i++) {
            testBuffer[i] = sinf(2.0f * M_PI * frequency * i / SAMPLE_RATE);
        }
        
        neuro.computeFFT(testBuffer, fftResult, sizes[s]);
        
        float dominantFreq = neuro.getDominantFrequency(fftResult, sizes[s] / 2);
        
        TEST_ASSERT_FLOAT_WITHIN(3.0f, frequency, dominantFreq);
    }
    
    neuro.setNumSamples(NUM_SAMPLES);
}

void test_fft_sample_rate_changes() {
    float frequency = 10.0f;
    float sampleRates[] = {125.0f, 250.0f, 500.0f};
    
    for (int s = 0; s < 3; s++) {
        neuro.setSampleRate(sampleRates[s]);
        
        for (int i = 0; i < NUM_SAMPLES; i++) {
            testBuffer[i] = sinf(2.0f * M_PI * frequency * i / sampleRates[s]);
        }
        
        neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
        
        float dominantFreq = neuro.getDominantFrequency(fftResult, NUM_SAMPLES / 2);
        
        TEST_ASSERT_FLOAT_WITHIN(3.0f, frequency, dominantFreq);
    }
    
    neuro.setSampleRate(SAMPLE_RATE);
}

void test_fft_nyquist_frequency() {
    float nyquistFreq = SAMPLE_RATE / 2.0f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = sinf(2.0f * M_PI * nyquistFreq * i / SAMPLE_RATE);
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    float maxFreq = fftResult[NUM_SAMPLES / 2 - 1].frequency;
    
    TEST_ASSERT_FLOAT_WITHIN(5.0f, nyquistFreq, maxFreq);
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_fft_single_frequency);
    RUN_TEST(test_fft_amplitude_accuracy);
    RUN_TEST(test_fft_multiple_frequencies);
    RUN_TEST(test_fft_frequency_resolution);
    RUN_TEST(test_fft_dc_component);
    RUN_TEST(test_fft_phase_shift);
    RUN_TEST(test_fft_noise_tolerance);
    RUN_TEST(test_fft_different_sample_sizes);
    RUN_TEST(test_fft_sample_rate_changes);
    RUN_TEST(test_fft_nyquist_frequency);
    
    UNITY_END();
}

void loop() {
}
