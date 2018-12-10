
/**********************************INCLUDES************************************/
#include <stdint.h>
/******************************************************************************/


/*********************************DATA TYPES***********************************/
typedef struct coordinate_tag
{
	uint8_t latitude_string[20];
	uint8_t longitude_string[20];
	double latitude;
	double longitude;
}coordinate_t;
/******************************************************************************/


/*********************************PROTOTYPES***********************************/
uint8_t parse_frame(uint8_t * frame, uint8_t size);
double calculate_distance(double lat1, double lon1, double lat2, double lon2);
/******************************************************************************/
