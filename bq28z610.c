#include "bq28z610.h"

static const int DATA_INDEX = 2; ///< Index of the byte from which data starts.
static const uint8_t ALT_MANUFACTURER_ACCESS = 0x3E;
static const uint16_t MANUFACTURER_STATUS = 0x0057;
static const uint16_t OPERATION_STATUS = 0x0054;
const float PERMIL = 0.001;
static const uint16_t DA_STATUS_1 = 0x0071;
static const uint16_t FET_CONTROL = 0x0022;
static const uint16_t CHG_FET = 0x001F;
static const uint16_t DSG_FET = 0x0020;

static const uint8_t TEMPERATURE = 0x06;
const float DECIPART = 0.1;
/**
  Request the device for 36 bytes per multiple requests using the Block Protocol.
  - The first 2 bytes represent the requested address.
  - 32 bytes contain data.
  - 1 byte is allocated for the checksum.
  - 1 byte denotes the total length.
*/
uint32_t requestBlock(uint8_t *buf)
{
    uint32_t actual = 0;

    uint8_t *bufPtr = buf;
    uint16_t data_len = 0;

    data_len = ADDR_SIZE;
    actual += requestBytes(bufPtr, &data_len);

    data_len = PAYLOAD_MAX_SIZE;
    bufPtr += ADDR_SIZE;

    // I2C_Read32BytesFromSlave(bufPtr);
    actual += requestBytes(bufPtr, &data_len);

    data_len = CHECKSUM_AND_LENGTH_SIZE;
    bufPtr += PAYLOAD_MAX_SIZE;
    actual += requestBytes(bufPtr, &data_len);

    return actual;
}

uint8_t AltManufacturerAccess(const uint16_t MACSubcmd, uint8_t *retval, uint8_t *len)
{

    uint8_t buf[RESPONSE_MAX_SIZE];
    send_command_with_sub(ALT_MANUFACTURER_ACCESS, MACSubcmd);

    // 这里是5ms
    Delay(1000);
    send_command(ALT_MANUFACTURER_ACCESS);

    const uint32_t count = requestBlock(buf);

    const uint8_t isDataValid = validate(buf);
    if (isDataValid)
    {
        const uint32_t _len = buf[LENGTH_INDEX] - SERVICE_SIZE;
        for (uint32_t i = 0; i < _len; i++)
        {
            retval[i] = buf[DATA_INDEX + i];
        }
        *len = _len;
    }

    return isDataValid;
}

uint16_t get_operation_status(void)
{
    uint8_t buf[RESPONSE_MAX_SIZE], len = 0;
    memset(buf, 0, sizeof(buf));
    if (!AltManufacturerAccess(OPERATION_STATUS, buf, &len))
        return 0;

    uint16_t operationStatus = composeWord(buf, 0, 1);

    return operationStatus;
}

uint8_t get_afe_status(void)
{
    uint16_t manufacturingStatus = get_manufacturing_status();
    // 取Bit4的FET_EN状态 例1001 1010  0-失能 1-使能
    return (manufacturingStatus & 0x10) >> 4;
}

// CHG (Bit 2): CHG FET status
uint8_t get_chg(void)
{
    uint16_t operantionStatus = get_operation_status();

    return (operantionStatus & 0x04) >> 2;
}

// DSG (Bit 1): DSG FET status
uint8_t get_dsg(void)
{
    uint16_t operantionStatus = get_operation_status();

    return (operantionStatus & 0x02) >> 1;
}

uint16_t get_manufacturing_status(void)
{
    uint8_t buf[RESPONSE_MAX_SIZE], len = 0;
    memset(buf, 0, sizeof(buf));
    if (!AltManufacturerAccess(MANUFACTURER_STATUS, buf, &len))
        return 0;

    uint16_t manufacturingStatus = composeWord(buf, 0, 1);

    return manufacturingStatus;
}

float Voltage(void)
{
    send_command(VOLTAGE);
    const float retval = PERMIL * requestWord();
    return retval;
}

// 电压值需要乘以0.001
uint16_t get_voltage(void)
{
    send_command(VOLTAGE);
    return requestWord();
}

/**
  @brief 12.1.7 0x0C/0D Current
  @returns the measured current from the coulomb counter.
*/
uint16_t get_current(void)
{
    send_command(CURRENT);
    uint16_t retval = requestWord();
    return retval;
}

/**
  @brief 12.2.37 AltManufacturerAccess() 0x0071 DAStatus1

  @returns 32 bytes of data on MACData() in the following format:
  @returns
  - AAaa: Cell Voltage 1
  - BBbb: Cell Voltage 2
  - CCcc:
  - DDdd:
  - EEee: BAT Voltage. Voltage at the VC2 (BAT) terminal
  - FFff: PACK Voltage
  - GGgg: Cell Current 1. Simultaneous current measured during Cell Voltage1 measurement
  - HHhh: Cell Current 2. Simultaneous current measured during Cell Voltage2 measurement
  - IIii:
  - JJjj:
  - KKkk: Cell Power 1. Calculated using Cell Voltage1 and Cell Current 1 data
  - LLll: Cell Power 2. Calculated using Cell Voltage2 and Cell Current 2 data
  - MMmm:
  - NNnn:
  - OOoo: Power calculated by Voltage() × Current()
  - PPpp: Average Power. Calculated by Voltage() × AverageCurrent()
*/
void DAStatus1(uint8_t *retval)
{
    uint8_t buf[RESPONSE_MAX_SIZE], len = 0;
    memset(buf, 0, sizeof(buf));
    if (!AltManufacturerAccess(DA_STATUS_1, buf, &len))
        return;

    for (int i = 0; i < len; i++)
        retval[i] = buf[i];
}

uint32_t get_cell_vol(void)
{

    uint8_t buf[RESPONSE_MAX_SIZE];
    DAStatus1(buf);

    uint16_t cellVoltage1 = composeWord(buf, CELL_VOLTAGE_1, 1);
    uint16_t cellVoltage2 = composeWord(buf, CELL_VOLTAGE_2, 1);

    uint32_t cellVoltage = ((uint32_t)cellVoltage1 << 16) | (uint32_t)cellVoltage2;
    return cellVoltage;
}

void FETControl(void)
{
    send_command_with_sub(ALT_MANUFACTURER_ACCESS, FET_CONTROL);
    // delay 400ms
    Delay(6000);
}

void ChargeFET(void)
{
    send_command_with_sub(ALT_MANUFACTURER_ACCESS, CHG_FET);
    // delay 400ms
    Delay(6000);
}

void DischargeFET(void)
{
    send_command_with_sub(ALT_MANUFACTURER_ACCESS, DSG_FET);
    // delay 400ms
    Delay(6000);
}
/**
  @brief 12.1.4 0x06/07 Temperature
  @returns an unsigned integer value of temperature in units ( 0.1 k)
  measured by the gas gauge, and is used for the gauging algorithm.
  It reports either InternalTemperature() or external thermistor temperature,
  depending on the setting of the [TEMPS] bit in Pack configuration.
*/
uint16_t get_temperature(void)
{

    send_command(TEMPERATURE);

    return requestWord();
    // const float celsius = KELVIN_TO_CELSIUS(kelvin);
    // return celsius;
}

void i2c_test(void)
{

    // uint8_t buf[RESPONSE_MAX_SIZE];
    send_command_with_sub(ALT_MANUFACTURER_ACCESS, 0x0057);

    // 这里是5ms
    Delay(1000);
    send_command(ALT_MANUFACTURER_ACCESS);

    // request2Bytes(buf, ADDR_SIZE);
}
