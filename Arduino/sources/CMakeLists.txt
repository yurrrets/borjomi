execute_process (
    COMMAND bash "-c" [=[ git describe --tags | sed -r "s/(\w+)\.(\w+).*/\1/" ]=]
    OUTPUT_VARIABLE BORJOMI_VERSION_MJ
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process (
    COMMAND bash "-c" [=[ git describe --tags | sed -r "s/(\w+)\.(\w+).*/\2/" ]=]
    OUTPUT_VARIABLE BORJOMI_VERSION_MN
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process (
    COMMAND bash "-c" [=[ git rev-list  `git rev-list --tags --no-walk --max-count=1`..HEAD --count ]=]
    OUTPUT_VARIABLE BORJOMI_VERSION_REV
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("BORJOMI_VERSION = ${BORJOMI_VERSION_MJ}.${BORJOMI_VERSION_MN}.${BORJOMI_VERSION_REV}")

add_library(BorjomiCommon INTERFACE)
target_compile_definitions(BorjomiCommon INTERFACE
    BORJOMI_VERSION_MJ=${BORJOMI_VERSION_MJ}
    BORJOMI_VERSION_MN=${BORJOMI_VERSION_MN}
    BORJOMI_VERSION_REV=${BORJOMI_VERSION_REV}
)

add_arduino_lib_dir(common-node common-node)
target_link_libraries(common-node PUBLIC ArduinoSTL BorjomiCommon)

add_arduino_lib_dir(lib-lcd lib-lcd)

add_arduino_fw_dir(master-node master-node)
target_link_libraries(master-node PRIVATE crc mcp_can SoftwareSerial EEPROM lib-lcd common-node)

add_arduino_fw_dir(master-node-cfg master-node-cfg)
target_link_libraries(master-node-cfg PRIVATE crc EEPROM common-node)

add_arduino_fw_dir(slave-node slave-node)
target_link_libraries(slave-node PRIVATE crc mcp_can SoftwareSerial EEPROM common-node)

add_arduino_fw_dir(slave-node-cfg slave-node-cfg)
target_link_libraries(slave-node-cfg PRIVATE crc EEPROM common-node)

# add_arduino_builtin_lib(LiquidCrystal)
add_arduino_fw_dir(test-lcd test-lcd)
target_link_libraries(test-lcd PRIVATE lib-lcd)