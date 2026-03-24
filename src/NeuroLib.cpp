#include "NeuroLib.h"
#include <string.h>

NeuroLib::NeuroLib() 
    : sample_rate_(DEFAULT_SAMPLE_RATE),
      num_samples_(DEFAULT_NUM_SAMPLES),
      num_channels_(DEFAULT_NUM_CHANNELS),
      serial_port_(nullptr),
      initialized_(false),
      delimiter_(','),
      packet_format_(""),
      fft_(nullptr) {
    v_real_.resize(num_samples_);
    v_imag_.resize(num_samples_);
}

NeuroLib::~NeuroLib() {
    end();
}

bool NeuroLib::begin(int baudRate, HardwareSerial& serialPort) {
    serial_port_ = &serialPort;
    serial_port_->begin(baudRate);
    initialized_ = true;
    
    fft_ = new ArduinoFFT<float>(v_real_.data(), v_imag_.data(), num_samples_, sample_rate_);
    
    return true;
}

void NeuroLib::end() {
    if (fft_) {
        delete fft_;
        fft_ = nullptr;
    }
    if (serial_port_) {
        serial_port_->end();
        serial_port_ = nullptr;
    }
    initialized_ = false;
}

void NeuroLib::setSampleRate(float sampleRate) {
    sample_rate_ = sampleRate;
    if (fft_ && initialized_) {
        delete fft_;
        fft_ = new ArduinoFFT<float>(v_real_.data(), v_imag_.data(), num_samples_, sample_rate_);
    }
}

void NeuroLib::setNumSamples(uint16_t numSamples) {
    if (numSamples < 16 || numSamples > 512) {
        Serial.println("Warning: numSamples should be between 16 and 512");
        return;
    }
    
    num_samples_ = numSamples;
    v_real_.resize(num_samples_);
    v_imag_.resize(num_samples_);
    
    if (fft_ && initialized_) {
        delete fft_;
        fft_ = new ArduinoFFT<float>(v_real_.data(), v_imag_.data(), num_samples_, sample_rate_);
    }
}

void NeuroLib::setNumChannels(uint8_t numChannels) {
    num_channels_ = numChannels;
}

float NeuroLib::getSampleRate() const {
    return sample_rate_;
}

uint16_t NeuroLib::getNumSamples() const {
    return num_samples_;
}

uint8_t NeuroLib::getNumChannels() const {
    return num_channels_;
}

bool NeuroLib::dataAvailable() {
    if (!initialized_ || !serial_port_) {
        return false;
    }
    return serial_port_->available() > 0;
}

EEGData NeuroLib::readData() {
    EEGData data = {0.0f, 0, 0};
    
    if (!initialized_ || !serial_port_) {
        return data;
    }
    
    if (serial_port_->available()) {
        String line = serial_port_->readStringUntil('\n');
        line.trim();
        
        if (line.length() > 0) {
            float values[3] = {0};
            if (parse_serial_data(line, values, 3)) {
                data.value = values[0];
                data.timestamp = millis();
                data.channel = (uint8_t)values[1];
            }
        }
    }
    
    return data;
}

int NeuroLib::readChannel(uint8_t channel, float* buffer, int bufferSize) {
    if (!initialized_ || !serial_port_ || !buffer) {
        return -1;
    }
    
    int count = 0;
    unsigned long startTime = millis();
    unsigned long timeout = 1000;
    
    while (count < bufferSize && (millis() - startTime) < timeout) {
        if (serial_port_->available()) {
            String line = serial_port_->readStringUntil('\n');
            line.trim();
            
            if (line.length() > 0) {
                float values[3] = {0};
                if (parse_serial_data(line, values, 3)) {
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
    if (!input || !output || size > num_samples_) {
        return;
    }
    
    for (uint16_t i = 0; i < size; i++) {
        v_real_[i] = input[i];
        v_imag_[i] = 0.0f;
    }
    
    fft_->windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    fft_->compute(FFT_FORWARD);
    fft_->complexToMagnitude();
    
    for (uint16_t i = 0; i < size / 2; i++) {
        output[i].frequency = (i * sample_rate_) / size;
        output[i].magnitude = v_real_[i];
        output[i].phase = atan2(v_imag_[i], v_real_[i]);
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
    packet_format_ = format;
}

void NeuroLib::setDelimiter(char delimiter) {
    delimiter_ = delimiter;
}

bool NeuroLib::parse_serial_data(String& data, float* values, uint8_t numValues) {
    int index = 0;
    int startIndex = 0;
    
    while (index < numValues && startIndex < data.length()) {
        int endIndex = data.indexOf(delimiter_, startIndex);
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

int NeuroLib::find_substring_index(const String& haystack, const String& needle) {
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
