
#include "LiquidCrystalDisplayEx.h"

template <uint8_t Cols, uint8_t Rows, typename LCD> class LiquidCrystalDisplayCache : public Print
{
  public:
    LiquidCrystalDisplayCache(LCD &lcd) : lcd(lcd)
    {
    }

    void begin(uint8_t charsize = LCD_5x8DOTS)
    {
        lcd.begin(Cols, Rows, charsize);
        for (uint8_t r = 0; r < Rows; r++)
        {
            for (uint8_t c = 0; c < Cols; c++)
            {
                current_data[r][c] = ' ';
            }
        }
        flush();
    }

    void setCursor(uint8_t col, uint8_t row)
    {
        current_col = col;
        current_row = row;
    }

    size_t write(uint8_t v) override
    {
        if (current_row >= Rows || current_col >= Cols)
        {
            return 0;
        }
        current_data[current_row][current_col] = v;
        current_col++;
        return 1;
    }

    using Print::write;

    void flush() override
    {
        for (uint8_t r = 0; r < Rows; r++)
        {
            if (memcmp(current_data[r], last_data[r], Cols) == 0)
            {
                continue;
            }
            memcpy(last_data[r], current_data[r], Cols);
            lcd.setCursor(0, r);
            lcd.write(last_data[r], Cols);
        }
    }

  private:
    LCD &lcd;
    uint8_t current_data[Rows][Cols] = {};
    uint8_t last_data[Rows][Cols] = {};
    uint8_t current_row = 0, current_col = 0;
};