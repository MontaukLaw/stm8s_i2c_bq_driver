#ifndef _KEY_H_
#define _KEY_H_

#include "stm8s.h"
#include "stm8s_gpio.h"

#define LONG_PRESS_TIME 10000
#define CLICKED_COUNTER 8

#define SHORT_PRESS_COUNTER_MIN 30
#define SHORT_PRESS_COUNTER_MAX 100

#define KEY_BUTTON_PORT             GPIOA
#define KEY_BUTTON_PIN              GPIO_PIN_1

void key_process(void);

void key_init(void);

#endif