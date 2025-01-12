#ifndef _MAIN_H__
#define _MAIN_H__   

#include "stm8s.h"
#include "i2c_driver.h"
#include "bq28z610.h"
#include "stm8s_uart1.h"
#include "stm8s_clk.h"
#include "comm.h"
#include "key.h"
#include "stm8s_tim2.h"
#include "stm8s_exti.h"
#include "stm8s_gpio.h"
#include "stm8s_awu.h"

extern uint8_t inactivity_second;

void Enter_LowPower_Mode(void);

#endif