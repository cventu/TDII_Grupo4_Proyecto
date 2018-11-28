
/**********************************INCLUDES************************************/
#include <stdint.h>
#include "../inc/GPIO.h"

#include "../inc/FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/******************************************************************************/


/**********************************DEFINES*************************************/
#define PCONP			(*((volatile uint32_t *) 0x400FC0C4UL))
#define	PCLKSEL		((volatile uint32_t *) 0x400FC1A8UL)
#define	PCLKSEL0	PCLKSEL[0]
#define	PCLKSEL1	PCLKSEL[1]

#define	ISER			((volatile uint32_t *) 0xE000E100UL)
#define	ISER0			ISER[0]

#define	DIR_UART0	((volatile uint32_t *) 0x4000C000UL)
#define	U0RBR			DIR_UART0[0]
#define	U0THR			DIR_UART0[0]
#define	U0DLL			DIR_UART0[0]
#define	U0IER			DIR_UART0[1]
#define	U0DLM			DIR_UART0[1]
#define	U0IIR			DIR_UART0[2]
#define	U0LCR			DIR_UART0[3]

#define DIR_UART1	((volatile uint32_t *) 0x40010000UL)
#define	U1RBR			DIR_UART1[0]
#define	U1THR			DIR_UART1[0]
#define	U1DLL			DIR_UART1[0]
#define	U1IER			DIR_UART1[1]
#define	U1DLM			DIR_UART1[1]
#define	U1IIR			DIR_UART1[2]
#define	U1LCR			DIR_UART1[3]

#define	DIR_UART2	((volatile uint32_t *) 0x40098000UL)
#define	U2RBR			DIR_UART2[0]
#define	U2THR			DIR_UART2[0]
#define	U2DLL			DIR_UART2[0]
#define	U2IER			DIR_UART2[1]
#define	U2DLM			DIR_UART2[1]
#define	U2IIR			DIR_UART2[2]
#define	U2LCR			DIR_UART2[3]

#define	DIR_UART3	((volatile uint32_t *) 0x4009C000UL)
#define	U3RBR			DIR_UART3[0]
#define	U3THR			DIR_UART3[0]
#define	U3DLL			DIR_UART3[0]
#define	U3IER			DIR_UART3[1]
#define	U3DLM			DIR_UART3[1]
#define	U3IIR			DIR_UART3[2]
#define	U3LCR			DIR_UART3[3]
/******************************************************************************/


/*********************************DATA TYPES***********************************/
typedef struct uart_config_tag
{
	uint8_t uart_number;
	uint32_t baudrate;
}uart_config_t;
/******************************************************************************/


/*********************************PROTOTYPES***********************************/
void uart_init(uart_config_t * uart_cfg);
void uart_send_data(uint8_t uart, uint8_t * data, uint16_t data_size);
/******************************************************************************/


/*********************************VARIABLES************************************/
uint8_t tx_in_progress[3];
extern xQueueHandle queue_uart_0_tx;
extern xQueueHandle queue_uart_0_rx;
extern xQueueHandle queue_uart_1_tx;
extern xQueueHandle queue_uart_1_rx;
extern xQueueHandle queue_uart_2_tx;
extern xQueueHandle queue_uart_2_rx;
/******************************************************************************/

void uart_init(uart_config_t * uart_cfg)
{
	uint32_t Fdiv = (SystemCoreClock / 16) / uart_cfg->baudrate;

	switch(uart_cfg->uart_number)
	{
		case 0:
			PCONP |= 0x01<<3;
			PCLKSEL0 &= ~(0x03<<6);
			PCLKSEL0 |= (0x01<<6);
			set_pinsel(0, 2, 1);
			set_pinsel(0, 3, 1);
			U0LCR = 0x83;
			U0DLM = Fdiv/256;
			U0DLL = Fdiv%256;
			U0LCR = 0x03;
			U0IER = 0x03;
			ISER0 |= (1<<5);
			break;

		case 1:
			PCONP |= 0x01<<4;
			PCLKSEL0 &= ~(0x03<<8);
			PCLKSEL0 |= (0x01<<8);
			set_pinsel(0, 15, 1);
			set_pinsel(0, 16, 1);
			U1LCR = 0x83;
			U1DLM = Fdiv/256;
			U1DLL = Fdiv%256;
			U1LCR = 0x03;
			U1IER = 0x03;
			ISER0 |= (1<<6);
			break;

		case 2:
			PCONP |= 0x01<<24;
			PCLKSEL1 &= ~(0x03<<16);
			PCLKSEL1 |= (0x01<<16);
			set_pinsel(0, 10, 1);
			set_pinsel(0, 11, 1);
			U2LCR = 0x83;
			U2DLM = Fdiv/256;
			U2DLL = Fdiv%256;
			U2LCR = 0x03;
			U2IER = 0x03;
			ISER0 |= (1<<7);
			break;
	}
}


void uart_send_data(uint8_t uart, uint8_t * data, uint16_t data_size)
{
	uint16_t data_index=0;
	uint8_t byte;

	switch(uart)
	{
		case 0:
			while(data_index < data_size)
			{
				xQueueSend(queue_uart_0_tx, &(data[data_index]), portMAX_DELAY);
				data_index++;
			}

			if (tx_in_progress[0] == 0)
			{
				tx_in_progress[0]=1;
				xQueueReceive(queue_uart_0_tx, &byte, portMAX_DELAY);
				U0THR = byte;
			}
			break;

		case 1:
			while(data_index < data_size)
			{
				xQueueSend(queue_uart_1_tx, &(data[data_index]), portMAX_DELAY);
				data_index++;
			}

			if (tx_in_progress[1] == 0)
			{
				tx_in_progress[1]=1;
				xQueueReceive(queue_uart_1_tx, &byte, portMAX_DELAY);
				U1THR = byte;
			}
			break;

		case 2:
			while(data_index < data_size)
			{
				xQueueSend(queue_uart_2_tx, &(data[data_index]), portMAX_DELAY);
				data_index++;
			}

			if (tx_in_progress[2] == 0)
			{
				tx_in_progress[2]=1;
				xQueueReceive(queue_uart_2_tx, &byte, portMAX_DELAY);
				U2THR = byte;
			}
			break;
	}
}


void UART0_IRQHandler (void)
{
	uint8_t iir;
	uint8_t tx_byte;
	uint8_t rx_byte;
	int ret;
	portBASE_TYPE HigherPriorityTaskWoken = 0;

	iir = U0IIR;

	if (iir & 0x02)
	{
		ret= xQueueReceiveFromISR(queue_uart_0_tx, &tx_byte, &HigherPriorityTaskWoken);

		if (ret == pdFALSE)
		{
			tx_in_progress[0] = 0;
		}
		else
		{
			U0THR = tx_byte;
		}
	}

	if (iir & 0x04)
	{
		rx_byte = U0RBR;
		xQueueSendFromISR(queue_uart_0_rx, &rx_byte, &HigherPriorityTaskWoken);
	}

	portEND_SWITCHING_ISR(HigherPriorityTaskWoken);
}


void UART1_IRQHandler (void)
{
	uint8_t iir;
	uint8_t tx_byte;
	uint8_t rx_byte;
	int ret;
	portBASE_TYPE HigherPriorityTaskWoken = 0;

	iir = U1IIR;

	if (iir & 0x02)
	{
		ret= xQueueReceiveFromISR(queue_uart_1_tx, &tx_byte, &HigherPriorityTaskWoken);

		if (ret == pdFALSE)
		{
			tx_in_progress[1] = 0;
		}
		else
		{
			U1THR = tx_byte;
		}
	}

	if (iir & 0x04)
	{
		rx_byte = U1RBR;
		xQueueSendFromISR(queue_uart_1_rx, &rx_byte, &HigherPriorityTaskWoken);
	}

	portEND_SWITCHING_ISR(HigherPriorityTaskWoken);
}


void UART2_IRQHandler (void)
{
	uint8_t iir;
	uint8_t tx_byte;
	uint8_t rx_byte;
	int ret;
	portBASE_TYPE HigherPriorityTaskWoken = 0;

	iir = U2IIR;

	if (iir & 0x02)
	{
		ret= xQueueReceiveFromISR(queue_uart_2_tx, &tx_byte, &HigherPriorityTaskWoken);

		if (ret == pdFALSE)
		{
			tx_in_progress[2] = 0;
		}
		else
		{
			U2THR = tx_byte;
		}
	}

	if (iir & 0x04)
	{
		rx_byte = U2RBR;
		xQueueSendFromISR(queue_uart_2_rx, &rx_byte, &HigherPriorityTaskWoken);
	}

	portEND_SWITCHING_ISR(HigherPriorityTaskWoken);
}
