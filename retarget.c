#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <stdio.h>

extern UART_HandleTypeDef huart2;

int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
