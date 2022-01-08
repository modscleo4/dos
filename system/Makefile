BUILD_DIR=../build/system

all: dir sys_init

dir:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/libc

sys_init:
	make -C init

clean:
	make -C init clean
