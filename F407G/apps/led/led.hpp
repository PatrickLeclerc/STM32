#ifndef LED_HPP
#define LED_HPP
#include "app.hpp"
#include "gpio.hpp"

typedef enum LED_COLOR{
	LED_R = (1<<14), // pin14 on GPIOD 
	LED_G = (1<<12), // pin12 on GPIOD 
	LED_B = (1<<15), // pin15 on GPIOD 
	LED_O = (1<<13)  // pin13 on GPIOD 
}LED_COLOR_t;

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
    void on(LED_COLOR_t pins);
    void off(LED_COLOR_t pins);
};
#endif
