#include <LiquidCrystal.h>
#include "Button.h"

// lcd pins
#define PIN_RS 8
#define PIN_EN 9
#define PIN_D4 4
#define PIN_D5 5
#define PIN_D6 6
#define PIN_D7 7
#define PIN_BL 10

int buttonValue;
LiquidCrystal lcd(PIN_RS, PIN_EN, PIN_D4, PIN_D5, PIN_D6, PIN_D7);

Button btn(A0);

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  btn.begin();

  lcd.setCursor(0, 0);
  lcd.print("Alarmy");

  lcd.setCursor(0, 1);
  lcd.print("Press Key:");
}

void loop() {  
  lcd.setCursor(10, 1);
  if (btn.pressed()) {
    lcd.print(btn.direction());
  }
}
