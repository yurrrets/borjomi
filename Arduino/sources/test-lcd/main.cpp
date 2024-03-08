#include <Arduino.h>
#include "LiquidCrystalDisplayEx.h"
#include "LiquidCrystalDisplayEx.ipp"

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to

// constexpr int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
// LiquidCrystalPinIface4bit<false> lcd_iface(rs, en, d4, d5, d6, d7);

constexpr int latchPin = 8, clockPin = 12, dataPin = 11;
LiquidCrystalShiftRegShiftOutIface lcd_iface(latchPin, clockPin, dataPin);

LiquidCrystalDisplay lcd(lcd_iface);

void setup()
{
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    // Print a message to the LCD.
    lcd.print("hello, world!");
}

void loop()
{
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print(millis() / 1000);
}