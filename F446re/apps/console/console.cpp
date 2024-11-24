#include "console.hpp"

#include <string>
#include <cstring>
#include <stdio.h>

volatile int console_tx_busy = 0;
volatile int console_line_rdy = 0;
volatile char console_rx_buff[CONSOLE_RX_BUFF_SIZE] = {0};

Console::Console(uint32_t baud)
{
	USART_CFG_t u2 = {
        .regs = 0,
        .n = 2,
        .br = baud,
        .rxe = 1,
        .txe = 1,
        .rxie = 1,
        .txie = 0,
        .dmaRxE = 0,
        .dmaTxE = 1
    };
    usart = USART(u2);
    GPIO_CFG_t u2_gpio_rx = {
        .regs = 0,
        .port = 'A',
        .pins = 0x4,
        .speed = GPIO_SPEED_4_8MHz,
        .mode = GPIO_MODE_AF,
        .af = 0x7
    };
    GPIO_CFG_t u2_gpio_tx = {
        .regs = 0,
        .port = 'A',
        .pins = 0x8,
        .speed = GPIO_SPEED_4_8MHz,
        .mode = GPIO_MODE_AF,
        .af = 0x7
    };
	gpio.push_back(GPIO(u2_gpio_rx));
    gpio.push_back(GPIO(u2_gpio_tx));
    
    DMA_CFG_t u2_dma_tx ={
        .regs       = 0,
		.n			= 1,
		.stream		= 6,
		.ch			= 4,
		.m0ar		= 0,
		.m1ar		= 0,
		.par		= (uint32_t)&(USART2->DR),
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
    dma_tx = DMA(u2_dma_tx);
    
    assemble_commands();
}

void Console::init(){
    rtc.init();
    for(auto& g:gpio)
        g.init();
    dma_tx.init();
    usart.init();
    cls_command();
    help_command();
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
void Console::assemble_commands(){
	commands.push_back(Command("help"   , "Displays this message" , std::bind(&Console::help_command  , this)));
	commands.push_back(Command("cls"    , "Clear the terminal"    , std::bind(&Console::cls_command   , this)));
	commands.push_back(Command("time"   , "Display RTC time"      , std::bind(&Console::time_command  , this)));
	commands.push_back(Command("echo"   , "Echo the inputs"       , std::bind(&Console::echo_command  , this), -1));
}
void Console::banner(){
    print(" \033[31m        \\\\                         //          \r\n");
    print(" \033[31m\\\\      (\033[37mo\033[31m>       \033[34mSTM32F446\033[31m       <\033[37mo\033[31m)       // \r\n");
    print(" \033[31m(\033[37mo\033[31m>     //\\                       /\\\\      <\033[37mo\033[31m)   \r\n");
    print("\033[0m_\033[31m(()\033[0m_____\033[31mV_/\033[0m______________________ \033[31m\\_V\033[0m _____\033[31m())\033[0m_ \r\n");
    print(" \033[31m||      ||                         ||       ||   \r\n");
    print(" \033[31m        ||                         ||            \r\n");
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
void Console::print(const char *format, ...){
	//Wait for last TX to complete
	while(console_tx_busy){}
	console_tx_busy = 1;

    // Format the thing
    static char buffer[128];
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);

	//Write
	dma_tx.cfg.regs->CR &= ~DMA_SxCR_EN;
	while(dma_tx.cfg.regs->CR & DMA_SxCR_EN){}
	dma_tx.cfg.regs->M0AR = (uint32_t) buffer;
	dma_tx.cfg.regs->PAR = (uint32_t)&(usart.cfg.regs->DR);
	dma_tx.cfg.regs->NDTR = (uint32_t) len;
	dma_tx.cfg.regs->CR |= DMA_SxCR_EN;
    va_end(args);
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

void DMA1_Stream6_IRQHandler(){
	if(DMA1->HISR & DMA_HISR_TCIF6){
		DMA1->HIFCR = DMA_HIFCR_CTCIF6;
		console_tx_busy = 0;
	}}

void USART2_IRQHandler(){
    static int i = 0;
	if(USART2->SR & USART_SR_RXNE){
        char new_char = (char)USART2->DR;
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
		USART2->SR &= ~USART_SR_RXNE;
	}
}
