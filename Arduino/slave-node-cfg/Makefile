# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile

BOARD_TAG    = nano     # nano or nano328

LOCAL_CPP_SRCS  ?= $(wildcard src/*.cpp)

ARDUINO_SKETCHBOOK = .
USER_LIB_PATH = $(realpath ../libraries)
ARDUINO_LIBS = ArduinoSTL MCP_CAN SPI CRC EEPROM common-node

include /usr/share/arduino/Arduino.mk

DEVICE_PATH=/dev/ttyUSB*

CPPFLAGS += -std=c++11

