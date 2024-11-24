#ifndef TIM_HPP
#define TIM_HPP
#include "drivers_common.hpp"
/* Enums */
typedef enum TIM_MMS{
	TIM_MMS_RST = 0,
	TIM_MMS_EN,
	TIM_MMS_UP,
	TIM_MMS_COMP_PULSE,
	TIM_MMS_OC1REF,
	TIM_MMS_OC2REF,
	TIM_MMS_OC3REF,
	TIM_MMS_OC4REF
}TIM_MMS_t;
typedef enum TIM_TS{
	TIM_TS_ITR0 = 0,
	TIM_TS_ITR1,
	TIM_TS_ITR2,
	TIM_TS_ITR3,
	TIM_TS_ED,
	TIM_TS_FILT1,
	TIM_TS_FILT2,
	TIM_TS_ETRF
}TIM_TS_t;
typedef enum TIM_SMS{
	TIM_SMS_DIS = 0,
	TIM_SMS_ENC1,
	TIM_SMS_ENC2,
	TIM_SMS_ENC3,
	TIM_SMS_RST,
	TIM_SMS_GATED,
	TIM_SMS_TRIG,
	TIM_SMS_EXT
}TIM_SMS_t;
typedef enum TIM_CCDS{
	TIM_CCDS_CCX = 0,
	TIM_CCDS_UP
}TIM_CCDS_t;
/* Structs */
typedef struct __attribute__((__packed__)){
	TIM_TypeDef *		regs;
	uint32_t 			n;
	uint32_t			psc;
	uint32_t			arr;
	uint32_t			urs;
	uint32_t			uie;
	TIM_MMS_t			mms;
	TIM_CCDS_t			ccds;
	TIM_TS_t			ts;
	TIM_SMS_t			sms;
}TIM_CFG_t;

class TIM : public Driver
{
private:
	/* data */
public:
	TIM_CFG_t cfg;
	TIM() : Driver(){};
	TIM(TIM_CFG_t config) : cfg(config){};
	void init();
	void deinit(){};
};

#endif
