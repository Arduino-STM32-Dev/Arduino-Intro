#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); 
SoftwareSerial BT(10, 11); // RX=10, TX=11

const int buttonPin = 2;  
const int ledPin1 = 9;  
const int ledPin2 = 8;  
const int lightPin = 4; // 光敏数字引脚 DO 接在 D4

int buttonState = 0;
int lastButtonState = 0;
int ledMode = 0; // 0=灭 1=亮 2=跳动

unsigned long previousMillis = 0; 
const long blinkInterval = 200; 
bool blinkState = LOW;          

char incomingChar;        

void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Thea, My Love   "); 
  lcd.setCursor(0, 1);
  if (ledMode == 0) {
    lcd.print("LED: OFF ");
  } else if (ledMode == 1) {
    lcd.print("LED: ON  ");
  } else if (ledMode == 2) {
    lcd.print("LED: Blink");
  }
}

void updateLED() {
  if (ledMode == 0) {
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);
  } else if (ledMode == 1) {
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, HIGH); 
  }
}

// ===== 读取光敏并上报 =====
void readAndReportLight() {
  int lightState = digitalRead(lightPin); // 读取数字信号 (0 或 1)
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Light Status:");
  
  lcd.setCursor(0, 1);
if (lightState == HIGH) { // 暗
    lcd.print("Status: Dark");
    if (ledMode == 0) {
      ledMode = 1;
      updateLED();
    }
    BT.println("Light: Dark (LED Auto ON)");
} else { // 亮
    lcd.print("Status: Bright");
    // 【新增】光线变亮，如果之前是自动亮起的，就自动关掉
    if (ledMode == 1) { 
      ledMode = 0;
      updateLED();
      BT.println("Light: Bright (LED Auto OFF)");
    } else {
      BT.println("Light: Bright");
    }
}
  
  delay(2000); // 停留2秒后切回主界面
  updateLCD();
}

void setup() {
  lcd.init();
  lcd.backlight();
  
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(lightPin, INPUT); // 光敏DO引脚设为输入
  
  ledMode = 0;
  updateLED();
  updateLCD(); 
  
  Serial.begin(9600); 
  BT.begin(38400); 
}

void loop() {
  // ===== 1. LED交替跳动逻辑 =====
  if (ledMode == 2) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= blinkInterval) {
      previousMillis = currentMillis;
      blinkState = !blinkState;
      digitalWrite(ledPin1, blinkState ? HIGH : LOW);
      digitalWrite(ledPin2, blinkState ? LOW : HIGH);
    }
  }

  // ===== 2. 按键本地控制逻辑 =====
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

  // ===== 3. 蓝牙控制逻辑 =====
  if (BT.available()) {
    incomingChar = BT.read(); 
    if (incomingChar == '\r' || incomingChar == '\n') return; 

    // 手机发送 'L'：读取光线状态
    if (incomingChar == 'L' || incomingChar == 'l') {
      readAndReportLight();
    }
    else if (incomingChar == '1') {
      ledMode = 1;
      updateLED();
      updateLCD();
    } 
    else if (incomingChar == '0') {
      ledMode = 0;
      updateLED();
      updateLCD();
    }
    else if (incomingChar == '2') {
      ledMode = 2;
      previousMillis = millis(); 
      updateLCD();
    }
    else if (incomingChar == '3') {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Happy Mid-Autumn");
      lcd.setCursor(0, 1);
      lcd.print("Festival");
    }
  }
}