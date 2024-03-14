#pragma once
#include "Print.h"
#include <sstream>

class Stream : public Print
{
  public:

    bool available();
    int parseInt();
    char read();
    char peek();
    size_t write(uint8_t c) override;

    std::string str();
    void seekg(size_t pos);

  private:
    std::stringstream _stream;
};
