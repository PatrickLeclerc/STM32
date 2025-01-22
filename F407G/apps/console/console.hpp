#include "app.hpp"
#include "rtc.hpp"

#include "command.hpp"
#include "comport.hpp"

#include <vector>
#include <stdarg.h>

// Interrupt related
class Console : public App
{
private:
    volatile const char * rx = (volatile const char*)(comport_rx_buff);
    RTCLOCK rtc;
    
    // Commands
    void banner();
    std::vector<Command> commands;
    std::vector<std::string> convert_args();
    void assemble_commands();
    void help_command();
    void cls_command();
    void time_command();
    void echo_command();

public:
    Comport com;
    Console() : Console(115200) {};
    Console(uint32_t baud);
    void init();
    void deinit(){};
    void processLine();
};

