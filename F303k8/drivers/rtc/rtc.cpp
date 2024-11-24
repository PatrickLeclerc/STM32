#include "rtc.hpp"

void RTCLOCK::init()
{
	/* PWR */
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CR |= PWR_CR_DBP;
	while(!(PWR->CR & PWR_CR_DBP)){}
	/* LSI */
	//RCC->CSR |=  RCC_CSR_LSION;
	//while(!(RCC->CSR & RCC_CSR_LSIRDY)){}
	
	///* LSE */
	//RCC->BDCR |=  RCC_BDCR_LSEON;
	//while(!(RCC->BDCR & RCC_BDCR_LSERDY)){}
	
	/* RCC */
	RCC->BDCR |= RCC_BDCR_RTCSEL_HSE | (31 << RCC_BDCR_RTCSEL_Pos);
	RCC->BDCR |= RCC_BDCR_BDRST; //Reset for RCC_BDCR_RTCSEL to be applied
	RCC->BDCR &= ~RCC_BDCR_BDRST;
	RCC->BDCR |= RCC_BDCR_RTCSEL_0 | RCC_BDCR_RTCSEL_1;
	RCC->BDCR |= RCC_BDCR_RTCEN;
	
	/* RTC */
	//Key
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;	
	
	///Init
	RTC->ISR |= RTC_ISR_INIT;
	while(!(RTC->ISR & RTC_ISR_INITF)){}
	
	//Prescaler
	RTC->PRER = (7999<<RTC_PRER_PREDIV_S_Pos);
	RTC->PRER |= (124<<RTC_PRER_PREDIV_A_Pos);
		
	//Adjust time and date
	uint32_t ht = __TIME__[0] - '0';
	uint32_t hu = __TIME__[1] - '0';
	uint32_t mt = __TIME__[3] - '0';
	uint32_t mu = __TIME__[4] - '0';
	uint32_t st = __TIME__[6] - '0';
	uint32_t su = __TIME__[7] - '0';
	RTC->TR = (ht << RTC_TR_HT_Pos) | (hu << RTC_TR_HU_Pos) | (mt << RTC_TR_MNT_Pos) | (mu << RTC_TR_MNU_Pos) | (st << RTC_TR_ST_Pos) | (su << RTC_TR_SU_Pos);
	
	//RTC shadow register
	//RTC->CR |= RTC_CR_BYPSHAD;
	//Start RTC
	RTC->ISR &= ~RTC_ISR_INIT;
	RTC->WPR = 0x00;
}

//time is 8 chars
void RTCLOCK::get_bcd(char* time)
{
	time[0] = '0' + ((RTC->TR & RTC_TR_HT_Msk) >> RTC_TR_HT_Pos);
	time[1] = '0' + ((RTC->TR & RTC_TR_HU_Msk) >> RTC_TR_HU_Pos);
	time[2] = ':';
	time[3] = '0' + ((RTC->TR & RTC_TR_MNT_Msk) >> RTC_TR_MNT_Pos);
	time[4] = '0' + ((RTC->TR & RTC_TR_MNU_Msk) >> RTC_TR_MNU_Pos);
	time[5] = ':';
	time[6] = '0' + ((RTC->TR & RTC_TR_ST_Msk) >> RTC_TR_ST_Pos);
	time[7] = '0' + ((RTC->TR & RTC_TR_SU_Msk) >> RTC_TR_SU_Pos);
}

/*
    time[0] <- H
    time[1] <- M
    time[2] <- S
*/
void RTCLOCK::get_num(int* time)
{
	time[0] = ((RTC->TR & RTC_TR_HT_Msk) >> RTC_TR_HT_Pos)*10 + ((RTC->TR & RTC_TR_HU_Msk) >> RTC_TR_HU_Pos);
	time[1] = ((RTC->TR & RTC_TR_MNT_Msk) >> RTC_TR_MNT_Pos)*10 + ((RTC->TR & RTC_TR_MNU_Msk) >> RTC_TR_MNU_Pos);
	time[2] = ((RTC->TR & RTC_TR_ST_Msk) >> RTC_TR_ST_Pos)*10 + ((RTC->TR & RTC_TR_SU_Msk) >> RTC_TR_SU_Pos);
}