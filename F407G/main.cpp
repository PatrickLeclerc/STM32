#include "f407g.hpp"  // STM32F4xx device header file
#include "sdio.hpp"  // SDIO test

F407G mcu;
SD sd;

void sd_command(){
    mcu.console.com.print("Init periph\r\n");
    sd.init();
    mcu.console.com.print("Init card\r\n");
    SD_ERROR_t res = sd.initCard();
    mcu.console.com.print("Init exited with %x\r\n",res);

    ////////
    uint32_t wr[512] = {};
    uint32_t rd[512] = {};
    for(uint32_t i = 0; i < 512;i++) wr[i] = i;

    sd.writeBlock(0,wr,512);
    sd.readBlock(0,rd,512);
    
    for(uint32_t i = 0; i < 512;i++)
        mcu.console.com.print("rd[%i] =  %x\r\n",i, rd[i]);
    mcu.console.com.print("Testing writes -> read arrays%x\r\n");

}

void hu_command(){
    mcu.console.com.print("huhuhu\r\n");
}

int main(void) {
    /* Add top-level commands to console */
    mcu.console.commands.push_back(Command("sd", "SD initialization", sd_command));
    mcu.console.commands.push_back(Command("hu", "Random command", hu_command));

    /* Print help and reset terminal */
    mcu.console.cls_command();
    mcu.console.help_command();
    
    while (1) {
        mcu.console.processLine();
    }
    return 0;
}
