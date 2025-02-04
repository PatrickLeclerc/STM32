#include "fs.hpp"
SD _sd;
const uint32_t BLOCK_SIZE_B = 512U;

extern "C"{
    DSTATUS disk_status (BYTE pdrv){
        return 0;
    }
    DSTATUS disk_initialize (BYTE pdrv){
    	DSTATUS stat;
    	_sd.init();
        if(SDR_Success == _sd.initCard()) return 0;
        return STA_NOINIT;
    	
    }
    DRESULT disk_read (
    	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
    	BYTE *buff,		/* Data buffer to store read data */
    	LBA_t sector,	/* Start sector in LBA */
    	UINT count		/* Number of sectors to read */
    )
    {
    	if(SDR_Success == _sd.readBlock(sector*BLOCK_SIZE_B,(uint32_t*)buff,count*BLOCK_SIZE_B)) return RES_OK;
        return RES_PARERR;
    }
    DRESULT disk_write (
    	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
    	const BYTE *buff,	/* Data to be written */
    	LBA_t sector,		/* Start sector in LBA */
    	UINT count			/* Number of sectors to write */
    )
    {
    	if(SDR_Success == _sd.writeBlock(sector*BLOCK_SIZE_B,(uint32_t*)buff,count*BLOCK_SIZE_B)) return RES_OK;
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

#include <iostream>
#include <sstream>

void FS::init(){
    FRESULT res;
    std::stringstream ss;
    // mount the default drive
    res = f_mount(&fatfs, "", 1);
    if(res != FR_OK) {
        ss << "f_mount() failed\r\n";
    }

    uint32_t freeClust;
    FATFS* fs_ptr = &fatfs;
    res = f_getfree("", &freeClust, &fs_ptr); // Warning! This fills fs.n_fatent and fs.csize!
    if(res != FR_OK) {
       ss << "f_getfree() failed\r\n";
    }

    uint32_t totalBlocks = (fatfs.n_fatent - 2) * fatfs.csize;
    uint32_t freeBlocks = freeClust * fatfs.csize;
    ss << "Total blocks " << totalBlocks << "\t(" << totalBlocks/2000 << " Mb)\r\n";
    ss << "Free blocks  " << freeBlocks <<  "\t(" << freeBlocks/2000 << " Mb)\r\n";

    DIR dir;
    res = f_opendir(&dir, "/");
    if(res != FR_OK) {
         ss << "f_opendir() failed\r\n";
    }

    FILINFO fileInfo;
    uint32_t totalFiles = 0;
    uint32_t totalDirs = 0;
    ss << "--------\r\nRoot directory:\r\n";
    for(;;) {
        res = f_readdir(&dir, &fileInfo);
        if((res != FR_OK) || (fileInfo.fname[0] == '\0')) {
            break;
        }
        
        if(fileInfo.fattrib & AM_DIR) {
            ss << "  DIR  " << fileInfo.fname << "\r\n";
            totalDirs++;
        } else {
            ss << "  FILE " << fileInfo.fname << "\r\n";
            totalFiles++;
        }
    }

    ss << "(total: " << totalDirs << " dirs, " << totalFiles << " files)\r\n--------\r\n";

    res = f_closedir(&dir);
    if(res != FR_OK) {
        ss << "f_closedir() failed\r\n";
    }

    ss << "Writing to log.txt...\r\n";
    char writeBuff[128];
    snprintf(writeBuff, sizeof(writeBuff), "Total blocks: %lu (%lu Mb); Free blocks: %lu (%lu Mb)\r\n",
        totalBlocks, totalBlocks / 2000,
        freeBlocks, freeBlocks / 2000);

    FIL logFile;
    res = f_open(&logFile, "boot.log", FA_OPEN_APPEND | FA_WRITE);
    if(res != FR_OK) {
        ss << "f_open() failed\r\n";
    }

    unsigned int bytesToWrite = sizeof(writeBuff)/sizeof(char);
    unsigned int bytesWritten;
    res = f_write(&logFile, writeBuff, bytesToWrite, &bytesWritten);
    if(res != FR_OK) {
        ss << "f_write() failed\r\n";
    }

    if(bytesWritten < bytesToWrite) {
        ss << "WARNING! Disk is full, bytesToWrite = ";//%lu, bytesWritten = %lu\r\n", bytesToWrite, bytesWritten;
    }

    res = f_close(&logFile);
    if(res != FR_OK) {
        ss << "f_close() failed\r\n";
    }

    ss << "Reading file...\r\n";
    FIL msgFile;
    res = f_open(&msgFile, "boot.log", FA_READ);
    if(res != FR_OK) {
        ss << "f_open() failed\r\n";
    }

    char readBuff[128];
    unsigned int bytesRead;
    res = f_read(&msgFile, readBuff, sizeof(readBuff)-1, &bytesRead);
    if(res != FR_OK) {
        ss << "f_read() failed\r\n";
    }

    readBuff[bytesRead] = '\0';
    ss << readBuff << "\r\n";

    res = f_close(&msgFile);
    if(res != FR_OK) {
        ss << "f_close() failed\r\n";
    }

    // Unmount
    res = f_mount(NULL, "", 0);
    if(res != FR_OK) {
       ss << "f_unmount() failed\r\n";
    }

    ss << "Done!\r\n";
    logs = ss.str();
}
