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
static osMessageQId cliQueueID;
static osThreadId cliTaskID;
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

	    if (cliQueueID == NULL)
		{
			/* Create Queue */
			osMessageQDef(CLI_Queue, BSP_CLI_QUEUE_LEN, char*);
			cliQueueID = osMessageCreate (osMessageQ(CLI_Queue), NULL);
		}

	    if (cliTaskID == NULL)
	    {
	    	/* Create Task */
			osThreadDef(CLI_Task, BSP_CLI_Task, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
			cliTaskID = osThreadCreate(osThread(CLI_Task), NULL);
	    }

	    if (cliTaskID != NULL)
	    {
	    	xReturn = 1;
	    }
	    else
	    {
			/* Could not create the task, so delete the queue again. */
	    	osMessageDelete( cliQueueID );
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
  void * pcPrintString = NULL;

  /* Allocate a buffer to hold the log message. */
  pcPrintString = pvPortMalloc(BSP_CLI_BUFFER_LEN);

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
    	if (osMessagePut(cliQueueID, (uint32_t)pcPrintString, osWaitForever) != osOK)
    	{
            /* The buffer was not sent so must be freed again. */
            vPortFree(pcPrintString );
        }
    }
    else
    {
      /* The buffer was not sent, so it must be freed. */
      vPortFree(pcPrintString);
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
	uint32_t PreviousWakeTime;
	osEvent event;

	/* Initialize the PreviousWakeTime variable with the current time. */
	PreviousWakeTime = osKernelSysTick();

	/* Infinite loop */
	for(;;)
	{
		/* Get the message from the queue */
		event = osMessageGet(cliQueueID, osWaitForever);
		if (event.status == osEventMessage)
		{
			BSP_CLI_Write((char*)event.value.v, strlen((char*)event.value.v));
			vPortFree((void*)event.value.v);
		}

		osDelayUntil( &PreviousWakeTime, 10);
	}
}

#endif
