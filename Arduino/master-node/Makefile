# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile

#BOARD_TAG    = uno
BOARD_TAG    = nano328

LOCAL_CPP_SRCS  ?= $(wildcard src/*.cpp)

ARDUINO_SKETCHBOOK = .
USER_LIB_PATH = $(realpath ../libraries)
ARDUINO_LIBS = SoftwareSerial MCP_CAN SPI CRC common-node ArduinoSTL EEPROM

include ../libraries/common-node/version.mk
include /usr/share/arduino/Arduino.mk

DEVICE_PATH=

ifeq ($(BOARD_TAG), uno)
DEVICE_PATH=/dev/ttyACM0
CPPFLAGS += -DDEBUG
endif

ifeq ($(BOARD_TAG), nano)
DEVICE_PATH=/dev/ttyUSB0
endif

ifeq ($(BOARD_TAG), nano328)
DEVICE_PATH=/dev/ttyUSB0
endif

CPPFLAGS += -std=c++11

