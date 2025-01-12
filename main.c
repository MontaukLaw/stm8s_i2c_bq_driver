#include "main.h"

volatile uint8_t inactivity_seconds = 0;

uint8_t data = 0;
uint8_t get_data = 0;

#if 0
void Enter_LowPower_Mode(void)
{
    /* 关闭不必要的外设以降低功耗 */
    UART1_Cmd(DISABLE);
    GPIO_DeInit(GPIOA); // 根据需求关闭特定 GPIO 引脚
    TIM2_DeInit();

    /* 选择低功耗模式，例如 Halt Mode */
    Power_HaltMode();

    /* 等待中断唤醒 */
    __asm("sim"); // 使能中断
}
#endif

static void UART1_Config(void)
{
    // 先对 UART1 做复位
    UART1_DeInit();

    // 初始化
    UART1_Init(9600,
               UART1_WORDLENGTH_8D,
               UART1_STOPBITS_1,
               UART1_PARITY_NO,
               (UART1_SyncMode_TypeDef)0, // 异步模式
               (UART1_Mode_TypeDef)(UART1_MODE_TX_ENABLE | UART1_MODE_RX_ENABLE));

    // 使能 UART1 外设 (注意：UARTD=0 表示使能)
    UART1_Cmd(ENABLE);

    // 使能 RXNE (接收数据寄存器非空) 中断 和 Overrun 中断
    UART1_ITConfig(UART1_IT_RXNE_OR, ENABLE);
    UART1_ITConfig(UART1_IT_IDLE, ENABLE);
    // UART1_ITConfig(UART1_IT_RXNE, ENABLE);
}

// 在MCU通电启动时，获取AFE的状态是否为1（使能），
// 如果为1则进行AFE开关切换为0（失能），即为保证MCU开机时电池板子的AFE为失能状态
void disabel_afe(void)
{
    uint8_t afeStatus = get_afe_status();
    Delay(10000);
    if (afeStatus == 0x01)
    {
        FETControl();
    }

    Delay(10000);
}

// 初始化 TIM2 作为软件计时器
void TIM2_Init_Config(void)
{
    // 关闭 TIM2
    TIM2_DeInit();

    // 配置 TIM2 以1秒为周期
    TIM2_TimeBaseInit(TIM2_PRESCALER_16384, 1000); // 1秒中断一次

    // 使能更新中断
    TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);

    // 启动 TIM2
    TIM2_Cmd(ENABLE);
}

/* 函数定义 */
void Reset_Inactivity_Timer(void)
{
    /* 重置计时器计数 */
    inactivity_seconds = 0;

    /* 重新启动 TIM2，如果它已被禁用 */
    if (!TIM2_GetFlagStatus(TIM2_FLAG_UPDATE))
    {
        TIM2_Cmd(ENABLE);
    }
}

void main(void)
{
    /* Initialization of the clock */
    /* Clock divider to HSI/1 */
    CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);

    BQ_I2C_Init();

    // UART1 时钟使能 (默认为开，也可显式使能)
    CLK_PeripheralClockConfig(CLK_PERIPHERAL_UART1, ENABLE);

    // 配置 UART1
    UART1_Config();

    disabel_afe();

    key_init();

    // 初始化 TIM2 作为软件计时器
    // TIM2_Init_Config();

    // 全局使能中断
    enableInterrupts();

    while (1)
    {
        // if (get_data)
        // {
        //     handle_comm(data);
        //     get_data = 0;
        // }

        if (timeToParseData)
        {
            handle_comm_multi_bytes();
            timeToParseData = 0;
            // 把这个也要置零
        }

        key_process();

        // get_manufacturing_status();
        // BQ_I2C_Test();
        // send_command(0x06);
        // get_voltage();
        // get_current();
        /* Insert delay */
        for (int i = 0; i < 1000; i++)
            ;
    }
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *   where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    // printf("Wrong parameters value: file %s on line %d\r\n", file, line);
    /* Infinite loop */
    while (1)
    {
    }
}
#endif