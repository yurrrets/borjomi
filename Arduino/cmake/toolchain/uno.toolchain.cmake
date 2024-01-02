set(ARDUINO_BOARD "AVR_UNO")
set(ARDUINO_MCU "atmega328p")
set(ARDUINO_F_CPU "16000000L")
set(ARDUINO_VARIANT "standard")
set(ARDUINO_AVRDUDE_PROTOCOL "arduino")
set(ARDUINO_AVRDUDE_SPEED "115200")
set(ARDUINO_USB Off)
set(ARDUINO_PORT "/dev/ttyACM0"
    CACHE STRING "The serial port for uploading using avrdude")

include(${CMAKE_CURRENT_LIST_DIR}/avr.toolchain.cmake)