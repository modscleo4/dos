BUILD_DIR=../build/system

all: dir sys_shell

dir:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/libc

sys_shell:
	make -C shell

clean:
	make -C shell clean
