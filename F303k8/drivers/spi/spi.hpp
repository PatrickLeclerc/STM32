#ifndef SPI_HPP
#define SPI_HPP
#include "drivers_common.hpp"

typedef enum SPI_CLK_DIV{
	SPI_CLK_DIV_2 = 0,
	SPI_CLK_DIV_4,
	SPI_CLK_DIV_8,
	SPI_CLK_DIV_16,
	SPI_CLK_DIV_32,
	SPI_CLK_DIV_64,
	SPI_CLK_DIV_128,
	SPI_CLK_DIV_256
}SPI_CLK_DIV_t;

typedef enum SPI_FRXTH{
	RXNE_HALF_FIFO = 0,
	RXNE_QUARTER_FIFO
}SPI_FRXTH_t;

typedef enum SPI_DATA_SIZE{
	DATA_4B = 0,
	DATA_5B,
	DATA_6B,
	DATA_7B,
	DATA_8B,
	DATA_9B,
	DATA_10B,
	DATA_11B,
	DATA_12B,
	DATA_13B,
	DATA_14B,
	DATA_15B,
	DATA_16B
}SPI_DATA_SIZE_t;

typedef struct __attribute__((__packed__)){
    SPI_TypeDef *		regs;
	uint32_t 			n;
	SPI_CLK_DIV_t		br;
	uint32_t			ssm;
	uint32_t			ssi;
	uint32_t			master;
	uint32_t			cpol;
	uint32_t			cpha;
    SPI_FRXTH_t         frxth;
    SPI_DATA_SIZE_t     ds;   
	uint32_t			nssp;
	uint32_t			ssoe;
	uint32_t			rxie;
	uint32_t			txie;
	uint32_t			dmaRxE;
	uint32_t			dmaTxE;
    uint32_t            en;
}SPI_CFG_t;

class SPI : public Driver
{
private:
    /* data */
public:
    SPI_CFG_t cfg;
    SPI() : Driver(){};
    SPI(SPI_CFG_t config) : cfg(config){};
    void init();
    uint32_t tx(uint32_t data);
};


#endif
