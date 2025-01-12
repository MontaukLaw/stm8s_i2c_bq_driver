#include "key.h"
#include "comm.h"

void key_init(void)
{
    GPIO_Init(KEY_BUTTON_PORT, KEY_BUTTON_PIN, GPIO_MODE_IN_PU_NO_IT);
    
    // 配置中断触发方式（下降沿）
    // EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOA, EXTI_SENSITIVITY_FALL_ONLY);
}

void key_process(void)
{
    static uint32_t downTime = 0;
    static uint8_t clickedCounter = 0;

    if (GPIO_ReadInputPin(KEY_BUTTON_PORT, KEY_BUTTON_PIN) == RESET)
    {
        downTime++;
        if (downTime > LONG_PRESS_TIME)
        {

            // SDSG
            ChargeFET();
            downTime = 0;
            // do something
        }
    }

    else
    {
        if (downTime > SHORT_PRESS_COUNTER_MIN && downTime < SHORT_PRESS_COUNTER_MAX)
        {
            // 松开后
            clickedCounter++;
            if (clickedCounter >= CLICKED_COUNTER)
            {
                // SAFE
                FETControl();
                clickedCounter = 0;
            }
        }
        downTime = 0;
    }
}