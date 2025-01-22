all:
	make clean
	make build
	make flash

build:
	cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake
	cmake --build build

clean:
	rm -rf -v ./build

flash:
	st-flash --serial 066AFF3231454D3043012639 --reset write build/bin/F407G.bin 0x8000000

reset:
	st-flash --serial 066AFF3231454D3043012639 reset

probe:
	st-info --probe

uart:
	picocom --echo -b 115200 /dev/ttyAMA0
