#include "clock.hpp"

Clock::Clock()
{
	// cpu 72MHz
	FLASH->ACR |= FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_1;
	// APB1 (low)		: 36MHz
	// APB2 (high)	: 72MHz
	RCC->CR |= (RCC_CR_HSEBYP | RCC_CR_HSEON);
	while(!(RCC->CR & RCC_CR_HSERDY)){}
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
	//HSE->PLL : 72 MHz
	RCC->CFGR |= (RCC_CFGR_PLLMUL12);	
	RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV;		
	RCC->CR |= RCC_CR_PLLON;		
	while(!(RCC->CR & RCC_CR_PLLRDY));
	RCC->CFGR |= RCC_CFGR_SW_PLL;	
	SystemCoreClockUpdate();
}
Clock::~Clock(){}
