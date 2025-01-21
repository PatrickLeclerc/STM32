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
		case 4:{
			RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
			cfg.regs = TIM4;
			TIM_IRQn = TIM4_IRQn;
			break;
		}
		case 5:{
			RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
			cfg.regs = TIM5;
			TIM_IRQn = TIM5_IRQn;
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
			TIM_IRQn = TIM1_UP_TIM10_IRQn;
			break;
		}
		case 8:{
			RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
			cfg.regs = TIM8;
			TIM_IRQn = TIM8_UP_TIM13_IRQn;
			break;
		}
		default: return;
	}

	/* TIM */
	cfg.regs->CR1 = TIM_CR1_ARPE;
	if(cfg.urs)
		cfg.regs->CR1 |= TIM_CR1_URS;
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
	cfg.regs->CR1 |= TIM_CR1_CEN;
	
	
}