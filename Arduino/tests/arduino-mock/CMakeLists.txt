file(GLOB src_files *.h *.cpp)
add_library(arduino-mock STATIC ${src_files})
target_include_directories(arduino-mock PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# for log support
target_compile_definitions(arduino-mock PRIVATE DEBUG_MESSAGES)
target_include_directories(arduino-mock PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../sources/common-node/src>
)