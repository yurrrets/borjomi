#ifndef CONFIG_H
#define CONFIG_H

#ifndef DEBUG
//#define DEBUG
#endif

//#define PC_START

#ifdef DEBUG
    class Stream;
    extern Stream &dbgSerial;
#endif

#define DC_VOLTAGE_MIN   (11.5)
#define DC_VOLTAGE_MAX   (13.2)

#endif // CONFIG_H

