#include "stream_ext.h"
#include <algorithm>

StreamExt::StreamExt(Stream &wrk)
    : wrk(wrk)
{
}

int StreamExt::readNext()
{
    return timedRead();
}

size_t StreamExt::readUntil(char terminator)
{
    size_t res = 0;
    int c;
    do
    {
        c = timedRead();
        res++;
    } while (c >= 0 && c != terminator);
    return res;
}

StreamExt::ReadResult StreamExt::readBytesUntilOneOf(const char *terminators, size_t terminator_length, char *buffer, size_t length)
{
    const char *tBegin = terminators;
    const char *tEnd = terminators + terminator_length;

    ReadResult res;
    if (length < 1) return res;
    while (res.sz < length) {
        res.terminator = timedRead();
        if (res.terminator < 0) break;
        if (tEnd != std::find(tBegin, tEnd, res.terminator)) break;
        *buffer++ = (char)res.terminator;
        res.sz++;
    }
    return res; // return number of characters, not including null terminator
}

size_t StreamExt::write(uint8_t val)
{
#ifdef DEBUG
    Serial.write(val);
#endif
    return wrk.write(val);
}

int StreamExt::available()
{
    return wrk.available();
}

int StreamExt::read()
{
    _lastReaded = wrk.read();
#ifdef DEBUG
    if (_lastReaded >= 0)
        Serial.write(_lastReaded);
#endif
    return _lastReaded;
}

int StreamExt::peek()
{
    return wrk.peek();
}

void StreamExt::flush()
{
    wrk.flush();
}

#include "stream_ext.h"
