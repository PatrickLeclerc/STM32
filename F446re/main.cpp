#include "f446re.hpp"
int main(){;
	F446re mcu;
	mcu.led.on();
	while(1){
		mcu.console.processLine();
	}
}
