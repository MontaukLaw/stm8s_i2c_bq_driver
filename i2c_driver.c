#include "i2c_driver.h"

__IO uint16_t slaveAddress = 0;
__IO uint32_t sEETimeout = sEE_LONG_TIMEOUT;
__IO uint8_t *sEEDataWritePointer;
__IO uint8_t sEEDataNum;

/**
 * @brief  DeInitializes peripherals used by the I2C EEPROM driver.
 * @param  None
 * @retval None
 */
void sEE_LowLevel_DeInit(void)
{
    /* sEE_I2C Peripheral Disable */
    I2C_Cmd(DISABLE);

    /* sEE_I2C DeInit */
    I2C_DeInit();

    /*!< sEE_I2C Peripheral clock disable */
    CLK_PeripheralClockConfig(sEE_I2C_CLK, DISABLE);

    /*!< GPIO configuration */
    /*!< Configure sEE_I2C pins: SCL */
    GPIO_Init(sEE_I2C_SCL_GPIO_PORT, sEE_I2C_SCL_PIN, GPIO_MODE_IN_PU_NO_IT);

    /*!< Configure sEE_I2C pins: SDA */
    GPIO_Init(sEE_I2C_SDA_GPIO_PORT, sEE_I2C_SDA_PIN, GPIO_MODE_IN_PU_NO_IT);
}

/**
 * @brief  Initializes peripherals used by the I2C EEPROM driver.
 * @param  None
 * @retval None
 */
void sEE_LowLevel_Init(void)
{
    /*!< sEE_I2C Peripheral clock enable */
    CLK_PeripheralClockConfig(sEE_I2C_CLK, ENABLE);
}

void BQ_I2C_Init(void)
{
    sEE_LowLevel_Init();

    /* I2C configuration */
    /* sEE_I2C Peripheral Enable */
    I2C_Cmd(ENABLE);
    /* sEE_I2C configuration after enabling it */
    I2C_Init(I2C_SPEED, 0x20, I2C_DUTYCYCLE_2, I2C_ACK_CURR,
             I2C_ADDMODE_7BIT, 16);

    /* Select the EEPROM address according to the state of E0, E1, E2 pins */
    slaveAddress = HW_ADDRESS;
}

uint32_t BQ_I2C_Test(void)
{
    /* While the bus is busy */
    sEETimeout = sEE_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(I2C_FLAG_BUSBUSY))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    /* Send START condition */
    I2C_GenerateSTART(ENABLE);

    /* Test on EV5 and clear it */
    sEETimeout = sEE_FLAG_TIMEOUT;
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    /* Send EEPROM address for write */
    sEETimeout = sEE_FLAG_TIMEOUT;
    I2C_Send7bitAddress((uint8_t)slaveAddress, I2C_DIRECTION_TX);

    /* Test on EV6 and clear it */
    sEETimeout = sEE_FLAG_TIMEOUT;
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    /* Send STOP condition */
    I2C_GenerateSTOP(ENABLE);

    /* Perform a read on SR1 and SR3 register to clear eventually pending flags */
    (void)sEE_I2C->SR1;
    (void)sEE_I2C->SR3;

    /* If all operations OK, return sEE_OK (0) */
    return sEE_OK;
}

uint32_t send_command(uint8_t command)
{

    sEETimeout = sEE_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(I2C_FLAG_BUSBUSY))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    /* Send START condition */
    I2C_GenerateSTART(ENABLE);

    sEETimeout = sEE_FLAG_TIMEOUT;
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    /* 设置I2C从器件地址，I2C主设备为写模式*/
    sEETimeout = sEE_FLAG_TIMEOUT;
    I2C_Send7bitAddress((uint8_t)slaveAddress, I2C_DIRECTION_TX);

    sEETimeout = sEE_FLAG_TIMEOUT;
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    /* 要写 */
    I2C_SendData(command);

    /* 测试EV6 ，检测从器件返回一个应答信号*/
    sEETimeout = sEE_FLAG_TIMEOUT;
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    /* 发结束位*/
    I2C_GenerateSTOP(ENABLE);

    /* Perform a read on SR1 and SR3 register to clear eventually pending flags */
    (void)sEE_I2C->SR1;
    (void)sEE_I2C->SR3;

    return sEE_OK;
}

uint32_t requestBytes(uint8_t *pBuffer, uint16_t *NumByteToRead)
{
    uint8_t i;
    /* Send START condition a second time */
    I2C_GenerateSTART(ENABLE);

    /* Test on EV5 and clear it (cleared by reading SR1 then writing to DR) */
    sEETimeout = sEE_FLAG_TIMEOUT;
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    I2C_Send7bitAddress((uint8_t)HW_ADDRESS, I2C_DIRECTION_RX);
    sEETimeout = sEE_FLAG_TIMEOUT;
    while (I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) == ERROR)
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    for (i = 0; i < (*NumByteToRead - 1); i++)
    {

        I2C_AcknowledgeConfig(I2C_ACK_CURR);

        while (I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR)
        {
            /* 等待数据到达，若需要可加入超时处理 */
        }
        pBuffer[i] = I2C_ReceiveData(); /* 读取 1 个字节 */
    }


    /*-----------------------------------------------------------------------------
     * 4. 最后一个字节，需要发送 NACK
     *    先关闭 ACK: I2C_AcknowledgeConfig(I2C_ACK_NONE)
     *    等待事件 EV7，再读取最后一字节
     *----------------------------------------------------------------------------*/
    I2C_AcknowledgeConfig(I2C_ACK_NONE);
   /*-----------------------------------------------------------------------------
     * 5. 读取完毕后发送 STOP，并恢复 ACK 使能（为后续其他通信做准备）
     *----------------------------------------------------------------------------*/
    I2C_GenerateSTOP(ENABLE);
    
    while (I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR)
    {
        /* 等待最后一个字节到达 */
    }
    pBuffer[*NumByteToRead - 1] = I2C_ReceiveData();

    /* 如果后续还有其他通信需要再次 ACK，可以在此恢复 */
    I2C_AcknowledgeConfig(I2C_ACK_CURR);
    
    return sEE_OK;
}

uint32_t requestBytes__(uint8_t *pBuffer, uint16_t *NumByteToRead)
{
    /* Send START condition a second time */
    I2C_GenerateSTART(ENABLE);

    /* Test on EV5 and clear it (cleared by reading SR1 then writing to DR) */
    sEETimeout = sEE_FLAG_TIMEOUT;
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    /* Send EEPROM address for read */
    I2C_Send7bitAddress((uint8_t)HW_ADDRESS, I2C_DIRECTION_RX);

    /* Read data from first byte until byte N-3 */
    if ((uint16_t)(*NumByteToRead) > 3)
    {
        /* Poll on BTF */
        sEETimeout = sEE_FLAG_TIMEOUT;
        while (!I2C_GetFlagStatus(I2C_FLAG_RXNOTEMPTY))
        // while (I2C_GetFlagStatus(I2C_FLAG_TRANSFERFINISHED) == RESET)
        {
            if ((sEETimeout--) == 0)
                return sEE_TIMEOUT_UserCallback();
        }

        /* Read a byte from the EEPROM */
        *pBuffer = I2C_ReceiveData();

        /* Point to the next location where the byte read will be saved */
        *pBuffer++;

        /* Decrement the read bytes counter */
        (uint16_t)(*NumByteToRead)--;
    }

    /*  Remains three data for read: data N-2, data N-1, Data N */
    /* Three Bytes Master Reception procedure (POLLING) ------------------------*/
    if ((uint16_t)(*NumByteToRead) == 3)
    {
        /* Data N-2 in DR and data N -1 in shift register */
        /* Poll on BTF */
        sEETimeout = sEE_FLAG_TIMEOUT;
        while (I2C_GetFlagStatus(I2C_FLAG_TRANSFERFINISHED) == RESET)
        {
            if ((sEETimeout--) == 0)
                return sEE_TIMEOUT_UserCallback();
        }

        /* Clear ACK */
        I2C_AcknowledgeConfig(I2C_ACK_NONE);

        /* Call User callback for critical section start (should typically disable interrupts) */
        sEE_EnterCriticalSection_UserCallback();

        /* Read Data N-2 */
        *pBuffer = I2C_ReceiveData();

        /* Point to the next location where the byte read will be saved */
        *pBuffer++;

        /* Program the STOP */
        I2C_GenerateSTOP(ENABLE);

        /* Read DataN-1 */
        *pBuffer = I2C_ReceiveData();

        /* Call User callback for critical section end (should typically re-enable interrupts) */
        sEE_ExitCriticalSection_UserCallback();

        /* Point to the next location where the byte read will be saved */
        *pBuffer++;

        /* Poll on RxNE */
        sEETimeout = sEE_FLAG_TIMEOUT;
        while (I2C_GetFlagStatus(I2C_FLAG_RXNOTEMPTY) == RESET)
        {
            if ((sEETimeout--) == 0)
                return sEE_TIMEOUT_UserCallback();
        }
        /* Read DataN */
        *pBuffer = I2C_ReceiveData();

        /* Reset the number of bytes to be read from the EEPROM */
        NumByteToRead = 0;
    }
    else

        /* If number of data to be read is 2 */
        /* Tow Bytes Master Reception procedure (POLLING) ---------------------------*/
        if ((uint16_t)(*NumByteToRead) == 2)
        {
            /* Enable acknowledgement on next byte (set POS and ACK bits)*/
            I2C_AcknowledgeConfig(I2C_ACK_NEXT);

            /* Wait on ADDR flag to be set (ADDR is still not cleared at this level */
            sEETimeout = sEE_FLAG_TIMEOUT;
            while (I2C_GetFlagStatus(I2C_FLAG_ADDRESSSENTMATCHED) == RESET)
            {
                if ((sEETimeout--) == 0)
                    return sEE_TIMEOUT_UserCallback();
            }

            /* Clear ADDR register by reading SR1 then SR3 register (SR1 has already been read) */
            (void)I2C->SR3;

            /* Disable Acknowledgement */
            I2C_AcknowledgeConfig(I2C_ACK_NONE);

            /* Wait for BTF flag to be set */
            sEETimeout = sEE_FLAG_TIMEOUT;
            while (I2C_GetFlagStatus(I2C_FLAG_TRANSFERFINISHED) == RESET)
            {
                if ((sEETimeout--) == 0)
                    return sEE_TIMEOUT_UserCallback();
            }

            /* Call User callback for critical section start (should typically disable interrupts) */
            sEE_EnterCriticalSection_UserCallback();

            /* Program the STOP */
            I2C_GenerateSTOP(ENABLE);

            /* Read Data N-1 */
            *pBuffer = I2C_ReceiveData();

            /* Point to the next location where the byte read will be saved */
            *pBuffer++;

            /* Call User callback for critical section end (should typically re-enable interrupts) */
            sEE_ExitCriticalSection_UserCallback();

            /* Read Data N */
            *pBuffer = I2C_ReceiveData();

            /* Reset the number of bytes to be read from the EEPROM */
            NumByteToRead = 0;
        }
        else

            /* If number of data to be read is 1 */
            /* One Byte Master Reception procedure (POLLING) ---------------------------*/
            if ((uint16_t)(*NumByteToRead) < 2)
            {
                /* Wait on ADDR flag to be set (ADDR is still not cleared at this level */
                sEETimeout = sEE_FLAG_TIMEOUT;
                while (I2C_GetFlagStatus(I2C_FLAG_ADDRESSSENTMATCHED) == RESET)
                {
                    if ((sEETimeout--) == 0)
                        return sEE_TIMEOUT_UserCallback();
                }

                /* Disable Acknowledgement */
                I2C_AcknowledgeConfig(I2C_ACK_NONE);

                /* Call User callback for critical section start (should typically disable interrupts) */
                sEE_EnterCriticalSection_UserCallback();

                /* Clear ADDR register by reading SR1 then SR3 register (SR1 has already been read) */
                (void)sEE_I2C->SR3;

                /* Send STOP Condition */
                I2C_GenerateSTOP(ENABLE);

                /* Call User callback for critical section end (should typically re-enable interrupts) */
                sEE_ExitCriticalSection_UserCallback();

                /* Wait for the byte to be received */
                sEETimeout = sEE_FLAG_TIMEOUT;
                while (I2C_GetFlagStatus(I2C_FLAG_RXNOTEMPTY) == RESET)
                {
                    if ((sEETimeout--) == 0)
                        return sEE_TIMEOUT_UserCallback();
                }

                /* Read the byte received from the EEPROM */
                *pBuffer = I2C_ReceiveData();

                /* Decrement the read bytes counter */
                (uint16_t)(*NumByteToRead)--;

                /* Wait to make sure that STOP control bit has been cleared */
                sEETimeout = sEE_FLAG_TIMEOUT;
                while (sEE_I2C->CR2 & I2C_CR2_STOP)
                {
                    if ((sEETimeout--) == 0)
                        return sEE_TIMEOUT_UserCallback();
                }

                /* Re-Enable Acknowledgement to be ready for another reception */
                I2C_AcknowledgeConfig(I2C_ACK_CURR);
            }
    /* If all operations OK, return sEE_OK (0) */
    return sEE_OK;
}

#define SLAVE_ADDR 0xAA /* 从机地址(7位)，最低位(读/写位)库中会自动处理 */
#define RX_DATA_SIZE 32 /* 需要读取的总字节数 */

/**
 * @brief  从地址为 0xAA 的 I2C 从机连续读取 32 个字节数据，
 *         最后一个字节发送 NACK 并产生 STOP。
 * @param  pBuffer : 存放读取数据的缓冲区指针，至少 32 字节大小
 * @retval None
 */
void I2C_Read32BytesFromSlave(uint8_t *pBuffer)
{
    uint8_t i;

    /*-----------------------------------------------------------------------------
     * 1. 生成 START 信号 (Master 模式开始)
     *    等待事件 EV5：I2C_EVENT_MASTER_MODE_SELECT
     *    表示已经进入主机模式
     *----------------------------------------------------------------------------*/
    I2C_GenerateSTART(ENABLE);
    while (I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT) == ERROR)
    {
        /* 可以在这里添加超时或错误处理 */
    }

    /*-----------------------------------------------------------------------------
     * 2. 发送从机地址和读取位
     *    等待事件 EV6：I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED
     *    表示从机地址被正确发送，且进入接收模式
     *----------------------------------------------------------------------------*/
    I2C_Send7bitAddress(HW_ADDRESS, I2C_DIRECTION_RX);
    while (I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) == ERROR)
    {
        /* 可以在这里添加超时或错误处理 */
    }

    /*-----------------------------------------------------------------------------
     * 3. 读取前 31 个字节，均发送 ACK
     *    每次等待事件 EV7：I2C_EVENT_MASTER_BYTE_RECEIVED
     *    表示 DR 寄存器里已经有一个字节可读
     *----------------------------------------------------------------------------*/
    for (i = 0; i < (RX_DATA_SIZE - 1); i++)
    {
        while (I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR)
        {
            /* 等待数据到达，若需要可加入超时处理 */
        }
        pBuffer[i] = I2C_ReceiveData(); /* 读取 1 个字节 */
    }

    /*-----------------------------------------------------------------------------
     * 4. 最后一个字节，需要发送 NACK
     *    先关闭 ACK: I2C_AcknowledgeConfig(I2C_ACK_NONE)
     *    等待事件 EV7，再读取最后一字节
     *----------------------------------------------------------------------------*/
    I2C_AcknowledgeConfig(I2C_ACK_NONE);

    while (I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR)
    {
        /* 等待最后一个字节到达 */
    }
    pBuffer[RX_DATA_SIZE - 1] = I2C_ReceiveData();

    /*-----------------------------------------------------------------------------
     * 5. 读取完毕后发送 STOP，并恢复 ACK 使能（为后续其他通信做准备）
     *----------------------------------------------------------------------------*/
    I2C_GenerateSTOP(ENABLE);

    /* 如果后续还有其他通信需要再次 ACK，可以在此恢复 */
    I2C_AcknowledgeConfig(I2C_ACK_CURR);
}

#if 0
int requestBytes__(uint8_t *buffer, int len)
{
    // BQ28Z610_I2C_ADDR
    int actual = 0;

    /* 等待空闲 */
    while (I2C_GetFlagStatus(I2C_FLAG_BUSBUSY))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    /* 发起始位 */
    I2C_GenerateSTART(ENABLE);
    sEETimeout = sEE_FLAG_TIMEOUT;
    /* 测试EV5 ，检测从器件返回一个应答信号*/
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    /* 设置I2C从器件地址，I2C主设备为读模式*/
    // I2C_Send7bitAddress(I2C1, BQ28Z610_I2C_ADDR, I2C_Direction_Receiver);
    sEETimeout = sEE_FLAG_TIMEOUT;
    I2C_Send7bitAddress((uint8_t)slaveAddress, I2C_DIRECTION_RX);

    /* 测试EV6 ，检测从器件返回一个应答信号*/
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    /*不断在从从设备读取数据 */
    while (len)
    {

        /* 读到最后一个字节*/
        if (len == 1)
        {
            /* 不需要应答*/
            I2C_AcknowledgeConfig(DISABLE);
            /* 发结束位*/
            I2C_GenerateSTOP(ENABLE);
        }
        else
        {
            /* 需要应答*/
            I2C_AcknowledgeConfig(ENABLE);
        }

        sEETimeout = sEE_FLAG_TIMEOUT;
        while (!I2C_GetFlagStatus(I2C_FLAG_RXNE))
        {
            over_time_counter++;
            if (over_time_counter > 200)
            {
                return -1; // 超时
            }
        }

        *buffer = I2C_ReceiveData(I2C1);
        /* 指针指向下个存放字节的地址*/
        buffer++;
        len--;

        actual++;
    }

    return actual;
}
#endif

uint16_t requestWord(void)
{
    uint8_t buf[] = {0, 0};
    uint16_t len = 2;
    requestBytes(buf, &len);
    return composeWord(buf, 0, 1);
}

uint32_t send_command_with_sub(uint8_t reg, uint16_t sub_command)
{
    sEETimeout = sEE_FLAG_TIMEOUT;
    /* 等待空闲 */
    while (I2C_GetFlagStatus(I2C_FLAG_BUSBUSY))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    /* 发起始位 */
    I2C_GenerateSTART(ENABLE);
    sEETimeout = sEE_FLAG_TIMEOUT;
    /* 测试EV5 ，检测从器件返回一个应答信号*/
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_MODE_SELECT))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    /* 设置I2C从器件地址，I2C主设备为写模式*/
    I2C_Send7bitAddress((uint8_t)slaveAddress, I2C_DIRECTION_TX);
    sEETimeout = sEE_FLAG_TIMEOUT;
    /* 测试EV6 ，检测从器件返回一个应答信号*/
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    /* 要写 0x3E  */
    I2C_SendData(reg);
    /* 测试EV6 ，检测从器件返回一个应答信号*/
    sEETimeout = sEE_FLAG_TIMEOUT;
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    I2C_SendData((uint8_t)(sub_command & 0x00ff));
    /* 测试EV8 ，检测从器件返回一个应答信号*/
    sEETimeout = sEE_FLAG_TIMEOUT;
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    I2C_SendData((uint8_t)((sub_command & 0xFF00) >> 8));
    /* 测试EV8 ，检测从器件返回一个应答信号*/
    sEETimeout = sEE_FLAG_TIMEOUT;
    while (!I2C_CheckEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if ((sEETimeout--) == 0)
            return sEE_TIMEOUT_UserCallback();
    }

    /* 发结束位*/
    I2C_GenerateSTOP(ENABLE);

    (void)sEE_I2C->SR1;
    (void)sEE_I2C->SR3;

    return sEE_OK;
}

void sEE_EnterCriticalSection_UserCallback(void)
{
    disableInterrupts();
}

/**
 * @brief  End of critical section: this callbacks should be typically used
 *         to re-enable interrupts when exiting a critical section of I2C communication
 *         You may use default callbacks provided into this driver by uncommenting the
 *         define USE_DEFAULT_CRITICAL_CALLBACK.
 *         Or you can comment that line and implement these callbacks into your
 *         application.
 * @param  None.
 * @retval None.
 */
void sEE_ExitCriticalSection_UserCallback(void)
{
    enableInterrupts();
}

uint32_t sEE_TIMEOUT_UserCallback(void)
{
    /* Block communication and all processes */
    while (1)
    {
    }
}
