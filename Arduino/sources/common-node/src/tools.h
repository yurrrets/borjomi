#ifndef TOOLS_H
#define TOOLS_H

#include <Arduino.h>
#include <crc.h>
#include <Stream.h>

/// Calcs CRC8 for type T.
/// Type T should have first or last field named "crc" of type uint8_t,
/// this is crc8 checksum of the whole data stored in T except for crc itself.
/// That is why checksum is called Partial.
template <typename T>
inline uint8_t Crc8Partial(const T &obj)
{
    static_assert(sizeof(decltype(T::crc)) == sizeof(uint8_t));
    static_assert((offsetof(T, crc) == 0) || (offsetof(T, crc) == sizeof(T)-sizeof(decltype(T::crc))));
    const uint8_t *data = reinterpret_cast<const uint8_t *>(&obj);
    if constexpr (offsetof(T, crc) == 0)
    {
        data++;
    }
    return crc8(data, sizeof(T)-1);
}

// Using CRTP pattern, enables updating and checking of crc8 checksum
template <typename T>
void UpdateCrc8(T &obj)
{
    obj.crc = Crc8Partial(obj);
}

template <typename T>
bool CheckCrc8(const T &obj)
{
    return obj.crc == Crc8Partial(obj);
}

// overload for manipulators like endl
typedef Stream & (*stream_manipulator)(Stream &);
inline Stream &operator << (Stream &stream, stream_manipulator m)
{
    return m(stream);
}

template <typename T>
inline Stream &operator << (Stream &stream, const T &val)
{
    stream.print(val);
    return stream;
}

template <typename T>
struct hex_type
{
    constexpr static int base = 16;
    T val;
    hex_type(const T &val) : val(val) {}
};
template <typename T> inline hex_type<T> hex(const T &val) { return hex_type<T>(val); }

template <typename T>
inline Stream &operator << (Stream &stream, const hex_type<T> &val)
{
   stream.print(val.val, val.base);
   return stream;
}

inline
Stream &endl(Stream &stream)
{
    stream.println();
    stream.flush();
    return stream;
}



#endif // TOOLS_H

