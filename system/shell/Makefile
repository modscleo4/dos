BUILD_DIR=../../build/system/shell
BIN=$(BUILD_DIR)/shell

CFLAGS=-ffreestanding -m32 -I ../libc/include -nostdlib

ASM_SRCS = $(wildcard *.asm)
ASM_OBJS = $(patsubst %.asm, $(BUILD_DIR)/%.obj, $(ASM_SRCS))
SRCS := $(wildcard *.c ../libc/*.c)
HDRS := $(wildcard *.h ../libc/include/*.h)
OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))

all: dir $(BIN)

dir:
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.obj: %.asm
	nasm -f elf32 $< -o $@

$(BUILD_DIR)/%.o: %.c $(HDRS)
	gcc $(CFLAGS) -c $< -o $@

$(BIN): $(ASM_OBJS) $(OBJS)
	ld -m elf_i386 -T shell.lds $^ -o $@

clean:
	rm $(OBJS)
