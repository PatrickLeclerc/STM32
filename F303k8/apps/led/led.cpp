#include "led.hpp"
LED::LED()
{
    GPIO_CFG_t led = {
        .regs = 0,
	    .port = 'A',
	    .pins = 1<<5,
	    .speed = GPIO_SPEED_4_8MHz,
	    .mode = GPIO_MODE_OUT,
	    .af = 0
    };
    gpio = GPIO(led);
}

void LED::init(){
    gpio.init();
}
void LED::deinit(){
    gpio.deinit();
}
void LED::on(){
    gpio.cfg.regs->ODR |= gpio.cfg.pins;
}
void LED::off(){
    gpio.cfg.regs->ODR &= ~gpio.cfg.pins;
}