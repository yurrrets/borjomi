#ifndef LiquidCrystal_h
#define LiquidCrystal_h

#include <inttypes.h>
#include "Print.h"

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
// #define LCD_8BITMODE 0x10
// #define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// low-level phisical layer (interface) description

struct LiquidCrystalIface
{
  virtual void init() = 0;
  virtual void setRs(uint8_t val) = 0;
  virtual void setRw(uint8_t val) = 0;
  virtual void setEnable(uint8_t val) = 0;
};

struct LiquidCrystalIface4bit
{
  static constexpr bool is4bit = true;
  virtual void write4bits(uint8_t val) = 0;
};

struct LiquidCrystalIface8bit
{
  static constexpr bool is4bit = false;
  virtual void write8bits(uint8_t val) = 0;
};

// std lib imports

template <bool B, class T = void>
struct enable_if
{
};

template <class T>
struct enable_if<true, T>
{
  typedef T type;
};

template <bool B, class T = void>
using enable_if_t = typename enable_if<B, T>::type;

// interface implementattion for direct pin connections

template <bool hasField>
struct RwPin;

template <>
struct RwPin<false>
{
  void init() {}
  void setRw(uint8_t) {}
};

template <>
struct RwPin<true>
{
  RwPin(uint8_t rw);
  void init();
  void setRw(uint8_t val);
protected:
  uint8_t _rw_pin;
};

template <bool isRwConnected>
struct LiquidCrystalPinIface : public LiquidCrystalIface, public RwPin<isRwConnected>
{
  LiquidCrystalPinIface(uint8_t rs, uint8_t rw, uint8_t enable)
      : RwPin<isRwConnected>(rw), _rs_pin(rs), _enable_pin(enable)
  {
  }

  LiquidCrystalPinIface(uint8_t rs, uint8_t enable)
      : _rs_pin(rs), _enable_pin(enable)
  {
  }

  void init() override;
  void setRs(uint8_t val) override;
  void setRw(uint8_t val) override;
  void setEnable(uint8_t val) override;

protected:
  uint8_t _rs_pin;
  uint8_t _enable_pin;
};

template <bool isRwConnected>
struct LiquidCrystalPinIface4bit : public LiquidCrystalPinIface<isRwConnected>, public LiquidCrystalIface4bit
{
  LiquidCrystalPinIface4bit(uint8_t rs, uint8_t rw, uint8_t enable,
                            uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
      : LiquidCrystalPinIface<isRwConnected>(rs, rw, enable)
  {
    initPins(d0, d1, d2, d3);
  }

  LiquidCrystalPinIface4bit(uint8_t rs, uint8_t enable,
                            uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
      : LiquidCrystalPinIface<isRwConnected>(rs, enable)
  {
    initPins(d0, d1, d2, d3);
  }

  void init() override;
  void write4bits(uint8_t val) override;

protected:
  void initPins(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
  uint8_t _data_pins[4];
};



template <bool isRwConnected>
struct LiquidCrystalPinIface8bit : public LiquidCrystalPinIface<isRwConnected>, public LiquidCrystalIface8bit
{
  LiquidCrystalPinIface8bit(uint8_t rs, uint8_t rw, uint8_t enable,
                            uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                            uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
      : LiquidCrystalPinIface<isRwConnected>(rs, rw, enable)
  {
    initPins(d0, d1, d2, d3, d4, d5, d6, d7);
  }

  LiquidCrystalPinIface8bit(uint8_t rs, uint8_t enable,
                            uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                            uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
      : LiquidCrystalPinIface<isRwConnected>(rs, enable)
  {
    initPins(d0, d1, d2, d3, d4, d5, d6, d7);
  }

  void init() override;
  void write8bits(uint8_t val) override;

protected:
  void initPins(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
  uint8_t _data_pins[8];
};


struct LiquidCrystalShiftRegIface : public LiquidCrystalIface, public LiquidCrystalIface4bit
{
  LiquidCrystalShiftRegIface() : _value(0) {}
  void setRs(uint8_t val) override;
  void setRw(uint8_t val) override;
  void setEnable(uint8_t val) override;
  void write4bits(uint8_t val) override;

protected:
  uint8_t _value;
  virtual void update() = 0;
};

struct LiquidCrystalShiftRegShiftOutIface : public LiquidCrystalShiftRegIface
{
  LiquidCrystalShiftRegShiftOutIface(uint8_t latch, uint8_t clock, uint8_t data)
    : _latch_pin(latch), _clock_pin(clock), _data_pin(data)
  {
  }
  void init() override;

protected:
  uint8_t _latch_pin;
  uint8_t _clock_pin;
  uint8_t _data_pin;

  void update() override;
};

#ifdef _SPI_H_INCLUDED
// Is not so good method. Would be good to have is_defined_v<T>,
// but it's hard to implement such template using SFINAE but w/o std lib.
// So for now just check with #ifdef
struct LiquidCrystalShiftRegSPIIface : public LiquidCrystalShiftRegIface
{
  LiquidCrystalShiftRegSPIIface(uint8_t latch)
    : _latch_pin(latch)
  {
  }
  void init() override;

protected:
  uint8_t _latch_pin;

  void update() override;
};

#endif

template <typename Interface>
class LiquidCrystalDisplay : public Print
{
public:
  LiquidCrystalDisplay(Interface &iface);

  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);

  void clear();
  void home();

  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();

  void setRowOffsets(int row1, int row2, int row3, int row4);
  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t);
  virtual size_t write(uint8_t);
  void command(uint8_t);

  using Print::write;

private:
  void send(uint8_t, uint8_t);
  void pulseEnable();
  void write4bits(uint8_t value);
  void write8bits(uint8_t value);

  Interface &_iface;

  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;

  uint8_t _initialized;

  uint8_t _numlines;
  uint8_t _row_offsets[4];
};

#endif
