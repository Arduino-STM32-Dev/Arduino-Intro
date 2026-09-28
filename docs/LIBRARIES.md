# 📚 依赖库说明

本目录（`libraries/`）保存了项目**实际运行验证过的**第三方库原样副本，方便离线安装和精确复现构建环境。

---

## 为什么要把库也提交进仓库？

Arduino 生态里的库更新频繁且**存在不兼容分支**。本项目就踩过这个坑：`LiquidCrystal_I2C` 库在不同分支中，LCD 初始化函数可能是 `lcd.init()`，也可能是 `lcd.begin(16, 2)` —— 用错版本**编译能过，但屏幕完全不动**，排查起来极其费时。

把验证过的库副本随仓库提交，可以保证：

| 收益 | 说明 |
| --- | --- |
| **可复现** | 任何人 clone 下来都能得到和作者完全一致的构建环境 |
| **离线可用** | 没有网络也能安装依赖 |
| **避开版本陷阱** | 不会被库管理器自动升级到不兼容版本 |
| **便于对照** | 出问题时可以直接读库源码，而不是猜 |

> ⚠️ 这些库的版权归各自作者所有，本项目只是原样分发。详见下方许可证说明。

---

## 依赖清单

| 库 | 版本 | 作者 / 维护者 | 用途 | 许可证 |
| --- | --- | --- | --- | --- |
| **DHT sensor library** | 1.4.7 | Adafruit | DHT11/DHT22 等温湿度传感器驱动 | MIT |
| **Adafruit Unified Sensor** | 1.1.15 | Adafruit | DHT 库的必需依赖（传感器抽象层） | Apache-2.0 |
| **LiquidCrystal_I2C** | 1.1.2 | Frank de Brabander / Marco Schwartz | I2C 接口 LCD 驱动（**本项目主用**） | 见下方说明 |
| **LiquidCrystal** | 1.0.7 | Arduino / Adafruit | 并行接口 LCD 驱动（早期实验用） | LGPL-2.1 |
| `Wire` | 内置 | Arduino | 硬件 I2C 通信（`#include <Wire.h>`） | — |
| `SoftwareSerial` | 内置 | Arduino | 软件模拟串口，用于蓝牙 | — |
| `EEPROM` | 内置 | Arduino | 未使用，预留 | — |

---

## 各库详解

### 1. DHT sensor library v1.4.7

- **来源**：<https://github.com/adafruit/DHT-sensor-library>
- **头文件**：`#include <DHT.h>`
- **本项目使用**：

```cpp
#include <DHT.h>
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

dht.begin();
float t = dht.readTemperature();   // 摄氏度
float h = dht.readHumidity();      // 相对湿度 %
```

- **关键 API**：

| 方法 | 说明 |
| --- | --- |
| `begin()` | 初始化传感器 |
| `readTemperature()` | 读摄氏温度，失败返回 `NaN` |
| `readTemperature(true)` | 读华氏温度 |
| `readHumidity()` | 读相对湿度，失败返回 `NaN` |
| `computeHeatIndex(t, h, isFahrenheit)` | 计算体感温度 |
| `read()` | 一次性读出温度和湿度（**比连续调两次 read 更快、更一致**） |

- **⚠️ 注意事项**：
  - 采样间隔必须 **≥ 1 秒**（DHT11 数据手册建议 2 秒）
  - **必须用 `isnan()` 检查返回值**，失败时返回 `NaN`
  - 型号必须与实际硬件一致（`DHT11` / `DHT22` / `DHT21` / `AM2302`），否则读数错误
- **依赖**：需要 `Adafruit_Unified_Sensor`（本项目已一并打包）
- **许可证**：MIT（Copyright © 2020 Adafruit Industries），原文见 [DHT_sensor_library/license.txt](../libraries/DHT_sensor_library/license.txt)

### 2. Adafruit Unified Sensor v1.1.15

- **来源**：<https://github.com/adafruit/Adafruit_Sensor>
- **作用**：提供统一的传感器抽象层（`Adafruit_Sensor` 基类、`sensors_event_t` 事件结构）。这是 DHT 库的**硬依赖**，缺失会导致编译报错：

```
fatal error: Adafruit_Sensor.h: No such file or directory
```

- **本项目使用**：**不直接调用**，仅作为 `DHT.h` 的传递依赖存在
- **头文件**：`#include <Adafruit_Sensor.h>`
- **许可证**：Apache-2.0，原文见 [Adafruit_Unified_Sensor/LICENSE.txt](../libraries/Adafruit_Unified_Sensor/LICENSE.txt)

### 3. LiquidCrystal_I2C v1.1.2 ⭐ 本项目主用

- **来源**：<https://github.com/marcoschwartz/LiquidCrystal_I2C>（DFRobot I2C LCD 的移植版）
- **头文件**：`#include <LiquidCrystal_I2C.h>`
- **本项目使用**：

```cpp
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);   // 地址, 列数, 行数

lcd.init();          // ⚠️ 本版本用 init()，不是 begin()
lcd.backlight();
lcd.setCursor(0, 0);
lcd.print("Thea, My Love!");
```

- **常用 API**：

| 方法 | 说明 |
| --- | --- |
| `init()` | 初始化（**本版本特有**，其它分支可能叫 `begin(cols, rows)`） |
| `backlight()` / `noBacklight()` | 开 / 关背光 |
| `setCursor(col, row)` | 定位光标 |
| `print(...)` | 输出（继承自 `Print` 类，支持各种类型和格式化） |
| `clear()` | 清屏并归位光标（耗时约 2ms，会导致可见闪烁，**不要频繁调用**） |
| `createChar(num, data[])` | 自定义字符（最多 8 个 5×8 点阵） |
| `scrollDisplayLeft()` / `scrollDisplayRight()` | 滚动显示 |
| `blink()` / `cursor()` | 显示闪烁光标 / 下划线光标 |

- **⚠️ 版本兼容性提醒**：这是本项目**最重要的坑之一**。

```cpp
lcd.init();         // ✅ 本项目打包的 v1.1.2 版本
lcd.begin(16, 2);   // ❌ 该版本没有这个函数，会编译报错
```

  社区里流传着多个 API 不兼容的分支。**请直接使用本仓库打包的版本**，或确认你安装的版本用哪个初始化函数。

- **许可证**：⚠️ **本项目打包的这份副本内未包含许可证文件**。上游项目（[marcoschwartz/LiquidCrystal_I2C](https://github.com/marcoschwartz/LiquidCrystal_I2C)）的仓库也未提供标准许可证识别文件，原始版本（DFRobot）采用 MIT 类许可。**如果你要再分发，请自行向原始作者确认授权条款。** 本仓库仅作为学习记录原样保留该副本。

### 4. LiquidCrystal v1.0.7（早期实验用）

- **来源**：<https://github.com/arduino-libraries/LiquidCrystal>
- **头文件**：`#include <LiquidCrystal.h>`
- **本项目使用**：仅用于 [firmware/screen_words](../firmware/screen_words/screen_words.ino)（阶段 2 的并行接线实验，**引脚与蓝牙冲突，不要与最终版混用**）

```cpp
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);   // RS, E, D4, D5, D6, D7
lcd.begin(16, 2);                        // ⚠️ 这个库用 begin(cols, rows)
```

- **许可证**：**LGPL-2.1**
- **LPGL 注意事项**：LGPL 允许你在专有项目中链接使用该库，但**如果你修改了库本身并分发，修改部分必须以 LGPL 开源**。本项目未修改该库，只是原样使用。
- ⚠️ 该库更新缓慢，官方已推荐改用 `hd44780` 库。本项目保留它仅为记录历史路径。

### 5. 内置库（无需安装）

| 库 | 头文件 | 本项目用途 |
| --- | --- | --- |
| `Wire` | `#include <Wire.h>` | 驱动 I2C 总线（LCD1602 的 SDA/SCL） |
| `SoftwareSerial` | `#include <SoftwareSerial.h>` | 在 D10/D11 上模拟串口连接 HC-05 |
| `EEPROM` | `#include <EEPROM.h>` | 未使用 |
| `SPI` | `#include <SPI.h>` | 未使用 |

> 💡 **关于 `SoftwareSerial` 的两个限制**（影响 D10/D11 的选择）：
> 1. **波特率上限约 115200**，且高波特率下容易丢字节。本项目用 38400 很稳。
> 2. **不能同时监听多个软串口**，且软串口接收期间会禁用中断，可能影响 `millis()` 精度（本项目影响可忽略）。
>
> 更好的选择是用 **D2/D3**（外部中断脚）做软串口，或者直接上 **Arduino Mega**（4 个硬件串口）。这是本项目记录的一个可优化点。

---

## 安装方法

### 方法 A：复制到库目录（推荐，离线可用）

把本目录下的 4 个库文件夹整体复制到 Arduino 库目录：

| 系统 | 路径 |
| --- | --- |
| Windows | `C:\Users\<用户名>\Documents\Arduino\libraries\` |
| macOS | `~/Documents/Arduino/libraries/` |
| Linux | `~/Arduino/libraries/` |

复制后**重启 Arduino IDE**。验证方法：打开 `文件 → 示例`，应能看到 `DHT sensor library`、`LiquidCrystal`、`LiquidCrystal_I2C` 等菜单项。

### 方法 B：库管理器在线安装

`工具 → 管理库`（或按 `Ctrl+Shift+I`）：

1. 搜索 **`DHT sensor library`** → 选 Adafruit 版本 → 安装时会弹出依赖提示，**务必选择「Install all」**（会自动装上 Adafruit Unified Sensor）
2. 搜索 **`LiquidCrystal I2C`** → 选 Frank de Brabander 版本

> ⚠️ 在线安装的版本可能与本仓库打包的不同。**如果屏幕不显示，第一件事就是检查 `lcd.init()` 是否需要改成 `lcd.begin(16,2)`。**

### 方法 C：ZIP 导入

Arduino IDE：`项目 → 包含库 → 添加 .ZIP 库`，选择对应的库压缩包。

---

## 构建环境记录

本项目验证通过的环境：

| 项目 | 值 |
| --- | --- |
| IDE | Arduino IDE 2.x（1.8.x 同样可用） |
| 开发板 | Arduino Uno（ATmega328P，16MHz，2KB SRAM / 32KB Flash） |
| 核心包 | Arduino AVR Boards |
| 蓝牙 | HC-05，SPP 经典蓝牙，**38400 波特率** |
| 验证日期 | 2025 |

**资源占用参考**（`temp_monitor` 编译后，实际数值以 IDE 输出为准）：

| 资源 | 占用 | 说明 |
| --- | --- | --- |
| Flash | 约 30% | 含 DHT + LCD + SoftwareSerial 库，Uno 的 32KB 完全够用 |
| SRAM | 约 25% | LCD 库和字符串常量是主要占用方 |

> 💡 **SRAM 是 Uno 的稀缺资源（仅 2KB）**。大量使用 `String`、长字符串字面量、大数组会导致 SRAM 耗尽，表现为**程序随机重启或行为诡异**。本项目全部使用 `const char*` 字面量和 `F()` 宏的安全范围内写法，未遇到该问题。如果要加大量文本，建议用 `Serial.print(F("..."))` 把字符串放进 Flash。

---

## 许可证汇总

| 库 | 许可证 | 是否允许商用 | 是否要求开源衍生代码 |
| --- | --- | --- | --- |
| DHT sensor library | MIT | ✅ | 否（保留版权声明即可） |
| Adafruit Unified Sensor | Apache-2.0 | ✅ | 否（保留声明 + 变更说明） |
| LiquidCrystal_I2C | ⚠️ 未标明 | ❓ 需向原作者确认 | ❓ |
| LiquidCrystal | LGPL-2.1 | ✅ | 修改库本身则需要 |

**本项目自有代码**（`firmware/`、`docs/`、根目录文档）采用 **MIT License**，见 [../LICENSE](../LICENSE)。
