
/**********************************INCLUDES************************************/
#include "../inc/FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "../inc/PET_BOARD.h"
#include "../inc/RGB.h"
#include "../inc/BUZZER.h"
/******************************************************************************/


/*********************************PROTOTYPES***********************************/
static void initHardware(void);
static void task(void * a);
void task_rgb(void * a);
void task_buzzer(void * a);
/******************************************************************************/


/*********************************VARIABLES************************************/
const uint32_t OscRateIn = 12000000;
const uint32_t RTCOscRateIn = 32768;

xQueueHandle queue_rgb;
xQueueHandle queue_buzzer;
/******************************************************************************/

int main(void)
{
	queue_rgb = xQueueCreate(1, sizeof(uint8_t));
	queue_buzzer = xQueueCreate(1, sizeof(uint8_t));

	initHardware();

	xTaskCreate(task, (const char *)"task", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_rgb, (const char *)"task_rgb", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_buzzer, (const char *)"task_buzzer", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);

	vTaskStartScheduler();

	while (1)
	{
	}
}


static void initHardware(void)
{
	rgb_config_t rgb_cfg;
	buzzer_config_t buzzer_cfg;

	SystemCoreClockUpdate();

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

	rgb_set(OFF);
	buzzer_set(ALERT);
}


static void task(void * a)
{
	static uint8_t colour=0;

	while (1)
	{
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
	}
}



