#include "Stream.h"
#include "Arduino.h"
#include "log.h"

bool Stream::available()
{
    auto pos = _stream.tellg();
    peek();
    return !_stream.eof();
}

int Stream::parseInt()
{
    int res;
    _stream >> res;
    return res;
}

char Stream::read()
{
    return _stream.get();
}

char Stream::peek()
{
    return _stream.peek();
}

std::string Stream::str()
{
    return _stream.str();
}

void Stream::str(const std::string &s)
{
    _stream.str(s);
    _stream.clear();
}

void Stream::seekg(size_t pos)
{
    _stream.seekg(pos);
    _stream.clear();
}

size_t Stream::write(uint8_t c)
{
    _stream.put(c);
    return 1;
}
