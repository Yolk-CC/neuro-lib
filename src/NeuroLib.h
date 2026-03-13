#ifndef NEUROLIB_H
#define NEUROLIB_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include <ArduinoFFT.h>
#include <vector>

#define NEUROLIB_VERSION "1.0.0"

#define DEFAULT_SAMPLE_RATE 250.0
#define DEFAULT_NUM_SAMPLES 64
#define DEFAULT_NUM_CHANNELS 1
#define DEFAULT_BAUD_RATE 115200

enum BrainwaveBand {
    DELTA = 0,    // 0.5-4 Hz
    THETA = 1,    // 4-8 Hz
    ALPHA = 2,    // 8-13 Hz
    BETA = 3,     // 13-30 Hz
    GAMMA = 4     // 30-100 Hz
};

struct EEGData {
    float value;
    unsigned long timestamp;
    uint8_t channel;
};

struct FFTResult {
    float frequency;
    float magnitude;
    float phase;
};

struct BrainwavePower {
    float delta;
    float theta;
    float alpha;
    float beta;
    float gamma;
};

class NeuroLib {
public:
    NeuroLib();
    ~NeuroLib();
    
    bool begin(int baudRate = DEFAULT_BAUD_RATE, 
               HardwareSerial& serialPort = Serial);
    void end();
    
    void setSampleRate(float sampleRate);
    void setNumSamples(uint16_t numSamples);
    void setNumChannels(uint8_t numChannels);
    
    float getSampleRate() const;
    uint16_t getNumSamples() const;
    uint8_t getNumChannels() const;
    
    bool dataAvailable();
    EEGData readData();
    int readChannel(uint8_t channel, float* buffer, int bufferSize);
    
    void computeFFT(float* input, FFTResult* output, uint16_t size);
    BrainwavePower calculateBrainwavePower(FFTResult* fft, uint16_t size);
    
    float getBandPower(FFTResult* fft, uint16_t size, BrainwaveBand band);
    float getDominantFrequency(FFTResult* fft, uint16_t size);
    
    void setPacketFormat(const String& format);
    void setDelimiter(char delimiter);
    
private:
    float _sampleRate;
    uint16_t _numSamples;
    uint8_t _numChannels;
    HardwareSerial* _serialPort;
    bool _initialized;
    char _delimiter;
    String _packetFormat;
    
    ArduinoFFT<float>* _fft;
    std::vector<float> _vReal;
    std::vector<float> _vImag;
    
    bool _parseSerialData(String& data, float* values, uint8_t numValues);
    int _findSubstringIndex(const String& haystack, const String& needle);
};

#endif // NEUROLIB_H
