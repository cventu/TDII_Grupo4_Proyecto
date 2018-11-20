
/**********************************INCLUDES************************************/
#include <stdint.h>
#include "../inc/GPIO.h"

#include "../inc/FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/******************************************************************************/


/**********************************DEFINES*************************************/
#define	BUZZER_OFF		0
#define BUZZER_ON			1
#define	LOW_BAT				2
#define	ALERT					3
/******************************************************************************/


/*********************************DATA TYPES***********************************/
typedef struct buzzer_config_tag
{
	uint8_t port;
	uint8_t pin;
}buzzer_config_t;
/******************************************************************************/


/*********************************PROTOTYPES***********************************/
void buzzer_init(buzzer_config_t * buzzer_cfg);
void buzzer_set(uint8_t status);
/******************************************************************************/


/*********************************VARIABLES************************************/
static uint8_t buzzer_port;
static uint8_t buzzer_pin;

extern xQueueHandle queue_buzzer;
/******************************************************************************/

void buzzer_init(buzzer_config_t * buzzer_cfg)
{
	buzzer_port = buzzer_cfg->port;
	buzzer_pin = buzzer_cfg->pin;

	set_pinsel(buzzer_port, buzzer_pin, 0);
	set_dir(buzzer_port, buzzer_pin, 1);
}


void buzzer_set(uint8_t status)
{
	xQueueSend(queue_buzzer, &status, 0);
}


void task_buzzer(void * a)
{
	static uint8_t status = BUZZER_OFF;

	while (1)
	{
		xQueueReceive(queue_buzzer, &status, portMAX_DELAY);

		if (status == BUZZER_OFF)
		{
			set_pin(buzzer_port, buzzer_pin, 1);
		}
		else
		{
			set_pin(buzzer_port, buzzer_pin, 0);
		}
	}
	/*
	set_pin(buzzer_port, buzzer_pin, 0);
	vTaskDelay(150 / portTICK_RATE_MS);
	set_pin(buzzer_port, buzzer_pin, 1);
	vTaskDelay(150 / portTICK_RATE_MS);
	set_pin(buzzer_port, buzzer_pin, 0);
	vTaskDelay(150 / portTICK_RATE_MS);
	set_pin(buzzer_port, buzzer_pin, 1);
	vTaskDelay(5000 / portTICK_RATE_MS);
	*/
}
