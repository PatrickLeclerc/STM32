#include "app.hpp"
#include "gpio.hpp"
#include "usart.hpp"
#include "dma.hpp"

#include <vector>

#define CONSOLE_RX_BUFF_SIZE 32
#define CONSOLE_TX_BUFF_SIZE 64

extern volatile int console_tx_busy;
extern volatile int console_line_rdy;
extern volatile char console_rx_buff[CONSOLE_RX_BUFF_SIZE];
extern volatile char console_tx_buff[CONSOLE_TX_BUFF_SIZE];

extern "C" void DMA1_Channel7_IRQHandler();
extern "C" void USART2_IRQHandler();

class Comport : public App
{
private:
    std::vector<GPIO> gpio;
    USART usart;
    DMA dma_tx;

public:
    const char * rx = (const char*)(console_rx_buff);
    char * tx = (char*)(console_tx_buff);
    Comport() : Comport(115200) {};
    Comport(uint32_t baud);
    ~Comport();
    void init();
    void print(const char *format, ...);
};

