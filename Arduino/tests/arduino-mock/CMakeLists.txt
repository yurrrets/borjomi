file(GLOB src_files *.h *.cpp)
add_library(arduino-mock STATIC ${src_files})
target_include_directories(arduino-mock PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})