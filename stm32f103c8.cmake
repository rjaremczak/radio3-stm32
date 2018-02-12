set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
#set(TOOLCHAIN "/usr/local/gcc-arm-none-eabi-5_4-2016q3")
#set(TOOLCHAIN "/usr/local/gcc-arm-none-eabi-6_2-2016q3")
set(TOOLCHAIN "/usr/local/gcc-arm-none-eabi-7-2017-q4-major")
set(CMAKE_VERBOSE_MAKEFILE on)

# specify the cross compiler
set(CMAKE_C_COMPILER ${TOOLCHAIN}/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN}/bin/arm-none-eabi-g++)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(COMMON_FLAGS "-g -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -Wall -Wextra")
set(CMAKE_ASM_FLAGS "${COMMON_FLAGS} -x assembler-with-cpp")
set(CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu11")
set(CMAKE_CXX_FLAGS "${COMMON_FLAGS} -std=c++17 -fabi-version=0 -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics")
set(CMAKE_EXE_LINKER_FLAGS "-T mem.ld -T libs.ld -T sections.ld -nostartfiles -Xlinker --gc-sections -L\"${CMAKE_SOURCE_DIR}/ldscripts/\" --specs=nano.specs")