#ifndef __BQ28Z610_H_
#define __BQ28Z610_H_

#include "stm8s.h"
#include "utils.h"

#include <string.h>
#include "i2c_driver.h"

#define BQ28Z610_I2C_ADDR 0xAA
#define ADDR_SIZE 2
#define CHECKSUM_SIZE 1
#define LENGTH_SIZE 1
#define CHECKSUM_INDEX 34
#define LENGTH_INDEX 35

#define CHECKSUM_SIZE 1                                        ///< Number of bytes for Checksum, 1.
                                                               ///< Number of bytes for Length, 1.
#define CHECKSUM_AND_LENGTH_SIZE (CHECKSUM_SIZE + LENGTH_SIZE) ///< Number of bytes for the Checksum and Length, 2.
#define SERVICE_SIZE (ADDR_SIZE + CHECKSUM_SIZE + LENGTH_SIZE) ///< Number of the service bytes (address, checksum, length), 4.

#define REQUEST_MAX_SIZE 32
#define PAYLOAD_MAX_SIZE REQUEST_MAX_SIZE
#define RESPONSE_MAX_SIZE 36

#define VOLTAGE 0x08
#define CURRENT 0x0C
#define CELL_VOLTAGE_1 0
#define CELL_VOLTAGE_2 2

void i2c_test(void);

uint16_t get_manufacturing_status(void);

float Voltage(void);

uint16_t get_voltage(void);

uint16_t get_current(void);

uint32_t get_cell_vol(void);

void FETControl(void);

uint16_t get_temperature(void);

void ChargeFET(void);

void DischargeFET(void);

uint8_t get_afe_status(void);

uint8_t get_chg(void);

uint8_t get_dsg(void);


#endif
