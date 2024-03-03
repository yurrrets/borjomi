#pragma once

class Stream
{
  public:
    template <typename T> void print(const T &value)
    {
    }

    template <typename Numeric> void print(const Numeric &value, int base)
    {
    }

    void println()
    {
    }

    void flush()
    {
    }
};