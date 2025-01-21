#include "f407g.hpp"
#include <string>
F407G::F407G(){
    std::string logs = "";
    logs += "RCC initialized\r\n";
    console.init();
    logs += "Console initialized\r\n";
    led.init();
    logs += "Led initialized\r\n";
    console.print("%s",logs.c_str());
}
F407G::~F407G(){}
