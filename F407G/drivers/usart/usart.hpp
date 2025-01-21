#ifndef USART_HPP
#define USART_HPP
#include "drivers_common.hpp"

typedef struct __attribute__((__packed__)){
	USART_TypeDef *		regs;
	uint32_t 			n;
	uint32_t			br;
	uint32_t			rxe;
	uint32_t			txe;
	uint32_t			rxie;
	uint32_t			txie;
	uint32_t			dmaRxE;
	uint32_t			dmaTxE;
}USART_CFG_t;

class USART : public Driver
{
public:
	USART_CFG_t cfg;
	USART() : Driver(){};
	USART(USART_CFG_t& config) : cfg(config){};
	void init();
	void deinit(){};
};

#endif