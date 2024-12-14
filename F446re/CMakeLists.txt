# CMakeLists.txt
cmake_minimum_required(VERSION 3.22)
project(F446re C CXX ASM)

# Set the build type (Debug or Release)
set(DEBUG 0)
if(DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directory
set(BUILD_DIR "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})

# Source files
set(C_SOURCES
    ARM/Core/Src/system_stm32f4xx.c
)

set(CXX_SOURCES
    # Drivers
    drivers/clock/clock.cpp
    drivers/gpio/gpio.cpp
    drivers/usart/usart.cpp
    drivers/rtc/rtc.cpp
    drivers/dma/dma.cpp
    drivers/tim/tim.cpp
    drivers/sdio/sdio.cpp

    # Apps
    apps/led/led.cpp
    apps/console/console.cpp
    nucleo/f446re.cpp
    main.cpp
)

set(ASM_SOURCES
    ARM/startup_stm32f446xx.s
)

# Include directories
include_directories(
    ARM/CMSIS/Device/ST/STM32F4xx/Include
    ARM/CMSIS/Include
    # Drivers
    drivers
    drivers/clock
    drivers/gpio
    drivers/usart
    drivers/rtc
    drivers/dma
    drivers/tim
    drivers/sdio
    # Apps
    apps
    apps/led
    apps/console
    # Top
    nucleo

#    drivers/tim

)

# Define the target and add sources
add_executable(${PROJECT_NAME}.elf ${C_SOURCES} ${CXX_SOURCES} ${ASM_SOURCES})

# MCU settings
set(CPU_FLAGS 
    -mcpu=cortex-m4 
    -mthumb 
    -mfpu=fpv4-sp-d16 
    -mfloat-abi=hard
)
set(C_DEFINES -DSTM32F446xx)

# Compiler and linker flags
target_compile_options(${PROJECT_NAME}.elf PRIVATE
    ${CPU_FLAGS}
    ${C_DEFINES}
    -Wall
    -fdata-sections
    -ffunction-sections
    $<$<CONFIG:Debug>:-g -gdwarf-2>
    -O3
)

target_link_options(${PROJECT_NAME}.elf PRIVATE
    ${CPU_FLAGS}
    -specs=nano.specs
    -T../ARM/STM32F446RETx_FLASH.ld
    -Wl,-Map=${BUILD_DIR}/${PROJECT_NAME}.map,--cref
    -Wl,--gc-sections
    -lc
    -lm
#    -lnosys
)

# Add custom commands to generate HEX and BIN files
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${BUILD_DIR}/${PROJECT_NAME}.elf ${BUILD_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary -S ${BUILD_DIR}/${PROJECT_NAME}.elf ${BUILD_DIR}/${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} ${BUILD_DIR}/${PROJECT_NAME}.elf
)
