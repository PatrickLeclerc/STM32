#ifndef RTCLOCK_HPP
#define RTCLOCK_HPP
#include "drivers_common.hpp"

class RTCLOCK : public Driver
{
public:
    RTCLOCK(){};
    void init();
    void deinit(){};
    void get_bcd(char* time);
    void get_num(int* time);
};

#endif
