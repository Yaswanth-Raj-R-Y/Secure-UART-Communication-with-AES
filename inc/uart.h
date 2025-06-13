#ifndef UART_H
#define UART_H

#include "stm32f401xc.h"

/**
 * @brief Function to initialize UART 1, 2 & 6. 
 * 
 * @param baud_rate Baude rate value as an parameter
*/
void uart1_init(uint32_t baud_rate);
void uart2_init(uint32_t baud_rate);
void uart6_init(uint32_t baud_rate);

#endif