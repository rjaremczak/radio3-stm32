project(radio3_stm32 CXX C ASM)
cmake_minimum_required(VERSION 3.6)

add_definitions(-DSTM32F10X_MD)
add_definitions(-DUSE_STDPERIPH_DRIVER)
add_definitions(-DHSE_VALUE=8000000)
add_definitions(-DOS_USE_TRACE_SEMIHOSTING_DEBUG)

file(GLOB_RECURSE USER_SOURCES "src/*.c" "src/*.cpp")
file(GLOB_RECURSE SYSTEM_SOURCES "system/src/*.c" "system/src/*.cpp")

include_directories(include)
include_directories(system/include)
include_directories(system/include/newlib)
include_directories(system/include/cmsis)
include_directories(system/include/stm32f1-stdperiph)

add_executable(${PROJECT_NAME}.elf ${USER_SOURCES} ${SYSTEM_SOURCES} ${LINKER_SCRIPT})

set(PROJECT_BUILD_DIR ${PROJECT_SOURCE_DIR}/build)
set(HEX_FILE ${PROJECT_BUILD_DIR}/${PROJECT_NAME}.hex)
set(MAP_FILE ${PROJECT_BUILD_DIR}/${PROJECT_NAME}.map)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${MAP_FILE}")

add_custom_command(TARGET ${PROJECT_NAME}.elf PRE_BUILD
        COMMAND ${PROJECT_SOURCE_DIR}/buildid-updater.sh ${PROJECT_SOURCE_DIR}/include/buildid.h
        COMMENT "Update build ID")

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMENT "Building ${HEX_FILE}")

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${TOOLCHAIN}/bin/arm-none-eabi-size --format=berkeley $<TARGET_FILE:${PROJECT_NAME}.elf>
        COMMENT "Size information of ${HEX_FILE}")
