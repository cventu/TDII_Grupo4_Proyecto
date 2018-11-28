
/**********************************INCLUDES************************************/
#include <string.h>
#include "../inc/FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "../inc/PET_BOARD.h"
#include "../inc/RGB.h"
#include "../inc/BUZZER.h"
#include "../inc/UART.h"
#include "../inc/NVM.h"
#include "../inc/ADC.h"
/******************************************************************************/


/*********************************PROTOTYPES***********************************/
static void initHardware(void);
static void task(void * a);
void task_rgb(void * a);
void task_buzzer(void * a);
static void task_adc(void * a);
/******************************************************************************/


/*********************************VARIABLES************************************/
const uint32_t OscRateIn = 12000000;
const uint32_t RTCOscRateIn = 32768;

xQueueHandle queue_rgb;
xQueueHandle queue_buzzer;
xQueueHandle queue_uart_0_tx;
xQueueHandle queue_uart_0_rx;
xQueueHandle queue_uart_1_tx;
xQueueHandle queue_uart_1_rx;
xQueueHandle queue_uart_2_tx;
xQueueHandle queue_uart_2_rx;
xQueueHandle queue_adc;
/******************************************************************************/

int main(void)
{
	queue_rgb = xQueueCreate(1, sizeof(uint8_t));
	queue_buzzer = xQueueCreate(1, sizeof(uint8_t));
	queue_uart_0_tx = xQueueCreate(100, sizeof(uint8_t));
	queue_uart_0_rx = xQueueCreate(100, sizeof(uint8_t));
	queue_uart_1_tx = xQueueCreate(100, sizeof(uint8_t));
	queue_uart_1_rx = xQueueCreate(100, sizeof(uint8_t));
	queue_uart_2_tx = xQueueCreate(100, sizeof(uint8_t));
	queue_uart_2_rx = xQueueCreate(100, sizeof(uint8_t));
	queue_adc = xQueueCreate(1, sizeof(uint16_t));

	initHardware();

	xTaskCreate(task, (const char *)"task", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_rgb, (const char *)"task_rgb", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_buzzer, (const char *)"task_buzzer", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_adc, (const char *)"task_adc", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);

	vTaskStartScheduler();

	while (1)
	{
	}
}


static void initHardware(void)
{
	rgb_config_t rgb_cfg;
	buzzer_config_t buzzer_cfg;
	uart_config_t uart_cfg;

	SystemCoreClockUpdate();

	nvm_init();

	rgb_cfg.red_port = RGB_RED_PORT;
	rgb_cfg.red_pin = RGB_RED_PIN;
	rgb_cfg.green_port = RGB_GREEN_PORT;
	rgb_cfg.green_pin = RGB_GREEN_PIN;
	rgb_cfg.blue_port = RGB_BLUE_PORT;
	rgb_cfg.blue_pin = RGB_BLUE_PIN;
	rgb_init(&rgb_cfg);

	buzzer_cfg.port = BUZZER_PORT;
	buzzer_cfg.pin = BUZZER_PIN;
	buzzer_init(&buzzer_cfg);

	uart_cfg.uart_number=UART_DEBUG;
	uart_cfg.baudrate=9600;
	uart_init(&uart_cfg);

	adc_init();

	rgb_set(OFF);
	buzzer_set(BUZZER_OFF);
}


static void task(void * a)
{
	//static uint8_t colour=1;
	//static uint8_t data;

	/*
	static char aux[50];
	static uint32_t reg;
	*/

	while (1)
	{
		vTaskDelay(5000 / portTICK_RATE_MS);
		adc_soc();

		/*
		nvm_set(STATUS, 21122112);
		nvm_set(LATITUDE, 604604);
		nvm_set(LONGITUDE, 15101510);
		nvm_set(NUMBER, 27052705);
		nvm_set(RADIUS, 12041204);
		uart_send_data(UART_DEBUG, (uint8_t*)"WROTE!\n", strlen("WROTE!\n"));
		*/

		/*
		reg = nvm_get(STATUS);
		sprintf(aux, "Status: %lu\n", reg);
		uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));

		reg = nvm_get(LATITUDE);
		sprintf(aux, "Latitude: %lu\n", reg);
		uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));

		reg = nvm_get(LONGITUDE);
		sprintf(aux, "Longitude: %lu\n", reg);
		uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));

		reg = nvm_get(NUMBER);
		sprintf(aux, "Number: %lu\n", reg);
		uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));

		reg = nvm_get(RADIUS);
		sprintf(aux, "Radius: %lu\n\n", reg);
		uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));
		*/

		/*
		buzzer_set(LOW_BAT);
		vTaskDelay(5000 / portTICK_RATE_MS);
		*/

		/*
		xQueueReceive(queue_uart_1_rx, &data, portMAX_DELAY);
		uart_send_data(UART_COMM, &data, 1);
		*/

		/*
		rgb_set(colour);

		colour++;

		if (colour == 8)
		{
			colour=0;
		}

		if (colour%2)
		{
			buzzer_set(BUZZER_ON);
		}
		else
		{
			buzzer_set(BUZZER_OFF);
		}

		vTaskDelay(2000 / portTICK_RATE_MS);
		*/
	}
}


static void task_adc(void * a)
{
	static uint16_t data;
	static char aux[50];

	while (1)
	{
		xQueueReceive(queue_adc, &data, portMAX_DELAY);
		sprintf(aux, "Value: %u\n", data);
		uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));
	}
}




