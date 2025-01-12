#include "utils.h"

bool validate(uint8_t *data)
{
    uint8_t sum = data[CHECKSUM_INDEX];

    const uint8_t len = data[LENGTH_INDEX];
    for (int i = 0; i < len - 2; i++)
        sum += data[i]; // exclude length byte itself

    const bool retval = (bool)(sum & 0xFF);

    return retval;
}

/*******************************************************************************
****入口参数：无
****出口参数：无
****函数备注：不精确延时函数
****版权信息：蓝旗嵌入式系统
*******************************************************************************/
void Delay(__IO uint32_t nCount)
{
    /* Decrement nCount value */
    while (nCount != 0)
    {
        nCount--;
    }
}

uint16_t composeWord(uint8_t *buf, uint32_t lsbIndex, uint8_t littleEndian)
{
    const int msbIndex = lsbIndex + (littleEndian ? 1 : -1);
    if (msbIndex < 0)
    {
        return 0;
    }
    return (buf[msbIndex] << 8) | buf[lsbIndex];
}
