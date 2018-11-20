
/**********************************INCLUDES************************************/
#include <stdint.h>
#include "../inc/GPIO.h"

#include "../inc/FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/******************************************************************************/


/**********************************DEFINES*************************************/
#define OFF			0
#define BLUE		1
#define	GREEN		2
#define	CYAN		3
#define	RED			4
#define	MAGENTA	5
#define YELLOW	6
#define WHITE		7
/******************************************************************************/


/*********************************DATA TYPES***********************************/
typedef struct rgb_config_tag
{
	uint8_t red_port;
	uint8_t red_pin;
	uint8_t green_port;
	uint8_t green_pin;
	uint8_t blue_port;
	uint8_t blue_pin;
}rgb_config_t;
/******************************************************************************/


/*********************************PROTOTYPES***********************************/
void rgb_init(rgb_config_t * rgb_cfg);
void rgb_set(uint8_t colour);
/******************************************************************************/


/*********************************VARIABLES************************************/
static uint8_t red_port;
static uint8_t red_pin;
static uint8_t green_port;
static uint8_t green_pin;
static uint8_t blue_port;
static uint8_t blue_pin;

extern xQueueHandle queue_rgb;
/******************************************************************************/

void rgb_init(rgb_config_t * rgb_cfg)
{
	red_port = rgb_cfg->red_port;
	red_pin = rgb_cfg->red_pin;
	green_port = rgb_cfg->green_port;
	green_pin = rgb_cfg->green_pin;
	blue_port = rgb_cfg->blue_port;
	blue_pin = rgb_cfg->blue_pin;

	set_pinsel(red_port, red_pin, 0);
	set_dir(red_port, red_pin, 1);

	set_pinsel(green_port, green_pin, 0);
	set_dir(green_port, green_pin, 1);

	set_pinsel(blue_port, blue_pin, 0);
	set_dir(blue_port, blue_pin, 1);
}


void rgb_set(uint8_t colour)
{
	xQueueSend(queue_rgb, &colour, 0);
}


void task_rgb(void * a)
{
	static uint8_t colour;

	while (1)
	{
		xQueueReceive(queue_rgb, &colour, portMAX_DELAY);

		switch(colour)
		{
			case OFF:
				set_pin(red_port, red_pin, 0);
				set_pin(green_port, green_pin, 0);
				set_pin(blue_port, blue_pin, 0);
				break;

			case BLUE:
				set_pin(red_port, red_pin, 0);
				set_pin(green_port, green_pin, 0);
				set_pin(blue_port, blue_pin, 1);
				break;

			case GREEN:
				set_pin(red_port, red_pin, 0);
				set_pin(green_port, green_pin, 1);
				set_pin(blue_port, blue_pin, 0);
				break;

			case CYAN:
				set_pin(red_port, red_pin, 0);
				set_pin(green_port, green_pin, 1);
				set_pin(blue_port, blue_pin, 1);
				break;

			case RED:
				set_pin(red_port, red_pin, 1);
				set_pin(green_port, green_pin, 0);
				set_pin(blue_port, blue_pin, 0);
				break;

			case MAGENTA:
				set_pin(red_port, red_pin, 1);
				set_pin(green_port, green_pin, 0);
				set_pin(blue_port, blue_pin, 1);
				break;

			case YELLOW:
				set_pin(red_port, red_pin, 1);
				set_pin(green_port, green_pin, 1);
				set_pin(blue_port, blue_pin, 0);
				break;

			case WHITE:
				set_pin(red_port, red_pin, 1);
				set_pin(green_port, green_pin, 1);
				set_pin(blue_port, blue_pin, 1);
				break;
		}
	}
}

