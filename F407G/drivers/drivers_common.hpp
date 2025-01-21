#ifndef DRIVERS_COMMON_HPP
#define DRIVERS_COMMON_HPP
extern "C"{
#include "stm32f407xx.h"
}
#include <stdint.h>
class Driver
{
public:
    Driver(){};
    ~Driver(){deinit();};
    void init(){};
    void deinit(){};
};

#endif