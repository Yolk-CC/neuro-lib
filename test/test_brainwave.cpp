#include <Arduino.h>
#include <unity.h>
#include <NeuroLib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NUM_SAMPLES 64
#define SAMPLE_RATE 250.0

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

void test_brainwave_delta_band() {
    float frequency = 2.0f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = sinf(2.0f * M_PI * frequency * i / SAMPLE_RATE);
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    BrainwavePower power = neuro.calculateBrainwavePower(fftResult, NUM_SAMPLES / 2);
    
    TEST_ASSERT_GREATER_THAN(power.theta, power.delta);
    TEST_ASSERT_GREATER_THAN(power.alpha, power.delta);
    TEST_ASSERT_GREATER_THAN(power.beta, power.delta);
    TEST_ASSERT_GREATER_THAN(power.gamma, power.delta);
    
    float deltaPower = neuro.getBandPower(fftResult, NUM_SAMPLES / 2, DELTA);
    TEST_ASSERT_GREATER_THAN(0.0f, deltaPower);
}

void test_brainwave_theta_band() {
    float frequency = 6.0f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = sinf(2.0f * M_PI * frequency * i / SAMPLE_RATE);
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    BrainwavePower power = neuro.calculateBrainwavePower(fftResult, NUM_SAMPLES / 2);
    
    TEST_ASSERT_GREATER_THAN(power.delta, power.theta);
    TEST_ASSERT_GREATER_THAN(power.alpha, power.theta);
    TEST_ASSERT_GREATER_THAN(power.beta, power.theta);
    TEST_ASSERT_GREATER_THAN(power.gamma, power.theta);
    
    float thetaPower = neuro.getBandPower(fftResult, NUM_SAMPLES / 2, THETA);
    TEST_ASSERT_GREATER_THAN(0.0f, thetaPower);
}

void test_brainwave_alpha_band() {
    float frequency = 10.0f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = sinf(2.0f * M_PI * frequency * i / SAMPLE_RATE);
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    BrainwavePower power = neuro.calculateBrainwavePower(fftResult, NUM_SAMPLES / 2);
    
    TEST_ASSERT_GREATER_THAN(power.alpha, power.delta);
    TEST_ASSERT_GREATER_THAN(power.alpha, power.theta);
    TEST_ASSERT_GREATER_THAN(power.alpha, power.beta);
    
    float alphaPower = neuro.getBandPower(fftResult, NUM_SAMPLES / 2, ALPHA);
    TEST_ASSERT_GREATER_THAN(0.0f, alphaPower);
}

void test_brainwave_beta_band() {
    float frequency = 20.0f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = sinf(2.0f * M_PI * frequency * i / SAMPLE_RATE);
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    BrainwavePower power = neuro.calculateBrainwavePower(fftResult, NUM_SAMPLES / 2);
    
    TEST_ASSERT_GREATER_THAN(power.beta, power.delta);
    TEST_ASSERT_GREATER_THAN(power.beta, power.theta);
    TEST_ASSERT_GREATER_THAN(power.beta, power.alpha);
    
    float betaPower = neuro.getBandPower(fftResult, NUM_SAMPLES / 2, BETA);
    TEST_ASSERT_GREATER_THAN(0.0f, betaPower);
}

void test_brainwave_gamma_band() {
    float frequency = 40.0f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = sinf(2.0f * M_PI * frequency * i / SAMPLE_RATE);
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    BrainwavePower power = neuro.calculateBrainwavePower(fftResult, NUM_SAMPLES / 2);
    
    TEST_ASSERT_GREATER_THAN(power.gamma, power.delta);
    TEST_ASSERT_GREATER_THAN(power.gamma, power.theta);
    TEST_ASSERT_GREATER_THAN(power.gamma, power.alpha);
    TEST_ASSERT_GREATER_THAN(power.gamma, power.beta);
    
    float gammaPower = neuro.getBandPower(fftResult, NUM_SAMPLES / 2, GAMMA);
    TEST_ASSERT_GREATER_THAN(0.0f, gammaPower);
}

void test_brainwave_band_boundaries() {
    float testFreqs[] = {0.5f, 4.0f, 8.0f, 13.0f, 30.0f};
    BrainwaveBand expectedBands[] = {DELTA, DELTA, THETA, ALPHA, BETA};
    
    for (int t = 0; t < 5; t++) {
        for (int i = 0; i < NUM_SAMPLES; i++) {
            testBuffer[i] = sinf(2.0f * M_PI * testFreqs[t] * i / SAMPLE_RATE);
        }
        
        neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
        
        BrainwavePower power = neuro.calculateBrainwavePower(fftResult, NUM_SAMPLES / 2);
        
        float maxPower = -1.0f;
        BrainwaveBand dominantBand = DELTA;
        
        if (power.delta > maxPower) {
            maxPower = power.delta;
            dominantBand = DELTA;
        }
        if (power.theta > maxPower) {
            maxPower = power.theta;
            dominantBand = THETA;
        }
        if (power.alpha > maxPower) {
            maxPower = power.alpha;
            dominantBand = ALPHA;
        }
        if (power.beta > maxPower) {
            maxPower = power.beta;
            dominantBand = BETA;
        }
        
        if (testFreqs[t] < 30.0f) {
            TEST_ASSERT_INT_WITHIN(1, expectedBands[t], dominantBand);
        }
    }
}

void test_brainwave_alpha_beta_ratio() {
    float alphaFreq = 10.0f;
    float betaFreq = 20.0f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = 2.0f * sinf(2.0f * M_PI * alphaFreq * i / SAMPLE_RATE) +
                       0.5f * sinf(2.0f * M_PI * betaFreq * i / SAMPLE_RATE);
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    BrainwavePower power = neuro.calculateBrainwavePower(fftResult, NUM_SAMPLES / 2);
    
    float alphaBetaRatio = power.alpha / (power.beta + 0.0001f);
    
    TEST_ASSERT_GREATER_THAN(1.0f, alphaBetaRatio);
    
#ifdef VERBOSE_TEST
    Serial.print("Alpha/Beta Ratio: ");
    Serial.println(alphaBetaRatio);
#endif
}

void test_brainwave_mixed_frequencies() {
    float freqs[] = {2.0f, 6.0f, 10.0f, 20.0f, 40.0f};
    float amplitudes[] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    int numFreqs = 5;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = 0.0f;
        for (int f = 0; f < numFreqs; f++) {
            testBuffer[i] += amplitudes[f] * sinf(2.0f * M_PI * freqs[f] * i / SAMPLE_RATE);
        }
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    BrainwavePower power = neuro.calculateBrainwavePower(fftResult, NUM_SAMPLES / 2);
    
    TEST_ASSERT_GREATER_THAN(0.0f, power.delta);
    TEST_ASSERT_GREATER_THAN(0.0f, power.theta);
    TEST_ASSERT_GREATER_THAN(0.0f, power.alpha);
    TEST_ASSERT_GREATER_THAN(0.0f, power.beta);
    TEST_ASSERT_GREATER_THAN(0.0f, power.gamma);
    
    float totalPower = power.delta + power.theta + power.alpha + power.beta + power.gamma;
    TEST_ASSERT_GREATER_THAN(0.0f, totalPower);
}

void test_brainwave_dominant_frequency_detection() {
    float dominantFreq = 12.0f;
    float weakFreq = 3.0f;
    
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = 3.0f * sinf(2.0f * M_PI * dominantFreq * i / SAMPLE_RATE) +
                       0.3f * sinf(2.0f * M_PI * weakFreq * i / SAMPLE_RATE);
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    float detectedFreq = neuro.getDominantFrequency(fftResult, NUM_SAMPLES / 2);
    
    TEST_ASSERT_FLOAT_WITHIN(2.0f, dominantFreq, detectedFreq);
    
    BrainwavePower power = neuro.calculateBrainwavePower(fftResult, NUM_SAMPLES / 2);
    TEST_ASSERT_GREATER_THAN(power.alpha, power.delta);
}

void test_brainwave_zero_signal() {
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = 0.0f;
    }
    
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    BrainwavePower power = neuro.calculateBrainwavePower(fftResult, NUM_SAMPLES / 2);
    
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, power.delta);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, power.theta);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, power.alpha);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, power.beta);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, power.gamma);
}

void test_brainwave_frequency_sweep() {
    float startFreq = 1.0f;
    float endFreq = 50.0f;
    int steps = 10;
    
    for (int s = 0; s < steps; s++) {
        float freq = startFreq + (endFreq - startFreq) * s / (steps - 1);
        
        for (int i = 0; i < NUM_SAMPLES; i++) {
            testBuffer[i] = sinf(2.0f * M_PI * freq * i / SAMPLE_RATE);
        }
        
        neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
        
        float detectedFreq = neuro.getDominantFrequency(fftResult, NUM_SAMPLES / 2);
        
        TEST_ASSERT_FLOAT_WITHIN(3.0f, freq, detectedFreq);
    }
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    
    RUN_TEST(test_brainwave_delta_band);
    RUN_TEST(test_brainwave_theta_band);
    RUN_TEST(test_brainwave_alpha_band);
    RUN_TEST(test_brainwave_beta_band);
    RUN_TEST(test_brainwave_gamma_band);
    RUN_TEST(test_brainwave_band_boundaries);
    RUN_TEST(test_brainwave_alpha_beta_ratio);
    RUN_TEST(test_brainwave_mixed_frequencies);
    RUN_TEST(test_brainwave_dominant_frequency_detection);
    RUN_TEST(test_brainwave_zero_signal);
    RUN_TEST(test_brainwave_frequency_sweep);
    
    UNITY_END();
}

void loop() {
}
