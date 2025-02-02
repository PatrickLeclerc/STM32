#ifndef SDIO_HPP
#define SDIO_HPP
#include "drivers_common.hpp"
#include "gpio.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>  // For std::hex and std::uppercase

/*enums*/
typedef enum SD_RESP_SIZE{
	SD_RESP_NONE    = (0x0 << SDIO_CMD_WAITRESP_Pos), // No response
	SD_RESP_SHORT = (0x1 << SDIO_CMD_WAITRESP_Pos), // Short response
	SD_RESP_LONG  = (0x2 << SDIO_CMD_WAITRESP_Pos)  // Long response
}SD_RESP_SIZE_t;

// Card type
typedef enum {
	SDCT_UNKNOWN = 0x00,
	SDCT_SDSC_V1 = 0x01,  // Standard capacity SD card v1.0
	SDCT_SDSC_V2 = 0x02,  // Standard capacity SD card v2.0
	SDCT_MMC     = 0x03,  // MMC
	SDCT_SDHC    = 0x04   // High capacity SD card (SDHC or SDXC)
}SD_CardType_t;

// SD functions result
typedef enum {
	SDR_Success             = 0x00,
	SDR_Timeout             = 0x01,  // Timeout
	SDR_CRCError            = 0x02,  // Response for command received but CRC check failed
	SDR_ReadError           = 0x03,  // Read block error (response for CMD17)
	SDR_WriteError          = 0x04,  // Write block error (response for CMD24)
	SDR_WriteErrorInternal  = 0x05,  // Write block error due to internal card error
	SDR_Unsupported         = 0x06,  // Unsupported card found
	SDR_BadResponse         = 0x07,
	SDR_SetBlockSizeFailed  = 0x08,  // Set block size command failed (response for CMD16)
	SDR_UnknownCard         = 0x09,
	SDR_NoResponse          = 0x0A,
	SDR_AddrOutOfRange      = 0x0B,  // Address out of range
	SDR_WriteCRCError       = 0x0C,  // Data write rejected due to a CRC error
	SDR_InvalidVoltage      = 0x0D,  // Unsupported voltage range
	SDR_DataTimeout         = 0x0E,  // Data block transfer timeout
	SDR_DataCRCFail         = 0x0F,  // Data block transfer CRC failed
	SDR_RXOverrun           = 0x10,  // Receive FIFO overrun
	SDR_TXUnderrun          = 0x11,  // Transmit FIFO underrun
	SDR_StartBitError       = 0x12,  // Start bit not detected on all data signals
	SDR_AddrMisaligned      = 0x13,  // A misaligned address which did not match the block length was used in the command
	SDR_BlockLenError       = 0x14,  // The transfer block length is not allowed for this card
	SDR_EraseSeqError       = 0x15,  // An error in the sequence of erase commands occurred
	SDR_EraseParam          = 0x16,  // An invalid selection of write-blocks for erase occurred
	SDR_WPViolation         = 0x17,  // Attempt to write to a protected block or to the write protected card
	SDR_LockUnlockFailed    = 0x18,  // Error in lock/unlock command
	SDR_ComCRCError         = 0x19,  // The CRC check of the previous command failed
	SDR_IllegalCommand      = 0x1A,  // Command is not legal for the the current card state
	SDR_CardECCFailed       = 0x1B,  // Card internal ECC was applied but failed to correct the data
	SDR_CCError             = 0x1C,  // Internal card controller error
	SDR_GeneralError        = 0x1D,  // A general or an unknown error occurred during the operation
	SDR_StreamUnderrun      = 0x1E,  // The card could not sustain data transfer in stream read operation
	SDR_StreamOverrun       = 0x1F,  // The card could not sustain data programming in stream mode
	SDR_CSDOverwrite        = 0x20,  // CSD overwrite error
	SDR_WPEraseSkip         = 0x21,  // Only partial address space was erased
	SDR_ECCDisabled         = 0x22,  // The command has been executed without using the internal ECC
	SDR_EraseReset          = 0x23,  // An erase sequence was cleared before executing
	SDR_AKESeqError         = 0x24,  // Error in the sequence of the authentication process
	SDR_UnknownError        = 0xFF   // Unknown error
} SD_ERROR_t;

// SD card description
typedef struct {
	uint8_t     Type;            // Card type (detected by SD_Init())
	uint32_t    Capacity;        // Card capacity (MBytes for SDHC/SDXC, bytes otherwise)
	uint32_t    BlockCount;      // SD card blocks count
	uint32_t    BlockSize;       // SD card block size (bytes), determined in SD_ReadCSD()
	uint32_t    MaxBusClkFreq;   // Maximum card bus frequency (MHz)
	uint8_t     CSDVer;          // SD card CSD register version
	uint16_t    RCA;             // SD card RCA address (only for SDIO)
	uint8_t     MID;             // SD card manufacturer ID
	uint16_t    OID;             // SD card OEM/Application ID
	uint8_t     PNM[5];          // SD card product name (5-character ASCII string)
	uint8_t     PRV;             // SD card product revision (two BCD digits: '6.2' will be 01100010b)
	uint32_t    PSN;             // SD card serial number
	uint16_t    MDT;             // SD card manufacturing date
	uint8_t     CSD[16];         // SD card CSD register (card structure data)
	uint8_t     CID[16];         // SD card CID register (card identification number)
	uint8_t     SCR[8];          // SD card SCR register (SD card configuration)
} SDCard_TypeDef;


typedef enum SD_WIDBUS{
	SD_WIDBUS_1 = 0,
	SD_WIDBUS_4 = 1,
	SD_WIDBUS_8 = 2
}SD_WIDBUS_t;

typedef struct __attribute__((__packed__)){
	SDIO_TypeDef *		regs;
	SD_WIDBUS_t         wid;
	uint32_t 			clk_div_slow;
	uint32_t 			clk_div_fast;
}SD_CFG_t; 



class SD : public Driver
{
private:
	std::vector<GPIO> gpio;
	inline void getCardInfo(void);
	inline void setFastClock(void);
	inline SD_ERROR_t getSCR(uint32_t *pSCR);
	inline SD_ERROR_t sendCommand(uint32_t cmdIndex, uint32_t argument, SD_RESP_SIZE_t responseSize);
	inline SD_ERROR_t getR1Resp(uint8_t cmd);
	inline SD_ERROR_t getR2Resp(uint32_t *pBuf);
	inline SD_ERROR_t getR3Resp(void);
	inline SD_ERROR_t getR6Resp(uint8_t cmd, uint16_t *pRCA);
	inline SD_ERROR_t getR7Resp(void);
	inline SD_ERROR_t stopTransfer(void);
	inline SD_ERROR_t getCardState(uint8_t *pState);
public:
	SDCard_TypeDef cardDef = {0};
	std::string logs = "";
	SD_CFG_t cfg;
	SD();
	void init();
	void deinit(){};
	SD_ERROR_t initCard();
	SD_ERROR_t readBlock(uint32_t addr, uint32_t *pBuf, uint32_t length);
	SD_ERROR_t writeBlock(uint32_t addr, uint32_t *pBuf, uint32_t length);
};

#ifdef USE_DMA_FOR_SDIO
// Initialize the DMA channel for SDIO peripheral (DMA2 Channel4)
// input:
//   pBuf - pointer to the memory buffer
//   length - size of the memory buffer in bytes (must be a multiple of 4, since the SDIO operates with 32-bit words)
//   direction - DMA channel direction, one of SDIO_DMA_DIR_xx values
// note: the DMA peripheral (DMA2) must be already enabled
void SD_Configure_DMA(uint32_t *pBuf, uint32_t length, uint8_t direction) {
	// Populate SDIO DMA channel handle
	SDIO_DMA_CH.Channel  = SDIO_DMA_CHANNEL;
	SDIO_DMA_CH.Instance = DMA_GetChannelPeripheral(SDIO_DMA_CHANNEL);
	SDIO_DMA_CH.ChIndex  = DMA_GetChannelIndex(SDIO_DMA_CHANNEL);
	SDIO_DMA_CH.Request  = SDIO_DMA_REQUEST;
	SDIO_DMA_CH.State    = DMA_STATE_RESET;

	// DMA channel configuration:
	//   channel priority: medium
	//   memory increment: enabled
	//   peripheral increment: disabled
	//   circular mode: disabled
	//   IRQ: disabled
	//   memory size: 32-bit
	//   peripheral size: 32-bit
	//   channel: disabled
	DMA_ConfigChannel(
			SDIO_DMA_CH.Channel,
			direction |
			DMA_MODE_NORMAL |
			DMA_MINC_ENABLE | DMA_PINC_DISABLE |
			DMA_PSIZE_32BIT | DMA_MSIZE_32BIT |
			DMA_PRIORITY_MEDIUM
		);
	DMA_SetAddrM(SDIO_DMA_CH.Channel, (uint32_t)pBuf);
	DMA_SetAddrP(SDIO_DMA_CH.Channel, (uint32_t)(&(cfg.regs->FIFO)));

	// Number of DMA transactions
	DMA_SetDataLength(SDIO_DMA_CH.Channel, length >> 2);

	// Map DMA request to DMA channel
	DMA_SetRequest(SDIO_DMA_CH.Instance, SDIO_DMA_CH.Request, SDIO_DMA_CH.ChIndex);

	// Clear SDIO DMA channel interrupt flags
	DMA_ClearFlags(SDIO_DMA_CH.Instance, SDIO_DMA_CH.ChIndex, DMA_CF_ALL);
}

// Start reading of data block from the SD card with DMA transfer
// input:
//   addr - address of the block to be read
//   pBuf - pointer to the buffer that will contain the received data
//   length - buffer length (must be multiple of 512)
// return: SD_ERROR_t value
SD_ERROR_t SD_ReadBlock_DMA(uint32_t addr, uint32_t *pBuf, uint32_t length) {
	SD_ERROR_t cmd_res = SDR_Success;
	uint32_t blk_count = length >> 9;

	// Initialize data control register and clear static SDIO flags
	cfg.regs->DCTRL = 0;
	cfg.regs->ICR = SDIO_ICR_STATIC;

	// Configure number of transactions and enable the SDIO DMA channel
	DMA_SetDataLength(SDIO_DMA_CH.Channel, length >> 2);
	DMA_EnableChannel(SDIO_DMA_CH.Channel);

	// SDSC card uses byte unit address and
	// SDHC/SDXC cards use block unit address (1 unit = 512 bytes)
	// For SDHC card addr must be converted to block unit address
	if (cardDef.Type == SDCT_SDHC) {
		addr >>= 9;
	}

	if (blk_count > 1) {
		// Send READ_MULT_BLOCK command
		SD_Cmd(SD_CMD_READ_MULT_BLOCK, addr, SD_RESP_SHORT); // CMD18
		cmd_res = SD_GetR1Resp(SD_CMD_READ_MULT_BLOCK);
	} else {
		// Send READ_SINGLE_BLOCK command
		SD_Cmd(SD_CMD_READ_SINGLE_BLOCK, addr, SD_RESP_SHORT); // CMD17
		cmd_res = SD_GetR1Resp(SD_CMD_READ_SINGLE_BLOCK);
	}
	if (cmd_res == SDR_Success) {
		// Data read timeout
		cfg.regs->DTIMER = SD_DATA_R_TIMEOUT;
		// Data length
		cfg.regs->DLEN   = length;
		// Data transfer:
		//   transfer mode: block
		//   direction: from card
		//   DMA: enabled
		//   block size: 2^9 = 512 bytes
		//   DPSM: enabled
		cfg.regs->DCTRL = SDMMC_DCTRL_DMAEN | SDMMC_DCTRL_DTDIR | (9 << 4) | SDMMC_DCTRL_DTEN;
	}

	return cmd_res;
}
#endif
#endif
