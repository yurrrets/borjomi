# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile

BOARD_TAG    = mini328     # nano or nano328 (old bootloader) or mini328 (new bootloader)

LOCAL_CPP_SRCS  ?= $(wildcard src/*.cpp)

ARDUINO_SKETCHBOOK = .
USER_LIB_PATH = $(realpath ../libraries)
ARDUINO_LIBS = ArduinoSTL MCP_CAN SPI CRC EEPROM common-node

include ../libraries/common-node/version.mk
include /usr/share/arduino/Arduino.mk

DEVICE_PATH=/dev/ttyUSB0

CPPFLAGS += -std=c++11  -DDEBUG
