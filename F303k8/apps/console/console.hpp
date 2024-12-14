#include "app.hpp"
#include "rtc.hpp"
#include "gpio.hpp"
#include "usart.hpp"
#include "dma.hpp"
#include "command.hpp"
#include "comport.hpp"
#include "sd.hpp"

#include <vector>


class Console : public App
{
private:
    // Peripherals
    Comport com;
    RTCLOCK rtc;

    
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
    void processLine();
};

