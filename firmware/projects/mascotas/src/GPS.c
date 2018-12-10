
/**********************************INCLUDES************************************/
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "../inc/UART.h"

#include "../inc/FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/******************************************************************************/


/**********************************DEFINES*************************************/
#define d2r (M_PI / 180.0)
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
double nmea_deg2double(double deg);
double calculate_distance(double lat1, double lon1, double lat2, double lon2);
/******************************************************************************/


/*********************************VARIABLES************************************/
extern xQueueHandle queue_coordinates;
/******************************************************************************/

uint8_t parse_frame(uint8_t * frame, uint8_t size)
{
	static uint8_t index=0;
	static uint8_t commas=0;
	coordinate_t new_coordinate;
	static uint8_t coord_index=0;
	static uint8_t satellites_string[5];
	static uint32_t satellites_int=0;
	uint8_t ret=0;
	static double aux_double;

	while(index < size)
	{
		if (frame[index] == ',')
		{
			commas++;
		}

		index++;
	}

	index=0;

	if (commas == 8)
	{
		while(frame[index] != ',')
		{
			index++;
		}

		index++;

		while(frame[index] != ',')
		{
			new_coordinate.latitude_string[coord_index]=frame[index];
			index++;
			coord_index++;
		}

		new_coordinate.latitude_string[coord_index]='\0';
		coord_index=0;
		index++;

		while(frame[index] != ',')
		{
			new_coordinate.longitude_string[coord_index]=frame[index];
			index++;
			coord_index++;
		}

		new_coordinate.longitude_string[coord_index]='\0';
		coord_index=0;
		index++;

		while(frame[index] != ',')
		{
			index++;
		}

		index++;

		while(frame[index] != ',')
		{
			index++;
		}

		index++;

		while(frame[index] != ',')
		{
			index++;
		}

		index++;

		while(frame[index] != ',')
		{
			satellites_string[coord_index]=frame[index];
			index++;
			coord_index++;
		}

		satellites_string[coord_index]='\0';
		coord_index=0;
		index++;

		satellites_int = strtol((char*)satellites_string, NULL, 10);

		if (satellites_int >= 4)
		{
			aux_double = strtod((char*)new_coordinate.latitude_string, NULL);
			aux_double *= (-1);
			new_coordinate.latitude = nmea_deg2double(aux_double);

			aux_double = strtod((char*)new_coordinate.longitude_string, NULL);
			aux_double *= (-1);
			new_coordinate.longitude = nmea_deg2double(aux_double);

			xQueueSend(queue_coordinates, &new_coordinate, portMAX_DELAY);
			ret=1;
		}
		else
		{
			ret=0;
		}
	}

	index=0;
	commas=0;
	coord_index=0;

	return ret;
}


double calculate_distance(double lat1, double lon1, double lat2, double lon2)
{
	static double pi80;
	static double r;
	static double dlat;
	static double dlon;
	static double a;
	static double c;
	static double z;
	static double ret;


	pi80 = M_PI / 180;
	lat1 *= pi80;
	lat2 *= pi80;
	lon1 *= pi80;
	lon2 *= pi80;

	r = 6372.797;
	dlat = lat2 - lat1;
	dlon = lon2 - lon1;

	a = sin(dlat/2) * sin(dlat/2) + cos(lat1) * cos(lat2) * sin(dlon/2) * sin(dlon/2);
	c = 2 * atan2(sqrt(a),sqrt(1-a));

	z = r*c;
	ret = z*1000;

	return ret;
	//return (r*c)*1000;
}



double nmea_deg2double(double deg)
{
	static double ret;
	static double decimales;

	ret = (int) deg/100 ;
	decimales = deg - (ret*100);
	decimales /= 60;
	ret += decimales;

	return ret;
}
