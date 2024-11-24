#include "spi.hpp"

void SPI::init()
{
    IRQn_Type SPI_IRQn;
	/* RCC */
	switch(cfg.n){
			case 1:{
				RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
				cfg.regs = SPI1;
				SPI_IRQn = SPI1_IRQn;
				break;
			}
			default:{
				return;
			}
	}

    // CR1
	cfg.regs->CR1 =  cfg.br     << SPI_CR1_BR_Pos;
	cfg.regs->CR1 |= cfg.master << SPI_CR1_MSTR_Pos;
	cfg.regs->CR1 |= cfg.ssm    << SPI_CR1_SSM_Pos;
	cfg.regs->CR1 |= cfg.ssi    << SPI_CR1_SSI_Pos;
	cfg.regs->CR1 |= cfg.cpha   << SPI_CR1_CPHA_Pos;
	cfg.regs->CR1 |= cfg.cpol   << SPI_CR1_CPOL_Pos;

    // CR2
	cfg.regs->CR2 =  cfg.frxth   << SPI_CR2_FRXTH_Pos;
	cfg.regs->CR2 |= cfg.ds      << SPI_CR2_DS_Pos;
	cfg.regs->CR2 |= cfg.nssp    << SPI_CR2_NSSP_Pos;
	cfg.regs->CR2 |= cfg.ssoe    << SPI_CR2_SSOE_Pos;
	cfg.regs->CR2 |= cfg.dmaTxE  << SPI_CR2_TXDMAEN_Pos;
	cfg.regs->CR2 |= cfg.dmaRxE  << SPI_CR2_RXDMAEN_Pos;
	
    // IRQ
	cfg.regs->CR2 |= cfg.rxie    << SPI_CR2_RXNEIE_Pos;
	cfg.regs->CR2 |= cfg.txie    << SPI_CR2_TXEIE_Pos;
    if(cfg.rxie || cfg.txie)
        NVIC_EnableIRQ(SPI_IRQn);
    
    // Enable
	cfg.regs->CR1 |= cfg.en << SPI_CR1_SPE_Pos;
}

uint32_t SPI::tx(uint32_t data){
    while (!(cfg.regs->SR & SPI_SR_TXE));
    cfg.regs->DR = data;
    while (!(cfg.regs->SR & SPI_SR_RXNE));
    return cfg.regs->DR;
}
