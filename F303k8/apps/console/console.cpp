#include "console.hpp"

#include <string>
#include <cstring>
#include <stdio.h>

volatile int console_tx_busy = 0;
volatile int console_line_rdy = 0;
volatile char console_rx_buff[CONSOLE_RX_BUFF_SIZE] = {0};

Console::Console(uint32_t baud)
{
    assemble_commands();
}

void Console::init(){
    rtc.init();
    com.init();
    cls_command();
    help_command();
}

void Console::assemble_commands(){
	commands.push_back(Command("help"   , "Displays this message"       , std::bind(&Console::help_command  , this)));
	commands.push_back(Command("cls"    , "Clear the terminal"          , std::bind(&Console::cls_command   , this)));
	commands.push_back(Command("time"   , "Display RTC time (FIX ME)"   , std::bind(&Console::time_command  , this)));
    commands.push_back(Command("echo"   , "Echo the inputs"             , std::bind(&Console::echo_command  , this), -1));
    
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
    print(" \033[33m        \\\\                         //          \r\n");
    print(" \033[33m\\\\      (\033[37mo\033[33m>       \033[32mSTM32F303\033[33m       <\033[37mo\033[33m)       // \r\n");
    print(" \033[33m(\033[37mo\033[33m>     //\\                       /\\\\      <\033[37mo\033[33m)   \r\n");
    print("\033[0m_\033[33m(()\033[0m_____\033[33mV_/\033[0m______________________ \033[33m\\_V\033[0m _____\033[33m())\033[0m_ \r\n");
    print(" \033[33m||      ||                         ||       ||   \r\n");
    print(" \033[33m        ||                         ||            \r\n");
    print("\033[0m \r\n");
}
void Console::processLine(){
    while(0 == console_line_rdy){}
    if(console_line_rdy == -1){
        console_line_rdy = 0;
        print("\r\nCommand exceeds %i char\r\n",CONSOLE_RX_BUFF_SIZE);
        return;
    }
    console_line_rdy = 0;
    // Basic terminal commands
    bool found = 0;
    std::vector<std::string> args = convert_args();
    uint8_t string_n = args.size();
    if(string_n == 0)
        found = 1;
    else
        for(auto& i : commands){
            if(i.name == args[0]){
                if((i.argc != (string_n -1) ) && (i.argc != -1)) 
                    print("Wrong number of arguments for %s\r\n",args[0].c_str());
                else
                    i.func();
                found = 1;
            }
        }
    
    if(0 == found)
        print("Invalid command: %s\r\n",rx);
}

void Console::help_command(){
    banner();
    print("-----------------------------------------\r\n");
    for (auto& cmd : commands) {
        print("%s\t- %s\r\n", cmd.name.c_str(), cmd.help.c_str());
    }
    print("-----------------------------------------\r\n");
}
void Console::time_command(){
	char time[9];
    time[8] = '\0';
	rtc.get_bcd(time);
	print("%s\r\n",time);
}
void Console::cls_command(){
	print("\033[2J");
}
void Console::echo_command(){
	std::vector<std::string> args = convert_args();
    for(auto &i : args)
        print("%s\r\n",i.c_str());
}
void Console::sd_command() {
    std::vector<std::string> args = convert_args();
    if (args[1] == "wake") {
        // Wake up (send 80+ clock pulses)
        sd.cs_high(); // Deassert CS
        sd.wait.mswait(1);
        for (int i = 0; i < 11; i++) {
            sd.spi.tx(0xFF); // Send 80+ clock pulses
        }
        sd.wait.uwait(100);
        print("Card woken up\r\n");
    }
    if (args[1] == "reset") {
        while(SPI1->SR & SPI_SR_BSY){}
        // Assert CS and prepare for CMD0
        sd.cs_low(); // Assert CS
        sd.wait.uwait(100);

        // Send CMD0 (GO_IDLE_STATE)
        sd.tx_buf[0] =0x40; // Command index: CMD0 = 0x40
        sd.tx_buf[1] =0x00; // Argument byte 1
        sd.tx_buf[2] =0x00; // Argument byte 2
        sd.tx_buf[3] =0x00; // Argument byte 3
        sd.tx_buf[4] =0x00; // Argument byte 4
        sd.tx_buf[5] =0x95; // CRC for CMD0 (pre-calculated)
        sd.tx();
        
        // Read the response
        for (int i = 0; i < 10; i++) { // Polling loop
            if(i&1){
                sd.tx();
            }
            else {
                sd.rx_en(1);
                sd.spi.tx(0xff);
                while(spi_rx_flag == 0){}
                spi_rx_flag = 0;
            }
            if (sd.rx_buf[0] != 0xFF) break;
        }

        sd.cs_high(); // Deassert CS
        sd.wait.mswait(100);

        // Print the response
        print("CMD0 Response: %x\r\n", sd.rx_buf[0]); // Should print 0x01
    }
}




void DMA1_Channel7_IRQHandler(){
	if(DMA1->ISR & DMA_ISR_TCIF7){
		DMA1->IFCR = DMA_IFCR_CTCIF7;
		console_tx_busy = 0;
	}}

void USART2_IRQHandler(){
    static int i = 0;
	if(USART2->ISR & USART_ISR_RXNE){
        char new_char = (char)USART2->RDR;
        if((new_char == '\r') || (new_char == '\r')){
            console_rx_buff[i] = '\0';
            i = 0;
            console_line_rdy = 1;
        }
        else if ((new_char == 0x8) || (new_char == 0x7F)) { // backspace or whatever picocom sends
            if(--i < 0) i = 0;
            console_rx_buff[i] = '\0';
        }
        else {
            console_rx_buff[i] = new_char;
            if(++i >= CONSOLE_RX_BUFF_SIZE){
                i = 0;
                console_line_rdy = -1;
            }
        }
		USART2->ISR &= ~USART_ISR_RXNE;
	}
}
