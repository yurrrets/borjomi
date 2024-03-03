file(GLOB src_files *.h *.cpp)
add_library(libraries-mock STATIC ${src_files})
target_include_directories(libraries-mock PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../sources/common-node/src>
)
target_link_libraries(libraries-mock PUBLIC arduino-mock gtest gmock)