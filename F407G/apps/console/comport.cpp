#include "comport.hpp"

#include <cstring>
#include <stdio.h>

volatile int comport_tx_busy = 0;
volatile int comport_line_rdy = 0;
volatile char comport_rx_buff[COMPORT_RX_BUFF_SIZE] = {0};

Comport::Comport(uint32_t baud)
{
	USART_CFG_t u2 = {
        .regs = 0,
        .n = 2,
        .br = baud,
        .rxe = 1,
        .txe = 1,
        .rxie = 1,
        .txie = 0,
        .dmaRxE = 0,
        .dmaTxE = 1
    };
    usart = USART(u2);
    GPIO_CFG_t u2_gpio = {
        .regs = 0,
        .port = 'A',
        .pins = 0x4 | 0x8, // rx | tx
        .speed = GPIO_SPEED_4_8MHz,
        .mode = GPIO_MODE_AF,
        .af = 0x7
    };
	gpio = GPIO(u2_gpio);
    
    DMA_CFG_t u2_dma_tx ={
        .regs       = 0,
		.n			= 1,
		.stream		= 6,
		.ch			= 4,
		.m0ar		= 0,
		.m1ar		= 0,
		.par		= (uint32_t)&(USART2->DR),
		.ndtr		= 0,
		.pl			= DMA_PL_LOW,
		.msize		= DMA_SIZE_B,
		.psize		= DMA_SIZE_B,
		.minc		= 1,
		.pinc		= 0,
		.dbm		= 0,
		.circ		= 0,
		.dir		= DMA_DIR_M2P,
		.tcie		= 1,
		.htie		= 0,
		.en			= 0};
    dma_tx = DMA(u2_dma_tx);
    
}

void Comport::init(){
    gpio.init();
    dma_tx.init();
    usart.init();
}

void Comport::print(const char *format, ...){
	//Wait for last TX to complete
	while(comport_tx_busy){}
	comport_tx_busy = 1;

    // Format the thing
    static char buffer[2048];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);

	//Write
	dma_tx.cfg.regs->CR &= ~DMA_SxCR_EN;
	while(dma_tx.cfg.regs->CR & DMA_SxCR_EN){}
	dma_tx.cfg.regs->M0AR = (uint32_t) buffer;
	dma_tx.cfg.regs->PAR = (uint32_t)&(usart.cfg.regs->DR);
	dma_tx.cfg.regs->NDTR = (uint32_t) len;
	dma_tx.cfg.regs->CR |= DMA_SxCR_EN;
    va_end(args);
}

void DMA1_Stream6_IRQHandler(){
	if(DMA1->HISR & DMA_HISR_TCIF6){
		DMA1->HIFCR = DMA_HIFCR_CTCIF6;
		comport_tx_busy = 0;
	}}

// put this up
void USART2_IRQHandler(){
    static int i = 0;
	if(USART2->SR & USART_SR_RXNE){
        char new_char = (char)USART2->DR;
        if((new_char == '\r') || (new_char == '\r')){
            comport_rx_buff[i] = '\0';
            i = 0;
            comport_line_rdy = 1;
        }
        else if ((new_char == 0x8) || (new_char == 0x7F)) { // backspace or whatever picocom sends
            if(--i < 0) i = 0;
            comport_rx_buff[i] = '\0';
        }
        else {
            comport_rx_buff[i] = new_char;
            if(++i >= COMPORT_RX_BUFF_SIZE){
                i = 0;
                comport_line_rdy = -1;
            }
        }
		USART2->SR &= ~USART_SR_RXNE;
	}
}
