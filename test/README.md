# NeuroLib 测试指南

本指南介绍如何运行 NeuroLib 的单元测试来验证 FFT 计算和脑电波分析功能。

## 目录结构

```
NeuroLib/
├── platformio.ini              # PlatformIO 配置文件
├── test/                       # 测试目录
│   ├── test_fft.cpp           # FFT 计算核心测试
│   ├── test_brainwave.cpp     # 脑电波频段分析测试
│   ├── test_serial_parser.cpp # 串口数据解析测试
│   └── data/                  # 测试数据文件
│       ├── generate_eeg_data.py      # EEG 数据生成器
│       ├── test_10hz_alpha.csv       # 10Hz Alpha 波测试数据
│       └── test_multi_frequency.csv  # 多频率混合测试数据
```

## 测试环境

### 方式一：Native 测试（推荐用于快速开发）

在您的计算机上直接运行测试，无需 ESP32 硬件。

**前提条件：**
- 安装 PlatformIO
- Python 3.7+

**运行测试：**
```bash
# 运行所有测试
pio test -e native

# 运行特定测试文件
pio test -e native -f test_fft
pio test -e native -f test_brainwave
pio test -e native -f test_serial_parser

# 详细输出
pio test -e native -v
```

### 方式二：ESP32 硬件测试（推荐用于最终验证）

在真实的 ESP32 硬件上运行测试。

**前提条件：**
- ESP32 开发板
- USB 数据线
- PlatformIO ESP32 平台

**运行测试：**
```bash
# 编译并上传测试到 ESP32
pio test -e esp32

# 打开串口监视器查看结果
pio device monitor

# 或者使用详细模式
pio test -e esp32_verbose
pio device monitor
```

## 测试内容

### 1. FFT 计算测试 (test_fft.cpp)

测试 FFT 算法的准确性和性能：

- ✅ **单频率测试**：验证单一频率信号的 FFT 检测
- ✅ **幅度精度测试**：验证不同幅度信号的响应
- ✅ **多频率测试**：验证同时检测多个频率的能力
- ✅ **频率分辨率测试**：验证频率分辨率计算
- ✅ **直流分量测试**：验证 DC 偏移的处理
- ✅ **相位偏移测试**：验证相位变化不影响频率检测
- ✅ **噪声容限测试**：验证含噪声信号的频率检测
- ✅ **不同采样大小测试**：验证 16/32/64/128 点 FFT
- ✅ **采样率变化测试**：验证不同采样率下的准确性
- ✅ **奈奎斯特频率测试**：验证最高可检测频率

### 2. 脑电波频段测试 (test_brainwave.cpp)

测试脑电波频段分析的准确性：

- ✅ **Delta 频段测试** (0.5-4 Hz)
- ✅ **Theta 频段测试** (4-8 Hz)
- ✅ **Alpha 频段测试** (8-13 Hz)
- ✅ **Beta 频段测试** (13-30 Hz)
- ✅ **Gamma 频段测试** (30-100 Hz)
- ✅ **频段边界测试**：验证频段分界的准确性
- ✅ **Alpha/Beta 比率测试**：计算相对功率比
- ✅ **混合频率测试**：验证多频段同时存在时的分析
- ✅ **主频检测测试**：验证主导频率识别
- ✅ **零信号测试**：验证无信号时的基线
- ✅ **频率扫描测试**：验证连续频率变化的跟踪

### 3. 串口数据解析测试 (test_serial_parser.cpp)

测试串口数据接收和解析功能：

- ✅ **CSV 格式解析**：标准逗号分隔格式
- ✅ **负值解析**：处理负数数据
- ✅ **科学计数法**：支持指数表示法
- ✅ **多通道解析**：处理多个通道数据
- ✅ **空格处理**：自动trim 空格
- ✅ **空行处理**：跳过空行
- ✅ **错误数据容错**：处理格式错误的数据
- ✅ **自定义分隔符**：支持分号、制表符等
- ✅ **通道缓冲区读取**：批量读取特定通道
- ✅ **通道过滤**：按通道号筛选数据
- ✅ **高频数据解析**：处理快速连续的数据
- ✅ **大数值解析**：处理极大/极小值
- ✅ **零值解析**：正确处理零值

## 使用测试数据生成器

Python 脚本可以生成模拟 EEG 数据用于测试：

**安装依赖：**
```bash
pip install pyserial
```

**运行生成器：**
```bash
# 发送连续数据
python test/data/generate_eeg_data.py COM3 115200 continuous

# 发送测试模式（单频信号）
python test/data/generate_eeg_data.py COM3 115200 pattern

# 从文件读取数据发送
python test/data/generate_eeg_data.py COM3 115200 file test/data/test_10hz_alpha.csv
```

**参数说明：**
- `COM3`: 串口号（Linux/Mac 使用 `/dev/ttyUSB0` 或 `/dev/tty.usbserial-*`）
- `115200`: 波特率
- `continuous`: 发送连续模拟 EEG 信号
- `pattern`: 发送特定频率的测试模式
- `file`: 从 CSV 文件读取数据

## 解读测试结果

### 测试通过示例
```
test_fft.cpp:25:test_fft_single_frequency [PASS]
test_fft.cpp:40:test_fft_amplitude_accuracy [PASS]
test_brainwave.cpp:30:test_brainwave_alpha_band [PASS]

-----------------------
10 Tests 0 Failures 0 Ignored
OK
```

### 测试失败示例
```
test_fft.cpp:25:test_fft_single_frequency [FAIL]
- Expected: 10.0 Within: 2.0
- Actual: 15.5

test_brainwave.cpp:45:test_brainwave_beta_band [FAIL]
- Expected: Greater Than 0.0
- Actual: -0.05
```

## 常见问题

### Q1: Native 测试编译失败
**解决方案：**
```bash
# 清理构建缓存
pio run --target clean

# 重新安装依赖
pio lib install ArduinoFFT

# 重新运行测试
pio test -e native
```

### Q2: ESP32 测试上传失败
**解决方案：**
- 检查 USB 连接
- 确认选择了正确的串口
- 按住 BOOT 按钮后按 RESET，再尝试上传
- 检查 platformio.ini 中的 board 配置

### Q3: 测试通过但实际运行异常
**解决方案：**
- 在 ESP32 硬件上重新运行测试
- 增加测试样本数量
- 检查实际输入数据格式
- 启用 VERBOSE_TEST 模式查看详细输出

### Q4: FFT 结果不准确
**可能原因：**
1. 采样率设置不正确
2. 样本数量不是 2 的幂
3. 信号频率超过奈奎斯特频率（采样率/2）
4. 信号包含过多噪声

**解决方案：**
```cpp
// 确保正确配置
neuro.setSampleRate(250.0);  // 设置正确的采样率
neuro.setNumSamples(64);     // 使用 2 的幂：16/32/64/128
```

## 自定义测试

### 添加新的测试用例

在相应的测试文件中添加测试函数：

```cpp
void test_my_custom_feature() {
    // 准备测试数据
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = /* ... */;
    }
    
    // 执行测试
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    // 验证结果
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expectedValue, actualValue);
    TEST_ASSERT_TRUE(condition);
    TEST_ASSERT_EQUAL(expected, actual);
}

// 在 setup() 中注册测试
void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_my_custom_feature);
    UNITY_END();
}
```

### 使用测试覆盖工具

```bash
# 生成代码覆盖率报告（需要额外配置）
pio test -e native --coverage
```

## 性能基准

在 ESP32 上的典型性能：
- **64 点 FFT 计算时间**: ~5-10ms
- **脑电波功率计算**: ~2-5ms
- **串口数据解析**: <1ms/行

## 最佳实践

1. **开发阶段**：使用 Native 测试快速迭代
2. **集成阶段**：在 ESP32 硬件上验证
3. **发布前**：运行所有测试确保质量
4. **持续集成**：将测试集成到 CI/CD 流程

## 贡献测试

欢迎提交新的测试用例！请确保：
- 测试具有明确的验证目标
- 包含足够的断言
- 在 PR 中说明测试覆盖的功能

## 更多信息

- [PlatformIO 单元测试文档](https://docs.platformio.org/en/latest/plus/unit-testing.html)
- [Unity 测试框架文档](https://github.com/ThrowTheSwitch/Unity)
- [ArduinoFFT 库文档](https://github.com/kosme/arduinoFFT)
