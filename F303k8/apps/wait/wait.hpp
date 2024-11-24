#ifndef Wait_HPP
#define Wait_HPP

#include "app.hpp"
#include "tim.hpp"

class Wait : public App
{
private:
    TIM tim;
public:
    Wait():Wait(17){};
    Wait(uint32_t tim_n);
    void init();
    void deinit();
    void uwait(uint32_t delay);
    void mswait(uint32_t delay);
};

#endif
