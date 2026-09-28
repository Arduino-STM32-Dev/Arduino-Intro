#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <DHT.h>  // 引入温湿度库

// 温湿度传感器定义
#define DHTPIN 7       // 数据引脚接在D7
#define DHTTYPE DHT11  // 传感器型号是DHT11
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2); 
SoftwareSerial BT(10, 11); // RX=10, TX=11

const int buttonPin = 2;  
const int ledPin1 = 9;  // 第一个灯接D9
const int ledPin2 = 8;  // 第二个灯接D8

int buttonState = 0;
int lastButtonState = 0;

// 定义LED模式：0=常灭，1=常亮，2=交替跳动
int ledMode = 0; 
unsigned long previousMillis = 0; 
const long blinkInterval = 200; 
bool blinkState = LOW;          
char incomingChar;        

// 统一更新屏幕
void updateLCD() {
  // 恢复第一行默认文字，末尾加空格防止残留
  lcd.setCursor(0, 0);
  lcd.print("Thea, My Love   "); 
  
  // 更新第二行状态
  lcd.setCursor(0, 1);
  if (ledMode == 0) {
    lcd.print("LED: OFF ");
  } else if (ledMode == 1) {
    lcd.print("LED: ON  ");
  } else if (ledMode == 2) {
    lcd.print("LED: Blink");
  }
}

// 统一更新物理LED状态（针对常亮/常灭）
void updateLED() {
  if (ledMode == 0) {
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);
  } else if (ledMode == 1) {
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, HIGH); 
  }
}

void setup() {
  lcd.init();
  lcd.backlight();
  dht.begin(); // 初始化温湿度传感器
  
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  
  ledMode = 0;
  updateLED();
  updateLCD(); 
  
  Serial.begin(9600); 
  BT.begin(38400); // 保持你测试成功的波特率
}

void loop() {
  // ========== 1. 非阻塞LED交替跳动逻辑 ==========
  if (ledMode == 2) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= blinkInterval) {
      previousMillis = currentMillis;
      blinkState = !blinkState;
      digitalWrite(ledPin1, blinkState ? HIGH : LOW);
      digitalWrite(ledPin2, blinkState ? LOW : HIGH);
    }
  }

  // ========== 2. 按键本地控制逻辑 ==========
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW && lastButtonState == HIGH) {
    if (ledMode == 0 || ledMode == 2) {
      ledMode = 1;
    } else {
      ledMode = 0;
    }
    updateLED();
    updateLCD(); 
    delay(150); 
  }
  lastButtonState = buttonState;

  // ========== 3. 蓝牙控制逻辑 ==========
  if (BT.available()) {
    incomingChar = BT.read(); 
    if (incomingChar == '\r' || incomingChar == '\n') return; 

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
    // 手机发送 '2'：有趣的交替跳动
    else if (incomingChar == '2') {
      ledMode = 2;
      previousMillis = millis(); 
      updateLCD();
    }
    // 手机发送 '3'：中秋快乐！
    else if (incomingChar == '3') {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Happy Mid-Autumn");
      lcd.setCursor(0, 1);
      lcd.print("Festival");
    }
    // 手机发送 'T'：读取温湿度
    else if (incomingChar == 'T' || incomingChar == 't') {
      float t = dht.readTemperature(); // 读温度
      float h = dht.readHumidity();    // 读湿度

      // 检查读取是否失败（DHT11偶尔会抽风）
      if (isnan(t) || isnan(h)) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sensor Error!");
        BT.println("Sensor Error! Check wiring."); // 发回手机
      } else {
        // 屏幕显示温湿度
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Temp: ");
        lcd.print(t, 1); // 保留1位小数
        lcd.print(" C");
        
        lcd.setCursor(0, 1);
        lcd.print("Humi: ");
        lcd.print(h, 0); // 湿度取整数
        lcd.print(" %");

        // 通过蓝牙把数据发回给手机App
        BT.print("Temp: ");
        BT.print(t, 1);
        BT.print("C, Humi: ");
        BT.print(h, 0);
        BT.println("%");
      }
    }
  }
}