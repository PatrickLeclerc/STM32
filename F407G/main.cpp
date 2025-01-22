#include "f407g.hpp"  // STM32F4xx device header file

int main(void) {
    F407G mcu;
    while (1) {
       mcu.console.processLine();
    }

    return 0;
}
