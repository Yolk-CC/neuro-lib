#include "NeuroLib.h"
#include <string.h>

NeuroLib::NeuroLib() 
    : _sampleRate(DEFAULT_SAMPLE_RATE),
      _numSamples(DEFAULT_NUM_SAMPLES),
      _numChannels(DEFAULT_NUM_CHANNELS),
      _serialPort(nullptr),
      _initialized(false),
      _delimiter(','),
      _packetFormat(""),
      _fft(nullptr) {
    _vReal.resize(_numSamples);
    _vImag.resize(_numSamples);
}

NeuroLib::~NeuroLib() {
    end();
}

bool NeuroLib::begin(int baudRate, HardwareSerial& serialPort) {
    _serialPort = &serialPort;
    _serialPort->begin(baudRate);
    _initialized = true;
    
    _fft = new ArduinoFFT<float>(_vReal.data(), _vImag.data(), _numSamples, _sampleRate);
    
    return true;
}

void NeuroLib::end() {
    if (_fft) {
        delete _fft;
        _fft = nullptr;
    }
    if (_serialPort) {
        _serialPort->end();
        _serialPort = nullptr;
    }
    _initialized = false;
}

void NeuroLib::setSampleRate(float sampleRate) {
    _sampleRate = sampleRate;
    if (_fft && _initialized) {
        delete _fft;
        _fft = new ArduinoFFT<float>(_vReal.data(), _vImag.data(), _numSamples, _sampleRate);
    }
}

void NeuroLib::setNumSamples(uint16_t numSamples) {
    if (numSamples < 16 || numSamples > 512) {
        Serial.println("Warning: numSamples should be between 16 and 512");
        return;
    }
    
    _numSamples = numSamples;
    _vReal.resize(_numSamples);
    _vImag.resize(_numSamples);
    
    if (_fft && _initialized) {
        delete _fft;
        _fft = new ArduinoFFT<float>(_vReal.data(), _vImag.data(), _numSamples, _sampleRate);
    }
}

void NeuroLib::setNumChannels(uint8_t numChannels) {
    _numChannels = numChannels;
}

float NeuroLib::getSampleRate() const {
    return _sampleRate;
}

uint16_t NeuroLib::getNumSamples() const {
    return _numSamples;
}

uint8_t NeuroLib::getNumChannels() const {
    return _numChannels;
}

bool NeuroLib::dataAvailable() {
    if (!_initialized || !_serialPort) {
        return false;
    }
    return _serialPort->available() > 0;
}

EEGData NeuroLib::readData() {
    EEGData data = {0.0f, 0, 0};
    
    if (!_initialized || !_serialPort) {
        return data;
    }
    
    if (_serialPort->available()) {
        String line = _serialPort->readStringUntil('\n');
        line.trim();
        
        if (line.length() > 0) {
            float values[3] = {0};
            if (_parseSerialData(line, values, 3)) {
                data.value = values[0];
                data.timestamp = millis();
                data.channel = (uint8_t)values[1];
            }
        }
    }
    
    return data;
}

int NeuroLib::readChannel(uint8_t channel, float* buffer, int bufferSize) {
    if (!_initialized || !_serialPort || !buffer) {
        return -1;
    }
    
    int count = 0;
    unsigned long startTime = millis();
    unsigned long timeout = 1000;
    
    while (count < bufferSize && (millis() - startTime) < timeout) {
        if (_serialPort->available()) {
            String line = _serialPort->readStringUntil('\n');
            line.trim();
            
            if (line.length() > 0) {
                float values[3] = {0};
                if (_parseSerialData(line, values, 3)) {
                    if ((uint8_t)values[1] == channel) {
                        buffer[count++] = values[0];
                    }
                }
            }
        }
        delay(1);
    }
    
    return count;
}

void NeuroLib::computeFFT(float* input, FFTResult* output, uint16_t size) {
    if (!input || !output || size > _numSamples) {
        return;
    }
    
    for (uint16_t i = 0; i < size; i++) {
        _vReal[i] = input[i];
        _vImag[i] = 0.0f;
    }
    
    _fft->windowize(size);
    _fft->compute(FFT_FORWARD, size);
    _fft->complexToMagnitude(size);
    
    for (uint16_t i = 0; i < size / 2; i++) {
        output[i].frequency = _fft->freqToIndex(i, _sampleRate, size);
        output[i].magnitude = _vReal[i];
        output[i].phase = atan2(_vImag[i], _vReal[i]);
    }
}

BrainwavePower NeuroLib::calculateBrainwavePower(FFTResult* fft, uint16_t size) {
    BrainwavePower power = {0};
    
    power.delta = getBandPower(fft, size, DELTA);
    power.theta = getBandPower(fft, size, THETA);
    power.alpha = getBandPower(fft, size, ALPHA);
    power.beta = getBandPower(fft, size, BETA);
    power.gamma = getBandPower(fft, size, GAMMA);
    
    return power;
}

float NeuroLib::getBandPower(FFTResult* fft, uint16_t size, BrainwaveBand band) {
    float power = 0.0f;
    int count = 0;
    
    float minFreq, maxFreq;
    switch (band) {
        case DELTA:
            minFreq = 0.5f;
            maxFreq = 4.0f;
            break;
        case THETA:
            minFreq = 4.0f;
            maxFreq = 8.0f;
            break;
        case ALPHA:
            minFreq = 8.0f;
            maxFreq = 13.0f;
            break;
        case BETA:
            minFreq = 13.0f;
            maxFreq = 30.0f;
            break;
        case GAMMA:
            minFreq = 30.0f;
            maxFreq = 100.0f;
            break;
        default:
            return 0.0f;
    }
    
    for (uint16_t i = 0; i < size / 2; i++) {
        if (fft[i].frequency >= minFreq && fft[i].frequency <= maxFreq) {
            power += fft[i].magnitude * fft[i].magnitude;
            count++;
        }
    }
    
    return (count > 0) ? power / count : 0.0f;
}

float NeuroLib::getDominantFrequency(FFTResult* fft, uint16_t size) {
    float maxMagnitude = 0.0f;
    float dominantFreq = 0.0f;
    
    for (uint16_t i = 1; i < size / 2; i++) {
        if (fft[i].magnitude > maxMagnitude) {
            maxMagnitude = fft[i].magnitude;
            dominantFreq = fft[i].frequency;
        }
    }
    
    return dominantFreq;
}

void NeuroLib::setPacketFormat(const String& format) {
    _packetFormat = format;
}

void NeuroLib::setDelimiter(char delimiter) {
    _delimiter = delimiter;
}

bool NeuroLib::_parseSerialData(String& data, float* values, uint8_t numValues) {
    int index = 0;
    int startIndex = 0;
    
    while (index < numValues && startIndex < data.length()) {
        int endIndex = data.indexOf(_delimiter, startIndex);
        if (endIndex == -1) {
            endIndex = data.length();
        }
        
        String token = data.substring(startIndex, endIndex);
        token.trim();
        
        if (token.length() > 0) {
            values[index++] = token.toFloat();
        }
        
        startIndex = endIndex + 1;
    }
    
    return (index >= numValues);
}

int NeuroLib::_findSubstringIndex(const String& haystack, const String& needle) {
    if (needle.length() == 0) return -1;
    
    for (int i = 0; i <= haystack.length() - needle.length(); i++) {
        bool found = true;
        for (int j = 0; j < needle.length(); j++) {
            if (haystack[i + j] != needle[j]) {
                found = false;
                break;
            }
        }
        if (found) return i;
    }
    
    return -1;
}
