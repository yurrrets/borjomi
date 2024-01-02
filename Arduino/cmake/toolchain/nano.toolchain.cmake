set(ARDUINO_BOARD "AVR_NANO")
set(ARDUINO_MCU "atmega168")
set(ARDUINO_F_CPU "16000000L")
set(ARDUINO_VARIANT "standard")
set(ARDUINO_AVRDUDE_PROTOCOL "arduino")
set(ARDUINO_AVRDUDE_SPEED "19200")
set(ARDUINO_USB Off)
set(ARDUINO_PORT "/dev/ttyUSB0"
    CACHE STRING "The serial port for uploading using avrdude")

include(${CMAKE_CURRENT_LIST_DIR}/avr.toolchain.cmake)