#ifndef _UTILS_H_
#define _UTILS_H_

#include "stm8s.h"
#include "bq28z610.h"

// #define byte uint8_t
// #define bool uint8_t
// #define int uint32_t

bool validate(uint8_t *data);

void Delay(__IO uint32_t nCount);

uint16_t composeWord(uint8_t *buf, uint32_t lsbIndex, uint8_t littleEndian);

uint32_t LSIMeasurment(void);

void Soft_Reset(void);

#endif
