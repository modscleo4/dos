BUILD_DIR=../build/system

all: dir libc bin

dir:
	mkdir -p $(BUILD_DIR)

.PHONY: libc bin

libc:
	make -C libc

bin:
	make -C bin

clean:
	make -C libc clean
	make -C bin clean
