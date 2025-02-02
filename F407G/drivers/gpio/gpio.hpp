#ifndef GPIO_HPP
#define GPIO_HPP
#include "drivers_common.hpp"

/*enums*/
typedef enum GPIO_SPEED{
	GPIO_SPEED_4_8MHz = 0,
	GPIO_SPEED_25_50MHz,
	GPIO_SPEED_50_100MHz,
	GPIO_SPEED_100_180MHz
}GPIO_SPEED_t;

typedef enum GPIO_MODE{
	GPIO_MODE_IN = 0,
	GPIO_MODE_OUT,
	GPIO_MODE_AF,
	GPIO_MODE_AN
}GPIO_MODE_t;

/*Structs*/
typedef struct __attribute__((__packed__)) {
	GPIO_TypeDef* 		regs;
	char 				port;
	uint32_t			pins;
	GPIO_SPEED_t		speed;
	GPIO_MODE_t			mode;
	uint32_t			af;
}GPIO_CFG_t;

class GPIO : public Driver
{
public:
	GPIO_CFG_t cfg;
	GPIO() : Driver(){};
	GPIO(GPIO_CFG_t& config);
	~GPIO();
	void init();
	void deinit(){};
};


#endif