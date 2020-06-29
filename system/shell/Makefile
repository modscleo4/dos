BUILD_DIR=../build/system/shell
SHELL=$(BUILD_DIR)/system/shell/shell

CFLAGS=-ffreestanding -m32 -I libc/include -nostdlib

SHELL_SRCS := $(wildcard *.c)
SHELL_HDRS := $(wildcard *.h)
SHELL_OBJS := $(patsubst %.c, $(BUILD_DIR)/%.o, $(SHELL_SRCS))

all: $(SHELL)

$(BUILD_DIR)/%.o: %.c $(SHELL_HDRS)
	gcc $(CFLAGS) -c $< -o $@

$(KERNEL): $(SHELL_OBJS)
	ld -m elf_i386 -T Sshell.lds $^ -o $@

clean:
	rm $(SHELL_OBJS)
