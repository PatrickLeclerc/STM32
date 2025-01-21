# toolchain.cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Define compiler paths
set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-none-eabi-g++")
set(CMAKE_ASM_COMPILER "arm-none-eabi-gcc")
set(CMAKE_OBJCOPY "arm-none-eabi-objcopy")
set(CMAKE_SIZE "arm-none-eabi-size")

# Set compiler flags for ASM and C
set(CMAKE_ASM_FLAGS "${CPU_FLAGS} -Wall -fdata-sections -ffunction-sections")
set(CMAKE_C_FLAGS "${CPU_FLAGS} ${C_DEFINES} -Wall -fdata-sections -ffunction-sections --specs=nosys.specs")
set(CMAKE_CXX_FLAGS "${CPU_FLAGS} ${C_DEFINES} -Wall -fdata-sections -ffunction-sections --specs=nosys.specs")
