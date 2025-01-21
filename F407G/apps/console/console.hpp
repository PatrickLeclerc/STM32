#include "app.hpp"
#include "rtc.hpp"
#include "gpio.hpp"
#include "usart.hpp"
#include "dma.hpp"
#include "command.hpp"

#include <vector>
#include <stdarg.h>

// Interrupt related
#define CONSOLE_RX_BUFF_SIZE 32
extern volatile int console_tx_busy;
extern volatile int console_line_rdy;
extern volatile char console_rx_buff[CONSOLE_RX_BUFF_SIZE];
extern "C" void DMA1_Stream6_IRQHandler();
extern "C" void USART2_IRQHandler();

class Console : public App
{
private:
    // Peripherals
    GPIO gpio;
    RTCLOCK rtc;
    USART usart;
    DMA dma_tx;

    const char * rx = (const char*)(console_rx_buff);
    void banner();
    // Commands
    std::vector<Command> commands;
    std::vector<std::string> convert_args();
    void assemble_commands();
    void help_command();
    void time_command();
    void echo_command();
    void cls_command();

public:
    Console() : Console(115200) {};
    Console(uint32_t baud);
    void init();
    void deinit(){};
    void print(const char *format, ...);
    void processLine();
};

