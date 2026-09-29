#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <DHT.h>   // 温湿度库

LiquidCrystal_I2C lcd(0x27, 16, 2); 
SoftwareSerial BT(10, 11); // RX=10, TX=11

// ===== 温湿度传感器定义 =====
#define DHTPIN 7
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ===== 引脚定义 =====
const int buttonPin = 2;  
const int ledPin1 = 9;  
const int ledPin2 = 8;  
const int lightPin = 4; 
const int soundPin = 5; 
const int buzzerPin = 3; 

int buttonState = 0;
int lastButtonState = 0;
int ledMode = 0; 

unsigned long previousMillis = 0; 
const long blinkInterval = 200; 
bool blinkState = LOW;          
char incomingChar;        
unsigned long lastSoundTime = 0;
const long soundCooldown = 1000; // 增加冷却时间到1秒

// ===== 新增：蜂鸣器防自激变量 =====
unsigned long lastBuzzerTime = 0; 
const long buzzerCooldown = 600; // 蜂鸣器响完后，600毫秒内忽略声音传感器

// ===== 【低电平触发蜂鸣器专用发声函数】 =====
void beep(int duration) {
  lastBuzzerTime = millis(); // 记录蜂鸣器开始发声的时间
  digitalWrite(buzzerPin, LOW);  // 低电平触发发声
  delay(duration);
  digitalWrite(buzzerPin, HIGH); // 高电平停止
  delay(50);
}

void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("Thea, My Love   "); 
  lcd.setCursor(0, 1);
  if (ledMode == 0) lcd.print("LED: OFF ");
  else if (ledMode == 1) lcd.print("LED: ON  ");
  else if (ledMode == 2) lcd.print("LED: Blink");
}

void updateLED() {
  if (ledMode == 0) {
    digitalWrite(ledPin1, LOW); digitalWrite(ledPin2, LOW);
  } else if (ledMode == 1) {
    digitalWrite(ledPin1, HIGH); digitalWrite(ledPin2, HIGH); 
  }
}

void playAlert() {
  beep(200); delay(100); beep(200); delay(100); beep(200);
}

void readAndReportLight() {
  int lightState = digitalRead(lightPin);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Light Status:");
  lcd.setCursor(0, 1);
  if (lightState == HIGH) {
    lcd.print("Status: Dark");
    if (ledMode == 0) { ledMode = 1; updateLED(); }
    BT.println("Light: Dark (LED Auto ON)");
  } else {
    lcd.print("Status: Bright");
    BT.println("Light: Bright");
  }
  delay(2000);
  updateLCD();
}

// ===== 读取温湿度并上报 =====
void readAndReportDHT() {
  float t = dht.readTemperature(); // 读温度
  float h = dht.readHumidity();    // 读湿度

  // DHT11 偶尔会读取失败，必须判空，否则屏幕会显示 nan
  if (isnan(t) || isnan(h)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error!");
    BT.println("Sensor Error! Check wiring.");  // 错误也回传手机
    beep(200);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(t, 1);
    lcd.print(" C");

    lcd.setCursor(0, 1);
    lcd.print("Humi: ");
    lcd.print(h, 0);
    lcd.print(" %");

    // 通过蓝牙把数据发回手机
    BT.print("Temp: ");
    BT.print(t, 1);
    BT.print("C, Humi: ");
    BT.print(h, 0);
    BT.println("%");
  }
  delay(2000);   // 停留 2 秒后切回主界面
  updateLCD();
}

void setup() {
  lcd.init();
  lcd.backlight();
  dht.begin();   // 初始化温湿度传感器
  
  pinMode(ledPin1, OUTPUT); pinMode(ledPin2, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(lightPin, INPUT);
  pinMode(soundPin, INPUT); 
  pinMode(buzzerPin, OUTPUT); 
  
  digitalWrite(buzzerPin, HIGH); // 初始化时关闭蜂鸣器
  
  ledMode = 0;
  updateLED();
  updateLCD(); 
  
  Serial.begin(9600); 
  BT.begin(38400); 
}

void loop() {
  // ===== 1. 声音传感器检测逻辑（增加防自激判断） =====
  // 只有在距离上次蜂鸣器响超过 600ms，且距离上次触发超过 1000ms 时，才响应声音
  if (digitalRead(soundPin) == LOW 
      && (millis() - lastSoundTime > soundCooldown) 
      && (millis() - lastBuzzerTime > buzzerCooldown)) {
      
    lastSoundTime = millis(); 
    beep(100); 
    lcd.setCursor(0, 0); lcd.print("Sound Detected! "); 
    BT.println(">> Sound Detected! <<"); 
    
    // LED闪烁提示
    digitalWrite(ledPin1, HIGH); digitalWrite(ledPin2, HIGH); delay(100);
    digitalWrite(ledPin1, LOW); digitalWrite(ledPin2, LOW); delay(100);
    digitalWrite(ledPin1, HIGH); digitalWrite(ledPin2, HIGH); delay(100);
    digitalWrite(ledPin1, LOW); digitalWrite(ledPin2, LOW); 
    updateLCD(); 
  }

  // ===== 2. LED跳动模式 =====
  if (ledMode == 2) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= blinkInterval) {
      previousMillis = currentMillis;
      blinkState = !blinkState;
      digitalWrite(ledPin1, blinkState ? HIGH : LOW);
      digitalWrite(ledPin2, blinkState ? LOW : HIGH);
      // 跳动时蜂鸣器跟着滴一声
      lastBuzzerTime = millis(); 
      digitalWrite(buzzerPin, LOW); delay(30); digitalWrite(buzzerPin, HIGH);
    }
  }

  // ===== 3. 按键本地控制 =====
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW && lastButtonState == HIGH) {
    if (ledMode == 0 || ledMode == 2) { ledMode = 1; beep(100); } 
    else { ledMode = 0; beep(100); }
    updateLED(); updateLCD(); 
    delay(150); 
  }
  lastButtonState = buttonState;

  // ===== 4. 蓝牙控制 =====
  if (BT.available()) {
    incomingChar = BT.read(); 
    if (incomingChar == '\r' || incomingChar == '\n') return; 

    if (incomingChar == 'L' || incomingChar == 'l') readAndReportLight();
    else if (incomingChar == 'T' || incomingChar == 't') readAndReportDHT();
    else if (incomingChar == 'B' || incomingChar == 'b') playAlert(); 
    else if (incomingChar == '1') { ledMode = 1; updateLED(); updateLCD(); beep(100); } 
    else if (incomingChar == '0') { ledMode = 0; updateLED(); updateLCD(); beep(100); }
    else if (incomingChar == '2') { ledMode = 2; previousMillis = millis(); updateLCD(); }
    else if (incomingChar == '3') {
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("Happy Mid-Autumn");
      lcd.setCursor(0, 1); lcd.print("Festival");
      beep(200); delay(100); beep(200); delay(100); beep(400); 
    }
  }
}