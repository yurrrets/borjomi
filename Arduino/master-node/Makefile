# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile

#BOARD_TAG    = uno
BOARD_TAG    = nano

LOCAL_CPP_SRCS  ?= $(wildcard src/*.cpp)

ARDUINO_SKETCHBOOK = .
USER_LIB_PATH = $(realpath ../libraries)
ARDUINO_LIBS = SoftwareSerial MCP_CAN SPI CRC common-node ArduinoSTL

include /usr/share/arduino/Arduino.mk

#DEVICE_PATH=/dev/ttyACM0
DEVICE_PATH=/dev/ttyUSB0

CPPFLAGS += -std=c++11 # -DDEBUG

