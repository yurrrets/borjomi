cmake_minimum_required(VERSION 3.19)
project(TestArduino)

include(cmake/lto.cmake)
include(cmake/arduino_core.cmake)
include(cmake/arduino_fw.cmake)

option(WITH_DEBUG_MESSAGES "Turns on debug messages to Serial console" OFF)

if (WITH_DEBUG_MESSAGES)
add_compile_definitions(DEBUG_MESSAGES)
endif()

add_arduino_builtin_lib(ArduinoSTL)
add_arduino_builtin_lib(SoftwareSerial)
add_arduino_builtin_lib(EEPROM)
add_subdirectory(libraries)
add_subdirectory(sources)

