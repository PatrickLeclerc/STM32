#include "led.hpp"

LED::LED()
{
    GPIO_CFG_t led = {
        .regs = 0,
	    .port = 'D',
	    .pins =  LED_R|LED_G|LED_B|LED_O,
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
    gpio.cfg.regs->BSRR |= gpio.cfg.pins;
}
void LED::on(LED_COLOR_t pins){
    gpio.cfg.regs->BSRR |= pins;
}

void LED::off(){
    gpio.cfg.regs->ODR &= ~gpio.cfg.pins;
}
void LED::off(LED_COLOR_t pins){
    gpio.cfg.regs->ODR &= ~pins;
}