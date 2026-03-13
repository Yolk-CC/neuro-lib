# NeuroLib - ESP32 EEG Processing Library

NeuroLib is a comprehensive Arduino library for processing EEG (Electroencephalography) data on ESP32 microcontrollers. It provides real-time signal acquisition and analysis capabilities, including FFT computation for brainwave frequency analysis.

## Features

- **Real-time EEG Data Acquisition**: Receive and process EEG data from serial interfaces
- **Multi-channel Support**: Handle data from multiple EEG channels simultaneously
- **FFT Analysis**: Built-in Fast Fourier Transform for frequency domain analysis
- **Brainwave Band Classification**: Automatic classification into Delta, Theta, Alpha, Beta, and Gamma bands
- **Configurable Parameters**: Adjustable sample rates, buffer sizes, and channel counts
- **Simple API**: Easy-to-use interface for rapid prototyping and development

## Installation

### Method 1: Arduino IDE Library Manager
1. Open Arduino IDE
2. Go to Sketch > Include Library > Manage Libraries
3. Search for "NeuroLib"
4. Click Install

### Method 2: Manual Installation
1. Download this library as a ZIP file
2. In Arduino IDE, go to Sketch > Include Library > Add .ZIP Library
3. Select the downloaded ZIP file

## Dependencies

This library requires the following dependencies:
- [ArduinoFFT](https://github.com/kosme/arduinoFFT) - For FFT computations

Install the dependency via Arduino IDE Library Manager or manually download it.

## Quick Start

```cpp
#include <NeuroLib.h>

NeuroLib neuro;

void setup() {
  Serial.begin(115200);
  
  if (!neuro.begin(115200, Serial)) {
    Serial.println("Failed to initialize NeuroLib!");
    return;
  }
  
  neuro.setSampleRate(250.0);  // Set sample rate to 250 Hz
  neuro.setNumSamples(64);     // Set FFT size to 64 samples
}

void loop() {
  if (neuro.dataAvailable()) {
    EEGData data = neuro.readData();
    Serial.println(data.value);
  }
}
```

## API Reference

### Initialization
- `bool begin(int baudRate = 115200, HardwareSerial& serialPort = Serial)` - Initialize the library
- `void end()` - Stop and clean up resources

### Configuration
- `void setSampleRate(float sampleRate)` - Set sampling rate in Hz
- `void setNumSamples(uint16_t numSamples)` - Set number of samples for FFT (16-512)
- `void setNumChannels(uint8_t numChannels)` - Set number of EEG channels
- `void setPacketFormat(const String& format)` - Set expected data packet format
- `void setDelimiter(char delimiter)` - Set delimiter for parsing data packets

### Data Acquisition
- `bool dataAvailable()` - Check if new data is available
- `EEGData readData()` - Read a single EEG data point
- `int readChannel(uint8_t channel, float* buffer, int bufferSize)` - Read multiple samples from a specific channel

### Analysis Functions
- `void computeFFT(float* input, FFTResult* output, uint16_t size)` - Compute FFT on input data
- `BrainwavePower calculateBrainwavePower(FFTResult* fft, uint16_t size)` - Calculate power in each brainwave band
- `float getBandPower(FFTResult* fft, uint16_t size, BrainwaveBand band)` - Get power in specific band
- `float getDominantFrequency(FFTResult* fft, uint16_t size)` - Find frequency with highest magnitude

### Constants
- `DEFAULT_SAMPLE_RATE` (250.0 Hz)
- `DEFAULT_NUM_SAMPLES` (64)
- `DEFAULT_NUM_CHANNELS` (1)
- `DEFAULT_BAUD_RATE` (115200)

## Data Format

The library expects EEG data in CSV format over serial connection:
`value,channel,timestamp`

For example:
```
1.234,0,1234567890
2.345,0,1234567891
```

## Brainwave Bands

The library classifies frequencies into the following bands:
- **Delta**: 0.5-4 Hz (Deep sleep, unconsciousness)
- **Theta**: 4-8 Hz (Drowsiness, meditation)
- **Alpha**: 8-13 Hz (Relaxed, awake)
- **Beta**: 13-30 Hz (Active thinking, focus)
- **Gamma**: 30-100 Hz (High-level cognitive processing)

## Examples

The library includes several examples:
- **BasicEEG**: Simple data acquisition and display
- **FFTAnalysis**: Real-time FFT and brainwave power calculation
- **MultiChannel**: Processing data from multiple EEG channels
- **RealTimePlot**: Output formatted data for plotting applications

## Hardware Setup

Connect your EEG device to the ESP32's serial pins (typically GPIO1 and GPIO3 for Serial). Ensure proper voltage levels and electrical safety measures when working with EEG equipment.

## Troubleshooting

1. **No data received**: Check serial connection and data format
2. **FFT errors**: Ensure sample size is a power of 2 (16, 32, 64, 128, etc.)
3. **Memory issues**: Reduce sample size or optimize other parts of your code

## Contributing

Contributions are welcome! Please submit pull requests or open issues for bug reports and feature suggestions.

## License

MIT License - see LICENSE file for details.

## Acknowledgments

This library builds upon the ArduinoFFT library and follows Arduino library standards for compatibility with the Arduino ecosystem.