BUILD_DIR=../../build/system/init
BIN=$(BUILD_DIR)/init.elf

CFLAGS=-ffreestanding -m32 -I ../libc/include -nostdlib -Wl,--stack,0x4000

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
	ld -m elf_i386 -T init.lds $^ -o $@

clean:
	rm $(OBJS)
