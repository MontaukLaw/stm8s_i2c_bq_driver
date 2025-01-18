#ifndef _I2C_DRIVER_H_
#define _I2C_DRIVER_H_

#include "stm8s.h"
#include "stm8s_i2c.h"
#include "utils.h"
#include "comm.h"

#define sEE_I2C                          I2C  
#define sEE_I2C_CLK                      CLK_PERIPHERAL_I2C
#define sEE_I2C_SCL_PIN                  GPIO_PIN_1                  /* PC.01 */
#define sEE_I2C_SCL_GPIO_PORT            GPIOE                       /* GPIOE */
#define sEE_I2C_SDA_PIN                  GPIO_PIN_2                  /* PC.00 */
#define sEE_I2C_SDA_GPIO_PORT            GPIOE                       /* GPIOE */
#define sEE_M24C64_32

#define sEE_FLAG_TIMEOUT ((uint32_t)0x1000)
#define sEE_LONG_TIMEOUT ((uint32_t)(10 * sEE_FLAG_TIMEOUT))

#define HW_ADDRESS 0xAA /* E0 = E1 = E2 = 0 */

#define I2C_SPEED 100000
#define I2C_SLAVE_ADDRESS7 0xAA

#define sEE_OK 0
#define sEE_FAIL 1

void BQ_I2C_Init(void);

uint32_t sEE_TIMEOUT_UserCallback(void);

uint32_t BQ_I2C_Test(void);

uint32_t send_command(uint8_t command);

// int requestBytes(uint8_t *buffer, int len);

uint32_t requestBytes(uint8_t *pBuffer, uint16_t *NumByteToRead);

uint16_t requestWord(void);

void sEE_EnterCriticalSection_UserCallback(void);
void sEE_ExitCriticalSection_UserCallback(void);

uint32_t send_command_with_sub(uint8_t reg, uint16_t sub_command);

void I2C_Read32BytesFromSlave(uint8_t *pBuffer);

#endif // _I2C_DRIVER_H_
