#include "dma.hpp"
void DMA::init(){
	IRQn_Type DMA_IRQn;
	/*RCC*/
	if(cfg.n==1){
		RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
		switch(cfg.stream){
			case 0:{
				cfg.regs = DMA1_Stream0;
				DMA_IRQn = DMA1_Stream0_IRQn;
				break;
			}
			case 1:{
				cfg.regs = DMA1_Stream1;
				DMA_IRQn = DMA1_Stream1_IRQn;
				break;
			}
			case 2:{
				cfg.regs = DMA1_Stream2;
				DMA_IRQn = DMA1_Stream2_IRQn;
				break;
			}
			case 3:{
				cfg.regs = DMA1_Stream3;
				DMA_IRQn = DMA1_Stream3_IRQn;
				break;
			}
			case 4:{
				cfg.regs = DMA1_Stream4;
				DMA_IRQn = DMA1_Stream4_IRQn;
				break;
			}
			case 5:{
				cfg.regs = DMA1_Stream5;
				DMA_IRQn = DMA1_Stream5_IRQn;
				break;
			}
			case 6:{
				cfg.regs = DMA1_Stream6;
				DMA_IRQn = DMA1_Stream6_IRQn;
				break;
			}
			case 7:{
				cfg.regs = DMA1_Stream7;
				DMA_IRQn = DMA1_Stream7_IRQn;
				break;
			}
			default:{
				return;
			}
			
		}
		
	}
		
	else if(cfg.n==2){
		RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;	
		switch(cfg.stream){
			case 0:{
				cfg.regs = DMA2_Stream0;
				DMA_IRQn = DMA2_Stream0_IRQn;
				break;
			}
			case 1:{
				cfg.regs = DMA2_Stream1;
				DMA_IRQn = DMA2_Stream1_IRQn;
				break;
			}
			case 2:{
				cfg.regs = DMA2_Stream2;
				DMA_IRQn = DMA2_Stream2_IRQn;
				break;
			}
			case 3:{
				cfg.regs = DMA2_Stream3;
				DMA_IRQn = DMA2_Stream3_IRQn;
				break;
			}
			case 4:{
				cfg.regs = DMA2_Stream4;
				DMA_IRQn = DMA2_Stream4_IRQn;
				break;
			}
			case 5:{
				cfg.regs = DMA2_Stream5;
				DMA_IRQn = DMA2_Stream5_IRQn;
				break;
			}
			case 6:{
				cfg.regs = DMA2_Stream6;
				DMA_IRQn = DMA2_Stream6_IRQn;
				break;
			}
			case 7:{
				cfg.regs = DMA2_Stream7;
				DMA_IRQn = DMA2_Stream7_IRQn;
				break;
			}
			default:{
				return;
			}
			
		}
	}
	else return;

	/* Stream and Channel */
	cfg.regs->CR = 0U;
	while(cfg.regs->CR != 0U){}
	cfg.regs->CR |= (cfg.ch<<DMA_SxCR_CHSEL_Pos);
	
	/* M0AR, M1AR, PAR NDTR*/
	cfg.regs->M0AR = cfg.m0ar;
	cfg.regs->M1AR = cfg.m1ar;
	cfg.regs->PAR  = cfg.par;
	cfg.regs->NDTR = cfg.ndtr;
		
	/* Size, Priority and Increment */
	cfg.regs->CR |= ((uint32_t)cfg.psize << DMA_SxCR_PSIZE_Pos) | ((uint32_t)cfg.msize << DMA_SxCR_MSIZE_Pos);
	cfg.regs->CR |= DMA_SxCR_PL_Msk;
	cfg.regs->CR |= (cfg.pinc << DMA_SxCR_PINC_Pos) | (cfg.minc << DMA_SxCR_MINC_Pos);
	cfg.regs->FCR = 0U;
	
	/* Direction */
	cfg.regs->CR |= ((uint32_t)cfg.dir << DMA_SxCR_DIR_Pos);
	
	/* CIRC */
	cfg.regs->CR |= (cfg.circ << DMA_SxCR_CIRC_Pos);
	
	/* DBM */
	cfg.regs->CR |= (cfg.dbm << DMA_SxCR_DBM_Pos);
	
	/* IRQ */
	cfg.regs->CR |= (cfg.htie << DMA_SxCR_HTIE_Pos) | (cfg.tcie << DMA_SxCR_TCIE_Pos);
	if(cfg.htie || cfg.tcie)
		NVIC_EnableIRQ(DMA_IRQn);
	
	/* Enable */
	if(cfg.en)
		cfg.regs->CR |= DMA_SxCR_EN;
}
void DMA::reload(){	
	cfg.regs->CR &= ~DMA_SxCR_EN;
	while(cfg.regs->CR & DMA_SxCR_EN){}
	/* M0AR, M1AR, PAR NDTR*/
	cfg.regs->M0AR = cfg.m0ar;
	cfg.regs->M1AR = cfg.m1ar;
	cfg.regs->PAR  = cfg.par;
	cfg.regs->NDTR = cfg.ndtr;
}
void DMA::enable(){	
	cfg.regs->CR |= DMA_SxCR_EN;
}
void DMA::disable(){	
	cfg.regs->CR &= ~DMA_SxCR_EN;
}
