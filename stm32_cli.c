/*
 * stm32_cli.c
 *
 *  Created on: Oct 19, 2019
 *      Author: harganda
 */

#include "stm32_cli.h"

#if (configBSP_CLI_ENABLE_TASK == 1)
#include "cmsis_os.h"
#endif

#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef* cliHandler = NULL;

#if (configBSP_CLI_ENABLE_TASK == 1)
osMessageQId cliQueueHandle;
static void BSP_CLI_Task(void * argument);
#endif

uint8_t BSP_CLI_Init(UART_HandleTypeDef* pxUARTHandler)
{
	uint8_t xReturn = 0;

    if (pxUARTHandler != NULL)
	{
    	cliHandler = pxUARTHandler;

		if (HAL_UART_STATE_READY == cliHandler->gState)
		{
			/* If UART Handler has been already initialized do nothing */
		}
		else
		{
			cliHandler->Instance = USART1;
			cliHandler->Init.BaudRate = 115200;
			cliHandler->Init.WordLength = UART_WORDLENGTH_8B;
			cliHandler->Init.StopBits = UART_STOPBITS_1;
			cliHandler->Init.Parity = UART_PARITY_NONE;
			cliHandler->Init.Mode = UART_MODE_TX_RX;
			cliHandler->Init.HwFlowCtl = UART_HWCONTROL_NONE;
			cliHandler->Init.OverSampling = UART_OVERSAMPLING_16;
			cliHandler->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
			cliHandler->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
			if (HAL_UART_Init(pxUARTHandler) != HAL_OK)
			{
				// Error_Handler();
			}
		}

#if (configBSP_CLI_ENABLE_TASK == 1)
		if( cliQueueHandle == NULL )
		{
			/* Create the queue used to pass pointers to strings to the logging task. */
			cliQueueHandle = xQueueCreate( BSP_CLI_QUEUE_LEN, sizeof( char ** ) );

			if( cliQueueHandle != NULL )
			{
				if( xTaskCreate( BSP_CLI_Task, "CLI", configMINIMAL_STACK_SIZE * 4, NULL, 0, NULL ) == pdPASS )
				{
					xReturn = 1;
				}
				else
				{
					/* Could not create the task, so delete the queue again. */
					vQueueDelete( cliQueueHandle );
				}
			}
		}
#else
		xReturn = 1;
#endif

	}

    return xReturn;
}

uint16_t BSP_CLI_Write(const char* src, uint16_t len)
{
  if ((src != NULL) && (len > 0) && (cliHandler != NULL))
  {
    HAL_UART_Transmit(cliHandler, (uint8_t*)src, len, 100);
  }
  return len;
}

#if (configBSP_CLI_ENABLE_TASK == 1)

uint16_t BSP_CLI_Printf(const char * fmt, ...)
{
  size_t xLength = 0;
  va_list args;
  char * pcPrintString = NULL;

  /* Allocate a buffer to hold the log message. */
  pcPrintString = (char*)pvPortMalloc(BSP_CLI_BUFFER_LEN);

  if(pcPrintString != NULL)
  {
    /* There are a variable number of parameters. */
    va_start( args, fmt );
    xLength = vsnprintf(pcPrintString, BSP_CLI_BUFFER_LEN, fmt, args);
    va_end( args );

    /* Only send the buffer to the logging task if it is not empty. */
    if(xLength > 0)
    {
        /* Send the string to the logging task for IO. */
        if(xQueueSend(cliQueueHandle, &pcPrintString, 0) != pdPASS)
        {
            /* The buffer was not sent so must be freed again. */
            vPortFree( ( void * ) pcPrintString );
        }
    }
    else
    {
      /* The buffer was not sent, so it must be freed. */
      vPortFree((void*)pcPrintString);
    }
  }

  return xLength;
}
#else

static char cliBuffer[BSP_CLI_BUFFER_LEN];

uint16_t BSP_CLI_Printf(const char* fmt, ...)
{
  uint16_t len = 0;
  va_list args;
  va_start(args, fmt);
  vsnprintf(cliBuffer, BSP_CLI_BUFFER_LEN, fmt, args);
  len = BSP_CLI_Write(&cliBuffer[0], strlen(cliBuffer));
  va_end(args);
  return len;
}

#endif

#if (configBSP_CLI_ENABLE_TASK == 1)

static void BSP_CLI_Task(void * argument)
{
	char * pcReceivedString = NULL;

	/* Infinite loop */
	for(;;)
	{
		if(xQueueReceive(cliQueueHandle, &pcReceivedString, 0) == pdPASS)
		{
			BSP_CLI_Write(pcReceivedString, strlen(pcReceivedString));
			vPortFree((void*)pcReceivedString);
		}
	}
}

#endif
