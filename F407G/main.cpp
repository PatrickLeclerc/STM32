#include "f407g.hpp"  // STM32F4xx device header file
#include "sdio.hpp"  // SDIO test

#define BLOCK_SIZE_B 512

F407G mcu;
SD sd;

void sd_command(){
    const uint32_t szw = BLOCK_SIZE_B/4;
    uint32_t block_0_wr[szw] = {};
    uint32_t block_512_wr[szw] = {};
    uint32_t block_0_rd[szw] = {};
    uint32_t block_512_rd[szw] = {};
    for(uint32_t i = 0; i < szw;i++){
        block_0_wr[i] = 0xB0FF0000 | i;
        block_512_wr[i] = 0xB1AA0000 | i;
    }
    sd.writeBlock(0,block_0_wr,BLOCK_SIZE_B);
    sd.readBlock(0,block_0_rd,BLOCK_SIZE_B);
    sd.writeBlock(BLOCK_SIZE_B,block_512_wr,BLOCK_SIZE_B);
    sd.readBlock(BLOCK_SIZE_B,block_512_rd,BLOCK_SIZE_B);
    
    for(uint32_t i = 0; i < szw;i++)  mcu.console.com.print("block_0_rd[%i] =  %x\r\n",i, block_0_rd[i]);
    for(uint32_t i = 0; i < szw;i++)  mcu.console.com.print("block_512_rd[%i] =  %x\r\n",i, block_512_rd[i]);
    for(uint32_t i = 0; i < szw;i++){
        block_0_wr[i] = 0xDEAD0000 | i;
    }
    sd.writeBlock(0,block_0_wr,BLOCK_SIZE_B);
    sd.readBlock(0,block_0_rd,BLOCK_SIZE_B);
    for(uint32_t i = 0; i < szw;i++)  mcu.console.com.print("block_0_rd[%i] =  %x\r\n",i, block_0_rd[i]);
} 

void hu_command(){
    mcu.console.com.print("huhuhu\r\n");
}

int main(void) {
    /* SD card */
    sd.init();
    SD_ERROR_t res = sd.initCard();
    mcu.console.com.print("Init SD CARD %x\r\n",res);

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
