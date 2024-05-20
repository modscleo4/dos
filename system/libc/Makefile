BUILD_DIR=../../build/system/libc
LIB_DIR=../../build/system/lib
STATIC_LIB=$(LIB_DIR)/libc.a
DYNAMIC_LIB=$(LIB_DIR)/libc.so

CC=gcc
CFLAGS=-ffreestanding -m32 -I ./include -nostdlib -nolibc -nodefaultlibs -Wl,--stack,0x4000 -Wno-packed-bitfield-compat -fno-common -fpic -fms-extensions
LDFLAGS=-m elf_i386 -shared -fpic

ASM_SRCS = $(wildcard *.asm)
ASM_OBJS = $(patsubst %.asm, $(BUILD_DIR)/%.obj, $(ASM_SRCS))
SRCS := $(wildcard *.c sys/*.c)
HDRS := $(wildcard include/*.h include/sys/*.h)
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))

all: dir $(STATIC_LIB) $(DYNAMIC_LIB)

dir:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/sys
	mkdir -p $(LIB_DIR)

$(BUILD_DIR)/%.obj: %.asm
	nasm -f elf32 $< -o $@

$(BUILD_DIR)/%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

$(STATIC_LIB): $(ASM_OBJS) $(OBJS)
	ar rcs $@ $^

$(DYNAMIC_LIB): $(ASM_OBJS) $(OBJS)
	ld $(LDFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) $(ASM_OBJS) $(LIB)
