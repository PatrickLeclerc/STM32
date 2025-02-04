#include "f407g.hpp"  // STM32F4xx device header file
#include "sdio.hpp"  // SDIO test

const uint32_t BLOCK_SIZE_B = 512U;

F407G mcu;
SD sd;

void sd8_command(){
    mcu.console.com.print("Inside\r\n");
    uint8_t block_0_wr[BLOCK_SIZE_B] = {};
    uint8_t block_1_wr[BLOCK_SIZE_B] = {};
    uint8_t block_rd[2*BLOCK_SIZE_B] = {};
    mcu.console.com.print("block init\r\n");
    for(uint32_t i = 0; i < BLOCK_SIZE_B;i++){
        block_0_wr[i] = 0x0F | i;
        block_1_wr[i] = 0xA0 | i;
    }
    sd.writeBlock(0            ,(uint32_t*)block_0_wr, BLOCK_SIZE_B);
    sd.writeBlock(BLOCK_SIZE_B ,(uint32_t*)block_1_wr, BLOCK_SIZE_B);
    mcu.console.com.print("w8 done\r\n");
    sd.readBlock(0,(uint32_t*)block_rd,2*BLOCK_SIZE_B);
    mcu.console.com.print("r8 done\r\n");
    //
    for(uint32_t i = 0; i < 2*BLOCK_SIZE_B;i++)  mcu.console.com.print("block_rd[%i] =  %x\r\n",i, block_rd[i]);

} 

void sd32_command(){
    uint32_t block_0_wr[BLOCK_SIZE_B/4] = {};
    uint32_t block_1_wr[BLOCK_SIZE_B/4] = {};
    uint32_t block_rd[2*BLOCK_SIZE_B/4] = {};
    for(uint32_t i = 0; i < BLOCK_SIZE_B/4;i++){
        block_0_wr[i] = 0x80000000 | i;
        block_1_wr[i] = 0xC0000000 | i;
    }
    sd.writeBlock(0,block_0_wr,BLOCK_SIZE_B);
    sd.writeBlock(BLOCK_SIZE_B,block_1_wr,BLOCK_SIZE_B);
    sd.readBlock(0,block_rd,2*BLOCK_SIZE_B);
    
    for(uint32_t i = 0; i < BLOCK_SIZE_B/2;i++)  mcu.console.com.print("block_rd[%i] =  %x\r\n",i, block_rd[i]);

} 

int main(void) {
    /* SD card */
    sd.init();
    SD_ERROR_t res = sd.initCard();

    /* Add top-level commands to console */
    mcu.console.commands.push_back(Command("sd8" , "SD 8b test", sd8_command));
    mcu.console.commands.push_back(Command("sd32", "SD 32b test", sd32_command));

    /* Print help and reset terminal */
    mcu.console.cls_command();
    mcu.console.help_command();
    
    while (1) {
        mcu.console.processLine();
    }
    return 0;
}
