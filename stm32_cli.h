/*
 * stm32_cli.h
 *
 *  Created on: Oct 19, 2019
 *      Author: harganda
 */

#ifndef __STM32_CLI_H_
#define __STM32_CLI_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdarg.h>

/* Board specific includes */
#include "stm32746g_discovery.h"
#include "stm32f7xx_hal_uart.h"

#define BSP_CLI_BUFFER_LEN (128)
#define BSP_CLI_QUEUE_LEN  (16)

/* Enable task usage, will dynamically allocate strings */
#define configBSP_CLI_ENABLE_TASK (1)

 /**
  * @brief  Initialize UART port to use as CLI.
  * @param  pxUARTHandler: UART Handler pointer to use as CLI.
 * @retval : Initialization status (OK: 1, Error: 0).
  */
uint8_t BSP_CLI_Init(UART_HandleTypeDef* pxUARTHandler);

/**
 * @brief  Initialize the ft5336 communication bus
 *         from MCU to FT5336 : ie I2C channel initialization (if required).
  * @param  src: Pointer to data source.
  * @param  len: Source length.
 * @retval : Number of bytes written.
 */
uint16_t BSP_CLI_Write(const char* src, uint16_t len);

/**
 * @brief  Print to CLI with format.
 * @param  fmt: Formatter string.
 * @retval : Number of bytes written.
 */
uint16_t BSP_CLI_Printf(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* __STM32_CLI_H_ */
