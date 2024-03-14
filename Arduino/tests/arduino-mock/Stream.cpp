#include "Stream.h"

bool Stream::available()
{
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

void Stream::seekg(size_t pos)
{
    _stream.seekg(pos);
}

size_t Stream::write(uint8_t c)
{
    _stream.put(c);
    return 1;
}
