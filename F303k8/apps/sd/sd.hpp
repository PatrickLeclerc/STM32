#include "app.hpp"
#include "wait.hpp"
#include "spi.hpp"
#include "gpio.hpp"
#include "dma.hpp"
#include <vector>

// Interrupt related
#define SPI_BUFF_SIZE 64
extern volatile uint32_t spi_rx_flag;
extern volatile uint32_t spi_tx_flag;
extern volatile uint8_t rx_buffer[SPI_BUFF_SIZE];
extern volatile uint8_t tx_buffer[SPI_BUFF_SIZE];
extern "C" void DMA1_Channel2_IRQHandler();
extern "C" void DMA1_Channel3_IRQHandler();

class SD : public App
{
private:
    // Perpiherals
    DMA dma_tx;
    DMA dma_rx;
    std::vector<GPIO> gpio;
    // Commands
    uint32_t expected_resp_size = 0;
    void sd_crc7();
    void send_cmd(uint32_t cmdn, uint32_t arg);
    void send_dummy();
    void toggle_cs();
public:
    uint8_t* tx_buf = (uint8_t*)(tx_buffer);
    const uint8_t* rx_buf = (const uint8_t*)(rx_buffer);
    SPI spi;
    Wait wait;
    // SPI RX / TX
    void tx();
	void rx_en(uint32_t size);
    void tx_rx(uint32_t size);
    // CS
    void cs_high(){
        GPIOA->ODR |=  (1<<4);
        //wait.uwait(10);
    };
    void cs_low() {
        GPIOA->ODR &= ~(1<<4);
        //wait.uwait(10);
    };
    SD();
    void init();
    void deinit();
    int init_card();
};
