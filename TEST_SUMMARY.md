# NeuroLib 单元测试系统总结

## 📦 已创建的文件

### 核心测试文件
1. **`test/test_fft.cpp`** - FFT 计算核心测试（10 个测试用例）
   - 单频率检测
   - 幅度精度验证
   - 多频率分析
   - 频率分辨率
   - DC 分量处理
   - 相位偏移
   - 噪声容限
   - 多采样尺寸
   - 采样率变化
   - 奈奎斯特频率

2. **`test/test_brainwave.cpp`** - 脑电波频段分析测试（11 个测试用例）
   - Delta/Theta/Alpha/Beta/Gamma 频段测试
   - 频段边界精度
   - Alpha/Beta 比率
   - 混合频率分析
   - 主导频率识别
   - 零信号基线
   - 频率扫描

3. **`test/test_serial_parser.cpp`** - 串口数据解析测试（14 个测试用例）
   - CSV 格式解析
   - 负数/科学计数法
   - 多通道处理
   - 自定义分隔符
   - 错误容错
   - 通道过滤

### 测试工具和数据
4. **`test/data/generate_eeg_data.py`** - Python EEG 数据生成器
   - 生成模拟脑电波信号
   - 支持多种测试模式
   - 可发送特定频率波形

5. **`test/data/test_10hz_alpha.csv`** - 10Hz Alpha 波测试数据
6. **`test/data/test_multi_frequency.csv`** - 多频率混合测试数据

### 配置文件
7. **`platformio.ini`** - PlatformIO 测试配置
   - Native 测试环境（PC 运行）
   - ESP32 硬件测试环境
   - 详细输出模式

### 文档
8. **`test/README.md`** - 完整测试指南
9. **`TESTING_QUICKSTART.md`** - 5 分钟快速入门

## 🎯 测试架构

```
┌─────────────────────────────────────────┐
│         PlatformIO Test Framework        │
├─────────────────────────────────────────┤
│  Unity Test Framework                    │
├─────────────────────────────────────────┤
│  test_fft.cpp      │ 10 tests           │
│  test_brainwave.cpp│ 11 tests           │
│  test_serial_parser│ 14 tests           │
├─────────────────────────────────────────┤
│         NeuroLib Library                │
│  - FFT Computation                      │
│  - Brainwave Analysis                   │
│  - Serial Parsing                       │
└─────────────────────────────────────────┘
```

## 📊 测试覆盖统计

- **总测试用例**: 35 个
- **测试类别**: 3 大类
  - FFT 计算：10 个
  - 脑电波分析：11 个
  - 串口解析：14 个
- **测试环境**: 2 种
  - Native (PC)
  - ESP32 (Hardware)

## 🔍 测试的波形类型

### 1. 单频正弦波
```cpp
// 测试特定频率的检测精度
sinf(2π * 10Hz * t)  // Alpha 波
sinf(2π * 20Hz * t)  // Beta 波
```

### 2. 多频混合波
```cpp
// 测试频段分离能力
sin(2π*10Hz*t) + sin(2π*20Hz*t) + sin(2π*5Hz*t)
```

### 3. 噪声信号
```cpp
// 测试噪声容限
signal + noise * 0.2
```

### 4. DC 偏移
```cpp
// 测试基线处理
constant_value = 2.5
```

### 5. 频率扫描
```cpp
// 测试频率跟踪
1Hz → 50Hz (线性扫描)
```

## 🚀 快速使用

### 运行所有测试
```bash
cd c:\DVSense\openSrc\NeuroLib
pio test -e native
```

### 运行特定测试
```bash
# FFT 测试
pio test -e native -f test_fft

# 脑电波测试
pio test -e native -f test_brainwave

# 串口解析测试
pio test -e native -f test_serial_parser
```

### 在 ESP32 上测试
```bash
pio test -e esp32
pio device monitor
```

## ✅ 测试验证的波形特性

### 频域特性
- ✅ 频率检测精度（±2Hz）
- ✅ 幅度响应线性度
- ✅ 相位不变性
- ✅ 频率分辨率
- ✅ 奈奎斯特极限

### 时域特性
- ✅ 采样率准确性
- ✅ 缓冲区管理
- ✅ 实时性能

### 噪声特性
- ✅ 信噪比容限
- ✅ 基线漂移处理
- ✅ 异常值过滤

### 频段分析
- ✅ Delta (0.5-4 Hz)
- ✅ Theta (4-8 Hz)
- ✅ Alpha (8-13 Hz)
- ✅ Beta (13-30 Hz)
- ✅ Gamma (30-100 Hz)

## 📈 预期测试结果

### 在 Native 环境
```
-----------------------
35 Tests 0 Failures 0 Ignored
OK
```

### 在 ESP32 环境
```
test_fft.cpp:25:test_fft_single_frequency [PASS]
test_fft.cpp:40:test_fft_amplitude_accuracy [PASS]
...
35 Tests 0 Failures 0 Ignored
OK

Execution time: 2.5 seconds
```

## 🔧 自定义测试

### 添加新测试用例
```cpp
void test_my_waveform() {
    // 1. 准备测试数据
    for (int i = 0; i < NUM_SAMPLES; i++) {
        testBuffer[i] = /* 你的波形 */;
    }
    
    // 2. 执行 FFT
    neuro.computeFFT(testBuffer, fftResult, NUM_SAMPLES);
    
    // 3. 验证结果
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected, actual);
}

// 在 setup() 中注册
RUN_TEST(test_my_waveform);
```

## 📚 相关文档

- **快速入门**: [TESTING_QUICKSTART.md](TESTING_QUICKSTART.md)
- **完整指南**: [test/README.md](test/README.md)
- **主 README**: [README.md](README.md)

## 💡 最佳实践

1. **开发时**: 使用 `pio test -e native` 快速测试
2. **发布前**: 使用 `pio test -e esp32` 硬件验证
3. **调试时**: 使用 `-v` 参数查看详细输出
4. **CI/CD**: 集成到自动化测试流程

## 🎓 学习资源

- PlatformIO 单元测试：https://docs.platformio.org/en/latest/plus/unit-testing.html
- Unity 测试框架：https://github.com/ThrowTheSwitch/Unity
- ArduinoFFT 库：https://github.com/kosme/arduinoFFT

---

**测试系统已就绪！** ✨

开始测试：`pio test -e native`
