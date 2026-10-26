#include "Global_Inc.h"


#define VOFA_CH_COUNT 1U

void VOFA_JustFloat_Send1(float ch0)
{
    uint8_t tx_buf[VOFA_CH_COUNT * sizeof(float) + 4U];
    float data[VOFA_CH_COUNT] = {ch0};

    memcpy(tx_buf, data, sizeof(data));

    tx_buf[sizeof(data) + 0U] = 0x00U;
    tx_buf[sizeof(data) + 1U] = 0x00U;
    tx_buf[sizeof(data) + 2U] = 0x80U;
    tx_buf[sizeof(data) + 3U] = 0x7FU;

    HAL_UART_Transmit(&huart1, tx_buf, (uint16_t)sizeof(tx_buf), 10U);
}

#define VOFA_CH_COUNT2 2U

void VOFA_JustFloat_Send2(float ch0, float ch1)
{
    uint8_t tx_buf[VOFA_CH_COUNT2 * sizeof(float) + 4U];
    float data[VOFA_CH_COUNT2] = {ch0, ch1};

    memcpy(tx_buf, data, sizeof(data));

    tx_buf[sizeof(data) + 0U] = 0x00U;
    tx_buf[sizeof(data) + 1U] = 0x00U;
    tx_buf[sizeof(data) + 2U] = 0x80U;
    tx_buf[sizeof(data) + 3U] = 0x7FU;

    HAL_UART_Transmit(&huart1, tx_buf, (uint16_t)sizeof(tx_buf), 10U);
}

#define VOFA_CH_COUNT3 3U

void VOFA_JustFloat_Send3(float ch0, float ch1, float ch2)
{
    uint8_t tx_buf[VOFA_CH_COUNT3 * sizeof(float) + 4U];
    float data[VOFA_CH_COUNT3] = {ch0, ch1, ch2};

    memcpy(tx_buf, data, sizeof(data));

    tx_buf[sizeof(data) + 0U] = 0x00U;
    tx_buf[sizeof(data) + 1U] = 0x00U;
    tx_buf[sizeof(data) + 2U] = 0x80U;
    tx_buf[sizeof(data) + 3U] = 0x7FU;

    HAL_UART_Transmit(&huart1, tx_buf, (uint16_t)sizeof(tx_buf), 10U);
}

#define VOFA_CH_COUNT4 4U

void VOFA_JustFloat_Send4(float ch0, float ch1, float ch2, float ch3)
{
    uint8_t tx_buf[VOFA_CH_COUNT4 * sizeof(float) + 4U];
    float data[VOFA_CH_COUNT4] = {ch0, ch1, ch2, ch3};

    memcpy(tx_buf, data, sizeof(data));

    tx_buf[sizeof(data) + 0U] = 0x00U;
    tx_buf[sizeof(data) + 1U] = 0x00U;
    tx_buf[sizeof(data) + 2U] = 0x80U;
    tx_buf[sizeof(data) + 3U] = 0x7FU;

    HAL_UART_Transmit(&huart1, tx_buf, (uint16_t)sizeof(tx_buf), 10U);
}

#define VOFA_CH_COUNT5 5U

void VOFA_JustFloat_Send5(float ch0, float ch1, float ch2, float ch3, float ch4)
{
    uint8_t tx_buf[VOFA_CH_COUNT5 * sizeof(float) + 4U];
    float data[VOFA_CH_COUNT5] = {ch0, ch1, ch2, ch3, ch4};

    memcpy(tx_buf, data, sizeof(data));

    tx_buf[sizeof(data) + 0U] = 0x00U;
    tx_buf[sizeof(data) + 1U] = 0x00U;
    tx_buf[sizeof(data) + 2U] = 0x80U;
    tx_buf[sizeof(data) + 3U] = 0x7FU;

    HAL_UART_Transmit(&huart1, tx_buf, (uint16_t)sizeof(tx_buf), 10U);
}

#define VOFA_CH_COUNT6 6U

void VOFA_JustFloat_Send6(float ch0, float ch1, float ch2, float ch3, float ch4, float ch5)
{
    uint8_t tx_buf[VOFA_CH_COUNT6 * sizeof(float) + 4U];
    float data[VOFA_CH_COUNT6] = {ch0, ch1, ch2, ch3, ch4, ch5};

    memcpy(tx_buf, data, sizeof(data));

    tx_buf[sizeof(data) + 0U] = 0x00U;
    tx_buf[sizeof(data) + 1U] = 0x00U;
    tx_buf[sizeof(data) + 2U] = 0x80U;
    tx_buf[sizeof(data) + 3U] = 0x7FU;

    HAL_UART_Transmit(&huart1, tx_buf, (uint16_t)sizeof(tx_buf), 10U);
}

#define VOFA_CH_COUNT7 7U

void VOFA_JustFloat_Send7(float ch0, float ch1, float ch2, float ch3, float ch4, float ch5, float ch6)
{
    uint8_t tx_buf[VOFA_CH_COUNT7 * sizeof(float) + 4U];
    float data[VOFA_CH_COUNT7] = {ch0, ch1, ch2, ch3, ch4, ch5, ch6};

    memcpy(tx_buf, data, sizeof(data));

    tx_buf[sizeof(data) + 0U] = 0x00U;
    tx_buf[sizeof(data) + 1U] = 0x00U;
    tx_buf[sizeof(data) + 2U] = 0x80U;
    tx_buf[sizeof(data) + 3U] = 0x7FU;

    HAL_UART_Transmit(&huart1, tx_buf, (uint16_t)sizeof(tx_buf), 10U);
}

#define VOFA_CH_COUNT8 8U

void VOFA_JustFloat_Send8(float ch0, float ch1, float ch2, float ch3, float ch4, float ch5, float ch6, float ch7)
{
    uint8_t tx_buf[VOFA_CH_COUNT8 * sizeof(float) + 4U];
    float data[VOFA_CH_COUNT8] = {ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7};

    memcpy(tx_buf, data, sizeof(data));

    tx_buf[sizeof(data) + 0U] = 0x00U;
    tx_buf[sizeof(data) + 1U] = 0x00U;
    tx_buf[sizeof(data) + 2U] = 0x80U;
    tx_buf[sizeof(data) + 3U] = 0x7FU;

    HAL_UART_Transmit(&huart1, tx_buf, (uint16_t)sizeof(tx_buf), 10U);
}

#define VOFA_CH_COUNT9 9U

void VOFA_JustFloat_Send9(float ch0, float ch1, float ch2, float ch3, float ch4, float ch5, float ch6, float ch7, float ch8)
{
    uint8_t tx_buf[VOFA_CH_COUNT9 * sizeof(float) + 4U];
    float data[VOFA_CH_COUNT9] = {ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7, ch8};

    memcpy(tx_buf, data, sizeof(data));

    tx_buf[sizeof(data) + 0U] = 0x00U;
    tx_buf[sizeof(data) + 1U] = 0x00U;
    tx_buf[sizeof(data) + 2U] = 0x80U;
    tx_buf[sizeof(data) + 3U] = 0x7FU;

    HAL_UART_Transmit(&huart1, tx_buf, (uint16_t)sizeof(tx_buf), 10U);
}

#define VOFA_CH_COUNT10 10U

void VOFA_JustFloat_Send10(float ch0, float ch1, float ch2, float ch3, float ch4, float ch5, float ch6, float ch7, float ch8, float ch9)
{
    uint8_t tx_buf[VOFA_CH_COUNT10 * sizeof(float) + 4U];
    float data[VOFA_CH_COUNT10] = {ch0, ch1, ch2, ch3, ch4, ch5, ch6, ch7, ch8, ch9};

    memcpy(tx_buf, data, sizeof(data));

    tx_buf[sizeof(data) + 0U] = 0x00U;
    tx_buf[sizeof(data) + 1U] = 0x00U;
    tx_buf[sizeof(data) + 2U] = 0x80U;
    tx_buf[sizeof(data) + 3U] = 0x7FU;

    HAL_UART_Transmit(&huart1, tx_buf, (uint16_t)sizeof(tx_buf), 10U);
}
