#ifndef TOOLS_H
#define TOOLS_H

#include <Arduino.h>
#include <crc.h>

/// Calcs CRC8 for type T.
/// Type T should have first field of type uint8_t,
/// this is crc8 checksum of the whole next data stored in T.
/// That is why checksum is called Partial.
template <typename T>
uint8_t Crc8Partial(const T &val)
{
    const uint8_t *data = reinterpret_cast<const uint8_t *>(&val);
    data++;
    return crc8(data, sizeof(T)-1);
}


#endif // TOOLS_H

