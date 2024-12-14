#include "sdio.hpp"

SD::SD(){
	GPIO_CFG_t cmd = {
        .regs = 0,
        .port = 'D',
        .pins = (1<<2),
        .speed = GPIO_SPEED_25_50MHz,
        .mode = GPIO_MODE_AF,
        .af = 12
    };
	GPIO_CFG_t d0_d3 = {
        .regs = 0,
        .port = 'C',
        .pins = (1<<8) | (1<<11),
        .speed = GPIO_SPEED_25_50MHz,
        .mode = GPIO_MODE_AF,
        .af = 12
    };
	GPIO_CFG_t d1_d2_clk = {
        .regs = 0,
        .port = 'B',
        .pins = (1<<0) | (1<<1) | (1<<2),
        .speed = GPIO_SPEED_25_50MHz,
        .mode = GPIO_MODE_AF,
        .af = 12
    };
	gpio.push_back(GPIO(cmd));
	gpio.push_back(GPIO(d0_d3));
	gpio.push_back(GPIO(d1_d2_clk));
	// SDIO Specifics
	cfg.regs = SDIO;
    cfg.wid = SD_WIDBUS_4;
    cfg.clk_div_slow = (SystemCoreClock / 400000  ) - 2; // 400KHz = SYS_CLK / [CLKDIV + 2].
    cfg.clk_div_fast = (SystemCoreClock / 10000000) - 2; // 10MHz = SYS_CLK / [CLKDIV + 2].
}

void SD::init(){
	// Init GPIO
	for(auto &g : gpio) g.init();
	// Init RCC
	RCC->APB2ENR |= RCC_APB2ENR_SDIOEN;
    RCC->DCKCFGR2 |= RCC_DCKCFGR2_SDIOSEL; // Use SYS_CLK 
	// Power On
	cfg.regs->POWER |= SDIO_POWER_PWRCTRL_Msk;
    // Config
    cfg.regs->CLKCR = cfg.wid << SDIO_CLKCR_WIDBUS_Pos;
    //cfg.regs->CLKCR |= cfg.clk_div_slow << SDIO_CLKCR_CLKDIV_Pos;
    cfg.regs->CLKCR |= SDIO_CLKCR_CLKDIV;
    cfg.regs->CLKCR |= SDIO_CLKCR_CLKEN;
    for (volatile int i = 0; i < 100000; i++); // Wait for clock to stabilize
}

void SD::sendCommand(uint32_t cmdIndex, uint32_t argument, uint32_t responseType) {
    // Clear previous command flags
    SDIO->ICR = 0xFFFFFFFF;

    // Set the command argument
    SDIO->ARG = argument;

    // Configure the command register
    SDIO->CMD = (cmdIndex & SDIO_CMD_CMDINDEX) | (responseType & SDIO_CMD_WAITRESP);

    // Wait for command to complete
    while (!(SDIO->STA & (SDIO_STA_CMDSENT | SDIO_STA_CMDREND | SDIO_STA_CTIMEOUT)));

    // Handle errors if needed
    if (SDIO->STA & SDIO_STA_CTIMEOUT) {
        // Timeout error
        // You can return an error code or handle it as needed
    }
    if (SDIO->STA & SDIO_STA_CCRCFAIL) {
        // CRC error
        // You can return an error code or handle it as needed
    }
}
uint32_t SD::getResponse(uint32_t responseReg) {
    // Return the specified response register value
    switch (responseReg) {
        case 1: return SDIO->RESP1;
        case 2: return SDIO->RESP2;
        case 3: return SDIO->RESP3;
        case 4: return SDIO->RESP4;
        default: return 0;
    }
}

void SD::CMD0(void) {
    sendCommand(0, 0x00000000, SDIO_CMD_WAITRESP_NO); // No response expected
}

uint8_t SD::CMD8(void) {
    sendCommand(8, 0x000001AA, SDIO_CMD_WAITRESP_SHORT); // Argument: 0x1AA (Voltage 2.7-3.6V, Check pattern)

    // Check response
    if (SDIO->STA & SDIO_STA_CTIMEOUT) {
        return 0; // Timeout: Possibly an old card (not SDHC/SDXC)
    }

    uint32_t response = getResponse(1); // Read response register RESP1
    if ((response & 0xFFF) != 0x1AA) {
        return 0; // Invalid response
    }

    return 1; // CMD8 successful, SDHC/SDXC card detected
}

uint8_t SD::ACMD41(void) {
    uint32_t response;

    // Loop until the card is ready
    do {
        // Send CMD55 (APP_CMD)
        sendCommand(55, 0x00000000, SDIO_CMD_WAITRESP_SHORT);
        if (SDIO->STA & SDIO_STA_CTIMEOUT) {
            return 0; // Timeout error
        }

        // Send ACMD41
        sendCommand(41, 0x40000000, SDIO_CMD_WAITRESP_SHORT); // Argument: HCS (Host Capacity Support)
        if (SDIO->STA & SDIO_STA_CTIMEOUT) {
            return 0; // Timeout error
        }

        response = getResponse(1); // Read response register RESP1
    } while (!(response & 0x80000000)); // Wait for the card to indicate it is ready (bit 31 set)

    return 1; // ACMD41 successful
}
