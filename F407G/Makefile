all:
	make clean
	make build
	make flash

build:
	cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake
	cmake --build build
	#arm-none-eabi-gcc build/bin/F407G.elf -Xlinker -Map=output.lst

clean:
	rm -rf -v ./build

flash:
	st-flash --serial 066AFF3231454D3043012639 --reset write build/bin/F407G.bin 0x8000000

rst: reset
reset:
	st-flash --serial 066AFF3231454D3043012639 reset

probe:
	st-info --probe

uart:
	picocom --echo -b 115200 /dev/ttyAMA0
	