#include "app.hpp"
#include "gpio.hpp"
#include "usart.hpp"
#include "dma.hpp"

#include <stdarg.h>

extern "C" void DMA1_Stream6_IRQHandler();
extern "C" void USART2_IRQHandler();

#define COMPORT_RX_BUFF_SIZE 32
extern volatile int comport_line_rdy;
extern volatile char comport_rx_buff[COMPORT_RX_BUFF_SIZE];

class Comport : public App
{
private:
    GPIO gpio;
    USART usart;
    DMA dma_tx;
public:
    Comport() : Comport(115200) {};
    Comport(uint32_t baud);
    void init();
    void deinit(){};
    void print(const char *format, ...);
};

