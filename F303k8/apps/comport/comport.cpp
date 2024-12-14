#include "comport.hpp"

#include <string>
#include <cstring>
#include <stdio.h>

#include <vector>
#include <stdarg.h>

volatile int console_tx_busy = 0;
volatile int console_line_rdy = 0;
volatile char console_rx_buff[CONSOLE_RX_BUFF_SIZE] = {0};
volatile char console_tx_buff[CONSOLE_TX_BUFF_SIZE] = {0};


void Comport::init(){
    for(auto& g:gpio)
        g.init();
    dma_tx.init();
    usart.init();
}
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
    GPIO_CFG_t u2_gpio_rx = {
        .regs = 0,
        .port = 'A',
        .pins = 1 << 2,
        .speed = GPIO_SPEED_4_8MHz,
        .mode = GPIO_MODE_AF,
        .af = 0x7
    };
    GPIO_CFG_t u2_gpio_tx = {
        .regs = 0,
        .port = 'A',
        .pins = 1 << 15,
        .speed = GPIO_SPEED_4_8MHz,
        .mode = GPIO_MODE_AF,
        .af = 0x7
    };
	gpio.push_back(GPIO(u2_gpio_rx));
    gpio.push_back(GPIO(u2_gpio_tx));
    
    DMA_CFG_t u2_dma_tx ={
        .regs       = 0,
		.n			= 1,
		.ch			= 7,
		.mar		= 0,
		.par		= (uint32_t)&(USART2->RDR),
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

Comport::~Comport()
{
}

void Comport::print(const char *format, ...){
	//Wait for last TX to complete
	while(console_tx_busy){}
	console_tx_busy = 1;

    // Format the thing
    static char buffer[128];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);

	//Write
	DMA1_Channel7->CCR &= ~DMA_CCR_EN;
	while(DMA1_Channel7->CCR & DMA_CCR_EN){}
	DMA1_Channel7->CMAR = (uint32_t) buffer;
	DMA1_Channel7->CPAR = (uint32_t)&(USART2->TDR);
	DMA1_Channel7->CNDTR = (uint32_t) len;
	DMA1_Channel7->CCR |= DMA_CCR_EN;
    va_end(args);
}


void DMA1_Channel7_IRQHandler(){
	if(DMA1->ISR & DMA_ISR_TCIF7){
		DMA1->IFCR = DMA_IFCR_CTCIF7;
		console_tx_busy = 0;
	}}

void USART2_IRQHandler(){
    static int i = 0;
	if(USART2->ISR & USART_ISR_RXNE){
        char new_char = (char)USART2->RDR;
        if((new_char == '\r') || (new_char == '\r')){
            console_rx_buff[i] = '\0';
            i = 0;
            console_line_rdy = 1;
        }
        else if ((new_char == 0x8) || (new_char == 0x7F)) { // backspace or whatever picocom sends
            if(--i < 0) i = 0;
            console_rx_buff[i] = '\0';
        }
        else {
            console_rx_buff[i] = new_char;
            if(++i >= CONSOLE_RX_BUFF_SIZE){
                i = 0;
                console_line_rdy = -1;
            }
        }
		USART2->ISR &= ~USART_ISR_RXNE;
	}
}

