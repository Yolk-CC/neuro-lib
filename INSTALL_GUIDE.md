# NeuroLib 库安装和故障排除指南

## ⚠️ 重要：正确的安装步骤

### 方法 1：使用 Arduino 文档目录（推荐，无需管理员权限）

1. **找到 Arduino 文档目录**
   ```
   C:\Users\你的用户名\Documents\Arduino\libraries\
   ```

2. **复制库文件**
   - 将整个 `d:\MyWork\neuro_lib` 文件夹复制
   - 粘贴到 `C:\Users\你的用户名\Documents\Arduino\libraries\NeuroLib`
   
3. **确保目录结构正确**
   ```
   C:\Users\你的用户名\Documents\Arduino\libraries\NeuroLib\
   ├── src/
   │   ├── NeuroLib.h
   │   ├── NeuroLib.cpp
   │   ├── BluetoothConfig.h
   │   └── BluetoothConfig.cpp
   ├── examples/
   │   └── ...
   └── library.properties
   ```

4. **重启 Arduino IDE**

---

### 方法 2：使用 Arduino IDE 安装目录（需要管理员权限）

1. **以管理员身份运行文件资源管理器**
   - 右键文件资源管理器 → 以管理员身份运行

2. **复制库文件到**
   ```
   D:\Program Files\Arduino IDE\resources\app\libraries\NeuroLib\
   ```

3. **确保目录结构正确**（同上）

4. **重启 Arduino IDE**

---

## 🔧 解决"找不到头文件"问题

### 问题 1：示例代码使用双引号包含

**症状：** 编译时提示 `fatal error: BluetoothConfig.h: No such file found`

**原因：** Arduino IDE 对 `src` 目录的处理方式

**解决方案 A：修改示例代码（推荐）**

打开示例文件，将：
```cpp
#include "BluetoothConfig.h"
#include "NeuroLib.h"
```

改为：
```cpp
#include <NeuroLib.h>
#include <BluetoothConfig.h>
```

**解决方案 B：在库根目录创建头文件副本**

在 `NeuroLib` 库的根目录创建 `NeuroLib.h` 和 `BluetoothConfig.h` 文件，内容为：
```cpp
// NeuroLib.h (在库根目录)
#include "src/NeuroLib.h"
```

```cpp
// BluetoothConfig.h (在库根目录)
#include "src/BluetoothConfig.h"
```

---

### 问题 2：库文件位置不正确

**检查步骤：**

1. 打开 Arduino IDE
2. 点击 **文件** → **首选项**
3. 查看 **代码存放文件夹** 的路径
4. 确保 NeuroLib 库在正确的 `libraries` 目录下

**验证库是否被识别：**

1. 打开 Arduino IDE
2. 点击 **工具** → **管理库**
3. 在搜索框输入 "NeuroLib"
4. 应该能看到 "NeuroLib by DVSense"

如果看不到，说明库没有被正确识别。

---

### 问题 3：缺少依赖库

**症状：** 编译时提示其他库找不到

**需要安装的依赖：**

1. **ArduinoFFT**
   - 工具 → 管理库
   - 搜索 "ArduinoFFT"
   - 安装 "ArduinoFFT" by Enrique Condes

2. **ESP32 开发板支持包**
   - 文件 → 首选项
   - 在"附加开发板管理器 URL"中添加：
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - 工具 → 开发板 → 开发板管理器
   - 搜索 "esp32"
   - 安装 "esp32 by Espressif Systems"

---

### 问题 4：缓存问题

**解决方案：清除 Arduino IDE 缓存**

1. 关闭 Arduino IDE
2. 删除以下目录（如果存在）：
   ```
   C:\Users\你的用户名\AppData\Local\Arduino15\packages\esp32
   ```
3. 重新打开 Arduino IDE
4. 重新选择开发板

---

## ✅ 验证安装成功

### 步骤 1：检查库示例

1. 打开 Arduino IDE
2. 点击 **文件** → **示例**
3. 滚动到列表底部
4. 应该能看到 **NeuroLib** 分类
5. 展开后应该有：
   - BasicEEG
   - FFTAnalysis
   - EEGWithBluetoothConfig
   - MultiChannel
   - RealTimePlot

### 步骤 2：编译测试

1. 打开 **文件** → **示例** → **NeuroLib** → **BasicEEG**
2. 选择你的 ESP32 开发板
3. 点击 **验证** 按钮
4. 应该看到 "编译完成"

### 步骤 3：检查包含路径

在 Arduino IDE 中创建一个测试程序：

```cpp
#include <Arduino.h>
#include <NeuroLib.h>
#include <BluetoothConfig.h>

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("NeuroLib installed successfully!");
}

void loop() {
  // Empty
}
```

编译这个程序，如果没有错误，说明库安装成功。

---

## 🐛 常见错误及解决方案

### 错误 1：`#include <Arduino.h> not found`

**原因：** 没有选择正确的开发板

**解决：**
- 工具 → 开发板 → ESP32 Arduino → 选择你的开发板型号

---

### 错误 2：`#include <WiFi.h> not found`

**原因：** ESP32 开发板支持包未正确安装

**解决：**
1. 重新安装 ESP32 开发板支持包
2. 确保使用的是官方 ESP32 包，不是第三方包

---

### 错误 3：`undefined reference to NeuroLib::NeuroLib()`

**原因：** 库文件没有被正确编译

**解决：**
1. 确保 `library.properties` 文件存在且正确
2. 确保 `.cpp` 文件在 `src` 目录下
3. 重启 Arduino IDE

---

### 错误 4：多个库冲突

**症状：** 提示重复定义或冲突

**解决：**
1. 检查 `libraries` 目录下是否有多个 NeuroLib 文件夹
2. 只保留一个正确的版本
3. 清除缓存并重启 Arduino IDE

---

## 📝 完整的库文件结构

正确的 NeuroLib 库结构应该是：

```
NeuroLib/
├── src/
│   ├── NeuroLib.h           # 主头文件
│   ├── NeuroLib.cpp         # 主实现
│   ├── BluetoothConfig.h    # 蓝牙配置头文件
│   └── BluetoothConfig.cpp  # 蓝牙配置实现
├── examples/
│   ├── BasicEEG/
│   │   └── BasicEEG.ino
│   ├── FFTAnalysis/
│   │   └── FFTAnalysis.ino
│   ├── EEGWithBluetoothConfig/
│   │   ├── EEGWithBluetoothConfig.ino
│   │   └── README.md
│   ├── MultiChannel/
│   │   └── MultiChannel.ino
│   └── RealTimePlot/
│       └── RealTimePlot.ino
├── library.properties       # 库描述文件（必须）
├── README.md                # 说明文档
└── keywords.txt             # 语法高亮（可选）
```

---

## 🎯 快速修复脚本（PowerShell）

如果你有管理员权限，可以运行以下脚本快速修复：

```powershell
# 设置路径
$sourcePath = "d:\MyWork\neuro_lib"
$destPath = "C:\Users\$env:USERNAME\Documents\Arduino\libraries\NeuroLib"

# 删除旧版本（如果存在）
if (Test-Path $destPath) {
    Remove-Item $destPath -Recurse -Force
}

# 复制文件
Copy-Item $sourcePath $destPath -Recurse

Write-Host "NeuroLib library installed successfully!"
Write-Host "Please restart Arduino IDE."
```

---

## 📞 仍然有问题？

如果以上方法都不能解决问题，请提供：

1. 完整的错误信息
2. Arduino IDE 版本
3. ESP32 开发板型号
4. 库的安装路径
5. 尝试编译的示例名称

这样可以更准确地定位问题。
