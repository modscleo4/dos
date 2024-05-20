BUILD_DIR=../build/system/bin

all: dir init sh bmp cat ls

dir:
	mkdir -p $(BUILD_DIR)

.PHONY: init sh bmp cat ls

init:
	make -C init

sh:
	make -C sh

bmp:
	make -C bmp

cat:
	make -C cat

ls:
	make -C ls

clean:
	make -C init clean
	make -C sh clean
	make -C bmp clean
	make -C cat clean
	make -C ls clean
