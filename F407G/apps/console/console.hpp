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
public:
    Console();
    // Special characters
    const std::string CLS = "\033[2J";
    // Colors
    const std::string RST = "\e[37m";
    const std::string RED = "\e[31m";
    const std::string GRN = "\e[32m";
    const std::string YEL = "\e[33m";
    const std::string BLU = "\e[34m";
    const std::string MAG = "\e[35m";
    const std::string CYN = "\e[36m";
    const std::string WHT = "\e[37m";

    Comport com;
    std::vector<Command> commands;
    
    std::vector<std::string> convert_args();

    void banner();
    void help_command();
    void cls_command();
    void echo_command();

    void init();
    void deinit(){};
    void processLine();
};

