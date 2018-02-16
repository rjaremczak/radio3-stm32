cmake_minimum_required(VERSION 3.9)
project(radio3_stm32 VERSION 3.3 LANGUAGES CXX C ASM)
string(TIMESTAMP BUILD_TIMESTAMP %s)

add_definitions(-DSTM32F10X_MD)
add_definitions(-DUSE_STDPERIPH_DRIVER)
add_definitions(-DHSE_VALUE=8000000)

include_directories(src)
include_directories(lib_stm32/system/include)
include_directories(lib_stm32/system/include/newlib)
include_directories(lib_stm32/system/include/cmsis)
include_directories(lib_stm32/system/include/stm32f1-stdperiph)
include_directories(lib_stm32/core)
include_directories(lib_stm32/debug)
include_directories(lib_stm32/timer)
include_directories(lib_stm32/io)
include_directories(lib_stm32/usb_fs_driver)
include_directories(lib_stm32/usb_vcom)

set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_EXE_LINKER_FLAGS "-mcpu=cortex-m3 -mthumb -ffreestanding -ffunction-sections -Wall -Wextra -T ${CMAKE_SOURCE_DIR}/stm32f103c8t6_flash.ld -nostartfiles -Xlinker --gc-sections --specs=nano.specs  -Wl,-Map=${PROJECT_BINARY_DIR}/radio3.map")

set_property(SOURCE src/Radio3.cpp APPEND PROPERTY COMPILE_DEFINITIONS PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
set_property(SOURCE src/Radio3.cpp APPEND PROPERTY COMPILE_DEFINITIONS PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR})
set_property(SOURCE src/Radio3.cpp APPEND PROPERTY COMPILE_DEFINITIONS BUILD_TIMESTAMP=${BUILD_TIMESTAMP})

file(GLOB_RECURSE USER_SOURCES "src/*.c*" "src/*.h")
file(GLOB_RECURSE LIB_STM32_SOURCES "lib_stm32/*.c*" "lib_stm32/*.h")

add_executable(radio3.elf ${USER_SOURCES} ${LIB_STM32_SOURCES})
target_compile_features(radio3.elf PUBLIC cxx_std_17 c_std_11)

target_compile_options(radio3.elf
        PUBLIC -mcpu=cortex-m3 -mthumb -ffreestanding -fmessage-length=0 -ffunction-sections -fdata-sections -Wall -Wextra -Wno-register
        PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-fabi-version=0 -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics>)

add_custom_command(TARGET radio3.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex radio3.elf ${PROJECT_BINARY_DIR}/radio3.hex
        COMMAND ${TOOLCHAIN}/bin/arm-none-eabi-size --format=berkeley radio3.elf
        COMMENT "post build steps")