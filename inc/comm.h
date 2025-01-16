#ifndef _COMM_H_
#define _COMM_H_

#include "stm8s.h"
#include "stm8s_uart1.h"
#include "bq28z610.h"

#define RX_BUFFER_SIZE 16

extern uint8_t uart_data;
extern uint8_t ifGetData;

extern volatile uint8_t g_RxCount;
extern volatile uint8_t timeToParseData;
extern volatile char g_RxBuffer[];

void handle_comm(uint8_t data);

void handle_comm_multi_bytes(void);

void Reset_Inactivity_Timer(void);

void Enter_LowPower_Mode(void);

void send_i2c_error(void);

#endif
