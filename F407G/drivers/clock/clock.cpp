#include "clock.hpp"

Clock::Clock(uint32_t core_clk)
{
	/*APB1 45MHz Max*/
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;
	/*APB2 90MHz Max*/
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;
	/*HSE*/
	RCC->CR = (RCC->CR & ~RCC_CR_HSEBYP) | RCC_CR_HSEON; 
	while(!(RCC->CR & RCC_CR_HSERDY)){}
	/*PLLSRC*/
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE; // 8 MHz
	/*PLLM*/
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM_Msk;
	RCC->PLLCFGR |= (4U<<RCC_PLLCFGR_PLLM_Pos); // VCO_IN = 8 MHz / 4 = 2MHz
	/*PLLN*/
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN_Msk;
	RCC->PLLCFGR |= (core_clk<<RCC_PLLCFGR_PLLN_Pos); // VCO_OUT = 2MHz * 180 = 360 MHz
	/*PLLP*/
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP_Msk; // PLLCLK = 360 MHz / 2 = 180 MHz 
	/*PLLQ*/
	RCC->PLLCFGR |= (8U<<RCC_PLLCFGR_PLLQ_Pos); // SDIO_CLK = 360 MHz / 8 = 45 MHz
	/*FLASHMEM*/
	FLASH->ACR |= FLASH_ACR_PRFTEN|FLASH_ACR_LATENCY_5WS;
	/*Activate PLL*/
	RCC->CR |= RCC_CR_PLLON;
	while(!(RCC->CR & RCC_CR_PLLRDY)){}
	/*Switch clock source to PLL*/
	RCC->CFGR |= RCC_CFGR_SW_PLL;

	SystemCoreClockUpdate();
}
Clock::~Clock(){}
