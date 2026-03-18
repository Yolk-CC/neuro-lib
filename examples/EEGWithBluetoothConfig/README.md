# EEG Signal Acquisition with Bluetooth Configuration

这个示例展示了如何使用 ESP32 双模蓝牙功能，通过 WiFi 热点配置蓝牙连接，并接收 EEG 数据进行 FFT 分析。

## 功能特性

- ✅ 自动开启 WiFi 热点（AP 模式）
- ✅ 基于 Web 的配置界面（手机/电脑可访问）
- ✅ 蓝牙设备扫描和连接
- ✅ 实时接收蓝牙数据
- ✅ FFT 频域分析
- ✅ 脑电波功率分析（Delta/Theta/Alpha/Beta/Gamma）

## 硬件要求

- ESP32 开发板（支持双模蓝牙）
- 蓝牙 EEG 设备

## 使用步骤

### 1. 上传程序

使用 Arduino IDE 或 PlatformIO 将 `EEGWithBluetoothConfig.ino` 上传到 ESP32。

### 2. 连接 WiFi 热点

ESP32 启动后会自动开启 WiFi 热点：
- **SSID**: `NeuroLib_Config`
- **密码**: `neurolib123`

使用手机或电脑连接到这个 WiFi 网络。

### 3. 访问配置页面

打开浏览器，访问：`http://192.168.4.1`

你将看到一个美观的配置界面。

### 4. 扫描并连接蓝牙设备

1. 点击 "Scan for Devices" 按钮
2. 等待扫描完成（约 5 秒）
3. 在设备列表中点击你的 EEG 设备
4. 确认连接

### 5. 查看分析结果

连接成功后，打开 Arduino IDE 的串口监视器（波特率：115200），你将看到：

```
========================================
  NeuroLib EEG with Bluetooth Config
========================================

Starting Bluetooth configuration... Success!
WiFi Access Point started
SSID: NeuroLib_Config
Password: neurolib123
Web Interface: http://192.168.4.1

NeuroLib configured: 250.00 Hz, 64 samples, 1 channel(s)

System ready!
Connect to your EEG device via the web interface

=== Brainwave Power Analysis ===
Delta (0.5-4 Hz):   0.0234 uV^2
Theta (4-8 Hz):     0.0156 uV^2
Alpha (8-13 Hz):    0.0421 uV^2
Beta (13-30 Hz):    0.0189 uV^2
Gamma (30-100 Hz):  0.0078 uV^2
Dominant Frequency: 10.50 Hz
Brain State: Relaxed/Alert
================================

Samples/sec: 250
```

## 数据格式说明

示例代码假设 EEG 数据通过蓝牙以 **float 类型（4 字节）** 的形式传输。

如果你的设备使用不同的数据格式，需要修改 `loop()` 函数中的数据解析部分：

```cpp
// 当前实现：接收 4 字节 float 数据
static uint8_t dataBuffer[4];
static int dataIndex = 0;

dataBuffer[dataIndex++] = buffer[i];

if (dataIndex == 4) {
    float value = *((float*)dataBuffer);
    processEEGData(value);
    dataIndex = 0;
}
```

### 常见数据格式适配

#### 1. 字符串格式（如 "123.45\n"）
```cpp
String line = serialPort_.readStringUntil('\n');
float value = line.toFloat();
processEEGData(value);
```

#### 2. 16 位整数格式
```cpp
static uint8_t dataBuffer[2];
static int dataIndex = 0;

dataBuffer[dataIndex++] = buffer[i];

if (dataIndex == 2) {
    int16_t value = (dataBuffer[1] << 8) | dataBuffer[0];
    processEEGData((float)value / 1000.0f); // 转换为浮点数
    dataIndex = 0;
}
```

## 参数配置

你可以在代码中修改以下参数：

```cpp
#define BUFFER_SIZE 64          // FFT 缓冲区大小（必须是 2 的幂）
#define SAMPLE_RATE 250.0f      // 采样率（Hz）
#define NUM_CHANNELS 1          // 通道数
```

### FFT 缓冲区大小

可选值：16, 32, 64, 128, 256, 512

- **较小值**（16-32）：更快的响应，但频率分辨率较低
- **较大值**（256-512）：更高的频率分辨率，但响应较慢

### 采样率

应根据你的 EEG 设备的实际采样率设置：
- 常见值：250Hz, 500Hz, 1000Hz

## 脑电波频段说明

| 频段 | 频率范围 | 相关状态 |
|------|----------|----------|
| Delta | 0.5-4 Hz | 深度睡眠 |
| Theta | 4-8 Hz | 困倦/冥想 |
| Alpha | 8-13 Hz | 放松/清醒 |
| Beta | 13-30 Hz | 活跃/专注 |
| Gamma | 30-100 Hz | 高级认知处理 |

## 故障排除

### 无法连接到 WiFi 热点

1. 确保 ESP32 已正确供电
2. 检查串口输出，确认 AP 启动成功
3. 尝试重启 ESP32

### 扫描不到蓝牙设备

1. 确保 EEG 设备已进入配对模式
2. 确保 EEG 设备在有效范围内（< 10 米）
3. 检查 EEG 设备是否已连接到其他设备

### 连接后没有数据

1. 检查 EEG 设备是否正在发送数据
2. 确认数据格式匹配（见"数据格式说明"）
3. 检查串口监视器输出

### Web 界面无法访问

1. 确认设备已连接到正确的 WiFi 热点
2. 尝试清除浏览器缓存
3. 使用无痕模式访问

## 高级功能

### 查看详细的 FFT 结果

取消注释 `performFFTAnalysis()` 函数中的 `printFFTResult()` 调用：

```cpp
void performFFTAnalysis() {
    // ... 其他代码 ...
    
    // 取消注释以查看详细 FFT 结果
    printFFTResult();
}
```

### 自定义脑电波阈值

修改 `printBrainwaveAnalysis()` 函数中的判断逻辑：

```cpp
if (brainwavePower.alpha > maxPower) {
    maxPower = brainwavePower.alpha;
    brainState = "Relaxed/Alert";
}
```

## 技术支持

如有问题，请参考：
- NeuroLib 文档
- ESP32 Arduino 核心文档
- ArduinoFFT 库文档

## 许可证

MIT License
