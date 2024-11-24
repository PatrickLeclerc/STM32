#ifndef CLOCK_HPP
#define CLOCK_HPP
#include "drivers_common.hpp"

class Clock : public Driver
{
private:
    
public:
    Clock() : Clock(180U){};
    Clock(uint32_t core_clk);
    ~Clock();
};

#endif