#include "sd.hpp"

#define CMD0    0x40  // GO_IDLE_STATE
#define CMD8    0x48  // SEND_IF_COND
#define CMD55   0x77  // APP_CMD
#define CMD58   0x7A  // READ_OCR command
#define ACMD41  0x69  // SD_SEND_OP_COND

void DMA1_Channel2_IRQHandler(){
    if(DMA1->ISR & DMA_ISR_TCIF2)
    {
        DMA1->IFCR = DMA_IFCR_CTCIF2;
        spi_rx_flag = 1;
    }
}
void DMA1_Channel3_IRQHandler(){
    if(DMA1->ISR & DMA_ISR_TCIF3)
    {
        DMA1->IFCR = DMA_IFCR_CTCIF3;
        spi_tx_flag = 1;
    }
}

volatile uint32_t spi_rx_flag = 0;
volatile uint32_t spi_tx_flag = 0;
volatile uint8_t rx_buffer[SPI_BUFF_SIZE] = {0};
volatile uint8_t tx_buffer[SPI_BUFF_SIZE] = {0};

SD::SD()
{
    const uint32_t nss  = 1 << 4;
    const uint32_t sclk = 1 << 5;
    const uint32_t miso = 1 << 6;
    const uint32_t mosi = 1 << 7;
    GPIO_CFG_t g = {
        .regs = 0,
        .port = 'A',
        .pins = sclk | miso | mosi,
        .speed = GPIO_SPEED_25_50MHz,
        .mode = GPIO_MODE_AF,
        .af = 5U
    };
    GPIO_CFG_t cs = {
        .regs = 0,
        .port = 'A',
        .pins = nss,
        .speed = GPIO_SPEED_25_50MHz,
        .mode = GPIO_MODE_OUT,
        .af = 0U
    };
    gpio.push_back(g);
    gpio.push_back(cs);

    wait = Wait(17);

    SPI_CFG_t s = {
        .regs = 0,
        .n = 1,
        .br = SPI_CLK_DIV_256, // 281.250 KHz
        .ssm = 1,
        .ssi = 1,
        .master = 1,
        .cpol = 0,
        .cpha = 0,
        .frxth = RXNE_QUARTER_FIFO,
        .ds = DATA_8B,
        .nssp = 0,
        .ssoe = 0,
        .rxie = 0,
        .txie = 0,
        .dmaRxE = 1,//1,
        .dmaTxE = 1,//1,
        .en = 1,
    };
    spi = SPI(s);

    DMA_CFG_t spi_dma_tx ={
        .regs       = 0,
		.n			= 1,
		.ch			= 3,
		.mar		= 0,
		.par		= (uint32_t)&(SPI1->DR),
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
    DMA_CFG_t spi_dma_rx ={
        .regs       = 0,
		.n			= 1,
		.ch			= 2,
		.mar		= 0,
		.par		= (uint32_t)&(SPI1->DR),
		.ndtr		= 0,
		.pl			= DMA_PL_LOW,
		.msize		= DMA_SIZE_B,
		.psize		= DMA_SIZE_B,
		.minc		= 1,
		.pinc		= 0,
		.dbm		= 0,
		.circ		= 0,
		.dir		= DMA_DIR_P2M,
		.tcie		= 1,
		.htie		= 0,
		.en			= 0};
    dma_tx = DMA(spi_dma_tx);
    dma_rx = DMA(spi_dma_rx);
}

void SD::init(){
    wait.init();
    for(auto& g : gpio)
        g.init();
    dma_rx.init();
    dma_tx.init();
    spi.init();
}
void SD::deinit(){
    spi.deinit();
    dma_rx.deinit();
    dma_tx.deinit();
    for(auto& g : gpio)
        g.deinit();
    wait.deinit();
}

int SD::init_card() {
    const int timeout = 8;
    // Send 80 clock cycles to wake up the SD card
    const uint8_t dummy = 0xFF;
    cs_high();
    for (int i = 0; i < 10; i++) {
        spi.tx(0xFF);
    }
    wait.mswait(10);
    
    // Wait for R1 response (should be 0x01 for idle state)
    int cnt = 0;
    uint32_t res = 0;
    do {
        spi.tx(0xFF);
        cs_low();
        spi.tx(0xFF);
        spi.tx(CMD0);
        spi.tx(0x00);
        spi.tx(0x00);
        spi.tx(0x00);
        spi.tx(0x00);
        spi.tx(0x95);
        res = spi.tx(0xff);
        spi.tx(0xFF);
        cs_high();
        spi.tx(0xFF);
        if(cnt++ > timeout) return 0xdead0000;
    } while(res != 1);

    // Send CMD8 to check voltage range
    cnt = 0;
    cs_low();
    send_cmd(CMD8, 0x1AA); 
    //send_dummy();
    cs_high();
    return tx_buf[5];

    // Check OCR (bytes 1, 2, 3)
    //uint32_t ocr = (rx_buf[1] << 24) | (rx_buf[2] << 16) | (rx_buf[3] << 8) | rx_buf[4];
    //if (ocr != 0x1AA) {
    //    return ocr;  // Error handling, incorrect voltage range
    //}
    // Send ACMD41 until the card is ready
    

    uint32_t rdy_cnt = 0;
    cnt = 0;
    do {
        send_cmd(CMD55, 0); 
        send_dummy();
        if(rx_buf[0] != 0x01) return 0xdead0055;
        send_cmd(ACMD41, 0x40000000);
        send_dummy();
        wait.mswait(100);
        if(cnt > timeout) return 0xdead0A41;
    } while (rx_buf[0] != 0x00);

    return rx_buf[0]; // SD card is initialized
}

void SD::rx_en(uint32_t size){
	while(spi.cfg.regs->SR & SPI_SR_BSY){}
	dma_rx.cfg.regs->CCR &= ~DMA_CCR_EN;
	dma_rx.cfg.regs->CMAR = (uint32_t)rx_buf;
	dma_rx.cfg.regs->CNDTR = size;
	dma_rx.cfg.regs->CCR |= DMA_CCR_EN;
}

void SD::tx(){
	while(spi.cfg.regs->SR & SPI_SR_BSY){}
	dma_tx.cfg.regs->CCR &= ~DMA_CCR_EN;
	dma_tx.cfg.regs->CMAR = (uint32_t)tx_buf;
	dma_tx.cfg.regs->CNDTR = 6U;
	dma_tx.cfg.regs->CCR |= DMA_CCR_EN;
    while(spi_tx_flag == 0){}
    spi_tx_flag = 0;
}
void SD::tx_rx(uint32_t size){
    rx_en(size);
    tx();
    while(spi_tx_flag == 0){}
    spi_tx_flag = 0;
    while(spi_rx_flag == 0){}
    spi_rx_flag = 0;
}
void SD::send_cmd(uint32_t cmdn, uint32_t arg){
    tx_buf[0] = (uint8_t) ((cmdn  & 0x3FU) | 0x40U);
    tx_buf[1] = (uint8_t) ((arg >> 24U) & 0xFFU);
    tx_buf[2] = (uint8_t) ((arg >> 16U) & 0xFFU);
    tx_buf[3] = (uint8_t) ((arg >> 8U ) & 0xFFU);
    tx_buf[4] = (uint8_t) ((arg >> 0U ) & 0xFFU);
    
    // basic
    int resp_size = 0;
    switch (cmdn)
    {
        case 8U:{
            resp_size = 5;
            break;
        }
        case 58U:{
            resp_size = 5;
            break;
        }
        default: {
            resp_size = 1;
        }
    }
    /* Append CRC to last byte */
    sd_crc7();
    
    /* Send command */
    rx_en(resp_size);
    expected_resp_size = resp_size;
    tx();
    
    while(spi_tx_flag == 0){}
    spi_tx_flag = 0;

    while(spi_rx_flag == 0){}
    spi_rx_flag = 0;
}

void SD::send_dummy(){
    /* Dummy command setup*/
    tx_buf[0] = 0xFF;
    tx_buf[1] = 0xFF;
    tx_buf[2] = 0xFF;
    tx_buf[3] = 0xFF;
    tx_buf[4] = 0xFF;
    tx_buf[5] = 0xFF;
    
    /* Send Dummy  */
    rx_en(expected_resp_size);
    tx();
    
    /*Wait for things to be received*/
    while(spi_tx_flag == 0){}
    spi_tx_flag = 0;
    while(spi_rx_flag == 0){}
    spi_rx_flag = 0;
}

void SD::sd_crc7() {
    uint8_t crc = 0;  // Initial CRC value
    uint8_t polynomial = 0x89;  // x^7 + x^3 + 1

    for (int byte = 0; byte < 5; byte++) {  // Only first 5 bytes are used for CRC
        crc ^= tx_buf[byte];  // XOR current byte into CRC
        for (int bit = 0; bit < 8; bit++) {  // Process each bit
            if (crc & 0x80) {  // If MSB is 1
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }
    // Return only the {7-bit CRC  , 1'b1}
    tx_buf[5] = (crc << 1 | 1);
}