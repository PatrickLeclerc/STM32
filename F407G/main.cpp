#include "f407g.hpp"  // STM32F4xx device header file

F407G mcu;

int main(){
    /* Print help and reset terminal */
    mcu.console.cls_command();

    mcu.console.com.print("%s\r\n",mcu.fs.logs.c_str());
    mcu.console.help_command();

    
   
    /* Console */
    while (1) {
        mcu.console.processLine();
    }

}
