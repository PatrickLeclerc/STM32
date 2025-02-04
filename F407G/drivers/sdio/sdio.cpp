#include "sdio.hpp"
// Timeout value for waiting loops
#define SD_CMD_TIMEOUT 100000;

// SD card capacity
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

// SDIO timeout for data transfer ((180MHz / CLKDIV / 1000) * timeout_ms)
#define SD_DATA_R_TIMEOUT             ((uint32_t)((180000000U / (0 + 2U) / 1000U) * 100U)) // Data read timeout is 100ms
#define SD_DATA_W_TIMEOUT             ((uint32_t)((180000000U / (0 + 2U) / 1000U) * 250U)) // Date write timeout is 250ms

// SD commands  index
#define SD_CMD_GO_IDLE_STATE          ((uint8_t)0U)
#define SD_CMD_SEND_OP_COND           ((uint8_t)1U)  // MMC only
#define SD_CMD_ALL_SEND_CID           ((uint8_t)2U)  // Not supported in SPI mode
#define SD_CMD_SEND_REL_ADDR          ((uint8_t)3U)  // Not supported in SPI mode
#define SD_CMD_SWITCH_FUNC            ((uint8_t)6U)
#define SD_CMD_SEL_DESEL_CARD         ((uint8_t)7U)  // Not supported in SPI mode
#define SD_CMD_HS_SEND_EXT_CSD        ((uint8_t)8U)
#define SD_CMD_SEND_CSD               ((uint8_t)9U)
#define SD_CMD_SEND_CID               ((uint8_t)10U)
#define SD_CMD_READ_DAT_UNTIL_STOP    ((uint8_t)11U) // Not supported in SPI mode
#define SD_CMD_STOP_TRANSMISSION      ((uint8_t)12U)
#define SD_CMD_SEND_STATUS            ((uint8_t)13U)
#define SD_CMD_GO_INACTIVE_STATE      ((uint8_t)15U) // Not supported in SPI mode
#define SD_CMD_SET_BLOCKLEN           ((uint8_t)16U)
#define SD_CMD_READ_SINGLE_BLOCK      ((uint8_t)17U)
#define SD_CMD_READ_MULT_BLOCK        ((uint8_t)18U)
#define SD_CMD_WRITE_DAT_UNTIL_STOP   ((uint8_t)20U) // Not supported in SPI mode
#define SD_CMD_WRITE_BLOCK            ((uint8_t)24U)
#define SD_CMD_WRITE_MULTIPLE_BLOCK   ((uint8_t)25U)
#define SD_CMD_PROG_CSD               ((uint8_t)27U)
#define SD_CMD_SET_WRITE_PROT         ((uint8_t)28U) // Not supported in SPI mode
#define SD_CMD_CLR_WRITE_PROT         ((uint8_t)29U) // Not supported in SPI mode
#define SD_CMD_SEND_WRITE_PROT        ((uint8_t)30U) // Not supported in SPI mode
#define SD_CMD_ERASE                  ((uint8_t)38U)
#define SD_CMD_LOCK_UNLOCK            ((uint8_t)42U)
#define SD_CMD_APP_CMD                ((uint8_t)55U)
#define SD_CMD_READ_OCR               ((uint8_t)58U) // Read OCR register
#define SD_CMD_CRC_ON_OFF             ((uint8_t)59U) // On/Off CRC check by SD Card (in SPI mode)
// Following commands are SD Card Specific commands.
// SD_CMD_APP_CMD should be sent before sending these commands.
#define SD_CMD_SET_BUS_WIDTH          ((uint8_t)6U)  // ACMD6
#define SD_CMD_SD_SEND_OP_COND        ((uint8_t)41U) // ACMD41
#define SD_CMD_SET_CLR_CARD_DETECT    ((uint8_t)42U) // ACMD42
#define SD_CMD_SEND_SCR               ((uint8_t)51U) // ACMD51
// Check pattern for R6 resp
#define SD_CHECK_PATTERN              ((uint32_t)0x000001AAU)
// Trials count for ACMD41
#define SD_ACMD41_TRIALS              ((uint32_t)0x0000FFFFU)
// Argument for ACMD41 to select voltage window
#define SD_OCR_VOLTAGE                ((uint32_t)0x80100000U)
// SDIO transfer flags
#define SDIO_XFER_COMMON_FLAGS        (SDIO_STA_DTIMEOUT | SDIO_STA_DCRCFAIL | SDIO_STA_STBITERR)
// SDIO flags for single block receive
#define SDIO_RX_SB_FLAGS              (SDIO_XFER_COMMON_FLAGS | SDIO_STA_DBCKEND | SDIO_STA_RXOVERR)
// SDIO flags for multiple block receive
#define SDIO_RX_MB_FLAGS              (SDIO_XFER_COMMON_FLAGS | SDIO_STA_DATAEND | SDIO_STA_RXOVERR)
// SDIO flags for single block transmit
#define SDIO_TX_SB_FLAGS              (SDIO_XFER_COMMON_FLAGS | SDIO_STA_DBCKEND | SDIO_STA_TXUNDERR)
// SDIO flags for multiple block transmit
#define SDIO_TX_MB_FLAGS              (SDIO_XFER_COMMON_FLAGS | SDIO_STA_DATAEND | SDIO_STA_TXUNDERR)
// SDIO transfer error flags
#define SDIO_XFER_ERROR_FLAGS         (SDIO_XFER_COMMON_FLAGS | SDIO_STA_TXUNDERR | SDIO_STA_RXOVERR)
// Card state (OCR[12:9] bits CURRENT_STATE)
#define SD_STATE_IDLE                 ((uint8_t)0x00U) // Idle
#define SD_STATE_READY                ((uint8_t)0x01U) // Ready
#define SD_STATE_IDENT                ((uint8_t)0x02U) // Identification
#define SD_STATE_STBY                 ((uint8_t)0x03U) // Stand-by
#define SD_STATE_TRAN                 ((uint8_t)0x04U) // Transfer
#define SD_STATE_DATA                 ((uint8_t)0x05U) // Sending data
#define SD_STATE_RCV                  ((uint8_t)0x06U) // Receive data
#define SD_STATE_PRG                  ((uint8_t)0x07U) // Programming
#define SD_STATE_DIS                  ((uint8_t)0x08U) // Disconnect
#define SD_STATE_ERROR                ((uint8_t)0xFFU) // Error or unknown state

SD::SD(){
	GPIO_CFG_t cmd = {
        .regs = 0,
        .port = 'D',
        .pins = (1<<2),
        .speed = GPIO_SPEED_25_50MHz,
        .mode = GPIO_MODE_AF,
        .af = 12
    };
	GPIO_CFG_t d0_d1_d2_d3_clk = {
        .regs = 0,
        .port = 'C',
        .pins = (1<<8) | (1<<9) | (1<<10) | (1<<11) | (1<<12),
        .speed = GPIO_SPEED_25_50MHz,
        .mode = GPIO_MODE_AF,
        .af = 12
    };
	gpio.push_back(GPIO(cmd));
	gpio.push_back(GPIO(d0_d1_d2_d3_clk));
	// SDIO Specifics
	cfg.regs = SDIO;
    cfg.wid = SD_WIDBUS_1; // Doesn't work for 4B intf :(
    cfg.clk_div_slow = ((SystemCoreClock / 4U) / 400000  ) - 2; // 400KHz = FCLK45USB_CLK / [CLKDIV + 2].
    cfg.clk_div_fast = ((SystemCoreClock / 4U) / 10000000) - 2; // 10MHz = FCLK45USB_CLK / [CLKDIV + 2].
}

void SD::init(){
	// Init GPIO
	for(auto &g : gpio) g.init();
	// Init RCC
	RCC->APB2ENR |= RCC_APB2ENR_SDIOEN;

	// Power On
	cfg.regs->POWER |= SDIO_POWER_PWRCTRL_Msk;

    // Config
    cfg.regs->CLKCR = cfg.wid << SDIO_CLKCR_WIDBUS_Pos;
    
    //cfg.regs->CLKCR |= cfg.clk_div_slow << SDIO_CLKCR_CLKDIV_Pos;
    cfg.regs->CLKCR |= SDIO_CLKCR_CLKDIV;
    cfg.regs->CLKCR |= SDIO_CLKCR_CLKEN;
    for (volatile int i = 0; i < 100000; i++); // Wait for clock to stabilize
}

inline SD_ERROR_t SD::sendCommand(uint32_t cmdIndex, uint32_t argument, SD_RESP_SIZE_t responseSize) {
    // Clear all previous flags
    cfg.regs->ICR = 0xFFFFFFFF;

    // Set command argument
    cfg.regs->ARG = argument;

    // Set command index and response type
    cfg.regs->CMD = (cmdIndex & SDIO_CMD_CMDINDEX) | responseSize | SDIO_CMD_CPSMEN;
    return SDR_Success;
}

// Check R1 response
// input:
//   cmd - the sent command
// return: SD_ERROR_t
inline SD_ERROR_t SD::getR1Resp(uint8_t cmd) {
	volatile uint32_t wait = SD_CMD_TIMEOUT;
	uint32_t respR1;

	// Wait for response, error or timeout
	while (!(cfg.regs->STA & (SDIO_STA_CCRCFAIL | SDIO_STA_CMDREND | SDIO_STA_CTIMEOUT)) && --wait);

	// Timeout?
	if ((cfg.regs->STA & SDIO_STA_CTIMEOUT) && (wait == 0)) {
		cfg.regs->ICR = SDIO_ICR_CTIMEOUTC;
		return SDR_Timeout;
	}

	// CRC fail?
	if (cfg.regs->STA & SDIO_STA_CCRCFAIL) {
		cfg.regs->ICR = SDIO_ICR_CCRCFAILC;
		return SDR_CRCError;
	}

	// Illegal command?
	if (cfg.regs->RESPCMD != cmd) {
		return SDR_IllegalCommand;
	}

	// Clear the static SDIO flags
	cfg.regs->ICR = SDIO_ICR_STATIC;

	// Get a R1 response and analyze it for errors
	respR1 = cfg.regs->RESP1;
	if (!(respR1 & SD_OCR_ALL_ERRORS))      { return SDR_Success;          }
	if (respR1 & SD_OCR_OUT_OF_RANGE)       { return SDR_AddrOutOfRange;   }
	if (respR1 & SD_OCR_ADDRESS_ERROR)      { return SDR_AddrMisaligned;   }
	if (respR1 & SD_OCR_BLOCK_LEN_ERROR)    { return SDR_BlockLenError;    }
	if (respR1 & SD_OCR_ERASE_SEQ_ERROR)    { return SDR_EraseSeqError;    }
	if (respR1 & SD_OCR_ERASE_PARAM)        { return SDR_EraseParam;       }
	if (respR1 & SD_OCR_WP_VIOLATION)       { return SDR_WPViolation;      }
	if (respR1 & SD_OCR_LOCK_UNLOCK_FAILED) { return SDR_LockUnlockFailed; }
	if (respR1 & SD_OCR_COM_CRC_ERROR)      { return SDR_ComCRCError;      }
	if (respR1 & SD_OCR_ILLEGAL_COMMAND)    { return SDR_IllegalCommand;   }
	if (respR1 & SD_OCR_CARD_ECC_FAILED)    { return SDR_CardECCFailed;    }
	if (respR1 & SD_OCR_CC_ERROR)           { return SDR_CCError;          }
	if (respR1 & SD_OCR_ERROR)              { return SDR_GeneralError;     }
	if (respR1 & SD_OCR_STREAM_R_UNDERRUN)  { return SDR_StreamUnderrun;   }
	if (respR1 & SD_OCR_STREAM_W_OVERRUN)   { return SDR_StreamOverrun;    }
	if (respR1 & SD_OCR_CSD_OVERWRITE)      { return SDR_CSDOverwrite;     }
	if (respR1 & SD_OCR_WP_ERASE_SKIP)      { return SDR_WPEraseSkip;      }
	if (respR1 & SD_OCR_CARD_ECC_DISABLED)  { return SDR_ECCDisabled;      }
	if (respR1 & SD_OCR_ERASE_RESET)        { return SDR_EraseReset;       }
	if (respR1 & SD_OCR_AKE_SEQ_ERROR)      { return SDR_AKESeqError;      }

	return SDR_Success;
}

// Check R2 response
// input:
//   pBuf - pointer to the data buffer to store the R2 response
// return: SD_ERROR_t value
inline SD_ERROR_t SD::getR2Resp(uint32_t *pBuf) {
	volatile uint32_t wait = SD_CMD_TIMEOUT;

	// Wait for response, error or timeout
	while (!(cfg.regs->STA & (SDIO_STA_CCRCFAIL | SDIO_STA_CMDREND | SDIO_STA_CTIMEOUT)) && --wait);

	// Timeout?
	if ((cfg.regs->STA & SDIO_STA_CTIMEOUT) && (wait == 0)) {
		cfg.regs->ICR = SDIO_ICR_CTIMEOUTC;
		return SDR_Timeout;
	}

	// CRC fail?
	if (cfg.regs->STA & SDIO_STA_CCRCFAIL) {
		cfg.regs->ICR = SDIO_ICR_CCRCFAILC;
		return SDR_CRCError;
	}

	// Clear the static SDIO flags
	cfg.regs->ICR = SDIO_ICR_STATIC;

	// SDIO_RESP[1..4] registers contains the R2 response
#ifdef __GNUC__
	// Use GCC built-in intrinsics (fastest, less code) (GCC v4.3 or later)
	*pBuf++ = __builtin_bswap32(cfg.regs->RESP1);
	*pBuf++ = __builtin_bswap32(cfg.regs->RESP2);
	*pBuf++ = __builtin_bswap32(cfg.regs->RESP3);
	*pBuf   = __builtin_bswap32(cfg.regs->RESP4);
#else
	// Use ARM 'REV' instruction (fast, a bit bigger code than GCC intrinsics)
	*pBuf++ = __REV(cfg.regs->RESP1);
	*pBuf++ = __REV(cfg.regs->RESP2);
	*pBuf++ = __REV(cfg.regs->RESP3);
	*pBuf   = __REV(cfg.regs->RESP4);
/*
	// Use SHIFT, AND and OR (slower, biggest code)
	*pBuf++ = SWAP_UINT32(cfg.regs->RESP1);
	*pBuf++ = SWAP_UINT32(cfg.regs->RESP2);
	*pBuf++ = SWAP_UINT32(cfg.regs->RESP3);
	*pBuf   = SWAP_UINT32(cfg.regs->RESP4);
*/
#endif

	return SDR_Success;
}

// Check R3 response
// return: SD_ERROR_t value
inline SD_ERROR_t SD::getR3Resp(void) {
	volatile uint32_t wait = SD_CMD_TIMEOUT;

	// Wait for response, error or timeout
	while (!(cfg.regs->STA & (SDIO_STA_CCRCFAIL | SDIO_STA_CMDREND | SDIO_STA_CTIMEOUT)) && --wait);

	// Timeout?
	if ((cfg.regs->STA & SDIO_STA_CTIMEOUT) && (wait == 0)) {
		cfg.regs->ICR = SDIO_ICR_CTIMEOUTC;
		return SDR_Timeout;
	}

	// Clear the static SDIO flags
	cfg.regs->ICR = SDIO_ICR_STATIC;

	return SDR_Success;
}

// Check R6 response (RCA)
// return: SD_ERROR_t value
inline SD_ERROR_t SD::getR6Resp(uint8_t cmd, uint16_t *pRCA) {
	volatile uint32_t wait = SD_CMD_TIMEOUT;
	uint32_t respR6;

	// Wait for response, error or timeout
	while (!(cfg.regs->STA & (SDIO_STA_CCRCFAIL | SDIO_STA_CMDREND | SDIO_STA_CTIMEOUT)) && --wait);

	// Timeout?
	if ((cfg.regs->STA & SDIO_STA_CTIMEOUT) && (wait == 0)) {
		cfg.regs->ICR = SDIO_ICR_CTIMEOUTC;
		return SDR_Timeout;
	}

	// CRC fail?
	if (cfg.regs->STA & SDIO_STA_CCRCFAIL) {
		cfg.regs->ICR = SDIO_ICR_CCRCFAILC;
		return SDR_CRCError;
	}

	// Illegal command?
	if (cfg.regs->RESPCMD != cmd) {
		return SDR_IllegalCommand;
	}

	// Clear the static SDIO flags
	cfg.regs->ICR = SDIO_ICR_STATIC;

	// Get a R6 response and analyze it for errors
	respR6 = cfg.regs->RESP1;
	if (!(respR6 & (SD_R6_ILLEGAL_CMD | SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_COM_CRC_FAILED))) {
		*pRCA = (uint16_t)(respR6 >> 16);
		return SDR_Success;
	}
	if (respR6 & SD_R6_GENERAL_UNKNOWN_ERROR) { return SDR_UnknownError;   }
	if (respR6 & SD_R6_ILLEGAL_CMD)           { return SDR_IllegalCommand; }
	if (respR6 & SD_R6_COM_CRC_FAILED)        { return SDR_ComCRCError;    }

	return SDR_Success;
}

// Check R7 response
// return: SD_ERROR_t value
inline SD_ERROR_t SD::getR7Resp(void) {
	volatile uint32_t wait = SD_CMD_TIMEOUT;

	// Wait for response, error or timeout
	while (!(cfg.regs->STA & (SDIO_STA_CCRCFAIL | SDIO_STA_CMDREND | SDIO_STA_CTIMEOUT)) && --wait);

	// Timeout?
	if ((cfg.regs->STA & SDIO_STA_CTIMEOUT) || (wait == 0)) {
		cfg.regs->ICR = SDIO_ICR_CTIMEOUTC;
		return SDR_Timeout;
	}

	// Clear command response received flag
	if (cfg.regs->STA & SDIO_STA_CMDREND) {
		cfg.regs->ICR = SDIO_ICR_CMDRENDC;
		return SDR_Success;
	}

	return SDR_NoResponse;
}

// Taken from https://github.com/LonelyWolf/stm32/tree/master/stm32l4-sdio/src
SD_ERROR_t SD::initCard(void){
    SD_ERROR_t err;
    volatile uint32_t wait;
    uint32_t sd_type = SD_STD_CAPACITY; // SD card capacity
    cardDef.Type = SDCT_UNKNOWN;

	// CMD0
	wait = SD_CMD_TIMEOUT;
	sendCommand(SD_CMD_GO_IDLE_STATE, 0x00, SD_RESP_NONE);
	while (!(cfg.regs->STA & (SDIO_STA_CTIMEOUT | SDIO_STA_CMDSENT)) && --wait);
	if ((cfg.regs->STA & SDIO_STA_CTIMEOUT) || !wait) {
		return SDR_Timeout;
	}

	// CMD8: SEND_IF_COND. Send this command to verify SD card interface operating condition
	// Argument: - [31:12]: Reserved (shall be set to '0')
	//           - [11:08]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
	//           - [07:00]: Check Pattern (recommended 0xAA)
	sendCommand(SD_CMD_HS_SEND_EXT_CSD, SD_CHECK_PATTERN, SD_RESP_SHORT); // CMD8
	err = getR7Resp();
	if (err == SDR_Success) {
		// SD v2.0 or later
		// Check echo-back of check pattern
		if ((cfg.regs->RESP1 & 0x01FF) != (SD_CHECK_PATTERN & 0x01FF)) {
			return SDR_Unsupported;
		}
		sd_type = SD_HIGH_CAPACITY; // SD v2.0 or later

		// Issue ACMD41 command
		wait = SD_ACMD41_TRIALS;
		while (--wait) {
			// Send leading command for ACMD<n> command
			sendCommand(SD_CMD_APP_CMD, 0, SD_RESP_SHORT); // CMD55 with RCA 0
			err = getR1Resp(SD_CMD_APP_CMD);
			if (err != SDR_Success) {
				return err;
			}

			// ACMD41 - initiate initialization process.
			// Set 3.0-3.3V voltage window (bit 20)
			// Set HCS bit (30) (Host Capacity Support) to inform card what host support high capacity
			// Set XPC bit (28) (SDXC Power Control) to use maximum performance (SDXC only)
			sendCommand(SD_CMD_SD_SEND_OP_COND, SD_OCR_VOLTAGE | sd_type, SD_RESP_SHORT);
			err = getR3Resp();
			if (err != SDR_Success) {
				return err;
			}
			if (cfg.regs->RESP1 & (1 << 31)) {
				// The SD card has finished the power-up sequence
				break;
			}
		}
		if (wait == 0) {
			// Unsupported voltage range
			return SDR_InvalidVoltage;
		}

		// This is SDHC/SDXC card?
		cardDef.Type = (cfg.regs->RESP1 & SD_HIGH_CAPACITY) ? SDCT_SDHC : SDCT_SDSC_V2;
	} else if (err == SDR_Timeout) {
		// SD v1.x or MMC
		// Issue CMD55 to reset 'Illegal command' bit of the SD card
		sendCommand(SD_CMD_APP_CMD, 0, SD_RESP_SHORT); // CMD55 with RCA 0
		getR1Resp(SD_CMD_APP_CMD);

		// Issue ACMD41 command with zero argument
		wait = SD_ACMD41_TRIALS;
		while (--wait) {
			// Send leading command for ACMD<n> command
			sendCommand(SD_CMD_APP_CMD, 0, SD_RESP_SHORT); // CMD55 with RCA 0
			err = getR1Resp(SD_CMD_APP_CMD);
			if (err != SDR_Success) {
				return err;
			}

			// Send ACMD41 - initiate initialization process (bit HCS = 0)
			sendCommand(SD_CMD_SD_SEND_OP_COND, SD_OCR_VOLTAGE, SD_RESP_SHORT); // ACMD41
			err = getR3Resp();
			if (err == SDR_Timeout) {
				// MMC will not respond to this command
				break;
			}
			if (err != SDR_Success) {
				return err;
			}
			if (cfg.regs->RESP1 & (1 << 31)) {
				// The SD card has finished the power-up sequence
				break;
			}
		}
		if (wait == 0) {
			// Unknown/Unsupported card type
			return SDR_UnknownCard;
		}
		if (err != SDR_Timeout) {
			// SD v1.x
			cardDef.Type = SDCT_SDSC_V1; // SDv1
		} else {
			// MMC or not SD memory card

			wait = SD_ACMD41_TRIALS;
			while (--wait) {
				// Issue CMD1: initiate initialization process.
                
				sendCommand(SD_CMD_SEND_OP_COND, SD_OCR_VOLTAGE, SD_RESP_SHORT); // CMD1
				err = getR3Resp();
				if (err != SDR_Success) {
					return err;
				}
				if (cfg.regs->RESP1 & (1 << 31)) {
					// The SD card has finished the power-up sequence
					break;
				}
			}
			if (wait == 0) {
				return SDR_UnknownCard;
			}
			cardDef.Type = SDCT_MMC; // MMC
		}
	} else {
		return err;
	}

	// Now the CMD2 and CMD3 commands should be issued in cycle until timeout to enumerate all cards on the bus
	// Since this module suitable to work with single card, issue this commands one time only

	// Send ALL_SEND_CID command
	sendCommand(SD_CMD_ALL_SEND_CID, 0, SD_RESP_LONG); // CMD2
	err = getR2Resp((uint32_t *)cardDef.CID); // response is a value of the CID/CSD register
	if (err != SDR_Success) {
		return err;
	}

	if (cardDef.Type != SDCT_MMC) {
		// Send SEND_REL_ADDR command to ask the SD card to publish a new RCA (Relative Card Address)
		// Once the RCA is received the card state changes to the stand-by state
		sendCommand(SD_CMD_SEND_REL_ADDR, 0, SD_RESP_SHORT); // CMD3
		err = getR6Resp(SD_CMD_SEND_REL_ADDR, (uint16_t *)(&cardDef.RCA));
		if (err != SDR_Success) {
			return err;
		}
	} else {
		////////////////////////////////////////////////////////////////
		// This part has not been tested due to lack of MMCmicro card //
		////////////////////////////////////////////////////////////////

		// For MMC card host should set a RCA value to the card by SET_REL_ADDR command
		sendCommand(SD_CMD_SEND_REL_ADDR, SDIO_MMC_RCA << 16, SD_RESP_SHORT); // CMD3
		err = getR1Resp(SD_CMD_SEND_REL_ADDR);
		if (err != SDR_Success) {
			return err;
		}
		cardDef.RCA = SDIO_MMC_RCA;
	}

	// Send SEND_CSD command to retrieve CSD register from the card
	sendCommand(SD_CMD_SEND_CSD, cardDef.RCA << 16, SD_RESP_LONG); // CMD9
	err = getR2Resp((uint32_t *)cardDef.CSD);
	if (err != SDR_Success) {
		return err;
	}

	// Parse the values of CID and CSD registers
	getCardInfo();

	// Now card must be in stand-by mode, from this point it is possible to increase bus speed
	setFastClock();

	// Put the SD card to the transfer mode
	sendCommand(SD_CMD_SEL_DESEL_CARD, cardDef.RCA << 16, SD_RESP_SHORT); // CMD7
	err = getR1Resp(SD_CMD_SEL_DESEL_CARD); // In fact R1b response here
	if (err != SDR_Success) {
		return err;
	}

	// Disable the pull-up resistor on CD/DAT3 pin of card
	// Send leading command for ACMD<n> command
    
	sendCommand(SD_CMD_APP_CMD, cardDef.RCA << 16, SD_RESP_SHORT); // CMD55
	err = getR1Resp(SD_CMD_APP_CMD);
	if (err != SDR_Success) {
		return err;
	}
	// Send SET_CLR_CARD_DETECT command
	sendCommand(SD_CMD_SET_CLR_CARD_DETECT, 0, SD_RESP_SHORT); // ACMD42
	err = getR1Resp(SD_CMD_SET_CLR_CARD_DETECT);
	if (err != SDR_Success) {
		return err;
	}


	// Read the SCR register
	if (cardDef.Type != SDCT_MMC) {
		// MMC card doesn't support this feature
		// Warning: this function set block size to 8 bytes
		getSCR((uint32_t *)cardDef.SCR);
	}

	// For SDv1, SDv2 and MMC card must set block size
	// The SDHC/SDXC always have fixed block size (512 bytes)

	if ((cardDef.Type == SDCT_SDSC_V1) || (cardDef.Type == SDCT_SDSC_V2) || (cardDef.Type == SDCT_MMC)) {
		sendCommand(SD_CMD_SET_BLOCKLEN, 512, SD_RESP_SHORT); // CMD16
		err = getR1Resp(SD_CMD_SET_BLOCKLEN);
		if (err != SDR_Success) {
			return SDR_SetBlockSizeFailed;
		}
	}

	return SDR_Success;
}


// Parse information about specific card
// note: CSD/CID register values already must be in the cardDef structure
inline void SD::getCardInfo(void) {
	uint32_t dev_size;
	uint32_t dev_size_mul;

	// Parse the CSD register
	cardDef.CSDVer = cardDef.CSD[0] >> 6; // CSD version
	if (cardDef.Type != SDCT_MMC) {
		// SD
		cardDef.MaxBusClkFreq = cardDef.CSD[3];
		if (cardDef.CSDVer == 0) {
			// CSD v1.00 (SDSCv1, SDSCv2)
			dev_size  = (uint32_t)(cardDef.CSD[6] & 0x03) << 10; // Device size
			dev_size |= (uint32_t)cardDef.CSD[7] << 2;
			dev_size |= (cardDef.CSD[8] & 0xc0) >> 6;
			dev_size_mul  = (cardDef.CSD[ 9] & 0x03) << 1; // Device size multiplier
			dev_size_mul |= (cardDef.CSD[10] & 0x80) >> 7;
			cardDef.BlockCount  = dev_size + 1;
			cardDef.BlockCount *= 1 << (dev_size_mul + 2);
			cardDef.BlockSize   = 1 << (cardDef.CSD[5] & 0x0f); // Maximum read data block length
		} else {
			// CSD v2.00 (SDHC, SDXC)
			dev_size  = (cardDef.CSD[7] & 0x3f) << 16;
			dev_size |=  cardDef.CSD[8] << 8;
			dev_size |=  cardDef.CSD[9]; // C_SIZE
			cardDef.BlockSize = 512;
			cardDef.BlockCount = dev_size + 1;
			// BlockCount >= 65535 means that this is SDXC card
		}
		cardDef.Capacity = cardDef.BlockCount * cardDef.BlockSize;
	} else {
		// MMC
		cardDef.MaxBusClkFreq = cardDef.CSD[3];
		dev_size  = (uint32_t)(cardDef.CSD[6] & 0x03) << 8; // C_SIZE
		dev_size += (uint32_t)cardDef.CSD[7];
		dev_size <<= 2;
		dev_size += cardDef.CSD[8] >> 6;
		cardDef.BlockSize = 1 << (cardDef.CSD[5] & 0x0f); // MMC read block length
		dev_size_mul = ((cardDef.CSD[9] & 0x03) << 1) + ((cardDef.CSD[10] & 0x80) >> 7);
		cardDef.BlockCount = (dev_size + 1) * (1 << (dev_size_mul + 2));
		cardDef.Capacity = cardDef.BlockCount * cardDef.BlockSize;
	}

	// Parse the CID register
	if (cardDef.Type != SDCT_MMC) {
		// SD card
		cardDef.MID = cardDef.CID[0];
		cardDef.OID = (cardDef.CID[1] << 8) | cardDef.CID[2];
		cardDef.PNM[0] = cardDef.CID[3];
		cardDef.PNM[1] = cardDef.CID[4];
		cardDef.PNM[2] = cardDef.CID[5];
		cardDef.PNM[3] = cardDef.CID[6];
		cardDef.PNM[4] = cardDef.CID[7];
		cardDef.PRV = cardDef.CID[8];
		cardDef.PSN = (cardDef.CID[9] << 24) | (cardDef.CID[10] << 16) | (cardDef.CID[11] << 8) | cardDef.CID[12];
		cardDef.MDT = ((cardDef.CID[13] << 8) | cardDef.CID[14]) & 0x0fff;
	} else {
		// MMC
		cardDef.MID = 0x00;
		cardDef.OID = 0x0000;
		cardDef.PNM[0] = '*';
		cardDef.PNM[1] = 'M';
		cardDef.PNM[2] = 'M';
		cardDef.PNM[3] = 'C';
		cardDef.PNM[4] = '*';
		cardDef.PRV = 0;
		cardDef.PSN = 0x00000000;
		cardDef.MDT = 0x0000;
	}
}

inline void SD::setFastClock() {
	uint32_t clk;
	clk  = cfg.regs->CLKCR;
	clk &= ~SDIO_CLKCR_CLKDIV;
	clk |= (cfg.clk_div_fast & SDIO_CLKCR_CLKDIV);
	cfg.regs->CLKCR = clk;
}

// Retrieve the SD card SCR register value
// input:
//   pSCR - pointer to the buffer for SCR register (8 bytes)
// return: SD_ERROR_t value
// note: card must be in transfer mode, not supported by MMC

inline SD_ERROR_t SD::getSCR(uint32_t *pSCR) {
	SD_ERROR_t cmd_res;

	// Set block size to 8 bytes
	sendCommand(SD_CMD_SET_BLOCKLEN, 8, SD_RESP_SHORT); // CMD16
	cmd_res = getR1Resp(SD_CMD_SET_BLOCKLEN);
	if (cmd_res != SDR_Success) {
		return cmd_res;
	}

	// Send leading command for ACMD<n> command
	sendCommand(SD_CMD_APP_CMD, cardDef.RCA << 16, SD_RESP_SHORT); // CMD55
	cmd_res = getR1Resp(SD_CMD_APP_CMD);
	if (cmd_res != SDR_Success) {
		return cmd_res;
	}

	// Clear the data flags
	cfg.regs->ICR = SDIO_ICR_DATA;

	// Configure the SDIO data transfer
	cfg.regs->DTIMER = SD_DATA_R_TIMEOUT; // Data read timeout
	cfg.regs->DLEN   = 8; // Data length in bytes
	// Data transfer:
	//   - type: block
	//   - direction: card -> controller
	//   - size: 2^3 = 8bytes
	//   - DPSM: enabled
	cfg.regs->DCTRL  = SDIO_DCTRL_DTDIR | (3 << 4) | SDIO_DCTRL_DTEN;

	// Send SEND_SCR command
	sendCommand(SD_CMD_SEND_SCR, 0, SD_RESP_SHORT); // ACMD51
	cmd_res = getR1Resp(SD_CMD_SEND_SCR);
	if (cmd_res != SDR_Success) {
		return cmd_res;
	}

	// Receive the SCR register value
	while (!(cfg.regs->STA & (SDIO_STA_RXOVERR | SDIO_STA_DCRCFAIL | SDIO_STA_DTIMEOUT | SDIO_STA_DBCKEND | SDIO_STA_STBITERR))) {//SDIO_STA_STBITERR
		// Read word when data available in receive FIFO
		if (cfg.regs->STA & SDIO_STA_RXDAVL) *pSCR++ = cfg.regs->FIFO;
	}

	// Check for errors
	if (cfg.regs->STA & (SDIO_STA_DTIMEOUT | SDIO_STA_DCRCFAIL | SDIO_STA_RXOVERR | SDIO_STA_STBITERR)) {//SDIO_STA_STBITERR
		if (cfg.regs->STA & SDIO_STA_DTIMEOUT) { cmd_res = SDR_DataTimeout;   }
		if (cfg.regs->STA & SDIO_STA_DCRCFAIL) { cmd_res = SDR_DataCRCFail;   }
		if (cfg.regs->STA & SDIO_STA_RXOVERR)  { cmd_res = SDR_RXOverrun;     }
		if (cfg.regs->STA & SDIO_STA_STBITERR) { cmd_res = SDR_StartBitError; }//SDIO_STA_STBITERR
	}

	// Clear the static SDIO flags
	cfg.regs->ICR = SDIO_ICR_STATIC;

	return cmd_res;
}

// Read block of data from the SD card
// input:
//   addr - address of the block to be read
//   pBuf - pointer to the buffer that will contain the received data
//   length - buffer length (must be multiple of 512 bytes)
// return: SDResult value
SD_ERROR_t SD::readBlock(uint32_t addr, uint32_t *pBuf, uint32_t length) {
	SD_ERROR_t cmd_res = SDR_Success;
	uint32_t blk_count = length >> 9; // Sectors in block
	register uint32_t STA; // to speed up SDIO flags checking
	register uint32_t STA_mask; // mask for SDIO flags checking

	// Initialize the data control register
	SDIO->DCTRL = 0;

	// SDSC card uses byte unit address and
	// SDHC/SDXC cards use block unit address (1 unit = 512 bytes)
	// For SDHC card addr must be converted to block unit address
	if (cardDef.Type == SDCT_SDHC) {
		addr >>= 9;
	}

	// Clear the static SDIO flags
	SDIO->ICR = SDIO_ICR_STATIC;

	if (blk_count > 1) {
		// Prepare bit checking variable for multiple block transfer
		STA_mask = SDIO_RX_MB_FLAGS;
		// Send READ_MULT_BLOCK command
		sendCommand(SD_CMD_READ_MULT_BLOCK, addr, SD_RESP_SHORT); // CMD18
		cmd_res = getR1Resp(SD_CMD_READ_MULT_BLOCK);
	} else {
		// Prepare bit checking variable for single block transfer
		STA_mask = SDIO_RX_SB_FLAGS;
		// Send READ_SINGLE_BLOCK command
		sendCommand(SD_CMD_READ_SINGLE_BLOCK, addr, SD_RESP_SHORT); // CMD17
		cmd_res = getR1Resp(SD_CMD_READ_SINGLE_BLOCK);
	}
	if (cmd_res != SDR_Success) {
		return cmd_res;
	}

	// Data read timeout
	SDIO->DTIMER = SD_DATA_R_TIMEOUT;
	// Data length
	SDIO->DLEN   = length;
	// Data transfer:
	//   transfer mode: block
	//   direction: to card
	//   DMA: disabled
	//   block size: 2^9 = 512 bytes
	//   DPSM: enabled
	SDIO->DCTRL  = SDIO_DCTRL_DTDIR | (9 << 4) | SDIO_DCTRL_DTEN;

	// Receive a data block from the SDIO
	// ----> TIME CRITICAL SECTION BEGIN <----
	do {
		STA = SDIO->STA;
		if (STA & SDIO_STA_RXFIFOHF) {
			// The receive FIFO is half full, there are at least 8 words in it
			*pBuf++ = SDIO->FIFO;
			*pBuf++ = SDIO->FIFO;
			*pBuf++ = SDIO->FIFO;
			*pBuf++ = SDIO->FIFO;
			*pBuf++ = SDIO->FIFO;
			*pBuf++ = SDIO->FIFO;
			*pBuf++ = SDIO->FIFO;
			*pBuf++ = SDIO->FIFO;
		}
	} while (!(STA & STA_mask));
	// <---- TIME CRITICAL SECTION END ---->

	// Send stop transmission command in case of multiple block transfer
	if ((cardDef.Type != SDCT_MMC) && (blk_count > 1)) {
		cmd_res = stopTransfer();
	}

	// Check for errors
	if (STA & SDIO_XFER_ERROR_FLAGS) {
		if (STA & SDIO_STA_DTIMEOUT) cmd_res = SDR_DataTimeout;
		if (STA & SDIO_STA_DCRCFAIL) cmd_res = SDR_DataCRCFail;
		if (STA & SDIO_STA_RXOVERR)  cmd_res = SDR_RXOverrun;
		if (STA & SDIO_STA_STBITERR) cmd_res = SDR_StartBitError;
	}

	// Read the data remnant from RX FIFO (if there is still any data)
	while (SDIO->STA & SDIO_STA_RXDAVL) {
		*pBuf++ = SDIO->FIFO;
	}

	// Clear the static SDIO flags
	SDIO->ICR = SDIO_ICR_STATIC;

	return cmd_res;
}

// Write block of data to the SD card
// input:
//   addr - address of the block to be written
//   pBuf - pointer to the buffer that will contain the received data
//   length - buffer length (must be multiple of 512)
// return: SDResult value
SD_ERROR_t SD::writeBlock(uint32_t addr, uint32_t *pBuf, uint32_t length) {
	SD_ERROR_t cmd_res = SDR_Success;
	uint32_t blk_count = length >> 9; // Sectors in block
	uint32_t STA; // To speed up SDIO flags checking
	register uint32_t STA_mask; // Mask for SDIO flags checking
	uint32_t data_sent = 0; // Counter of transferred bytes
	uint8_t card_state; // Card state

	// Initialize the data control register
	SDIO->DCTRL = 0;

	// SDSC card uses byte unit address and
	// SDHC/SDXC cards use block unit address (1 unit = 512 bytes)
	// For SDHC card addr must be converted to block unit address
	if (cardDef.Type == SDCT_SDHC) {
		addr >>= 9;
	}

	if (blk_count > 1) {
		// Prepare bit checking variable for multiple block transfer
		STA_mask = SDIO_TX_MB_FLAGS;
		// Send WRITE_MULTIPLE_BLOCK command
		sendCommand(SD_CMD_WRITE_MULTIPLE_BLOCK, addr, SD_RESP_SHORT); // CMD25
		cmd_res = getR1Resp(SD_CMD_WRITE_MULTIPLE_BLOCK);
	} else {
		// Prepare bit checking variable for single block transfer
		STA_mask = SDIO_TX_SB_FLAGS;
		// Send WRITE_BLOCK command
		sendCommand(SD_CMD_WRITE_BLOCK, addr, SD_RESP_SHORT); // CMD24
		cmd_res = getR1Resp(SD_CMD_WRITE_BLOCK);
	}
	if (cmd_res != SDR_Success) {
		return cmd_res;
	}

	// Clear the static SDIO flags
	SDIO->ICR = SDIO_ICR_STATIC;

	// Data write timeout
	SDIO->DTIMER = SD_DATA_W_TIMEOUT;
	// Data length
	SDIO->DLEN = length;
	// Data transfer:
	//   transfer mode: block
	//   direction: to card
	//   DMA: disabled
	//   block size: 2^9 = 512 bytes
	//   DPSM: enabled
	SDIO->DCTRL = (9 << 4) | SDIO_DCTRL_DTEN;

	// Transfer data block to the SDIO
	// ----> TIME CRITICAL SECTION BEGIN <----
	if (!(length & 0x1F)) {
		// The block length is multiple of 32, simplified transfer procedure can be used
		do {
			if ((SDIO->STA & SDIO_STA_TXFIFOHE) && (data_sent < length)) {
				// The TX FIFO is half empty, at least 8 words can be written
				SDIO->FIFO = *pBuf++;
				SDIO->FIFO = *pBuf++;
				SDIO->FIFO = *pBuf++;
				SDIO->FIFO = *pBuf++;
				SDIO->FIFO = *pBuf++;
				SDIO->FIFO = *pBuf++;
				SDIO->FIFO = *pBuf++;
				SDIO->FIFO = *pBuf++;
				data_sent += 32;
			}
		} while (!(SDIO->STA & STA_mask));
	} else {
		// Since the block length is not a multiple of 32, it is necessary to apply additional calculations
		do {
			if ((SDIO->STA & SDIO_STA_TXFIFOHE) && (data_sent < length)) {
				// TX FIFO half empty, at least 8 words can be written
				uint32_t data_left = length - data_sent;
				if (data_left < 32) {
					// Write last portion of data to the TX FIFO
					data_left = ((data_left & 0x03) == 0) ? (data_left >> 2) : ((data_left >> 2) + 1);
					data_sent += data_left << 2;
					while (data_left--) {
						SDIO->FIFO = *pBuf++;
					}
				} else {
					// Write 8 words to the TX FIFO
					SDIO->FIFO = *pBuf++;
					SDIO->FIFO = *pBuf++;
					SDIO->FIFO = *pBuf++;
					SDIO->FIFO = *pBuf++;
					SDIO->FIFO = *pBuf++;
					SDIO->FIFO = *pBuf++;
					SDIO->FIFO = *pBuf++;
					SDIO->FIFO = *pBuf++;
					data_sent += 32;
				}
			}
		} while (!(SDIO->STA & STA_mask));
	}
	// <---- TIME CRITICAL SECTION END ---->

	// Save STA register value for further analysis
	STA = SDIO->STA;

	// Send stop transmission command in case of multiple block transfer
	if ((cardDef.Type != SDCT_MMC) && (blk_count > 1)) {
		cmd_res = stopTransfer();
	}

	// Check for errors
	if (STA & SDIO_XFER_ERROR_FLAGS) {
		if (STA & SDIO_STA_DTIMEOUT) cmd_res = SDR_DataTimeout;
		if (STA & SDIO_STA_DCRCFAIL) cmd_res = SDR_DataCRCFail;
		if (STA & SDIO_STA_TXUNDERR) cmd_res = SDR_TXUnderrun;
		if (STA & SDIO_STA_STBITERR) cmd_res = SDR_StartBitError;
	}

	// Wait while the card is in programming state
	do {
		if (getCardState(&card_state) != SDR_Success) {
			break;
		}
	} while ((card_state == SD_STATE_PRG) || (card_state == SD_STATE_RCV));

	// Clear the static SDIO flags
	SDIO->ICR = SDIO_ICR_STATIC;

	return cmd_res;
}

// Abort an ongoing data transfer
// return: SDResult value
inline SD_ERROR_t SD::stopTransfer(void) {
	// Send STOP_TRANSMISSION command
	sendCommand(SD_CMD_STOP_TRANSMISSION, 0, SD_RESP_SHORT); // CMD12

	return getR1Resp(SD_CMD_STOP_TRANSMISSION);
}

// Get current SD card state
// input:
//   pState - pointer to the variable for current card state, one of SD_STATE_xx values
// return: SD_ERROR_t value
inline SD_ERROR_t SD::getCardState(uint8_t *pState) {
	SD_ERROR_t cmd_res;

	// Send SEND_STATUS command
	sendCommand(SD_CMD_SEND_STATUS, cardDef.RCA << 16, SD_RESP_SHORT); // CMD13
	cmd_res = getR1Resp(SD_CMD_SEND_STATUS);
	if (cmd_res != SDR_Success) {
		*pState = SD_STATE_ERROR;
		return cmd_res;
	}

	// Find out a card status
	*pState = (cfg.regs->RESP1 & 0x1e00) >> 9;

	// Check for errors
	return SDR_Success;
}