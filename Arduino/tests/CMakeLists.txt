cmake_minimum_required(VERSION 3.19)
project(tests)

add_subdirectory(googletest)
add_subdirectory(arduino-mock)
add_subdirectory(libraries-mock)
add_subdirectory(arduino-tests)