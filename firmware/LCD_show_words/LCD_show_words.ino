#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// 将0x27替换为你刚刚扫描到的地址
LiquidCrystal_I2C lcd(0x27, 16, 2); 

void setup() {
  lcd.init();      // 初始化屏幕
  lcd.backlight(); // 打开背光
  
  lcd.print("Thea, My Love!");
  lcd.setCursor(0, 1);
  lcd.print("By Arduino!");
}

void loop() {
  for (int thisChar = 0; thisChar < 10; thisChar++) {
    lcd.setCursor(14, 1);
    lcd.print(thisChar);
    delay(500);
  }
}