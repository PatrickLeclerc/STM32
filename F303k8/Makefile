all:
	make clean
	make build
	make flash
	make uart

build:
	cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake
	cmake --build build

clean:
	rm -rf -v ./build

flash:
	st-flash --serial 066CFF303032424257015749 --reset write build/bin/F303k8.bin 0x8000000

reset:
	st-flash --serial 066CFF303032424257015749 reset

probe:
	st-info --probe

uart:
	picocom --echo -b 115200 /dev/ttyACM1
