AAAAAAAAAAAAAAAQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQAQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqQQQQQQQQQQAAAA
// 引入 LiquidCrystal 库
#include <LiquidCrystal.h>

// 初始化引脚顺序：RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  // 设置屏幕的列数和行数 (16列 2行)
  lcd.begin(16, 2);
  
  // 打印欢迎信息
  lcd.print("Hello, TEST");
  lcd.setCursor(0, 1); // 光标移动到第二行第一列
  lcd.print("Arduino LCD Test");
}

void loop() {
  // 让屏幕显示一个动态的数字（0到9循环）
  for (int thisChar = 0; thisChar < 10; thisChar++) {
    lcd.setCursor(14, 1); // 定位到第二行末尾
    lcd.print(thisChar);
    delay(500);
  }
}