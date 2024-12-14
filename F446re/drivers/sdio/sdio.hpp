#ifndef SDIO_HPP
#define SDIO_HPP
#include "drivers_common.hpp"
#include "gpio.hpp"
#include <vector>

// Response type definitions for the SDIO CMD register
#define SDIO_CMD_WAITRESP_NO       (0x0 << 6) // No response
#define SDIO_CMD_WAITRESP_SHORT    (0x1 << 6) // Short response
#define SDIO_CMD_WAITRESP_LONG     (0x2 << 6) // Long response

/*enums*/
typedef enum SD_WIDBUS{
	SD_WIDBUS_1 = 0,
	SD_WIDBUS_4 = 1,
	SD_WIDBUS_8 = 2
}SD_WIDBUS_t;

typedef struct __attribute__((__packed__)){
	SDIO_TypeDef *		regs;
	SD_WIDBUS_t         wid;
	uint32_t 			clk_div_slow;
	uint32_t 			clk_div_fast;
}SD_CFG_t;

class SD : public Driver
{
private:
	std::vector<GPIO> gpio;
public:
	SD_CFG_t cfg;
	SD();
	void init();
	void deinit(){};
	void sendCommand(uint32_t cmdIndex, uint32_t argument, uint32_t responseType);
	uint32_t getResponse(uint32_t responseReg);

	void CMD0(void);
	uint8_t CMD8(void);
	uint8_t ACMD41(void);
};

#endif
