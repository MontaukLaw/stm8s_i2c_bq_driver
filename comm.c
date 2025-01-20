#include "comm.h"

volatile uint8_t g_RxCount = 0;
volatile char g_RxBuffer[RX_BUFFER_SIZE];
volatile uint8_t timeToParseData = 0;

void send_uart_word(uint16_t data)
{
    UART1_SendData8(data >> 8);
    // UART1_GetITStatus(UART1_IT_TXE);
    while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET)
        ;

    UART1_SendData8(data);
    while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET)
        ;
}

void send_uart_byte(uint8_t data)
{
    UART1_SendData8(data);
    while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET)
        ;
}

void send_uart_dword(uint32_t data)
{

    UART1_SendData8(data >> 24);
    while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET)
        ;

    UART1_SendData8(data >> 16);
    while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET)
        ;

    UART1_SendData8(data >> 8);
    while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET)
        ;

    UART1_SendData8(data);
    while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET)
        ;
}

void send_i2c_error(void)
{
    uint8_t i = 0;
    for (i = 0; i < 4; i++)
    {
        UART1_SendData8(0xFF);
        while (UART1_GetFlagStatus(UART1_FLAG_TXE) == RESET)
            ;
    }
}

void handle_comm_multi_bytes(void)
{
    uint16_t word_result = 0;
    uint8_t byte_result = 0;
    char data = 0, data2 = 0, data3 = 0, data4 = 0;
    if (g_RxCount == 0)
    {
        return;
    }
    // 查询电池总电压	V
    // 查询电流	C
    // 电芯1/2电压	VV
    // 温度	T
    // 获取AFE状态	AFE
    // 放电MOS状态	DSG
    // 充电MOS状态	CHG
    // AFE开关	SAFE
    // 放电MOS开关	SDSG
    // 充电MOS开关	SCHG
    switch (g_RxCount)
    {
    case 1:
        data = g_RxBuffer[0];

        switch (data)
        {
            // 查询电池总电压
        case 'V':
            word_result = get_voltage();
            // 串口回复
            send_uart_word(word_result);
            break;
            // 查询电流
        case 'C':
            word_result = get_current();
            send_uart_word(word_result);
            break;
            // 温度
        case 'T':
            word_result = get_temperature();
            send_uart_word(word_result);
            break;
        }
        break;
    case 2:
        data = g_RxBuffer[0];
        data2 = g_RxBuffer[1];

        if (data == 'V' && data2 == 'V')
        {
            // 电芯1/2电压
            uint32_t dword_result = get_cell_vol();
            send_uart_dword(dword_result);
        }

        break;
    case 3:

        data = g_RxBuffer[0];
        data2 = g_RxBuffer[1];
        data3 = g_RxBuffer[2];

        if (data == 'A' && data2 == 'F' && data3 == 'E')
        {
            // 获取AFE状态
            byte_result = get_afe_status();
            send_uart_byte(byte_result);
        }
        else if (data == 'D' && data2 == 'S' && data3 == 'G')
        {
            // 放电MOS状态
            byte_result = get_dsg();
            send_uart_byte(byte_result);
        }
        else if (data == 'C' && data2 == 'H' && data3 == 'G')
        {
            // 充电MOS状态
            byte_result = get_chg();
            send_uart_byte(byte_result);
        }
        break;
    case 4:

        data = g_RxBuffer[0];
        data2 = g_RxBuffer[1];
        data3 = g_RxBuffer[2];
        data4 = g_RxBuffer[3];

        if (data == 'S' && data2 == 'A' && data3 == 'F' && data4 == 'E')
        {
            // 控制AFE开关
            FETControl();
            // 要等一下, 不然bq28z610不会有反应.
            Delay(10000);
            // 返回AFE状态
            byte_result = get_afe_status();
            send_uart_byte(byte_result);
        }
        else if (data == 'S' && data2 == 'D' && data3 == 'S' && data4 == 'G')
        {
            // 切换放电MOS开关
            ChargeFET();
            Delay(10000);

            byte_result = get_dsg();
            send_uart_byte(byte_result);
        }
        else if (data == 'S' && data2 == 'C' && data3 == 'H' && data4 == 'G')
        {
            // 切换充电MOS开关
            DischargeFET();
            Delay(10000);

            byte_result = get_chg();
            send_uart_byte(byte_result);
        }
        break;
    }

#if 0
    if (g_RxCount == 1)
    {
        char data = g_RxBuffer[0];
        switch (data)
        {
            // 查询电池总电压
        case 'V':
            word_result = get_voltage();

            // 串口回复
            send_uart_word(word_result);
            break;
            // 查询电流
        case 'C':
            word_result = get_current();
            send_uart_word(word_result);
            break;
            // 温度
        case 'T':
            word_result = get_temperature();
            send_uart_word(word_result);
            break;
        }
    }
    else if (g_RxCount == 2)
    {
        char data = g_RxBuffer[0];
        char data2 = g_RxBuffer[1];
        if (data == 'V' && data2 == 'V')
        {
            // 电芯1/2电压
            uint32_t dword_result = get_cell_vol();
            send_uart_dword(dword_result);
        }
    }
    else if (g_RxCount == 3)
    {
        char data = g_RxBuffer[0];
        char data2 = g_RxBuffer[1];
        char data3 = g_RxBuffer[2];
        if (data == 'A' && data2 == 'F' && data3 == 'E')
        {
            // 获取AFE状态
            byte_result = get_afe_status();
            send_uart_byte(byte_result);
        }
        else if (data == 'D' && data2 == 'S' && data3 == 'G')
        {
            // 放电MOS状态
            byte_result = get_dsg();
            send_uart_byte(byte_result);
        }
        else if (data == 'C' && data2 == 'H' && data3 == 'G')
        {
            // 充电MOS状态
            byte_result = get_chg();
            send_uart_byte(byte_result);
        }
    }
    else if (g_RxCount == 4)
    {
        char data = g_RxBuffer[0];
        char data2 = g_RxBuffer[1];
        char data3 = g_RxBuffer[2];
        char data4 = g_RxBuffer[3];
        if (data == 'S' && data2 == 'A' && data3 == 'F' && data4 == 'E')
        {
            // 控制AFE开关
            FETControl();
            // 要等一下, 不然bq28z610不会有反应.
            Delay(10000);
            // 返回AFE状态
            byte_result = get_afe_status();
            send_uart_byte(byte_result);
        }
        else if (data == 'S' && data2 == 'D' && data3 == 'S' && data4 == 'G')
        {
            // 切换放电MOS开关
            ChargeFET();
            Delay(10000);

            byte_result = get_dsg();
            send_uart_byte(byte_result);
        }
        else if (data == 'S' && data2 == 'C' && data3 == 'H' && data4 == 'G')
        {
            // 切换充电MOS开关
            DischargeFET();
            Delay(10000);

            byte_result = get_chg();
            send_uart_byte(byte_result);
        }
    }
#endif
    g_RxCount = 0;
}

void handle_comm(uint8_t data)
{
    uint16_t word_result = 0;
    uint32_t dword_result = 0;

    switch (data)
    {
        // 获取电池电压
    case 0x01:
        // 回复2个字节的电压值, 实际值需要乘以0.001
        word_result = get_voltage();

        // 串口回复
        send_uart_word(word_result);

        break;

        // 获取电池电流
    case 0x02:
        word_result = get_current();
        send_uart_word(word_result);
        break;

        // 获取电芯1/2电压
    case 0x03:
        dword_result = get_cell_vol();
        send_uart_dword(dword_result);
        break;

        // 获取温度
    case 0x04:
        // 回复2个字节的温度值, 实际值需要乘以0.1
        word_result = get_temperature();
        send_uart_word(word_result);
        break;

        // 获取充电, 放电, AFE开关状态
        // 12.2.33 AltManufacturerAccess()
        // FET_EN (Bit 4): All FET Action
        // DSG_TEST (Bit 2): Discharge FET Test
        // CHG_TEST (Bit 1): Charge FET Test
    case 0x05:
        word_result = get_manufacturing_status();
        send_uart_word(word_result);
        break;

        // AFE开关控制
    case 0x0a:
        FETControl();
        break;

        // 放电FET控制
    case 0x0b:
        ChargeFET();
        break;

    case 0x0c:
        DischargeFET();
        break;

    default:
        break;
    }
}