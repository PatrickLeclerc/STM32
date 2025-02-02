#include "console.hpp"

#include <cstring>
#include <stdio.h>

Console::Console()
{
    commands.push_back(Command("help"   , "Displays this message" , std::bind(&Console::help_command  , this)));
    commands.push_back(Command("cls"    , "Clear the terminal"    , std::bind(&Console::cls_command   , this)));
    commands.push_back(Command("echo"   , "Echo the inputs"       , std::bind(&Console::echo_command  , this), -1));
}

void Console::init(){
    com.init();
}
std::vector<std::string> Console::convert_args(){
    std::vector<std::string> args;
    uint32_t i = 0;
    std::string arg = "";
    do{
        if((rx[i] != ' ') && (rx[i] != '\t') && (rx[i] != 0x0)) arg += rx[i];
        else if(arg != "") {
            args.push_back(arg);
            arg = "";
        }
    }while(rx[i++] != 0x0);
    return args;
}

void Console::banner(){
    std::string text = "";
    text += BLU;
    text += "╔══════════════════════════════════════════╗\r\n";
    text += "║███████╗██╗  ██╗ ██████╗ ███████╗ ██████╗ ║\r\n";
    text += "║██╔════╝██║  ██║██╔═████╗╚════██║██╔════╝ ║\r\n";text += CYN;
    text += "║█████╗  ███████║██║██╔██║    ██╔╝██║  ███╗║\r\n";
    text += "║██╔══╝  ╚════██║████╔╝██║   ██╔╝ ██║   ██║║\r\n";
    text += "║██║          ██║╚██████╔╝   ██║  ╚██████╔╝║\r\n";text += GRN;
    text += "║╚═╝          ╚═╝ ╚═════╝    ╚═╝   ╚═════╝ ║\r\n";
    text += "╚══════════════════════════════════════════╝\r\n";
    text += RST;
    com.print("%s", text.c_str());
}

void Console::processLine(){
    // Wait for interrupt
    while(0 == comport_line_rdy){}
    if(comport_line_rdy == -1){
        comport_line_rdy = 0;
        com.print("\r\nCommand exceeds %i char\r\n",COMPORT_RX_BUFF_SIZE);
        return;
    }
    comport_line_rdy = 0;
    // Find commands
    bool found = 0;
    std::vector<std::string> args = convert_args();
    uint8_t string_n = args.size();
    if(string_n == 0)
        found = 1;
    else
        for(auto& i : commands){
            if(i.name == args[0]){
                if((i.argc != (string_n -1) ) && (i.argc != -1)) 
                    com.print("Wrong number of arguments for %s\r\n",args[0].c_str());
                else
                    i.func();
                found = 1;
            }
        }
    
    if(0 == found)
        com.print("Invalid command: %s\r\n",rx);
}

void Console::help_command(){
    banner();
    for (auto& cmd : commands) {
        com.print("%-16s - %s\r\n", cmd.name.c_str(), cmd.help.c_str());
    }
}
void Console::cls_command(){
	com.print(CLS.c_str());
}
void Console::echo_command(){
	std::vector<std::string> args = convert_args();
    for(auto &i : args)
        com.print("%s\r\n",i.c_str());
}

void USART2_IRQHandler(){
    static int i = 0;
	if(USART2->SR & USART_SR_RXNE){
        char new_char = (char)USART2->DR;
        if((new_char == '\r') || (new_char == '\r')){
            comport_rx_buff[i] = '\0';
            i = 0;
            comport_line_rdy = 1;
        }
        else if ((new_char == 0x8) || (new_char == 0x7F)) { // backspace or whatever picocom sends
            if(--i < 0) i = 0;
            comport_rx_buff[i] = '\0';
        }
        else {
            comport_rx_buff[i] = new_char;
            if(++i >= COMPORT_RX_BUFF_SIZE){
                i = 0;
                comport_line_rdy = -1;
            }
        }
		USART2->SR &= ~USART_SR_RXNE;
	}
}

