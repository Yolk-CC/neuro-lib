# NeuroLib 测试快速入门

## 🚀 5 分钟快速开始

### 步骤 1: 安装 PlatformIO

如果你还没有安装 PlatformIO：

**方法 A: 使用 VS Code（推荐）**
1. 安装 [VS Code](https://code.visualstudio.com/)
2. 在 VS Code 中安装 PlatformIO IDE 扩展
3. 重启 VS Code

**方法 B: 使用命令行**
```bash
pip install platformio
```

### 步骤 2: 运行第一个测试

打开终端，进入 NeuroLib 目录：

```bash
cd c:\DVSense\openSrc\NeuroLib
```

运行 Native 测试（无需硬件）：

```bash
pio test -e native
```

你会看到类似输出：
```
Collected 3 tests
Verifying test_fft.cpp ...
test_fft.cpp:25:test_fft_single_frequency [PASS]
test_fft.cpp:40:test_fft_amplitude_accuracy [PASS]
...
Verifying test_brainwave.cpp ...
test_brainwave.cpp:30:test_brainwave_alpha_band [PASS]
...
Verifying test_serial_parser.cpp ...
test_serial_parser.cpp:45:test_parse_csv_format [PASS]
...

-----------------------
31 Tests 0 Failures 0 Ignored
OK
```

### 步骤 3: 查看特定测试

运行单个测试文件：
```bash
# 只测试 FFT 功能
pio test -e native -f test_fft

# 只测试脑电波分析
pio test -e native -f test_brainwave

# 只测试串口解析
pio test -e native -f test_serial_parser
```

### 步骤 4: 在 ESP32 上测试（可选）

如果你有 ESP32 开发板：

```bash
# 连接 ESP32 到电脑
# 编译并上传测试
pio test -e esp32

# 打开串口监视器查看结果
pio device monitor
```

按 `Ctrl+C` 退出监视器。

## 📊 测试覆盖的功能

### FFT 计算测试 (10 个测试用例)
- ✅ 单频率信号检测
- ✅ 幅度精度验证
- ✅ 多频率同时检测
- ✅ 频率分辨率
- ✅ DC 分量处理
- ✅ 相位偏移不影响频率
- ✅ 噪声容限
- ✅ 不同采样大小 (16/32/64/128)
- ✅ 采样率变化
- ✅ 奈奎斯特频率限制

### 脑电波分析测试 (11 个测试用例)
- ✅ Delta 频段 (0.5-4 Hz)
- ✅ Theta 频段 (4-8 Hz)
- ✅ Alpha 频段 (8-13 Hz)
- ✅ Beta 频段 (13-30 Hz)
- ✅ Gamma 频段 (30-100 Hz)
- ✅ 频段边界精度
- ✅ Alpha/Beta 比率计算
- ✅ 混合频率分析
- ✅ 主导频率识别
- ✅ 零信号基线
- ✅ 频率扫描跟踪

### 串口解析测试 (14 个测试用例)
- ✅ CSV 格式解析
- ✅ 负数处理
- ✅ 科学计数法
- ✅ 多通道数据
- ✅ 空格自动 trim
- ✅ 空行跳过
- ✅ 错误数据容错
- ✅ 自定义分隔符
- ✅ 通道缓冲区读取
- ✅ 通道过滤
- ✅ 高频数据处理
- ✅ 大数值处理
- ✅ 零值处理

## 🔧 常用命令

```bash
# 运行所有测试
pio test

# 运行特定环境
pio test -e native
pio test -e esp32

# 详细输出
pio test -v

# 清理构建文件
pio run --target clean

# 安装/更新依赖
pio lib install ArduinoFFT
```

## 📝 解读测试结果

### 通过测试
```
test_fft.cpp:25:test_fft_single_frequency [PASS]
✓ 表示测试通过
```

### 失败测试
```
test_fft.cpp:40:test_fft_amplitude_accuracy [FAIL]
- Expected: 5.0 Within: 0.1
- Actual: 4.2
✗ 表示测试失败，会显示期望值和实际值
```

## 🐛 故障排除

### 问题 1: `pio: command not found`
**解决：**
```bash
# 安装 PlatformIO
pip install platformio
# 或者使用完整路径
~/.platformio/penv/bin/pio test -e native
```

### 问题 2: 编译错误
**解决：**
```bash
# 清理并重新安装依赖
pio run --target clean
pio lib install ArduinoFFT
pio test -e native
```

### 问题 3: ESP32 上传失败
**解决：**
- 检查 USB 线是否连接好
- 确认串口号正确
- 在 platformio.ini 中修改 `board` 配置
- 尝试按住 BOOT 键后按 RESET

## 📖 下一步

1. **阅读完整测试文档**：查看 [test/README.md](test/README.md)
2. **查看测试代码**：打开 `test/test_fft.cpp` 等文件
3. **自定义测试**：根据你的需求添加新的测试用例
4. **集成到 CI/CD**：将测试集成到自动化流程中

## 💡 使用测试数据生成器

生成模拟 EEG 数据：

```bash
# 安装依赖
pip install pyserial

# 发送 10Hz Alpha 波测试数据
python test/data/generate_eeg_data.py COM3 115200 pattern
```

## 🎯 测试你的代码

当你修改 NeuroLib 代码后：

```bash
# 运行测试确保没有破坏现有功能
pio test -e native

# 如果所有测试通过，说明修改是安全的
```

## 📚 更多资源

- 完整测试文档：[test/README.md](test/README.md)
- PlatformIO 文档：https://docs.platformio.org/
- Unity 测试框架：https://github.com/ThrowTheSwitch/Unity

---

**祝你测试顺利！** 🎉

如有问题，请查看完整文档或提交 Issue。
