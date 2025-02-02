#include "f407g.hpp"
F407G::F407G(){
    rtc.init();
    console.commands.push_back(Command("time"   , "Display RTC time"        , std::bind(&F407G::time_command, this)));
    console.init();
    led.init();
}
F407G::~F407G(){}
void F407G::time_command(){
	char time[9];
    time[8] = '\0';
	rtc.get_bcd(time);
	console.com.print("%s\r\n",time);
}