# 基于 Arduino Uno 的蓝牙温湿度监控与智能灯控系统

> 一个从「点亮一颗 LED」开始的 Arduino 硬件探索项目。
> 从 16 脚接线的蜘蛛网，到拥有蓝牙交互、环境数据采集、非阻塞闪烁的完整 IoT 原型 —— 这里记录了全部代码、踩过的坑和最后跑通的完整方案。

[![Platform](https://img.shields.io/badge/platform-Arduino%20Uno-00979D?logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Language](https://img.shields.io/badge/language-C%2B%2B%20(Arduino)-00599C?logo=cplusplus&logoColor=white)](https://www.arduino.cc/reference/en/)
[![Bluetooth](https://img.shields.io/badge/wireless-HC--05%20SPP-0082FC?logo=bluetooth&logoColor=white)](#-手机蓝牙指令表)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

---

## 📖 项目简介

这是一个基于 **Arduino Uno** 的完整物联网（IoT）原型。项目最初从基础的 LED 与按键控制出发，逐步集成了 **LCD1602 显示屏**、**HC-05 蓝牙模块** 和 **DHT11 温湿度传感器**。

通过手机端的蓝牙串口助手，用户可以：

- 📱 远程控制 LED 的开关与闪烁模式
- 🌡️ 查询现场温湿度，并且数据会回传到手机屏幕
- 🎉 触发自定义的趣味交互（LCD 彩蛋文字）
- 🔘 同时保留物理按键的本地控制，拔掉手机也能用

**这个仓库的定位不只是「一份能跑的代码」，更是一份完整的学习轨迹档案**：`firmware/` 里的 5 个 sketch 按能力递进排列，`libraries/` 里保留了实际使用到的第三方库原样副本，[docs/ROADMAP.md](docs/ROADMAP.md) 记录了每一步的认知跃迁，[docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) 记录了几乎所有新手都会撞上的坑。

---

## ✨ 核心功能

| 功能 | 说明 | 对应代码 |
| --- | --- | --- |
| **蓝牙无线控制** | 手机端发送单字符 ASCII 指令，远程切换 LED 常亮 / 常关 / 交替闪烁 | [temp_monitor.ino](firmware/temp_monitor/temp_monitor.ino) |
| **环境数据采集与上报** | 集成 DHT11，手机发送 `T` 即可查询温湿度，结果同时显示在 LCD 并回传到手机 | [temp_monitor.ino](firmware/temp_monitor/temp_monitor.ino) |
| **非阻塞闪烁逻辑** | 摒弃 `delay()` 阻塞写法，用 `millis()` 时间戳实现双灯交替闪烁，闪烁期间蓝牙通信完全不丢包 | [temp_monitor.ino](firmware/temp_monitor/temp_monitor.ino) |
| **本地按键交互** | 物理按键一键切换 LED 常亮 / 常灭，带软件消抖，不依赖手机 | [use_button_control_led.ino](firmware/use_button_control_led/use_button_control_led.ino) |
| **界面状态机管理** | 用 `ledMode`（0/1/2）统一管理硬件状态与 LCD 刷新，杜绝屏幕显示与实际状态不一致 | [temp_monitor.ino](firmware/temp_monitor/temp_monitor.ino) |
| **彩蛋交互** | 发送 `3` 在 LCD 上显示个性化节日祝福文字 | [temp_monitor.ino](firmware/temp_monitor/temp_monitor.ino) |
| **软件串口隔离** | `SoftwareSerial`(D10/D11) 专供蓝牙，`Serial`(D0/D1) 保留给电脑端调试 | 全部蓝牙 sketch |

---

## 🛠️ 硬件清单

| 硬件名称 | 数量 | 备注 |
| --- | --- | --- |
| Arduino Uno | 1 | 主控板（本项目基于 R3 兼容板验证） |
| LCD1602 显示屏 | 1 | **带 I2C 转接板（PCF8574）**，大幅简化接线，必选 |
| HC-05 蓝牙模块 | 1 | 带底板，经典蓝牙，支持 SPP 协议 |
| DHT11 温湿度传感器 | 1 | 带底板（模块版已内置上拉电阻），DATA 引脚接 **D7** |
| LED 灯 | 2 | 分别接 **D9** 和 **D8**，需串联 220Ω 限流电阻 |
| 按键（轻触开关） | 1 | 接 **D2**，另一端接 GND，启用内部上拉 |
| 面包板 + 杜邦线 | 若干 | 建议公对公 / 公对母各备一套 |
| USB 数据线 | 1 | ⚠️ 必须是**数据线**，纯充电线会导致端口不识别 |

---

## 🔌 引脚接线表

### 完整版（temp_monitor / use_button_control_led）

| 外设 | Arduino 引脚 | 说明 |
| --- | --- | --- |
| LCD1602 (I2C) | `GND`, `5V`, `A4 (SDA)`, `A5 (SCL)` | I2C 专用引脚，仅需 4 根线 |
| HC-05 蓝牙 | `GND`, `5V`, `D10`, `D11` | 软串口。**HC-05 RX ← D11，HC-05 TX → D10（必须交叉）** |
| DHT11 | `GND`, `5V`, `D7 (DATA)` | 数据引脚固定 D7；模块版电源指示灯须点亮 |
| LED 1 | `D9` | 串联 220Ω 电阻到 GND |
| LED 2 | `D8` | 串联 220Ω 电阻到 GND |
| 按键 | `D2` | `INPUT_PULLUP`，另一端接 GND，按下为低电平 |

**文字版接线清单（可直接照着插）：**

```
LCD1602 I2C  →  GND / 5V / A4(SDA) / A5(SCL)
HC-05        →  GND / 5V / D10(接模块TX) / D11(接模块RX)
DHT11        →  GND / 5V / D7(DATA)
LED1         →  D9 —— 220Ω —— GND
LED2         →  D8 —— 220Ω —— GND
按键          →  D2 与 GND 之间
```

> 📐 更详细的接线说明、I2C 地址扫描方法和实物排布建议见 [docs/WIRING.md](docs/WIRING.md)。

### 早期版本（screen_words：并行 4 位接法）

| LCD 引脚 | Arduino 引脚 |
| --- | --- |
| RS | D12 |
| E（Enable） | D11 |
| D4 / D5 / D6 / D7 | D5 / D4 / D3 / D2 |
| VSS / VDD / V0 | GND / 5V / 电位器中间脚 |
| RW | GND |
| A / K（背光） | 5V（串 220Ω）/ GND |

> ⚠️ 这是本项目最早期的实验代码，占用了 6 个 IO 且与蓝牙软串口（D10/D11）引脚冲突，**不要与蓝牙代码同时使用**。保留它正是为了记录「从并行接线升级到 I2C」这条优化路径。

---

## 📱 手机蓝牙指令表

手机端安装「**蓝牙串口助手**」类 App（如 *Serial Bluetooth Terminal*），**以 ASCII/文本模式**发送（不要用 HEX 模式）：

| 指令 | 功能描述 | LCD 屏幕变化 |
| :---: | --- | --- |
| `1` | LED 常亮 | 第一行恢复默认，第二行显示 `LED: ON` |
| `0` | LED 关闭 | 第一行恢复默认，第二行显示 `LED: OFF` |
| `2` | LED 交替跳动（闪烁） | 第一行恢复默认，第二行显示 `LED: Blink` |
| `3` | 触发中秋彩蛋 | 第一行显示 `Happy Mid-Autumn`，第二行显示 `Festival` |
| `T` / `t` | 手动查询温湿度 | 屏幕显示温湿度，并通过蓝牙把数据回传到手机 |
| （物理按键） | 本地切换常亮 / 常灭 | 屏幕同步刷新状态 |

**手机端收到的数据示例：**

```
Temp: 26.3C, Humi: 58%
```

**电脑端串口监视器（9600 波特率）的调试输出示例：**

```
BT Received (HEX): 31     ← 收到 ASCII '1'
BT Received (HEX): 54     ← 收到 ASCII 'T'
```

> 🔍 完整指令协议（含换行符处理、HEX 对照、扩展指令建议）见 [docs/COMMANDS.md](docs/COMMANDS.md)。

---

## 📂 仓库目录结构

```
Arduino-Intro/
├── README.md                     ← 你正在读的这份文档
├── LICENSE                       ← MIT 许可证
├── .gitignore
├── firmware/                     ← 按学习顺序排列的 5 个 Arduino sketch
│   ├── led_running_on_off/       ← 第 1 步：外部 LED 闪烁 + 串口打印
│   ├── screen_words/             ← 第 2 步：LCD1602 并行接线驱动（早期实验，源文件含损坏行）
│   ├── LCD_show_words/           ← 第 3 步：升级为 I2C 接线驱动 LCD
│   ├── use_button_control_led/   ← 第 4 步：按键 + I2C LCD + 蓝牙控灯
│   └── temp_monitor/             ← 第 5 步：⭐ 最终完整版（+ DHT11 温湿度）
├── libraries/                    ← 项目实际依赖的第三方库原样副本
│   ├── Adafruit_Unified_Sensor/  ← v1.1.15，DHT 库的依赖
│   ├── DHT_sensor_library/       ← v1.4.7，DHT11/22 驱动
│   ├── LiquidCrystal/            ← v1.0.7，并行 LCD 库
│   └── LiquidCrystal_I2C/        ← v1.1.2，I2C LCD 库
└── docs/
    ├── WIRING.md                 ← 接线详解与 I2C 地址扫描
    ├── COMMANDS.md               ← 蓝牙指令协议完整参考
    ├── TROUBLESHOOTING.md        ← ⭐ 踩坑记录与排错手册
    ├── ROADMAP.md                ← 学习路径与认知跃迁记录
    └── LIBRARIES.md              ← 依赖库说明、版本与许可证
```

---

## 🚀 快速开始

### 第 1 步：安装 Arduino IDE

下载 [Arduino IDE 2.x](https://www.arduino.cc/en/software)（或使用旧版 1.8.x，两者均可）。

### 第 2 步：安装依赖库

**方式 A —— 手动安装（推荐，完全离线可用）**

把本仓库 `libraries/` 下的 4 个文件夹，整体复制到你的 Arduino 库目录：

| 操作系统 | 库目录路径 |
| --- | --- |
| Windows | `C:\Users\<你的用户名>\Documents\Arduino\libraries\` |
| macOS | `~/Documents/Arduino/libraries/` |
| Linux | `~/Arduino/libraries/` |

复制完成后重启 Arduino IDE，在 `文件 → 示例` 菜单下应能看到 `DHT sensor library`、`LiquidCrystal` 等条目。

**方式 B —— 库管理器在线安装**

在 Arduino IDE 中打开 `工具 → 管理库`，搜索并安装：

- `DHT sensor library`（Adafruit）→ 会提示同时安装 `Adafruit Unified Sensor`，选择 **Install all**
- `LiquidCrystal I2C`（Frank de Brabander）

### 第 3 步：扫描并确认 LCD 的 I2C 地址

LCD 背光亮但不出字，**九成是地址不对或对比度没调**。先用这个最小程序确认地址：在 IDE 中打开 `文件 → 示例 → Wire → i2c_scanner` 并上传，串口监视器（9600）会打印扫描到的地址，常见为 **`0x27`** 或 **`0x3F`**。

然后修改 sketch 中这一行，把 `0x27` 改成你实际扫到的地址：

```cpp
LiquidCrystal_I2C lcd(0x27, 16, 2);   // 如果你的模块是 0x3F 就改成 0x3F
```

### 第 4 步：确认蓝牙波特率

代码中使用的是 **38400**，这是本项目 HC-05 模块实测可用的速率：

```cpp
BT.begin(38400);
```

> ⚠️ 如果你的模块是 9600（市面上也很常见），请改成 `BT.begin(9600);`。
> 若手机发指令后电脑串口打印出 `FFFF`、`C7` 之类的乱码，**几乎可以确定是波特率不匹配**，详见 [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md)。

### 第 5 步：上传代码

1. 在 IDE 中选择开发板：`工具 → 开发板 → Arduino AVR Boards → Arduino Uno`
2. 选择端口：`工具 → 端口 → COMx (Arduino Uno)`
3. ⚠️ **上传前务必拔掉 HC-05 的 TX/RX 两根线**（软串口占用 D10/D11 会干扰下载）
4. 点击「上传」
5. 上传成功后插回蓝牙线，**按一下 Arduino 板上的 Reset 键**（否则蓝牙可能不工作）
6. 打开串口监视器，波特率设为 **9600**

### 第 6 步：手机连接蓝牙

1. 手机蓝牙设置里搜索并配对 `HC-05`（默认配对码 **`1234`** 或 `0000`）
2. 打开蓝牙串口助手 App，连接 `HC-05`
3. 发送 `1` —— LED 应该亮起，LCD 第二行显示 `LED: ON` 🎉

---

## 🔬 软件架构与关键实现逻辑

### 1. 软硬串口分离

```cpp
Serial.begin(9600);        // D0/D1 —— 保留给电脑端调试，不接外设
BT.begin(38400);           // SoftwareSerial BT(10, 11) —— 专供 HC-05
```

**为什么这么做？** D0/D1 同时也是代码下载通道，如果蓝牙直接接在这两个脚上，下载程序时会互相抢占，表现为上传失败或蓝牙收发异常。用 `SoftwareSerial` 把蓝牙挪到 D10/D11，调试口和通信口彻底解耦。

### 2. 非阻塞闪烁（`millis()` 替代 `delay()`）

```cpp
unsigned long previousMillis = 0;
const long blinkInterval = 200;

if (ledMode == 2) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= blinkInterval) {
    previousMillis = currentMillis;
    blinkState = !blinkState;
    digitalWrite(ledPin1, blinkState ? HIGH : LOW);
    digitalWrite(ledPin2, blinkState ? LOW : HIGH);   // 双灯反相 = 交替跳动
  }
}
```

这是本项目**最重要的一个技术点**。如果用 `delay(200)` 实现闪烁，MCU 在那 200ms 里什么都干不了，蓝牙缓冲区会溢出、按键会失灵、手机指令会丢。改用 `millis()` 时间戳比较（即经典的 *BlinkWithoutDelay* 模式），`loop()` 始终高速轮询，**闪烁期间蓝牙、按键、串口全部照常工作**。

> ⚠️ `currentMillis - previousMillis` 使用无符号减法，即使 `millis()` 溢出（约 49.7 天）也能正确计算，这是标准的安全写法。

### 3. 界面状态机管理

```cpp
int ledMode = 0;   // 0 = 常灭, 1 = 常亮, 2 = 交替闪烁
```

所有输入源（蓝牙、按键）都只修改 `ledMode` 这一个状态变量，然后统一调用 `updateLED()` 和 `updateLCD()` 刷新。这样**屏幕显示与硬件实际状态永远不会脱节**，也避免了多个分支里重复写 `digitalWrite` 造成的遗漏。

### 4. 字符串覆盖技巧

```cpp
lcd.print("LED: OFF ");    // 注意末尾的空格
lcd.print("LED: ON  ");    // 同样补足空格
```

LCD1602 是**字符型显示器，不会自动清除旧内容**。当 `OFF` 被 `ON` 覆盖时，第三位会残留一个 `F`，屏幕显示成 `LED: ONF`。因此在短字符串末尾**刻意补空格**把残留字符盖掉，是 LCD 编程里非常实用的小技巧。

### 5. 蓝牙输入过滤

```cpp
incomingChar = BT.read();
if (incomingChar == '\r' || incomingChar == '\n') return;   // 丢弃回车换行
```

手机 App 发送时通常会附带 `\r\n`。不过滤的话，缓冲区里会混入无意义的字节，导致 `Serial.print(incomingChar, HEX)` 打出一串 `D A`，干扰调试判断。

### 6. DHT11 读取与容错

```cpp
float t = dht.readTemperature();
float h = dht.readHumidity();
if (isnan(t) || isnan(h)) {
  lcd.print("Sensor Error!");
  BT.println("Sensor Error! Check wiring.");   // 同时把错误回传到手机
}
```

DHT11 偶尔会读取失败（返回 `NaN`），因此**必须做判空**，否则屏幕上会出现 `nan` 这种无意义显示。这里同时把错误信息通过蓝牙回传手机，让远程端也能第一时间发现问题。

---

## 🚨 调试踩坑记录

> 这些是新手的必经之路。完整版排错手册见 **[docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md)**。

### 1️⃣ LCD 背光亮但完全不显示内容

- **I2C 地址不对**：模块出厂常见 `0x27`，也有 `0x3F`。务必先用 `i2c_scanner` 扫描确认。
- **对比度没调**：转接板上的**蓝色电位器**就是调对比度的，慢慢旋转直到字符浮现。这是最容易被忽略的一步。
- **转接板虚焊**：DIY 焊接的 I2C 转接板，排针有一根没焊实就会整个不亮，用万用表通断档逐个确认。

### 2️⃣ 手机发指令，串口打印出 `FFFF` / `C7` 乱码

两个原因，按顺序排查：

- **波特率不匹配** 👉 模块实际是 38400（或 9600），而代码写的是另一个值。改 `BT.begin(...)` 的参数。
- **TX/RX 接反** 👉 必须交叉：**蓝牙 TX → Arduino D10，蓝牙 RX ← Arduino D11**。这是「接反线」的经典现场，串口能收到字节但全是乱的。

### 3️⃣ 上传代码失败 / 报 `avrdude: stk500_recv(): programmer is not responding`

- **拔掉蓝牙的 TX/RX 线再上传**，软串口占用 D10/D11 会干扰 bootloader。上传成功后再插回并**按 Reset**。
- 检查开发板和端口是否选对（`Arduino Uno` + 正确的 `COMx`）。
- 换一根 USB **数据线**（很多线只能充电）。

### 4️⃣ DHT11 频繁报 `Sensor Error!`

- **读取间隔必须大于 1 秒**。DHT11 采样率很低，连续快速读取必然失败。
- **VCC / GND 接触不良**：面包板孔位用久了会松，模块上的电源指示灯（红灯）必须常亮，否则就是没供上电。
- 数据线尽量短，远离 LED 的 PWM/开关走线，避免干扰。

### 5️⃣ LCD 显示 `LED: ONF` 之类的残留字符

短字符串覆盖长字符串时没清干净，按上文「字符串覆盖技巧」在末尾补空格即可。

### 6️⃣ 按键不灵敏 / 一次按下触发多次

代码里已有 `delay(150)` 软件消抖 + `lastButtonState` 边沿检测。如果仍然抖动，检查按键是否接触不良，或在硬件上并联 0.1µF 电容。

---

## 🎥 视频演示

> 📹 演示视频 / GIF 待补充（欢迎提交 PR）

**建议演示内容：**

1. 手机发送 `1` / `0`，LED 与 LCD 同步响应
2. 手机发送 `2`，双灯交替跳动 —— 同时连续发送其他指令证明**通信不丢包**
3. 按下物理按键，本地切换灯态
4. 手机发送 `T`，立刻收到温湿度回传
5. 彩蛋时刻：发送 `3` 点亮中秋祝福

---

## 🗺️ 未来计划 (To-Do)

- [ ] 将主控更换为自带 WiFi 的 **ESP32**，接入公有云（巴法云 / Blinker / MQTT），实现真正的无距离限制异地控制
- [ ] 增加 **OTA（空中升级）** 功能，免插线更新固件
- [ ] 优化电源管理，脱离电脑 USB，改用电池供电运行
- [ ] 增加 **5 秒定时自动上报**温湿度（当前为手机主动查询模式）
- [ ] 把代码模块化拆分（`lcd_ui` / `bt_proto` / `led_ctrl` / `sensor` 分文件），提升可维护性
- [ ] 补充演示视频与实物接线照片
- [ ] 增加 `platformio.ini`，支持 PlatformIO 一键构建

---

## 🤝 贡献与致谢

- 欢迎提交 Issue 反馈问题，或 PR 补充你的踩坑经验。
- 第三方库版权归各自作者所有，详见 [docs/LIBRARIES.md](docs/LIBRARIES.md)。

## 📄 许可证

本项目自有代码采用 **MIT License**，详见 [LICENSE](LICENSE)。`libraries/` 目录下的第三方库遵循其原始许可证。
