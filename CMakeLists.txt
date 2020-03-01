cmake_minimum_required(VERSION 3.15)
project(PiOs CXX ASM)

#set(CMAKE_ASM_COMPILER aarch64-linux-gnu-as)
set(CMAKE_ASM_COMPILER aarch64-none-elf-as)
set(CMAKE_ASM_FLAGS_DEBUG "")
set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_LINK_EXECUTABLE "aarch64-linux-gnu-gcc <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_CXX_LINK_EXECUTABLE "aarch64-none-elf-gcc <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_CXX_STANDARD_LIBRARIES "")
set(CMAKE_CXX_STANDARD_LIBRARIES_INIT "")
set(CMAKE_C_STANDARD_LIBRARIES "")

add_executable(PiOs kernel/loader.S
        kernel/helper.S
        kernel/main.cpp
        kernel/ktypes.h
        kernel/BCM2837.h
        kernel/uart1.h
        kernel/uart1.cpp
        kernel/gpio.h
        kernel/gpio.cpp
        kernel/mailbox.cpp
        kernel/mailbox.h
        kernel/uart0.cpp
        kernel/uart0.h
        kernel/emmc.cpp
        kernel/emmc.h
        kernel/timer.cpp
        kernel/timer.h)

target_compile_options(PiOs PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:
        --target=aarch64-none-elf -march=armv8-a -mtune=cortex-a72
        -ffreestanding -fno-builtin -nostdlib -nostdinc -nostdinc++
        -Wall -Wextra>)
target_link_options(PiOs PRIVATE
        "-Wl,-T${CMAKE_SOURCE_DIR}/kernel/linker.ld"
        -ffreestanding -fno-builtin -nostdlib -nostdinc -nostdinc++
        -Wall -Wextra)
add_custom_command(TARGET PiOs POST_BUILD
        COMMAND aarch64-none-elf-objcopy $<TARGET_FILE:PiOs> -O binary kernel8.img)