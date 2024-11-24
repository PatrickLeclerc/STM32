#include "wait.hpp"

Wait::Wait(uint32_t tim_n)
{
    TIM_CFG_t t={
	    .regs = 0,
	    .n = tim_n,
	    .psc = SystemCoreClock / 1000000 - 1,
	    .arr = 0,
	    .urs = 0,
	    .uie = 0,
	    .mms = TIM_MMS_RST,
	    .ccds = TIM_CCDS_CCX,
	    .ts = TIM_TS_ITR0,
	    .sms = TIM_SMS_DIS,
	    .opm = 1,
        .en = 0
    };
    tim = TIM(t);
}

void Wait::init(){
    tim.init();
}

void Wait::deinit(){
	tim.deinit();
}

void Wait::mswait(uint32_t delay)
{
    tim.cfg.regs->CR1 &= ~TIM_CR1_CEN;
	tim.cfg.regs->SR =0U;
	tim.cfg.regs->CNT =0U;
	tim.cfg.regs->PSC = 71999U;
	tim.cfg.regs->ARR = delay-1U;
	tim.cfg.regs->EGR |= TIM_EGR_UG;
	tim.cfg.regs->CR1 |= TIM_CR1_CEN;
	while(!(tim.cfg.regs->SR & TIM_SR_UIF)){}
}

void Wait::uwait(uint32_t delay)
{
    tim.cfg.regs->CR1 &= ~TIM_CR1_CEN;
	tim.cfg.regs->SR =0U;
	tim.cfg.regs->CNT =0U;
	tim.cfg.regs->PSC = 71U;
	tim.cfg.regs->ARR = delay-1U;
	tim.cfg.regs->EGR |= TIM_EGR_UG;
	tim.cfg.regs->CR1 |= TIM_CR1_CEN;
	while(!(tim.cfg.regs->SR & TIM_SR_UIF)){}
}
