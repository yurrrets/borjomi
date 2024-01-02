# Transforms the target.elf file into target.eep (EEPROM) and target.hex files.
# Also prints the size of each section in target.elf.
function(mark_as_arduino_avr target)
    set_target_properties(${target} PROPERTIES SUFFIX ".elf")
    target_link_libraries(${target} PUBLIC ArduinoCore)
    target_compile_options(${target} PRIVATE
        "-Wall"
        "-Wextra"
        "-pedantic"
    )
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_SIZE} ARGS 
            -A "$<TARGET_FILE:${target}>"
        USES_TERMINAL)
    add_custom_command(TARGET ${target} POST_BUILD
        BYPRODUCTS ${target}.eep
        COMMAND ${CMAKE_OBJCOPY} ARGS 
            -O ihex -j .eeprom
            --set-section-flags=.eeprom=alloc,load
            --no-change-warnings --change-section-lma 
            .eeprom=0
            "$<TARGET_FILE:${target}>"
            ${target}.eep)
    add_custom_command(TARGET ${target} POST_BUILD
        BYPRODUCTS ${target}.hex
        COMMAND ${CMAKE_OBJCOPY} ARGS 
            -O ihex -R .eeprom
            "$<TARGET_FILE:${target}>"
            ${target}.hex)

    # Creates a target upload-target that uses avrdude to upload target.hex to the
    # given serial port.
    add_custom_target(upload-${target}
        COMMAND ${ARDUINO_AVRDUDE} 
            -C${ARDUINO_AVRDUDE_CONF}
            -p${ARDUINO_MCU}
            -c${ARDUINO_AVRDUDE_PROTOCOL}
            -P${ARDUINO_PORT}
            -b${ARDUINO_AVRDUDE_SPEED}
            -D
            "-Uflash:w:$<TARGET_FILE_BASE_NAME:${target}>.hex:i"
        USES_TERMINAL)
    add_dependencies(upload-${target} ${target})

endfunction()