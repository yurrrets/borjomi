file(GLOB src_files *.h *.cpp)

# function(add_arduino_lib_dir libname dir)
#     if (TARGET_${libname})
#         return() # already added
#     endif()
#     set(TARGET_${libname} yes)
#     file(GLOB src_files
#         ${dir}/*.h ${dir}/*.cpp ${dir}/*.c ${dir}/*.S
#         ${dir}/src/*.h ${dir}/src/*.cpp ${dir}/src/*.c ${dir}/src/*.S)
#     add_library(${libname} STATIC ${src_files})
#     target_compile_features(${libname} PUBLIC cxx_std_11 c_std_11)
#     target_link_libraries(${libname} PUBLIC arduino-mock)
#     target_include_directories(${libname} PUBLIC ${dir} ${dir}/src)
# endfunction()


# add_arduino_lib_dir(CRC ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/CRC)

add_executable(arduino-tests 
    ${src_files}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sources/common-node/src/can_message.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sources/master-node/src/scenarios.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sources/master-node/src/cap_implementation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../sources/master-node/src/can_commands.cpp
)

target_include_directories(arduino-tests PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../sources/master-node/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../sources/common-node/src>
)
target_link_libraries(arduino-tests PRIVATE arduino-mock libraries-mock gtest gmock gmock_main)