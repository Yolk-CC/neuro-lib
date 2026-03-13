#!/usr/bin/env python3
"""
EEG Test Data Generator for NeuroLib
Generates synthetic EEG-like data for testing FFT analysis
"""

import math
import random
import time
import serial
import sys

class EEGDataGenerator:
    def __init__(self, sample_rate=250.0, num_channels=1):
        self.sample_rate = sample_rate
        self.num_channels = num_channels
        self.time = 0.0
        
    def generate_brainwave(self, frequency, amplitude, phase=0.0):
        value = amplitude * math.sin(2 * math.pi * frequency * self.time + phase)
        return value
    
    def generate_noise(self, amplitude=0.1):
        return random.gauss(0, amplitude)
    
    def generate_alpha_wave(self, amplitude=1.0):
        return self.generate_brainwave(10.0, amplitude)
    
    def generate_beta_wave(self, amplitude=0.5):
        return self.generate_brainwave(20.0, amplitude)
    
    def generate_theta_wave(self, amplitude=0.3):
        return self.generate_brainwave(6.0, amplitude)
    
    def generate_delta_wave(self, amplitude=0.2):
        return self.generate_brainwave(2.0, amplitude)
    
    def generate_gamma_wave(self, amplitude=0.1):
        return self.generate_brainwave(40.0, amplitude)
    
    def generate_realistic_eeg(self, add_noise=True):
        value = 0.0
        value += self.generate_alpha_wave(1.0)
        value += self.generate_beta_wave(0.5)
        value += self.generate_theta_wave(0.3)
        value += self.generate_delta_wave(0.2)
        value += self.generate_gamma_wave(0.1)
        
        if add_noise:
            value += self.generate_noise(0.1)
        
        return value
    
    def get_sample(self, channel=0, add_noise=True):
        value = self.generate_realistic_eeg(add_noise)
        timestamp = int(time.time() * 1000)
        self.time += 1.0 / self.sample_rate
        return value, channel, timestamp
    
    def reset(self):
        self.time = 0.0


def send_data(serial_port, generator, num_samples=100, delay=0.004):
    """Send EEG data over serial port"""
    try:
        print(f"Sending {num_samples} samples at {generator.sample_rate} Hz...")
        
        for i in range(num_samples):
            value, channel, timestamp = generator.get_sample(channel=0)
            data_line = f"{value:.4f},{channel},{timestamp}\n"
            serial_port.write(data_line.encode())
            
            if i % 10 == 0:
                print(f"Sent {i}/{num_samples} samples")
            
            time.sleep(delay)
        
        print("Data transmission complete!")
        
    except KeyboardInterrupt:
        print("\nTransmission interrupted")
    except Exception as e:
        print(f"Error: {e}")


def send_test_patterns(serial_port, generator):
    """Send specific test patterns for validation"""
    test_patterns = [
        ("10Hz Alpha Wave", 10.0, 1.0),
        ("20Hz Beta Wave", 20.0, 0.5),
        ("5Hz Theta Wave", 5.0, 0.3),
        ("2Hz Delta Wave", 2.0, 0.2),
        ("40Hz Gamma Wave", 40.0, 0.1),
    ]
    
    for name, freq, amp in test_patterns:
        print(f"\nSending {name}...")
        generator.reset()
        
        for i in range(256):
            value = amp * math.sin(2 * math.pi * freq * generator.time)
            timestamp = int(time.time() * 1000)
            data_line = f"{value:.4f},0,{timestamp}\n"
            serial_port.write(data_line.encode())
            generator.time += 1.0 / generator.sample_rate
            time.sleep(0.004)
        
        print(f"✓ {name} complete")


def main():
    if len(sys.argv) < 2:
        print("Usage: python generate_eeg_data.py <port> [baud_rate] [mode]")
        print("  port: Serial port (e.g., COM3, /dev/ttyUSB0)")
        print("  baud_rate: Baud rate (default: 115200)")
        print("  mode: 'continuous', 'pattern', or 'file' (default: continuous)")
        sys.exit(1)
    
    port = sys.argv[1]
    baud_rate = int(sys.argv[2]) if len(sys.argv) > 2 else 115200
    mode = sys.argv[3] if len(sys.argv) > 3 else "continuous"
    
    print(f"Connecting to {port} at {baud_rate} baud...")
    
    try:
        ser = serial.Serial(port, baud_rate, timeout=1)
        time.sleep(2)
        print("Connected!")
        
        generator = EEGDataGenerator(sample_rate=250.0)
        
        if mode == "pattern":
            send_test_patterns(ser, generator)
        elif mode == "file":
            filename = sys.argv[4] if len(sys.argv) > 4 else "eeg_data.txt"
            print(f"Reading data from {filename}...")
            with open(filename, 'r') as f:
                for line in f:
                    ser.write(line.encode())
                    time.sleep(0.004)
        else:
            send_data(ser, generator, num_samples=1000)
        
        ser.close()
        
    except serial.SerialException as e:
        print(f"Serial port error: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
