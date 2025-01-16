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
        // IWDG_ReloadCounter();
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

/**
 * @brief  Measure the LSI frequency using timer IC1 and update the calibration registers.
 * @note   It is recommended to use a timer clock frequency of at least 10MHz in order
 *         to obtain a better in the LSI frequency measurement.
 * @param  None
 * @retval None
 */
uint32_t LSIMeasurment(void)
{
    uint32_t lsi_freq_hz = 0x0;
    uint32_t fmaster = 0x0;
    uint16_t ICValue1 = 0x0;
    uint16_t ICValue2 = 0x0;

    /* Get master frequency */
    fmaster = CLK_GetClockFreq();

    /* Enable the LSI measurement: LSI clock connected to timer Input Capture 1 */
    AWU->CSR |= AWU_CSR_MSR;

#if defined(STM8S903) || defined(STM8S103) || defined(STM8S003) || defined(STM8S001)
    /* Measure the LSI frequency with TIMER Input Capture 1 */

    /* Capture only every 8 events!!! */
    /* Enable capture of TI1 */
    TIM1_ICInit(TIM1_CHANNEL_1, TIM1_ICPOLARITY_RISING, TIM1_ICSELECTION_DIRECTTI,
                TIM1_ICPSC_DIV8, 0);

    /* Enable TIM1 */
    TIM1_Cmd(ENABLE);

    /* wait a capture on cc1 */
    while ((TIM1->SR1 & TIM1_FLAG_CC1) != TIM1_FLAG_CC1)
        ;
    /* Get CCR1 value*/
    ICValue1 = TIM1_GetCapture1();
    TIM1_ClearFlag(TIM1_FLAG_CC1);

    /* wait a capture on cc1 */
    while ((TIM1->SR1 & TIM1_FLAG_CC1) != TIM1_FLAG_CC1)
        ;
    /* Get CCR1 value*/
    ICValue2 = TIM1_GetCapture1();
    TIM1_ClearFlag(TIM1_FLAG_CC1);

    /* Disable IC1 input capture */
    TIM1->CCER1 &= (uint8_t)(~TIM1_CCER1_CC1E);
    /* Disable timer2 */
    TIM1_Cmd(DISABLE);

#else
    /* Measure the LSI frequency with TIMER Input Capture 1 */

    /* Capture only every 8 events!!! */
    /* Enable capture of TI1 */
    TIM3_ICInit(TIM3_CHANNEL_1, TIM3_ICPOLARITY_RISING, TIM3_ICSELECTION_DIRECTTI,
                TIM3_ICPSC_DIV8, 0);

    /* Enable TIM3 */
    TIM3_Cmd(ENABLE);

    /* wait a capture on cc1 */
    while ((TIM3->SR1 & TIM3_FLAG_CC1) != TIM3_FLAG_CC1)
        ;
    /* Get CCR1 value*/
    ICValue1 = TIM3_GetCapture1();
    TIM3_ClearFlag(TIM3_FLAG_CC1);

    /* wait a capture on cc1 */
    while ((TIM3->SR1 & TIM3_FLAG_CC1) != TIM3_FLAG_CC1)
        ;
    /* Get CCR1 value*/
    ICValue2 = TIM3_GetCapture1();
    TIM3_ClearFlag(TIM3_FLAG_CC1);

    /* Disable IC1 input capture */
    TIM3->CCER1 &= (uint8_t)(~TIM3_CCER1_CC1E);
    /* Disable timer3 */
    TIM3_Cmd(DISABLE);
#endif /* (STM8S903) || (STM8S103) || (STM8S003) || (STM8S001) */

    /* Compute LSI clock frequency */
    lsi_freq_hz = (8 * fmaster) / (ICValue2 - ICValue1);

    /* Disable the LSI measurement: LSI clock disconnected from timer Input Capture 1 */
    AWU->CSR &= (uint8_t)(~AWU_CSR_MSR);

    return (lsi_freq_hz);
}
