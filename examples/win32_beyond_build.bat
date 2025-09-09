@echo off
set DEF_COMPILER_FLAGS=-mconsole -march=native -mtune=native -std=c89 -pedantic -nodefaultlibs -nostdlib -mno-stack-arg-probe -Xlinker /STACK:0x100000,0x100000 ^
-fno-builtin -ffreestanding -fno-asynchronous-unwind-tables -fuse-ld=lld ^
-Wall -Wextra -Werror -Wvla -Wconversion -Wdouble-promotion -Wsign-conversion -Wmissing-field-initializers -Wuninitialized -Winit-self -Wunused -Wunused-macros -Wunused-local-typedefs

set DEF_FLAGS_LINKER=-lkernel32 -lws2_32

REM Build the station
set SOURCE_NAME=win32_beyond_station
cc -s -O2 %DEF_COMPILER_FLAGS% %SOURCE_NAME%.c -o %SOURCE_NAME%.exe %DEF_FLAGS_LINKER%

REM satellite builds
set SOURCE_NAME=win32_beyond_satellite

REM Build the satellite
cc -s -O2 %DEF_COMPILER_FLAGS% %SOURCE_NAME%.c -o %SOURCE_NAME%.exe %DEF_FLAGS_LINKER%
