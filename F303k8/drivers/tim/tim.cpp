#include "tim.hpp"

void TIM::init(){
	/* RCC */
	IRQn_Type TIM_IRQn;
	switch(cfg.n){
		//APB1
		case 2:{
			RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
			cfg.regs = TIM2;
			TIM_IRQn = TIM2_IRQn;
			break;
		}
		case 3:{
			RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
			cfg.regs = TIM3;
			TIM_IRQn = TIM3_IRQn;
			break;
		}
		case 6:{
			RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
			cfg.regs = TIM6;
			TIM_IRQn = TIM6_DAC_IRQn;
			break;
		}
		case 7:{
			RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
			cfg.regs = TIM7;
			TIM_IRQn = TIM7_IRQn;
			break;
		}
		//APB2
		case 1:{
			RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
			cfg.regs = TIM1;
			TIM_IRQn = TIM1_UP_TIM16_IRQn;
			break;
		}
		case 17:{
			RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
			cfg.regs = TIM17;
			TIM_IRQn = TIM17_IRQn;
			break;
		}
		// Lots of missing timers here !
		default: return;
	}

	/* TIM */
	cfg.regs->CR1 = TIM_CR1_ARPE;
	if(cfg.urs)
		cfg.regs->CR1 |= TIM_CR1_URS;
	if(cfg.opm)
		cfg.regs->CR1 |= TIM_CR1_OPM;
	cfg.regs->PSC = cfg.psc;
	cfg.regs->ARR = cfg.arr;
	cfg.regs->CNT = 0U;
	
	/* Master */
	cfg.regs->CR1 |= (cfg.mms << TIM_CR2_MMS_Pos) | (cfg.ccds << TIM_CR2_CCDS_Pos);
	
	/* Slave */
	cfg.regs->SMCR |= (cfg.ts << TIM_SMCR_TS_Pos) | (cfg.sms << TIM_SMCR_SMS_Pos);
	
	/* NVIC and DMA */
	if(cfg.uie){
		cfg.regs->DIER |= TIM_DIER_UIE | TIM_DIER_UDE;
		NVIC_EnableIRQ(TIM_IRQn);
	}
	/*Update registers */
	cfg.regs->EGR |= TIM_EGR_UG;

	/* Enable */
	if(cfg.en)
		cfg.regs->CR1 |= TIM_CR1_CEN;
	
	
}