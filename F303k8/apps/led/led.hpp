#ifndef LED_HPP
#define LED_HPP
#include "app.hpp"
#include "gpio.hpp"
class LED : public App
{
private:
    GPIO gpio;
public:
    LED();
    void init();
    void deinit();
    void on();
    void off();
};
#endif
