#ifndef DMA_HPP
#define DMA_HPP
#include "drivers_common.hpp"
/* Enums */
typedef enum DMA_SIZE{
	DMA_SIZE_B = 0,
	DMA_SIZE_HW,
	DMA_SIZE_W
}DMA_SIZE_t;
typedef enum DMA_DIR{
	DMA_DIR_P2M = 0,	//PAR ->MXAR
	DMA_DIR_M2P,		//MXAR->PAR
	DMA_DIR_M2M			//PAR ->M0AR
}DMA_DIR_t;
typedef enum DMA_PL{
	DMA_PL_LOW = 0,
	DMA_PL_MEDIUM,
	DMA_PL_HIGH,
	DMA_PL_VERYHIGH
}DMA_PL_t;

/* Structs */
typedef struct __attribute__((__packed__)){
	DMA_Channel_TypeDef* regs;
	uint32_t 			n;
	uint32_t 			ch;
	uint32_t 			mar;
	uint32_t 			par;
	uint32_t 			ndtr;
	DMA_PL_t 			pl;
	DMA_SIZE_t 			msize;
	DMA_SIZE_t 			psize;
	uint32_t 			minc;
	uint32_t 			pinc;
	uint32_t 			dbm;
	uint32_t 			circ;
	DMA_DIR_t 			dir;
	uint32_t 			tcie;
	uint32_t 			htie;
	uint32_t			en;
}DMA_CFG_t;

class DMA : public Driver
{
public:
	DMA_CFG_t cfg;
	DMA() : Driver(){};
	DMA(DMA_CFG_t config) : cfg(config){};
	void init();
	void deinit(){};
	void reload();
	void enable();
	void disable();
};

#endif
