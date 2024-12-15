#ifndef SDIO_HPP
#define SDIO_HPP
#include "drivers_common.hpp"
#include "gpio.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>  // For std::hex and std::uppercase

// Timeout value for waiting loops
#define SD_STD_CAPACITY            ((uint32_t)0x00000000U)
#define SD_HIGH_CAPACITY           ((uint32_t)0x40000000U)

// Mask for errors in card status value
#define SD_OCR_ALL_ERRORS             ((uint32_t)0xFDFFE008U) // All possible error bits
#define SD_OCR_OUT_OF_RANGE           ((uint32_t)0x80000000U) // The command's argument was out of allowed range
#define SD_OCR_ADDRESS_ERROR          ((uint32_t)0x40000000U) // A misaligned address used in the command
#define SD_OCR_BLOCK_LEN_ERROR        ((uint32_t)0x20000000U) // The transfer block length is not allowed for this card
#define SD_OCR_ERASE_SEQ_ERROR        ((uint32_t)0x10000000U) // An error in the sequence of erase commands occurred
#define SD_OCR_ERASE_PARAM            ((uint32_t)0x08000000U) // An invalid selection of write-blocks for erase occurred
#define SD_OCR_WP_VIOLATION           ((uint32_t)0x04000000U) // Attempt to write to a protected block or to the write protected card
#define SD_OCR_LOCK_UNLOCK_FAILED     ((uint32_t)0x01000000U) // Sequence or password error in lock/unlock card command
#define SD_OCR_COM_CRC_ERROR          ((uint32_t)0x00800000U) // The CRC check of the previous command failed
#define SD_OCR_ILLEGAL_COMMAND        ((uint32_t)0x00400000U) // Command not legal for the card state
#define SD_OCR_CARD_ECC_FAILED        ((uint32_t)0x00200000U) // Card internal ECC was applied but failed to correct the data
#define SD_OCR_CC_ERROR               ((uint32_t)0x00100000U) // Internal card controller error
#define SD_OCR_ERROR                  ((uint32_t)0x00080000U) // A general or an unknown error occurred during the operation
#define SD_OCR_STREAM_R_UNDERRUN      ((uint32_t)0x00040000U) // The card could not sustain data transfer in stream read operation
#define SD_OCR_STREAM_W_OVERRUN       ((uint32_t)0x00020000U) // The card could not sustain data programming in stream mode
#define SD_OCR_CSD_OVERWRITE          ((uint32_t)0x00010000U) // CSD overwrite error
#define SD_OCR_WP_ERASE_SKIP          ((uint32_t)0x00008000U) // Only partial address space was erased
#define SD_OCR_CARD_ECC_DISABLED      ((uint32_t)0x00004000U) // The command has been executed without using the internal ECC
#define SD_OCR_ERASE_RESET            ((uint32_t)0x00002000U) // An erase sequence was cleared before executing
#define SD_OCR_AKE_SEQ_ERROR          ((uint32_t)0x00000008U) // Error in the sequence of the authentication process

// Bitmap to clear the SDIO static flags (command and data)
#define SDIO_ICR_STATIC               ((uint32_t)(SDIO_ICR_CCRCFAILC | SDIO_ICR_DCRCFAILC | SDIO_ICR_CTIMEOUTC | \
												SDIO_ICR_DTIMEOUTC | SDIO_ICR_TXUNDERRC | SDIO_ICR_RXOVERRC  | \
												SDIO_ICR_CMDRENDC  | SDIO_ICR_CMDSENTC  | SDIO_ICR_DATAENDC  | \
												SDIO_ICR_DBCKENDC))
// Bitmap to clear the SDIO command flags
#define SDIO_ICR_CMD                  ((uint32_t)(SDIO_ICR_CCRCFAILC | SDIO_ICR_CTIMEOUTC | \
												SDIO_ICR_CMDRENDC | SDIO_ICR_CMDSENTC))

// Bitmap to clear the SDIO data flags
#define SDIO_STA_STBITERR 0
#define SDIO_ICR_STBITERRC 0
#define SDIO_ICR_DATA                 ((uint32_t)(SDIO_ICR_RXOVERRC | SDIO_ICR_DCRCFAILC | \
												SDIO_ICR_DTIMEOUTC | SDIO_ICR_DBCKENDC | \
												SDIO_ICR_STBITERRC))
// R6 response error bits
#define SD_R6_GENERAL_UNKNOWN_ERROR   ((uint32_t)0x00002000U)
#define SD_R6_ILLEGAL_CMD             ((uint32_t)0x00004000U)
#define SD_R6_COM_CRC_FAILED          ((uint32_t)0x00008000U)

// RCA for the MMC card
#define SDIO_MMC_RCA                  ((uint16_t)0x0001U)

// SDIO timeout for data transfer ((48MHz / CLKDIV / 1000) * timeout_ms)
#define SD_DATA_R_TIMEOUT             ((uint32_t)((180000000U / (0 + 2U) / 1000U) * 100U)) // Data read timeout is 100ms
#define SD_DATA_W_TIMEOUT             ((uint32_t)((180000000U / (0 + 2U) / 1000U) * 250U)) // Date write timeout is 250ms

/*enums*/
typedef enum SD_RESP_SIZE{
	SD_RESP_NONE    = (0x0 << SDIO_CMD_WAITRESP_Pos), // No response
	SD_RESP_SHORT = (0x1 << SDIO_CMD_WAITRESP_Pos), // Short response
	SD_RESP_LONG  = (0x2 << SDIO_CMD_WAITRESP_Pos)  // Long response
}SD_RESP_SIZE_t;

// SD card response type
enum {
	SD_R1  = 0x01, // R1
	SD_R1b = 0x02, // R1b
	SD_R2  = 0x03, // R2
	SD_R3  = 0x04, // R3
	SD_R6  = 0x05, // R6 (SDIO only)
	SD_R7  = 0x06  // R7
}SD_RESP_TYPE_t;

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

#endif
