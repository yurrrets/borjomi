#ifndef STREAM_EXT_H
#define STREAM_EXT_H

#include <Arduino.h>

class StreamExt : public Stream
{
public:
    StreamExt(Stream &wrk);

    int lastReaded() { return _lastReaded; }
    int readNext();
    size_t readUntil(char terminator);

    struct ReadResult
    {
        size_t sz;
        int terminator;
        ReadResult(): sz(0), terminator(-1) {}
    };
    ReadResult readBytesUntilOneOf(const char *terminators, size_t terminator_length, char *buffer, size_t length);

    size_t write(uint8_t val);
    int available();
    int read();
    int peek();
    void flush();

private:
    Stream &wrk;
    int _lastReaded;
};

#endif // STREAM_EXT_H
