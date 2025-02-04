#include "f407g.hpp"  // STM32F4xx device header file
#include "sdio.hpp"  // SDIO test
#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

const uint32_t BLOCK_SIZE_B = 512U;

F407G mcu;
SD sd;

extern "C"{
    DSTATUS disk_status (BYTE pdrv){
        return 0;
    }
    DSTATUS disk_initialize (BYTE pdrv){
    	DSTATUS stat;
    	sd.init();
        if(SDR_Success == sd.initCard()) return 0;
        return STA_NOINIT;
    	
    }
    DRESULT disk_read (
    	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
    	BYTE *buff,		/* Data buffer to store read data */
    	LBA_t sector,	/* Start sector in LBA */
    	UINT count		/* Number of sectors to read */
    )
    {
    	if(SDR_Success == sd.readBlock(sector*BLOCK_SIZE_B,(uint32_t*)buff,count*BLOCK_SIZE_B)) return RES_OK;
        return RES_PARERR;
    }
    DRESULT disk_write (
    	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
    	const BYTE *buff,	/* Data to be written */
    	LBA_t sector,		/* Start sector in LBA */
    	UINT count			/* Number of sectors to write */
    )
    {
    	if(SDR_Success == sd.writeBlock(sector*BLOCK_SIZE_B,(uint32_t*)buff,count*BLOCK_SIZE_B)) return RES_OK;
    	return RES_PARERR;
    }
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
){
    switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;
        case GET_SECTOR_COUNT:
            return RES_OK;
        case GET_SECTOR_SIZE:
            return RES_OK;
        case GET_BLOCK_SIZE:
            return RES_OK;
        case CTRL_TRIM:
            return RES_OK;
        default:
            return RES_PARERR;  // Invalid command
    }
}

DWORD get_fattime ()
{
    time_t t;
    struct tm *stm;


    t = time(0);
    stm = localtime(&t);

    return (DWORD)(stm->tm_year - 80) << 25 |
           (DWORD)(stm->tm_mon + 1) << 21 |
           (DWORD)stm->tm_mday << 16 |
           (DWORD)stm->tm_hour << 11 |
           (DWORD)stm->tm_min << 5 |
           (DWORD)stm->tm_sec >> 1;
}
}

int main(){
    /* Print help and reset terminal */
    mcu.console.cls_command();
    mcu.console.help_command();

    /* FATFS EXMAMPLE */
    FATFS fs;
    FRESULT res;

    // mount the default drive
    res = f_mount(&fs, "", 1);
    if(res != FR_OK) {
        mcu.console.com.print("f_mount() failed, res = %d\r\n", res);
        while(1){}
    }

    uint32_t freeClust;
    FATFS* fs_ptr = &fs;
    res = f_getfree("", &freeClust, &fs_ptr); // Warning! This fills fs.n_fatent and fs.csize!
    if(res != FR_OK) {
        mcu.console.com.print("f_getfree() failed, res = %d\r\n", res);
        while(1){}
    }

    uint32_t totalBlocks = (fs.n_fatent - 2) * fs.csize;
    uint32_t freeBlocks = freeClust * fs.csize;
    mcu.console.com.print("Total blocks: %lu (%lu Mb)\r\n", totalBlocks, totalBlocks / 2000);
    mcu.console.com.print("Free blocks: %lu (%lu Mb)\r\n", freeBlocks, freeBlocks / 2000);

    DIR dir;
    res = f_opendir(&dir, "/");
    if(res != FR_OK) {
        mcu.console.com.print("f_opendir() failed, res = %d\r\n", res);
        while(1){}
    }

    FILINFO fileInfo;
    uint32_t totalFiles = 0;
    uint32_t totalDirs = 0;
    mcu.console.com.print("--------\r\nRoot directory:\r\n");
    for(;;) {
        res = f_readdir(&dir, &fileInfo);
        if((res != FR_OK) || (fileInfo.fname[0] == '\0')) {
            break;
        }
        
        if(fileInfo.fattrib & AM_DIR) {
            mcu.console.com.print("  DIR  %s\r\n", fileInfo.fname);
            totalDirs++;
        } else {
            mcu.console.com.print("  FILE %s\r\n", fileInfo.fname);
            totalFiles++;
        }
    }

    mcu.console.com.print("(total: %lu dirs, %lu files)\r\n--------\r\n", totalDirs, totalFiles);

    res = f_closedir(&dir);
    if(res != FR_OK) {
        mcu.console.com.print("f_closedir() failed, res = %d\r\n", res);
        while(1){}
    }

    mcu.console.com.print("Writing to log.txt...\r\n");

    char writeBuff[128];
    snprintf(writeBuff, sizeof(writeBuff), "Total blocks: %lu (%lu Mb); Free blocks: %lu (%lu Mb)\r\n",
        totalBlocks, totalBlocks / 2000,
        freeBlocks, freeBlocks / 2000);

    FIL logFile;
    res = f_open(&logFile, "log.txt", FA_OPEN_APPEND | FA_WRITE);
    if(res != FR_OK) {
        mcu.console.com.print("f_open() failed, res = %d\r\n", res);
        while(1){}
    }

    unsigned int bytesToWrite = sizeof(writeBuff)/sizeof(char);
    unsigned int bytesWritten;
    res = f_write(&logFile, writeBuff, bytesToWrite, &bytesWritten);
    if(res != FR_OK) {
        mcu.console.com.print("f_write() failed, res = %d\r\n", res);
        while(1){}
    }

    if(bytesWritten < bytesToWrite) {
        mcu.console.com.print("WARNING! Disk is full, bytesToWrite = %lu, bytesWritten = %lu\r\n", bytesToWrite, bytesWritten);
    }

    res = f_close(&logFile);
    if(res != FR_OK) {
        mcu.console.com.print("f_close() failed, res = %d\r\n", res);
        while(1){}
    }

    mcu.console.com.print("Reading file...\r\n");
    FIL msgFile;
    res = f_open(&msgFile, "log.txt", FA_READ);
    if(res != FR_OK) {
        mcu.console.com.print("f_open() failed, res = %d\r\n", res);
        while(1){}
    }

    char readBuff[128];
    unsigned int bytesRead;
    res = f_read(&msgFile, readBuff, sizeof(readBuff)-1, &bytesRead);
    if(res != FR_OK) {
        mcu.console.com.print("f_read() failed, res = %d\r\n", res);
        while(1){}
    }

    readBuff[bytesRead] = '\0';
    mcu.console.com.print("```\r\n%s\r\n```\r\n", readBuff);

    res = f_close(&msgFile);
    if(res != FR_OK) {
        mcu.console.com.print("f_close() failed, res = %d\r\n", res);
        while(1){}
    }

    // Unmount
    res = f_mount(NULL, "", 0);
    if(res != FR_OK) {
        mcu.console.com.print("Unmount failed, res = %d\r\n", res);
        while(1){}
    }

    mcu.console.com.print("Done!\r\n");
    
    /* Console */
    while (1) {
        mcu.console.processLine();
    }

}
