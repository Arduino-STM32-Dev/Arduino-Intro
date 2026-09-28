# 🗺️ 学习路径与认知跃迁记录

> 这份文档记录的是**项目是怎么一步步长出来的**。
> `firmware/` 里的 5 个 sketch 不是随意堆放的文件，而是一条完整的、由浅入深的能力曲线。

---

## 📈 五个阶段总览

```
阶段 1   led_running_on_off        点亮一颗 LED + 串口打印
   │      ── 理解 GPIO 输出、数字电平、串口监视器
   ▼
阶段 2   screen_words              并行接线驱动 LCD1602
   │      ── 理解 HD44780 时序、4 位模式、多引脚协同
   ▼
阶段 3   LCD_show_words            改用 I2C 转接板驱动 LCD
   │      ── 理解 I2C 总线、地址寻址、工程上的"降复杂度"决策
   ▼
阶段 4   use_button_control_led    按键 + LCD + 蓝牙控灯
   │      ── 理解输入检测、消抖、软硬串口分离、状态机
   ▼
阶段 5   temp_monitor          ⭐  加入 DHT11，物联网原型成型
          ── 理解传感器时序、双向通信、错误处理、非阻塞架构
```

---

## 阶段 1：点亮一颗 LED

**文件**：[firmware/led_running_on_off/led_running_on_off.ino](../firmware/led_running_on_off/led_running_on_off.ino)

```cpp
const int ledPin = 5;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("External LED test start");
}

void loop() {
  digitalWrite(ledPin, HIGH);
  Serial.println("External LED ON");
  delay(1000);
  digitalWrite(ledPin, LOW);
  Serial.println("External LED OFF");
  delay(1000);
}
```

### 学到的核心概念

| 概念 | 说明 |
| --- | --- |
| `pinMode(pin, OUTPUT)` | 把引脚配置为输出模式，可以主动驱动高低电平 |
| `digitalWrite(pin, HIGH/LOW)` | 输出 5V / 0V |
| `Serial.begin(9600)` | 打开调试串口，与电脑通信 |
| **限流电阻的必要性** | 不加电阻会烧 LED 甚至损伤 IO 口 |

### 为什么用 `Serial.println` 而不只是看灯闪？

这是一个很好的习惯：**让程序的内部状态可观测**。灯闪只告诉你「有输出」，而串口打印告诉你「代码走到了哪一行、输出的是什么」。后面所有阶段都靠这个习惯来定位问题 —— 当蓝牙收到乱码时，正是靠 `Serial.print(incomingChar, HEX)` 才发现是波特率问题。

### 这个阶段的局限

`delay(1000)` 意味着 MCU 每秒有 2 秒在「空转」。此时还看不出问题，但到阶段 4 引入蓝牙后，这个写法就成了必须解决的技术债。

---

## 阶段 2：第一次点屏（并行接线）

**文件**：[firmware/screen_words/screen_words.ino](../firmware/screen_words/screen_words.ino)

```cpp
#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);   // RS, E, D4, D5, D6, D7
```

### 遇到的实际困难

| 困难 | 具体表现 |
| --- | --- |
| **接线量巨大** | 12 根杜邦线，错一根就白屏 |
| **对比度难调** | 电位器转十几圈才找到位置，一度以为屏幕坏了 |
| **引脚被占满** | 一口气用掉 6 个 IO 口 |
| **与蓝牙冲突** | E 脚占用了 D11 —— 正是后来蓝牙软串口要用的引脚 |
| **文件损坏** | 串口乱码被误粘贴进源文件开头（见 [TROUBLESHOOTING.md #11](TROUBLESHOOTING.md#11-screenwords-文件损坏事件)） |

### 认知跃迁

这一阶段最大的收获不是「会点屏」，而是**第一次意识到硬件资源是有限的**。6 个 IO 口在 Uno 的 20 个可用 IO 里占了近三分之一，而项目才刚起步 —— 后面还要接蓝牙、传感器、按键。**如果继续这么接，路会走死。**

> 💡 **这是整个项目最重要的转折点**：从「能跑就行」转向「为后续扩展预留空间」。硬件项目和软件项目一样，**架构决策要在早期做**，越晚改代价越大。

---

## 阶段 3：I2C 升级 —— 第一次工程优化

**文件**：[firmware/LCD_show_words/LCD_show_words.ino](../firmware/LCD_show_words/LCD_show_words.ino)

```cpp
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);   // 地址, 列数, 行数

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.print("Thea, My Love!");
  lcd.setCursor(0, 1);
  lcd.print("By Arduino!");
}
```

### 这次升级的收益

| 对比项 | 并行（阶段 2） | I2C（阶段 3） |
| --- | --- | --- |
| IO 占用 | 6 个 | **2 个**（A4/A5，复用） |
| 接线数量 | 12 根 | **4 根** |
| 出错概率 | 高 | 低 |
| 与蓝牙冲突 | **是** | 否 |

**腾出来的 4 个 IO 口，正是后面接按键、双 LED 的前提。**

### 学到的核心概念

| 概念 | 说明 |
| --- | --- |
| **I2C 总线** | 两根线（SDA 数据 / SCL 时钟），可挂多个设备，靠**地址**区分 |
| **地址扫描** | 用 `Wire` 库遍历 1~126 地址，确认设备在不在 |
| **硬件抽象** | PCF8574 芯片把「并行 8 位」转成「I2C 串行」，程序视角完全改变 |
| **库的版本陷阱** | `lcd.init()` vs `lcd.begin(16,2)` —— 不同分支 API 不同 |

### 一个值得记住的工程原则

> **用「少量的复杂度」换取「结构上的简洁」，通常都是划算的。**
>
> 多花一块钱的 I2C 转接板，换回了 4 个 IO 口、8 根线的工作量，以及整个蓝牙方案的可行性。这次选择直接决定了项目后面能走多远。

---

## 阶段 4：交互闭环 —— 按键 + 蓝牙控灯

**文件**：[firmware/use_button_control_led/use_button_control_led.ino](../firmware/use_button_control_led/use_button_control_led.ino)

这是**第一个真正的交互系统**：本地按键和远程蓝牙两条输入路径，共同控制同一组 LED 和同一块屏幕。

### 关键技术点

#### 1. 软硬串口分离

```cpp
Serial.begin(9600);   // D0/D1 —— 电脑调试
BT.begin(38400);      // D10/D11 —— 蓝牙，SoftwareSerial 模拟
```

**为什么必须分开？** D0/D1 是下载程序的通道，蓝牙接上面会导致下载失败或通信异常。`SoftwareSerial` 用纯软件方式（定时器 + 引脚中断）在任意数字脚上模拟串口，把通信口和下载口解耦。

#### 2. 状态机雏形

```cpp
int ledMode = 0;   // 0=常灭, 1=常亮, 2=闪烁

void updateLCD() { /* 依据 ledMode 刷新屏幕 */ }
void updateLED() { /* 依据 ledMode 刷新硬件 */ }
```

**所有输入源都只改 `ledMode`，然后统一刷新。** 这个模式解决了阶段 4 出现的一个真实 bug：按键切灯之后屏幕没更新，显示和实际状态不一致。把「状态」和「表现」分离，是软件设计里最基本也最有效的手段。

#### 3. 边沿检测消抖

```cpp
if (buttonState == LOW && lastButtonState == HIGH) {   // 只在按下瞬间触发一次
  ...
  delay(150);
}
lastButtonState = buttonState;
```

`INPUT_PULLUP` 让引脚默认读到 `HIGH`，按下接地读到 `LOW`。加上 `lastButtonState` 比较，避免「按住不放时反复触发」。

#### 4. 蓝牙输入过滤

```cpp
if (incomingChar == '\r' || incomingChar == '\n') return;
```

手机 App 会追加 `\r\n`，不过滤会污染缓冲区。**这类「对端行为不可控」的边界处理，是通信编程的必修课。**

### 踩坑记录

| 坑 | 现象 | 原因 |
| --- | --- | --- |
| [LCD 残留字符](TROUBLESHOOTING.md#5-lcd-出现残留字符) | 显示 `LED: ONF` | 短字符串覆盖长字符串没清干净 |
| [蓝牙乱码](TROUBLESHOOTING.md#2-手机发指令收到-ffff--c7-乱码) | 串口打印 `FFFF` | 波特率不匹配 + TX/RX 接反 |
| [上传失败](TROUBLESHOOTING.md#3-上传代码失败) | `not in sync` | 蓝牙占了 D10/D11 干扰 bootloader |

---

## 阶段 5：物联网原型成型 ⭐

**文件**：[firmware/temp_monitor/temp_monitor.ino](../firmware/temp_monitor/temp_monitor.ino)

在阶段 4 的基础上加入 **DHT11 温湿度传感器**，并完成了几处重要的架构升级。

### 新增能力

| 能力 | 实现 |
| --- | --- |
| 环境数据采集 | `DHT dht(7, DHT11)` |
| 双向通信 | 手机发 `T` → Arduino 回传 `Temp: 26.3C, Humi: 58%` |
| 错误处理 | `isnan()` 判空，错误信息同时上屏 + 回传手机 |
| **双灯交替闪烁** | 两个 LED 反相驱动，视觉效果好于同相闪烁 |
| **非阻塞重构** | 用 `millis()` 时间戳替代 `delay()` |

### 架构升级 1：非阻塞闪烁

从阶段 1 的 `delay(1000)` 一路走来，到这里终于**必须**解决阻塞问题 —— 因为闪烁期间如果停止响应，蓝牙指令会丢、按键会失灵。

```cpp
// ❌ 旧写法：阻塞 200ms，期间蓝牙缓冲区会溢出
digitalWrite(ledPin1, HIGH); delay(200);
digitalWrite(ledPin1, LOW);  delay(200);

// ✅ 新写法：时间戳比较，loop() 全速轮询
if (currentMillis - previousMillis >= blinkInterval) {
  previousMillis = currentMillis;
  blinkState = !blinkState;
  ...
}
```

> **这是本项目技术上最有价值的一课**：`delay()` 在单任务小程序里无害，但在**任何需要同时响应多个输入**的系统里都是致命的。`millis()` 模式（即 Arduino 官方示例 *BlinkWithoutDelay*）是嵌入式开发的必备技能。

### 架构升级 2：双灯反相驱动

```cpp
digitalWrite(ledPin1, blinkState ? HIGH : LOW);
digitalWrite(ledPin2, blinkState ? LOW : HIGH);   // 注意：反相
```

两灯此亮彼灭，形成「交替跳动」的视觉效果，比同时闪烁更抓眼。**这是从「功能实现」迈向「体验设计」的一小步。**

### 架构升级 3：错误也要上报

```cpp
if (isnan(t) || isnan(h)) {
  lcd.print("Sensor Error!");
  BT.println("Sensor Error! Check wiring.");   // 远程端也要知道
}
```

如果只在 LCD 上提示，那么远程用户只会看到「发了指令没反应」，无法区分是**传感器坏了**还是**系统死了**。**把错误暴露到调用方，是可靠系统的基本要求。**

### 这个阶段暴露出的不足

| 不足 | 说明 | 改进方向 |
| --- | --- | --- |
| 无定时上报 | 必须手机主动查询 | 加 `millis()` 定时器，每 5 秒自动推送 |
| 单字符协议脆弱 | 任何误触发的字节都可能被当指令 | 升级为带帧头 + 校验的协议 |
| 代码全在一个文件 | 166 行，继续扩展会失控 | 拆成 `lcd_ui` / `bt_proto` / `led_ctrl` / `sensor` |
| 按键仍用 `delay()` | 与整体非阻塞理念不一致 | 改为 `millis()` 时间戳消抖 |
| 无 WiFi | 蓝牙距离限制在 10 米左右 | 换 ESP32，接云平台 |

这些都记录在根目录 [README.md 的未来计划](../README.md#-未来计划-to-do) 中。

---

## 🎯 能力矩阵

| 能力维度 | 阶段 1 | 阶段 2 | 阶段 3 | 阶段 4 | 阶段 5 |
| --- | :---: | :---: | :---: | :---: | :---: |
| GPIO 输出 | ✅ | ✅ | ✅ | ✅ | ✅ |
| 串口调试 | ✅ | ✅ | ✅ | ✅ | ✅ |
| 并行 LCD 驱动 | | ✅ | | | |
| I2C 总线 | | | ✅ | ✅ | ✅ |
| 数字输入 / 上拉 | | | | ✅ | ✅ |
| 按键消抖 | | | | ✅ | ✅ |
| 软串口 | | | | ✅ | ✅ |
| 状态机设计 | | | | ✅ | ✅ |
| 蓝牙双向通信 | | | | ✅ | ✅ |
| 单总线传感器 | | | | | ✅ |
| 错误处理 | | | | | ✅ |
| 非阻塞架构 | | | | | ✅ |
| 多任务协同 | | | | | ✅ |

---

## 🔮 下一段路

当前项目已经跑通了「**感知 → 处理 → 通信 → 执行**」的完整物联网闭环，但受限于距离（蓝牙约 10 米）。下一步的自然演进是：

```
Arduino Uno + HC-05  （蓝牙，~10m，局域网）
        │
        ▼
   ESP32 + WiFi       （云平台，无距离限制）
        │
        ▼
   MQTT + 手机 App    （订阅/发布，多设备协同）
        │
        ▼
   OTA 空中升级 + 电池供电（产品化）
```

**从阶段 1 到阶段 5 积累的所有技能都是可迁移的**：

- GPIO、I2C、串口、状态机、非阻塞架构 → 在 ESP32 上完全通用
- 蓝牙通信的经验 → 直接迁移到 WiFi/TCP（同样是「字节流 + 协议设计」）
- 踩坑记录里的排错方法论 → 面对任何新模块都适用

**硬件开发真正的门槛不是「记住某个模块怎么接」，而是建立起「先验证电源、再验证通信、最后验证逻辑」这套系统化的排错思维。** 这份仓库记录的就是这条思维方式的形成过程。
