# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile

# nano or nano328 (old bootloader) or mini328 (new bootloader)
BOARD_TAG    = mini328

LOCAL_CPP_SRCS  ?= $(wildcard src/*.cpp)

ARDUINO_SKETCHBOOK = .
USER_LIB_PATH = $(realpath ../libraries)
ARDUINO_LIBS = ArduinoSTL MCP_CAN SPI CRC EEPROM common-node

include ../libraries/common-node/version.mk
include /usr/share/arduino/Arduino.mk

DEVICE_PATH=/dev/ttyUSB0

ifeq ($(BOARD_TAG), uno)
DEVICE_PATH=/dev/ttyUSB0
CPPFLAGS += -DDEBUG
endif

ifeq ($(BOARD_TAG), nano)
DEVICE_PATH=/dev/ttyUSB0
endif

ifeq ($(BOARD_TAG), nano328)
DEVICE_PATH=/dev/ttyUSB0
endif

CPPFLAGS += -std=c++11


