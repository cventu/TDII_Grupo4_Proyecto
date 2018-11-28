
/**********************************INCLUDES************************************/
#include <stdint.h>
#include "../inc/GPIO.h"
#include "../inc/PET_BOARD.h"

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

#define	AD0CR			(*((volatile uint32_t *) 0x40034000))

#define	AD0DR			((volatile uint32_t *) 0x40034010)
#define	AD0DR0 		AD0DR[0]
#define	AD0DR1		AD0DR[1]
#define	AD0DR2		AD0DR[2]
#define	AD0DR3		AD0DR[3]
#define	AD0DR4		AD0DR[4]
#define	AD0DR5		AD0DR[5]
#define	AD0DR6		AD0DR[6]
#define	AD0DR7		AD0DR[7]

#define	AD0INTEN	(*((volatile uint32_t *) 0x4003400C))

#define	IPR 			((volatile uint32_t *)0xE000E400UL)
#define	IPR0			IPR[0]
#define	IPR1			IPR[1]
#define	IPR2			IPR[2]
#define	IPR3			IPR[3]
#define	IPR4			IPR[4]
#define	IPR5			IPR[5]
#define	IPR6			IPR[6]
#define	IPR7			IPR[7]
#define	IPR8			IPR[8]

#define	ISER			((volatile uint32_t *)0xE000E100UL)
#define	ICER			((volatile uint32_t *)0xE000E180UL)
#define	ISER0			ISER[0]
#define	ISER1			ISER[1]
#define	ICER0			ICER[0]
#define	ICER1			ICER[1]
/******************************************************************************/


/*********************************PROTOTYPES***********************************/
void adc_init(void);
void adc_soc(void);
void adc_eoc(void);
/******************************************************************************/


/*********************************VARIABLES************************************/
extern xQueueHandle queue_adc;
uint32_t global;
/******************************************************************************/

void adc_init(void)
{
	PCONP |= (1 << 12);
	AD0CR |= (1 << 21);

	PCLKSEL0 &= ~(0x03 << 24);
	PCLKSEL0 |= (0x03 << 24);

	set_pinsel(ADC_PORT, ADC_PIN, 3);
	set_pinmode(ADC_PORT, ADC_PIN, 2);

	AD0CR &= ~(0xFFFF);
	AD0CR |= (1 << ADC_CHANNEL);

	AD0INTEN &= ~(0x01<<8);
	AD0INTEN |= (0x01<< ADC_CHANNEL);

	IPR5 &= (0x1F << 19);
	IPR5 |= (0x1F << 19);

	ISER0 |= (0x01 << 22);
}


void ADC_IRQHandler (void)
{
	portBASE_TYPE HigherPriorityTaskWoken = 0;
	unsigned int valor_adc[7]; // Defino un vector valor_adc para guardar los valores de los registros AD0DRx.
	volatile uint16_t valor_final_adc;

	global++;

	valor_adc[5] = AD0DR5;					// DODR5 GUARDA EL VALOR DE CUENTA DEL ADC PERTENECIENTE AL  POTENCIOMETRO

	//if((valor_adc[5]>>30) == 0x02)			 // Solamente entro al if si DONE = 1 y OVERRUN = 0.
	//{										 								 // Cualquier otra combinación, descarto la conversión y espero la siguiente.
		valor_adc[5] = valor_adc[5] & 0x0000FFF0;
		valor_final_adc = valor_adc[5]>>4;      // Paso los valores a la variable global valor_final_adc
		xQueueSendFromISR(queue_adc, &valor_final_adc, &HigherPriorityTaskWoken);
		//adc_soc();
	//}

	portEND_SWITCHING_ISR(HigherPriorityTaskWoken);
}


void adc_soc(void)
{
	AD0CR |= (1 << 21); // Energizamos el ADC.
	AD0CR |= (1 << 24); // Solicitamos conversion
}


void adc_eoc(void)
{
	AD0CR &= ~(1 << 21); // Apago el conversor
}







