# 🌟 Arduino 蓝牙智能环境监测与控制终端

> 从「连屏幕都不亮」到「融合多种传感器的智能节点」。
> 本项目完整记录了从点灯起步，逐步迭代到蓝牙无线控制、多传感器数据采集、声光反馈与环境光自动控制的全部代码、接线与踩坑经验。

[![Platform](https://img.shields.io/badge/platform-Arduino%20Uno-00979D?logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Language](https://img.shields.io/badge/language-C%2B%2B%20(Arduino)-00599C?logo=cplusplus&logoColor=white)](https://www.arduino.cc/reference/en/)
[![Sensors](https://img.shields.io/badge/sensors-DHT11%20%7C%20Light%20%7C%20Sound-orange)](#-硬件清单)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

---

## 📖 项目简介

本项目是一个基于 **Arduino Uno** 的多功能嵌入式原型系统。项目经历了从基础的**点灯**、**屏幕显示**，逐步迭代到**蓝牙无线控制**、**多传感器数据采集**、**声光反馈**以及**环境光自动控制**的过程。

通过手机端蓝牙调试助手，用户可以实现远程交互，并随时获取当前环境的**温度、湿度、光线以及声音状态**；在脱离手机时，仍可通过物理按键完成本地控制。

**当前最终版本：[`firmware/Buzzer_module/`](firmware/Buzzer_module/Buzzer_module.ino)** —— 集成全部传感器与声光反馈的完整节点（温湿度 / 光线 / 声音 / 蜂鸣器 / 双 LED / 蓝牙 / LCD / 按键）。

---

## ✨ 功能特性

- **蓝牙双向通信**：手机发送单字符指令，控制 LED（常亮 / 常灭 / 交替跳动），并接收传感器数据回传。
- **环境监测**
  - **温湿度**：支持手动查询当前温度与湿度（DHT11）。
  - **光线检测**：检测环境明暗，环境过暗时**自动开灯**。
  - **声音检测**：检测到拍手或大声说话时，触发 **LED 闪烁 + 蜂鸣器提示**，并向手机推送通知。
- **智能报警音**：集成低电平触发有源蜂鸣器，支持**按键提示音**、**蓝牙指令音**和**警报音**。
- **LCD 状态显示**：通过 **I2C** 接口实时同步显示 LED 状态、传感器读数及自定义彩蛋信息。
- **本地实体按键**：脱离手机时，可通过物理按键一键开关灯。
- **抗自激振荡**：软件冷却机制解决了「蜂鸣器声音被声音传感器再次拾取」导致的死循环报警。

---

## 🛠️ 硬件清单

| 硬件名称 | 数量 | 备注 |
| --- | --- | --- |
| Arduino Uno | 1 | 主控板 |
| LCD1602 显示屏 | 1 | **必须配备 I2C 转接板** |
| HC-05 蓝牙模块 | 1 | 经典蓝牙，波特率 **38400** |
| DHT11 温湿度传感器 | 1 | 数据引脚接 **D7** |
| 光敏电阻模块 | 1 | **数字输出 DO** 接 **D4** |
| 声音传感器模块 | 1 | **数字输出 I/O** 接 **D5** |
| 有源蜂鸣器模块 | 1 | **低电平触发**，I/O 接 **D3** |
| LED 灯 | 2 | 分别接 **D9** 和 **D8**，各串联 220Ω 电阻 |
| 按键（轻触开关） | 1 | 接 **D2**，开启内部上拉 |
| 面包板 + 杜邦线 | 若干 | 建议公对公 / 公对母各备一套 |
| USB 数据线 | 1 | ⚠️ 必须是**数据线**，纯充电线不识别端口 |

---

## 🔌 引脚接线表

| 外设 | Arduino 引脚 | 说明 |
| --- | --- | --- |
| **LCD1602 (I2C)** | `GND`, `5V`, `A4 (SDA)`, `A5 (SCL)` | I2C 专用引脚，仅需 4 线 |
| **HC-05 蓝牙** | `GND`, `5V`, `D10`, `D11` | 软串口。**蓝牙 TX → D10，蓝牙 RX → D11（必须交叉）** |
| **DHT11** | `GND`, `5V`, `D7 (DATA)` | 数据引脚接 D7 |
| **光敏模块 DO** | `GND`, `5V`, `D4` | 探测明暗（数字量） |
| **声音模块 I/O** | `GND`, `5V`, `D5` | 探测声音强度（数字量） |
| **蜂鸣器 I/O** | `GND`, `5V`, `D3` | 低电平触发发声 |
| **LED 1** | `D9` | 需串联 **220Ω** 电阻 |
| **LED 2** | `D8` | 需串联 **220Ω** 电阻 |
| **按键** | `D2` | `INPUT_PULLUP`，另一端接 GND |

**文字版接线清单（可直接照着插）：**

```
LCD1602 I2C   →  GND / 5V / A4(SDA) / A5(SCL)
HC-05         →  GND / 5V / D10(接模块TX) / D11(接模块RX)
DHT11         →  GND / 5V / D7(DATA)
光敏模块 DO    →  GND / 5V / D4
声音模块 I/O   →  GND / 5V / D5
蜂鸣器 I/O     →  GND / 5V / D3
LED1          →  D9 —— 220Ω —— GND
LED2          →  D8 —— 220Ω —— GND
按键           →  D2 与 GND 之间
```

**引脚占用总览（D0–D13 + A4/A5）：**

```
D0/D1  串口调试（保留）       D2   按键
D3     蜂鸣器                 D4   光敏 DO
D5     声音模块               D6   空闲
D7     DHT11 数据             D8   LED2
D9     LED1                   D10  蓝牙 TX
D11    蓝牙 RX（软串口）       D12  空闲
D13    板载 LED               A4/A5  LCD I2C (SDA/SCL)
```

> 📐 更详细的接线说明、I2C 地址扫描方法与面包板布局见 **[docs/WIRING.md](docs/WIRING.md)**。

---

## 💻 手机蓝牙指令表

> 在手机端「**蓝牙串口助手**」中，以 **ASCII / 文本模式** 发送（不要用 HEX 模式）。

| 指令 | 功能描述 | 屏幕 / 声音反馈 |
| :---: | --- | --- |
| `1` | LED 常亮 | 第二行 `LED: ON`；蜂鸣器提示一声 |
| `0` | LED 关闭 | 第二行 `LED: OFF`；蜂鸣器提示一声 |
| `2` | LED 交替跳动（闪烁） | 第二行 `LED: Blink`；每次跳动伴随短促提示音 |
| `3` | 触发中秋彩蛋 | 显示 `Happy Mid-Autumn` 与 `Festival`，蜂鸣器响三声 |
| `L` / `l` | 查询光线状态 | 显示 `Status: Dark` 或 `Status: Bright`；若为 Dark 则**自动开灯** |
| `T` / `t` | 查询温湿度 | 显示 `Temp: 26.3 C` / `Humi: 58 %`，并回传手机 |
| `B` / `b` | 触发警报音 | 蜂鸣器急促响三声 |
| （物理按键） | 本地切换常亮 / 常灭 | 屏幕同步刷新，并伴随按键提示音 |

**手机端收到的数据示例：**

```
Light: Dark (LED Auto ON)
Light: Bright
>> Sound Detected! <<
Temp: 26.3C, Humi: 58%
```

> 🔍 完整指令协议（含 HEX 对照、扩展指令建议）见 **[docs/COMMANDS.md](docs/COMMANDS.md)**。

---

## 📂 仓库目录结构

```
Arduino-Intro/
├── README.md                     ← 你正在读的这份文档
├── LICENSE                       ← MIT 许可证
├── .gitignore / .gitattributes
├── firmware/                     ← 按学习顺序排列的 Arduino sketch
│   ├── led_running_on_off/       ← 阶段1：外部 LED 闪烁 + 串口打印
│   ├── screen_words/             ← 阶段2：LCD1602 并行接线（早期实验）
│   ├── LCD_show_words/           ← 阶段3：升级为 I2C 接线驱动 LCD
│   ├── use_button_control_led/   ← 阶段4：按键 + 蓝牙控灯
│   ├── temp_monitor/             ← 阶段5：加入 DHT11 温湿度
│   ├── Intelligent_LED/          ← 阶段6：加入光敏，实现环境光自动开灯
│   └── Buzzer_module/            ← 阶段7 ⭐ 最终完整版（全部传感器 + 声光）
├── Image_and_video/              ← 演示图片与视频
│   ├── Buzzer_sound.jpg          ← 蜂鸣器声光反馈
│   ├── Bluetooth-control.mp4     ← 蓝牙远程控制
│   ├── button_control.mp4        ← 物理按键控制
│   └── temp-humi-monitor.mp4     ← 温湿度监测
├── libraries/                    ← 项目依赖的第三方库原样副本
│   ├── Adafruit_Unified_Sensor/  ← v1.1.15（DHT 库的依赖）
│   ├── DHT_sensor_library/       ← v1.4.7
│   ├── LiquidCrystal/            ← v1.0.7（并行 LCD）
│   └── LiquidCrystal_I2C/        ← v1.1.2（I2C LCD，本项目主用）
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

下载 [Arduino IDE 2.x](https://www.arduino.cc/en/software)（1.8.x 同样可用）。

### 第 2 步：安装依赖库

**方式 A —— 手动安装（推荐，完全离线可用）**

把本仓库 `libraries/` 下的 4 个文件夹，整体复制到你的 Arduino 库目录：

| 系统 | 库目录路径 |
| --- | --- |
| Windows | `C:\Users\<用户名>\Documents\Arduino\libraries\` |
| macOS | `~/Documents/Arduino/libraries/` |
| Linux | `~/Arduino/libraries/` |

复制完成后**重启 Arduino IDE**，在 `文件 → 示例` 下应能看到 `DHT sensor library`、`LiquidCrystal` 等条目。

**方式 B —— 库管理器在线安装**

`工具 → 管理库`，搜索并安装：

- `DHT sensor library`（Adafruit）→ 提示依赖时选择 **Install all**（会自动装上 Adafruit Unified Sensor）
- `LiquidCrystal I2C`（Frank de Brabander）

### 第 3 步：确认 LCD 的 I2C 地址

LCD 背光亮但不出字，**九成是地址不对或对比度没调**。先上传 `文件 → 示例 → Wire → i2c_scanner` 扫描地址（常见 `0x27` 或 `0x3F`），再修改代码：

```cpp
LiquidCrystal_I2C lcd(0x27, 16, 2);   // 改成你扫到的地址
```

同时**缓慢旋转转接板上的蓝色电位器**调节对比度（要转十几圈），直到字符出现。

### 第 4 步：确认蓝牙波特率

```cpp
BT.begin(38400);   // 本项目实测该模块为 38400
```

> ⚠️ 如果你的模块是 9600，请改成 `BT.begin(9600);`。
> 若手机发指令后串口打印 `FFFF` / `C7` 之类乱码，**几乎可以确定是波特率不匹配**。

### 第 5 步：上传代码

1. `工具 → 开发板 → Arduino AVR Boards → Arduino Uno`
2. `工具 → 端口 → COMx (Arduino Uno)`
3. ⚠️ **上传前务必拔掉 HC-05 的 TX/RX 两根线**（软串口占用 D10/D11 会干扰下载）
4. 点击「上传」
5. 上传成功后插回蓝牙线，**按一下 Arduino 板上的 Reset 键**
6. 打开串口监视器，波特率设为 **9600**

### 第 6 步：手机连接

1. 手机蓝牙设置中配对 `HC-05`（配对码 **`1234`** 或 `0000`）
2. 打开蓝牙串口助手，连接 `HC-05`，确认底板 LED 由快闪变**慢闪**
3. **切换为 ASCII / 文本模式**，发送 `1` —— LED 应亮起，LCD 显示 `LED: ON` 🎉

---

## 🔬 核心软件架构与踩坑记录

> 完整排错手册见 **[docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md)**。

### 1. 非阻塞闪烁设计（`millis()` 替代 `delay()`）

实现 LED 交替跳动时**绝对不能使用 `delay()`**，否则会阻塞蓝牙接收和按键响应。项目采用 `millis()` 时间戳进行非阻塞轮询：

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

确保灯光闪烁的同时，手机指令依旧可以随时打断并接管控制。

> 💡 `currentMillis - previousMillis` 使用**无符号减法**，即使 `millis()` 约 49.7 天溢出一次也能正确计算。不要写成 `currentMillis > previousMillis + interval`。

**⚠️ 一个诚实的说明**：本项目在 `beep()`、按键消抖和传感器查询的 2 秒停留中**仍然使用了 `delay()`**，因为蜂鸣器发声本身需要精确的阻塞计时。这几处阻塞期间蓝牙仍可能丢包，是后续可优化点之一（见[未来计划](#-未来计划)）。

### 2. 软串口与蓝牙通信坑点

- **上传冲突**：使用 `SoftwareSerial` 时，上传代码前**必须拔掉蓝牙的 TX/RX 线**。上传成功后插回，并**按一下 Arduino Reset 键**。
- **波特率匹配**：蓝牙模块出厂默认可能是 9600，但本项目**实测该模块为 38400**。若手机发送指令后串口打印乱码（如 `FFFF` / `C7`），需确认波特率是否匹配。
- **发送模式**：手机 App 必须切换为 **ASCII / 文本模式**（非 HEX 模式），否则 Arduino 接收到的是十六进制数值（如 `0x01`）而非字符 `'1'`，永远匹配不上。

### 3. 传感器实战经验

- **DHT11 读取异常**：供电松动会导致 `Sensor Error!`。须确保 VCC 和 GND 接触良好，且**读取间隔大于 1 秒**。
- **光敏模块**：本项目使用的是**数字输出（DO）**模块，无法获取具体光照数值，只能判断「亮 / 暗」。需要手动旋转模块上的**蓝色电位器**来调节触发阈值。
- **声音传感器与蜂鸣器「自激振荡」**：蜂鸣器发出的声音会被声音传感器再次拾取，导致**死循环报警**。
  - **硬件层**：拉大两者距离、逆时针调节传感器灵敏度。
  - **软件层**：加入 `lastBuzzerTime` 冷却时间，蜂鸣器响完后的 **600ms** 内忽略声音传感器的触发；同时声音检测本身还有 **1000ms** 冷却。

```cpp
unsigned long lastBuzzerTime = 0;
const long buzzerCooldown = 600;

void beep(int duration) {
  lastBuzzerTime = millis();          // 记录发声时刻
  digitalWrite(buzzerPin, LOW);       // 低电平触发
  delay(duration);
  digitalWrite(buzzerPin, HIGH);
  delay(50);
}

// 冷却期内忽略声音触发，打破自激回路
if (digitalRead(soundPin) == LOW
    && (millis() - lastSoundTime > soundCooldown)
    && (millis() - lastBuzzerTime > buzzerCooldown)) { ... }
```

### 4. 超声波模块避坑（失败案例记录）

早期测试的 **HC-SR04 超声波模块通电后发热严重并导致开发板掉电**，判定为模块内部短路。

> ⚠️ **建议**：新手在使用此类大电流模块前，先确认 **VCC 和 GND 未接反**；如遇发热，**应立即拔除**，防止烧毁主板。

### 5. 两个版本的光敏逻辑差异

| 版本 | 环境变暗 | 环境变亮 |
| --- | --- | --- |
| [`Intelligent_LED`](firmware/Intelligent_LED/Intelligent_LED.ino) | 自动开灯 | **自动关灯** |
| [`Buzzer_module`](firmware/Buzzer_module/Buzzer_module.ino) ⭐ | 自动开灯 | 仅上报 `Light: Bright`，**不自动关灯** |

最终版选择「只自动开、不自动关」，是为了避免光线瞬时波动（如有人经过投下阴影）造成灯光反复开关。

### 6. 界面状态机与字符串覆盖技巧

所有输入源（蓝牙、按键）都只修改 `ledMode` 这一个状态变量，然后统一调用 `updateLED()` 与 `updateLCD()` 刷新，**避免屏幕显示与硬件状态脱节**。

LCD1602 是字符型显示器，**不会自动清除旧内容**。`OFF` 被 `ON` 覆盖时第三位会残留 `F`，显示成 `LED: ONF`。因此在短字符串末尾**刻意补空格**把残留字符盖掉：

```cpp
lcd.print("LED: OFF ");    // 末尾空格用于覆盖残留字符
lcd.print("LED: ON  ");
```

---

## 🎥 演示视频

| 演示内容 | 文件 |
| --- | --- |
| 蓝牙远程控制 LED + LCD 状态同步 | [Bluetooth-control.mp4](Image_and_video/Bluetooth-control.mp4) |
| 物理按键本地控制 | [button_control.mp4](Image_and_video/button_control.mp4) |
| 温湿度采集与手机端回传 | [temp-humi-monitor.mp4](Image_and_video/temp-humi-monitor.mp4) |
| 蜂鸣器声光反馈与声音检测 | [Buzzer_sound.jpg](Image_and_video/Buzzer_sound.jpg) |

> 📹 建议视频分两段：前半段直接展示**成品功能**，后半段展示**接线教程与踩坑排查**，兼顾开发者与新手两类观众。

---

## 🚀 未来计划

- [ ] 将主控更换为自带 WiFi 的 **ESP32**，接入**点灯科技（Blinker）**或**巴法云**，实现真正的异地远程控制。
- [ ] 接入 **0.96 寸 OLED** 屏幕，显示更多信息并支持中文字库。
- [ ] 整合**红外遥控器**，实现蓝牙与红外双模控制。
- [ ] **消除剩余的 `delay()`**：把 `beep()`、按键消抖、传感器查询的 2 秒停留改造为 `millis()` 状态机，做到全程非阻塞。
- [ ] **代码模块化拆分**：把 `readAndReportLight()`、`readAndReportDHT()`、`beep()` 等函数拆到独立 `.h` 文件，让主文件更简洁。
- [ ] 光敏改用 **AO 模拟输出**，获取具体光照数值而非仅有「亮 / 暗」二值。
- [ ] 增加 `platformio.ini`，支持 PlatformIO 一键构建。

---

## 🤝 贡献与致谢

- 欢迎提交 Issue 反馈问题，或 PR 补充你的踩坑经验。
- 第三方库版权归各自作者所有，详见 [docs/LIBRARIES.md](docs/LIBRARIES.md)。

## 📄 许可证

本项目自有代码采用 **MIT License**，详见 [LICENSE](LICENSE)。`libraries/` 目录下的第三方库遵循其原始许可证。
