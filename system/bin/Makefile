BUILD_DIR=../build/system/bin

all: dir init sh

dir:
	mkdir -p $(BUILD_DIR)

.PHONY: init sh

init:
	make -C init

sh:
	make -C sh

clean:
	make -C init clean
	make -C sh clean
