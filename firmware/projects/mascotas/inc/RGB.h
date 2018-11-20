
/**********************************INCLUDES************************************/
#include <stdint.h>
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
