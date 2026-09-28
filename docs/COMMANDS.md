# 📱 蓝牙指令协议参考

本文档描述本项目在 [firmware/temp_monitor/temp_monitor.ino](../firmware/temp_monitor/temp_monitor.ino) 中实现的串口通信协议。

---

## 1. 通信参数

| 项目 | 值 |
| --- | --- |
| 传输方式 | 经典蓝牙 SPP（串口透传协议） |
| 模块 | HC-05（兼容 HC-06 / JDY-31 等 SPP 模块） |
| Arduino 侧接口 | `SoftwareSerial BT(10, 11)` |
| 波特率 | **38400**（本项目实测值，代码见 `BT.begin(38400)`） |
| 数据位 / 校验 / 停止位 | 8 / N / 1（默认） |
| 编码 | ASCII 文本 |
| 调试串口 | `Serial`（D0/D1），**9600** 波特率，仅供电脑端查看 |

> ⚠️ **波特率是最高频的故障点**。如果你的 HC-05 是 9600 的，必须把代码中的 `BT.begin(38400);` 改成 `BT.begin(9600);`。判断依据：手机发指令后，电脑串口打印出 `FFFF`、`C7`、`80` 之类的乱码 → 波特率不匹配。

---

## 2. 指令表

**方向：手机 → Arduino**

| 指令 | 十六进制 | 功能 | LCD 第一行 | LCD 第二行 |
| :---: | :---: | --- | --- | --- |
| `1` | `0x31` | LED 常亮 | 恢复默认 | `LED: ON` |
| `0` | `0x30` | LED 关闭 | 恢复默认 | `LED: OFF` |
| `2` | `0x32` | LED 交替闪烁 | 恢复默认 | `LED: Blink` |
| `3` | `0x33` | 中秋彩蛋 | `Happy Mid-Autumn` | `Festival` |
| `T` | `0x54` | 查询温湿度（大写） | `Temp: 26.3 C` | `Humi: 58 %` |
| `t` | `0x74` | 查询温湿度（小写，等效） | 同上 | 同上 |

**忽略的输入：** `\r`（`0x0D`）、`\n`（`0x0A`）会被静默丢弃，不计入指令。

**方向：Arduino → 手机**

| 场景 | 返回内容 |
| --- | --- |
| 查询成功 | `Temp: 26.3C, Humi: 58%`（带换行） |
| 查询失败 | `Sensor Error! Check wiring.`（带换行） |

---

## 3. 代码实现

### 3.1 接收与过滤

```cpp
if (BT.available()) {
  incomingChar = BT.read();
  if (incomingChar == '\r' || incomingChar == '\n') return;  // 丢弃回车换行

  Serial.print("BT Received (HEX): ");
  Serial.println(incomingChar, HEX);   // 电脑端便于确认收到什么
  ...
}
```

**为什么必须过滤 `\r\n`？**
绝大多数手机串口 App 在发送时会自动追加 `\r\n`。不过滤的话，`BT.available()` 会一个字节一个字节地触发处理逻辑，缓冲区里混入无意义字节，调试时看到一串 `D` `A` 无从判断。

### 3.2 指令分发（if-else 链）

```cpp
if (incomingChar == '1') { ... }
else if (incomingChar == '0') { ... }
else if (incomingChar == '2') { ... }
else if (incomingChar == '3') { ... }
else if (incomingChar == 'T' || incomingChar == 't') { ... }
```

**为什么用 `if-else` 而不是 `switch`？**
`switch` 在本项目中完全可用且可读性更好。当时选择 `if-else` 是为了让每个分支能独立调整条件（比如 `T` 和 `t` 的双条件判断）。如果要扩展更多指令，建议重构为 `switch`，见下文「改进建议」。

### 3.3 温湿度查询与回传

```cpp
float t = dht.readTemperature();
float h = dht.readHumidity();

if (isnan(t) || isnan(h)) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sensor Error!");
  BT.println("Sensor Error! Check wiring.");   // 错误也回传手机
} else {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");  lcd.print(t, 1);  lcd.print(" C");
  lcd.setCursor(0, 1);
  lcd.print("Humi: ");  lcd.print(h, 0);  lcd.print(" %");

  BT.print("Temp: ");   BT.print(t, 1);
  BT.print("C, Humi: "); BT.print(h, 0);
  BT.println("%");
}
```

**设计要点：**

- `isnan()` 判空是**必须的**，DHT11 读取失败会返回 `NaN`，屏幕上会直接显示 `nan`
- LCD 用 1 位小数（`print(t, 1)`），湿度取整（`print(h, 0)`），因为 DHT11 的精度本来就只有 ±2℃ / ±5%RH，多显示小数位是假精度
- **错误信息也通过蓝牙回传**，让远程用户知道是传感器问题而不是系统死机

---

## 4. 手机 App 配置

### 推荐 App

| 平台 | App | 说明 |
| --- | --- | --- |
| Android | **Serial Bluetooth Terminal** | 免费、开源、支持宏指令，最推荐 |
| Android | 蓝牙串口助手（各种版本） | 界面简单，功能够用 |
| iOS | **LightBlue** / **Serial Bluetooth Terminal** | iOS 对经典蓝牙 SPP 支持有限，需 App 支持 |

### 关键设置

1. **模式必须选 ASCII / 文本**，不要选 HEX 模式
   - 选 HEX 时会发送 `0x01`（1 个字节，控制字符 `SOH`），而代码比较的是字符 `'1'`（ASCII 值 `0x31`），永远匹配不上 —— 这是极易踩的坑
2. **行尾设置**：建议设为 `CR+LF`（`\r\n`）或 `None`，代码两者都能处理
3. 连接后**等待 2~3 秒**再发指令，让蓝牙链路稳定

### 连接步骤

1. 手机「设置 → 蓝牙」搜索 `HC-05`，配对码 **`1234`**（部分模块为 `0000`）
2. 打开 App，在设备列表中选择 `HC-05` 连接
3. 确认 HC-05 底板 LED 由**快闪变为慢闪**（约 2 秒一次）→ 连接成功
4. 发送 `1` 测试

---

## 5. 调试方法

### 电脑端串口监视器（9600）

代码会把收到的每个字节以 HEX 形式打印出来：

```
BT Received (HEX): 31      ← 收到 '1'
BT Received (HEX): 30      ← 收到 '0'
BT Received (HEX): 32      ← 收到 '2'
BT Received (HEX): 33      ← 收到 '3'
BT Received (HEX): 54      ← 收到 'T'
```

### HEX 对照：从乱码反推问题

| 你在串口看到的 | 实际情况 | 结论 |
| --- | --- | --- |
| `31` `30` `32` | 完全正确 | ✅ 波特率和接线都对 |
| `FFFF` / `C7` / `80` | 采样点错位 | ❌ 波特率不匹配 |
| `CC` `E6` `66` | 位错乱 | ❌ 波特率不匹配，或 TX/RX 接反 |
| 完全没有输出 | 无数据到达 | ❌ TX/RX 接反，或没配对成功 |
| `D` `A` | 收到 `\r\n` | ⚠️ 被过滤了，正常现象但说明 App 在追加换行 |

**为什么波特率错了会打出 `FFFF`？**
串口是异步通信，靠起始位同步、按约定时长采样。波特率不匹配时，接收方在每个位的中间采到错误电平，一个正确字节可能被解析成两个错误字节，或者反过来。`FFFF` 这种「全 1」结果就是典型的采样完全失步。

---

## 6. 扩展建议

### 6.1 重构为 `switch` + 函数分离

```cpp
void handleCommand(char cmd) {
  switch (cmd) {
    case '1': setLedMode(1); break;
    case '0': setLedMode(0); break;
    case '2': setLedMode(2); break;
    case '3': showEasterEgg(); break;
    case 'T': case 't': reportDHT(); break;
    default:
      BT.print("Unknown command: ");
      BT.println(cmd);
      break;
  }
}
```

`default` 分支回传「未知指令」，调试体验会好很多。

### 6.2 建议新增的指令

| 指令 | 功能 | 实现思路 |
| --- | --- | --- |
| `4` | 设置闪烁频率（如 `F500` = 500ms） | 解析数字参数，写入 `blinkInterval` |
| `P` | 查询当前 LED 状态 | `BT.println(ledMode)` 等价输出 |
| `A` | 开启每 5 秒自动上报 | `millis()` 定时 + `autoReport` 标志位 |
| `S` | 停止自动上报 | 清标志位 |
| `R` | 软件复位 | `asm volatile ("jmp 0");` 或看门狗复位 |
| `B` | 查询蓝牙模块版本 | 进入 AT 模式发送 `AT+VERSION?` |

### 6.3 升级为带帧头的协议

单字符协议简单但不可靠：**任何误触发的单字节都可能被当作有效指令**。建议升级为带帧头和校验的格式：

```
#LED:1;CHK=4F\n
#DHT:?;CHK=8A\n
```

- `#` 作为帧头，同步丢失后可快速重新对齐
- `:` / `;` 分隔字段，便于解析多参数指令
- 校验和用于丢弃被干扰的报文

### 6.4 结构化数据回传（便于上位机解析）

当前回传的是人类可读字符串（`Temp: 26.3C, Humi: 58%`），适合人工查看但不利于程序解析。若要接入上位机或云平台，建议改为：

```cpp
BT.print("{\"t\":"); BT.print(t, 1);
BT.print(",\"h\":"); BT.print(h, 0);
BT.println("}");           // 输出：{"t":26.3,"h":58}
```

配合「5 秒定时上报」即可直接在手机端画曲线图。
