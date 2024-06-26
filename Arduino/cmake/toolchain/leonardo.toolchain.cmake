set(ARDUINO_BOARD "AVR_LEONARDO")
set(ARDUINO_MCU "atmega32u4")
set(ARDUINO_F_CPU "16000000L")
set(ARDUINO_VARIANT "leonardo")
set(ARDUINO_AVRDUDE_PROTOCOL "avr109")
set(ARDUINO_AVRDUDE_SPEED "57600")
set(ARDUINO_USB On)
set(ARDUINO_USB_VID "0x2341")
set(ARDUINO_USB_PID "0x8036")
set(ARDUINO_USB_PRODUCT "Arduino Leonardo")
set(ARDUINO_PORT "/dev/ttyACM0"
    CACHE STRING "The serial port for uploading using avrdude")

include(${CMAKE_CURRENT_LIST_DIR}/avr.toolchain.cmake)