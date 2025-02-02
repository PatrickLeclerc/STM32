#ifndef F407G_HPP
#define F407G_HPP

#include "clock.hpp"
#include "led.hpp"
#include "console.hpp"

class F407G
{
private:
    Clock clock;
public:
    F407G();
    ~F407G();
    void time_command();
    RTCLOCK rtc;
    LED led;
    Console console;
};

#endif