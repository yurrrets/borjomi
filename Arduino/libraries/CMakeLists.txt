add_arduino_lib_dir(crc CRC)

add_arduino_lib_dir(spi SPI)

add_arduino_lib_dir(mcp_can MCP_CAN)
target_link_libraries(mcp_can PUBLIC spi)