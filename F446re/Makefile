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
	st-flash --serial 0670FF495051717867112654 --reset write build/bin/F446re.bin 0x8000000

reset:
	st-flash --serial 0670FF495051717867112654 reset

probe:
	st-info --probe

uart:
	picocom --echo -b 115200 /dev/ttyACM0
