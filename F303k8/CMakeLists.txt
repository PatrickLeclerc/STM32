# CMakeLists.txt
cmake_minimum_required(VERSION 3.22)
project(F303k8 C CXX ASM)

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
    ARM/Core/Src/system_stm32f3xx.c
)

set(CXX_SOURCES
    # Drivers
    drivers/clock/clock.cpp
    drivers/gpio/gpio.cpp
    drivers/usart/usart.cpp
    drivers/rtc/rtc.cpp
    drivers/dma/dma.cpp
    drivers/tim/tim.cpp
    drivers/spi/spi.cpp
    # Apps
    apps/led/led.cpp
    #apps/console/console.cpp
    apps/comport/comport.cpp
    apps/wait/wait.cpp
    apps/sd/sd.cpp
    #nucleo/f303k8.cpp
    main.cpp
)

set(ASM_SOURCES
    ARM/startup_stm32f303x8.s
)

# Include directories
include_directories(
    ARM/CMSIS/Device/ST/STM32F3xx/Include
    ARM/CMSIS/Include
    # Drivers
    drivers
    drivers/clock
    drivers/gpio
    drivers/usart
    drivers/rtc
    drivers/dma
    drivers/tim
    drivers/spi
    # Apps
    apps
    #apps/led
    #apps/console
    apps/comport
    apps/wait
    apps/sd
    # Top
    #nucleo
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
set(C_DEFINES -DSTM32F303x8)

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
    -T../ARM/STM32F303K8Tx_FLASH.ld
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
