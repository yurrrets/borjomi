#include <sstream>

class Print
{
  public:
    virtual size_t write(uint8_t) = 0;

    template <typename T> void print(const T &value)
    {
        std::stringstream ss;
        ss << value;
        auto s = ss.str();
        for (auto &c : s)
        {
            write(c);
        }
    }

    template <typename Numeric> void print(const Numeric &value, int base)
    {
        char buffer[32] = {};
        itoa(value, buffer, base);
        print(std::string(buffer));
    }

    void println()
    {
        print('\n');
    }

    virtual void flush()
    {
    }
};
