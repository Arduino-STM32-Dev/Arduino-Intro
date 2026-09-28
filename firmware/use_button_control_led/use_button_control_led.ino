#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); 
SoftwareSerial BT(10, 11); // RX=10, TX=11

const int buttonPin = 2;  
const int ledPin = 9;     

int buttonState = 0;
int lastButtonState = 0;

// 定义LED模式：0=常灭，1=常亮，2=跳动（闪烁）
int ledMode = 0; 

// 用于非阻塞闪烁的变量
unsigned long previousMillis = 0; 
const long blinkInterval = 200; // 闪烁间隔（毫秒）
bool blinkState = LOW;          // 当前闪烁时的亮灭状态

char incomingChar;        

// 统一更新屏幕（关键修改：连第一行一起恢复）
void updateLCD() {
  // 1. 恢复第一行默认文字，末尾加空格防止残留
  lcd.setCursor(0, 0);
  lcd.print("Thea, My Love!   "); 
  
  // 2. 更新第二行状态
  lcd.setCursor(0, 1);
  if (ledMode == 0) {
    lcd.print("LED: OFF     ");
  } else if (ledMode == 1) {
    lcd.print("LED: ON      ");
  } else if (ledMode == 2) {
    lcd.print("LED: Blink    ");
  }
}

// 统一更新物理LED状态（针对常亮/常灭）
void updateLED() {
  if (ledMode == 0) {
    digitalWrite(ledPin, LOW);
  } else if (ledMode == 1) {
    digitalWrite(ledPin, HIGH);
  }
  // 模式2由loop里的millis控制，这里不处理
}

void setup() {
  lcd.init();
  lcd.backlight();
  
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  
  ledMode = 0;
  updateLED();
  updateLCD(); // 直接调用初始化显示，不用手动写print了
  
  Serial.begin(9600); 
  BT.begin(38400); // 保持你测试成功的波特率
}

void loop() {
  // ========== 1. 非阻塞LED跳动逻辑（针对模式2） ==========
  if (ledMode == 2) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= blinkInterval) {
      previousMillis = currentMillis;
      blinkState = !blinkState;
      digitalWrite(ledPin, blinkState ? HIGH : LOW);
    }
  }

  // ========== 2. 按键本地控制逻辑 ==========
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW && lastButtonState == HIGH) {
    // 按一下，在常亮和常灭之间切换
    if (ledMode == 0 || ledMode == 2) {
      ledMode = 1;
    } else {
      ledMode = 0;
    }
    updateLED();
    updateLCD(); // 恢复默认界面
    delay(150); // 按键消抖
  }
  lastButtonState = buttonState;

  // ========== 3. 蓝牙控制逻辑 ==========
  if (BT.available()) {
    incomingChar = BT.read(); 
    if (incomingChar == '\r' || incomingChar == '\n') return; // 过滤回车换行

    Serial.print("BT Received (HEX): ");
    Serial.println(incomingChar, HEX); 

    // 手机发送 '1'：常亮
    if (incomingChar == '1') {
      ledMode = 1;
      updateLED();
      updateLCD();
    } 
    // 手机发送 '0'：关闭
    else if (incomingChar == '0') {
      ledMode = 0;
      updateLED();
      updateLCD();
    }
    // 手机发送 '2'：有趣的跳动
    else if (incomingChar == '2') {
      ledMode = 2;
      previousMillis = millis(); // 重置时间戳
      updateLCD();
    }
    // 手机发送 '3'：中秋快乐！同时不影响灯的状态
    else if (incomingChar == '3') {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Happy Mid-Autumn   ");
      lcd.setCursor(0, 1);
      lcd.print("Festival!     ");
    }
  }
}