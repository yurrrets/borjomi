#include "LiquidCrystalDisplayEx.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"


RwPin<true>::RwPin(uint8_t rw)
  : _rw_pin(rw)
{
}

void RwPin<true>::init()
{
  pinMode(_rw_pin, OUTPUT);
  setRw(LOW);
}

void RwPin<true>::setRw(uint8_t val)
{
  digitalWrite(_rw_pin, val);
}

template <bool isRwConnected>
void LiquidCrystalPinIface<isRwConnected>::init()
{
  RwPin<isRwConnected>::init();

  pinMode(_rs_pin, OUTPUT);
  pinMode(_enable_pin, OUTPUT);

  setRs(LOW);
  setEnable(LOW);
}

template <bool isRwConnected>
void LiquidCrystalPinIface<isRwConnected>::setRs(uint8_t val)
{
  digitalWrite(_rs_pin, val);
}

template <bool isRwConnected>
void LiquidCrystalPinIface<isRwConnected>::setRw(uint8_t val)
{
  RwPin<isRwConnected>::setRw(val);
}

template <bool isRwConnected>
void LiquidCrystalPinIface<isRwConnected>::setEnable(uint8_t val)
{
  digitalWrite(_enable_pin, val);
}

template <bool isRwConnected>
void LiquidCrystalPinIface4bit<isRwConnected>::init()
{
  for (int i = 0; i < 4; ++i)
  {
    pinMode(_data_pins[i], OUTPUT);
  }
}

template <bool isRwConnected>
void LiquidCrystalPinIface4bit<isRwConnected>::write4bits(uint8_t val)
{
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(_data_pins[i], (val >> i) & 0x01);
  }
}

template <bool isRwConnected>
void LiquidCrystalPinIface4bit<isRwConnected>::initPins(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
  _data_pins[0] = d0;
  _data_pins[1] = d1;
  _data_pins[2] = d2;
  _data_pins[3] = d3;
}

template <bool isRwConnected>
void LiquidCrystalPinIface8bit<isRwConnected>::init()
{
  for (int i = 0; i < 8; ++i)
  {
    pinMode(_data_pins[i], OUTPUT);
  }
}

template <bool isRwConnected>
void LiquidCrystalPinIface8bit<isRwConnected>::write8bits(uint8_t val)
{
  for (int i = 0; i < 8; i++)
  {
    digitalWrite(_data_pins[i], (val >> i) & 0x01);
  }
}

template <bool isRwConnected>
void LiquidCrystalPinIface8bit<isRwConnected>::initPins(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                                                        uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  _data_pins[0] = d0;
  _data_pins[1] = d1;
  _data_pins[2] = d2;
  _data_pins[3] = d3;
  _data_pins[4] = d4;
  _data_pins[5] = d5;
  _data_pins[6] = d6;
  _data_pins[7] = d7;
}

void LiquidCrystalShiftRegIface::setRs(uint8_t val)
{
  bitWrite(_value, 0, val);
  update();
}

void LiquidCrystalShiftRegIface::setRw(uint8_t val)
{
  bitWrite(_value, 2, val);
  update();
}

void LiquidCrystalShiftRegIface::setEnable(uint8_t val)
{
  bitWrite(_value, 1, val);
  update();
}

void LiquidCrystalShiftRegIface::write4bits(uint8_t val)
{
  // preserve Rs, Rw, Enable in 4 lowest bits
  // update val in higher bits
  _value = (_value & 0x0F) | ((val & 0x0F) << 4); 
  update();
}

void LiquidCrystalShiftRegShiftOutIface::init()
{
  pinMode(_latch_pin, OUTPUT);
  pinMode(_clock_pin, OUTPUT);
  pinMode(_data_pin, OUTPUT);
  digitalWrite(_latch_pin, HIGH);
  digitalWrite(_clock_pin, LOW);
  digitalWrite(_data_pin, LOW);
}

void LiquidCrystalShiftRegShiftOutIface::update()
{
  // take the latchPin low so
  // the LEDs don't change while you're sending in bits:
  digitalWrite(_latch_pin, LOW);
  // shift out the bits:
  shiftOut(_data_pin, _clock_pin, MSBFIRST, _value);
  //take the latch pin high so the LEDs will light up:
  digitalWrite(_latch_pin, HIGH);
  // pause before next value:
  delayMicroseconds(10);
}

#ifdef _SPI_H_INCLUDED

void LiquidCrystalShiftRegSPIIface::init()
{
  pinMode(_latch_pin, OUTPUT);
  digitalWrite(_latch_pin, HIGH);
  SPI.begin();
}

void LiquidCrystalShiftRegSPIIface::update()
{
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  // take the latchPin low so
  // the LEDs don't change while you're sending in bits:
  digitalWrite(_latch_pin, LOW);
  // shift out the bits:
  SPI.transfer(_value);
  //take the latch pin high so the LEDs will light up:
  digitalWrite(_latch_pin, HIGH);
  SPI.endTransaction();
  // pause before next value:
  delayMicroseconds(10);
}

#endif

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

template <typename Interface>
LiquidCrystalDisplay<Interface>::LiquidCrystalDisplay(Interface &iface)
    : _iface(iface)
{
  _displayfunction = LCD_1LINE | LCD_5x8DOTS;
}

template <typename Interface>
void LiquidCrystalDisplay<Interface>::begin(uint8_t cols, uint8_t lines, uint8_t dotsize)
{
  _iface.init();

  if (lines > 1)
  {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;

  setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);

  // for some 1 line displays you can select a 10 pixel high font
  if ((dotsize != LCD_5x8DOTS) && (lines == 1))
  {
    _displayfunction |= LCD_5x10DOTS;
  }

  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way before 4.5V so we'll wait 50
  delayMicroseconds(50000);
  // Now we pull both RS and R/W low to begin commands
  _iface.setRs(LOW);
  _iface.setEnable(LOW);
  _iface.setRw(LOW);

  // put the LCD into 4 bit or 8 bit mode
  if constexpr (Interface::is4bit)
  {
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    write4bits(0x03);
    delayMicroseconds(4500); // wait min 4.1ms

    // second try
    write4bits(0x03);
    delayMicroseconds(4500); // wait min 4.1ms

    // third go!
    write4bits(0x03);
    delayMicroseconds(150);

    // finally, set to 4-bit interface
    write4bits(0x02);
  }
  else
  {
    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    command(LCD_FUNCTIONSET | _displayfunction);
    delayMicroseconds(4500); // wait more than 4.1ms

    // second try
    command(LCD_FUNCTIONSET | _displayfunction);
    delayMicroseconds(150);

    // third go
    command(LCD_FUNCTIONSET | _displayfunction);
  }

  // finally, set # lines, font size, etc.
  command(LCD_FUNCTIONSET | _displayfunction);

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);
}

template <typename Interface>
void LiquidCrystalDisplay<Interface>::setRowOffsets(int row0, int row1, int row2, int row3)
{
  _row_offsets[0] = row0;
  _row_offsets[1] = row1;
  _row_offsets[2] = row2;
  _row_offsets[3] = row3;
}

/********** high level commands, for the user! */
template <typename Interface>
void LiquidCrystalDisplay<Interface>::clear()
{
  command(LCD_CLEARDISPLAY); // clear display, set cursor position to zero
  delayMicroseconds(2000);   // this command takes a long time!
}

template <typename Interface>
void LiquidCrystalDisplay<Interface>::home()
{
  command(LCD_RETURNHOME); // set cursor position to zero
  delayMicroseconds(2000); // this command takes a long time!
}

template <typename Interface>
void LiquidCrystalDisplay<Interface>::setCursor(uint8_t col, uint8_t row)
{
  const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
  if (row >= max_lines)
  {
    row = max_lines - 1; // we count rows starting w/0
  }
  if (row >= _numlines)
  {
    row = _numlines - 1; // we count rows starting w/0
  }

  command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

// Turn the display on/off (quickly)
template <typename Interface>
void LiquidCrystalDisplay<Interface>::noDisplay()
{
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
template <typename Interface>
void LiquidCrystalDisplay<Interface>::display()
{
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
template <typename Interface>
void LiquidCrystalDisplay<Interface>::noCursor()
{
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
template <typename Interface>
void LiquidCrystalDisplay<Interface>::cursor()
{
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
template <typename Interface>
void LiquidCrystalDisplay<Interface>::noBlink()
{
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
template <typename Interface>
void LiquidCrystalDisplay<Interface>::blink()
{
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
template <typename Interface>
void LiquidCrystalDisplay<Interface>::scrollDisplayLeft(void)
{
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
template <typename Interface>
void LiquidCrystalDisplay<Interface>::scrollDisplayRight(void)
{
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
template <typename Interface>
void LiquidCrystalDisplay<Interface>::leftToRight(void)
{
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
template <typename Interface>
void LiquidCrystalDisplay<Interface>::rightToLeft(void)
{
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
template <typename Interface>
void LiquidCrystalDisplay<Interface>::autoscroll(void)
{
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
template <typename Interface>
void LiquidCrystalDisplay<Interface>::noAutoscroll(void)
{
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
template <typename Interface>
void LiquidCrystalDisplay<Interface>::createChar(uint8_t location, uint8_t charmap[])
{
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i = 0; i < 8; i++)
  {
    write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */

template <typename Interface>
void LiquidCrystalDisplay<Interface>::command(uint8_t value)
{
  send(value, LOW);
}

template <typename Interface>
size_t LiquidCrystalDisplay<Interface>::write(uint8_t value)
{
  send(value, HIGH);
  return 1; // assume sucess
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
template <typename Interface>
void LiquidCrystalDisplay<Interface>::send(uint8_t value, uint8_t mode)
{
  _iface.setRs(mode);
  _iface.setRw(LOW);

  if constexpr (Interface::is4bit)
  {
    write4bits(value >> 4);
    write4bits(value);
  }
  else
  {
    write8bits(value);
  }
}

template <typename Interface>
void LiquidCrystalDisplay<Interface>::pulseEnable(void)
{
  _iface.setEnable(LOW);
  delayMicroseconds(1);
  _iface.setEnable(HIGH);
  delayMicroseconds(1); // enable pulse must be >450ns
  _iface.setEnable(LOW);
  delayMicroseconds(100); // commands need > 37us to settle
}

template <typename Interface>
void LiquidCrystalDisplay<Interface>::write4bits(uint8_t value)
{
  if constexpr (Interface::is4bit)
  {
    _iface.write4bits(value);
    pulseEnable();
  }
}

template <typename Interface>
void LiquidCrystalDisplay<Interface>::write8bits(uint8_t value)
{
  if constexpr (!Interface::is4bit)
  {
    _iface.write8bits(value);
    pulseEnable();
  }
}
