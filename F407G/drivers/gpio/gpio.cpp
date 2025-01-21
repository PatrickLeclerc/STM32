#include "gpio.hpp"
GPIO::GPIO(GPIO_CFG_t& config) : cfg(config){}
GPIO::~GPIO()
{
	deinit();
}
void GPIO::init(){
	/* RCC */
	switch(cfg.port){
		case 'A':{
			RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
			cfg.regs = GPIOA;
			break;
		}
		case 'B':{
			RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
			cfg.regs = GPIOB;
			break;
		}
		case 'C':{
			RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
			cfg.regs = GPIOC;
			break;
		}
		case 'D':{
			RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
			cfg.regs = GPIOD;
			break;
		}
		default : return;
	}
	
	/* MODER and OSPEEDR */
	for(int i = 0; i < 16; i++)
		if((cfg.pins>>i)&1U){
			//Clear
			cfg.regs->MODER 	&= ~(3U	 <<(i*2));
			cfg.regs->OSPEEDR	&= ~(3U  <<(i*2));
			cfg.regs->AFR[i>=8]	&= ~(15U <<((i%8)*4));
			//Set
			cfg.regs->MODER 	|= (uint32_t)cfg.mode  <<(i*2);
			cfg.regs->OSPEEDR 	|= (uint32_t)cfg.speed <<(i*2);
			cfg.regs->AFR[i>=8]	|= cfg.af			   <<((i%8)*4);
		}
	cfg.regs->ODR = 0;
}
