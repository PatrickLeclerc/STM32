#include "f407g.hpp"  // STM32F4xx device header file
F407G mcu;

void hu_command(){
    mcu.console.com.print("huhuhu\r\n");
}

int main(void) {
    /* Add top-level commands to console */
    mcu.console.commands.push_back(Command("hu", "Random command", hu_command));

    /* Print help and reset terminal */
    mcu.console.cls_command();

    std::string msg = "";
    msg += mcu.console.RED.c_str();
    msg += "Starting app\r\n";
    msg += mcu.console.RST.c_str();
    mcu.console.com.print(msg.c_str());

    mcu.console.help_command();
    
    while (1) {
        mcu.console.processLine();
    }
    return 0;
}
