#include "f407g.hpp"  // STM32F4xx device header file

int main(void) {
    F407G mcu;
    mcu.comport.print("Hello World!\n");
    while (1) {
       // mcu.console.processLine();
    }

    return 0;
}
