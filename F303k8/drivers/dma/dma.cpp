#include "dma.hpp"
void DMA::init(){
	IRQn_Type DMA_IRQn;
	/*RCC*/
	if(cfg.n==1){
		RCC->AHBENR |= RCC_AHBENR_DMA1EN;
		switch(cfg.ch){
			case 0:{
				cfg.regs = DMA1_Channel1;
				DMA_IRQn = DMA1_Channel1_IRQn;
				break;
			}
			case 1:{
				cfg.regs = DMA1_Channel1;
				DMA_IRQn = DMA1_Channel1_IRQn;
				break;
			}
			case 2:{
				cfg.regs = DMA1_Channel2;
				DMA_IRQn = DMA1_Channel2_IRQn;
				break;
			}
			case 3:{
				cfg.regs = DMA1_Channel3;
				DMA_IRQn = DMA1_Channel3_IRQn;
				break;
			}
			case 4:{
				cfg.regs = DMA1_Channel4;
				DMA_IRQn = DMA1_Channel4_IRQn;
				break;
			}
			case 5:{
				cfg.regs = DMA1_Channel5;
				DMA_IRQn = DMA1_Channel5_IRQn;
				break;
			}
			case 6:{
				cfg.regs = DMA1_Channel6;
				DMA_IRQn = DMA1_Channel6_IRQn;
				break;
			}
			case 7:{
				cfg.regs = DMA1_Channel7;
				DMA_IRQn = DMA1_Channel7_IRQn;
				break;
			}
			default:{
				return;
			}
			
		}
		
	}
	else return;

	/* Stream and Channel */
	cfg.regs->CCR = 0U;
	while(cfg.regs->CCR != 0U){}
	
	/* M0AR, M1AR, PAR NDTR*/
	cfg.regs->CMAR = cfg.mar;
	cfg.regs->CPAR  = cfg.par;
	cfg.regs->CNDTR = cfg.ndtr;
		
	/* Size, Priority and Increment */
	cfg.regs->CCR |= ((uint32_t)cfg.psize << DMA_CCR_PSIZE_Pos) | ((uint32_t)cfg.msize << DMA_CCR_MSIZE_Pos);
	cfg.regs->CCR |= DMA_CCR_PL_Msk;
	cfg.regs->CCR |= (cfg.pinc << DMA_CCR_PINC_Pos) | (cfg.minc << DMA_CCR_MINC_Pos);
	
	/* Direction */
	cfg.regs->CCR |= ((uint32_t)cfg.dir << DMA_CCR_DIR_Pos);
	
	/* CIRC */
	cfg.regs->CCR |= (cfg.circ << DMA_CCR_CIRC_Pos);
	
	/* IRQ */
	cfg.regs->CCR |= (cfg.htie << DMA_CCR_HTIE_Pos) | (cfg.tcie << DMA_CCR_TCIE_Pos);
	if(cfg.htie || cfg.tcie)
		NVIC_EnableIRQ(DMA_IRQn);
	
	/* Enable */
	if(cfg.en)
		cfg.regs->CCR |= DMA_CCR_EN;
}
void DMA::reload(){	
	cfg.regs->CCR &= ~DMA_CCR_EN;
	while(cfg.regs->CCR & DMA_CCR_EN){}
	/* M0AR, M1AR, PAR NDTR*/
	cfg.regs->CMAR = cfg.mar;
	cfg.regs->CPAR  = cfg.par;
	cfg.regs->CNDTR = cfg.ndtr;
}
void DMA::enable(){	
	cfg.regs->CCR |= DMA_CCR_EN;
}
void DMA::disable(){	
	cfg.regs->CCR &= ~DMA_CCR_EN;
}
