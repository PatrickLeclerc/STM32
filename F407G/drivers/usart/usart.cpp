#include "usart.hpp"
void USART::init()
{
	IRQn_Type USART_IRQn;
	uint32_t apbn;
	/* RCC */
	switch(cfg.n){
			case 1:{
				RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
				cfg.regs = USART1;
				USART_IRQn = USART1_IRQn;
				apbn = 2;
				break;
			}
			case 2:{
				RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
				cfg.regs = USART2;
				USART_IRQn = USART2_IRQn;
				apbn = 1;
				break;
			}
			case 3:{
				RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
				cfg.regs = USART3;
				USART_IRQn = USART3_IRQn;
				apbn = 1;
				break;
			}
			case 4:{
				RCC->APB1ENR |= RCC_APB1ENR_UART4EN;
				cfg.regs = UART4;
				USART_IRQn = UART4_IRQn;
				apbn = 1;
				break;
			}
			case 5:{
				RCC->APB1ENR |= RCC_APB1ENR_UART5EN;
				cfg.regs = UART5;
				USART_IRQn = UART5_IRQn;
				apbn = 1;
				break;
			}
			case 6:{
				RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
				cfg.regs = USART6;
				USART_IRQn = USART6_IRQn;
				apbn = 2;
				break;
			}
			default:{
				return;
			}
	}
	//DMA
	if(cfg.dmaTxE)
		cfg.regs->CR3 |= USART_CR3_DMAT;
	if(cfg.dmaRxE)
		cfg.regs->CR3 |= USART_CR3_DMAR;
	
	/* Baudrate */
	if(apbn == 1)
		cfg.regs->BRR = 8 * SystemCoreClock / (16 * 2 * cfg.br);
	else 
		cfg.regs->BRR = 8 * SystemCoreClock / (16 * 4 * cfg.br);
	
	/* IRQ */
	cfg.regs->CR1 |= (cfg.rxie << USART_CR1_RXNEIE_Pos) | (cfg.txie << USART_CR1_TXEIE_Pos);
	if(cfg.rxie || cfg.txie)
		NVIC_EnableIRQ(USART_IRQn);
	
	/* Enable */
	cfg.regs->CR1 |= (cfg.rxe << USART_CR1_RE_Pos) | (cfg.txe << USART_CR1_TE_Pos);
	cfg.regs->CR1 |= USART_CR1_UE;
}
