
#include "f446re.hpp"

int main(void) {
	F446re mcu;
    // Initialize SD card
	uint32_t res = mcu.sd.initCard();
	mcu.console.print("SD Initialization logs:\r\n%s", mcu.sd.logs.c_str());
    mcu.sd.logs = "";
    // Test Writes
	mcu.console.print("SD Writes\r\n");
    uint8_t tab_w[512] = {};
    for(int i = 0; i < 512; i++) tab_w[i] = i;
    mcu.sd.writeBlock(0,(uint32_t*)(tab_w),512);
    // Test Reads
	mcu.console.print("SD Reads\r\n");
    uint8_t tab_r[512] = {};
    mcu.sd.readBlock(0,(uint32_t*)(tab_r),512);
    for(int i = 0; i < 512; i++) 
        mcu.console.print("tab[%i]:%i\r\n",i,tab_r[i]);
    while (1) {
        mcu.console.processLine();
    }
}
