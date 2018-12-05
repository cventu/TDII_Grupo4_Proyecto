
/**********************************INCLUDES************************************/
#include <stdint.h>
/******************************************************************************/


/**********************************DEFINES*************************************/
#define PCONP			(*((volatile uint32_t *) 0x400FC0C4UL))

#define	DIR_GPREG	((volatile uint32_t *) 0x40024044UL)
#define	GPREG0			DIR_GPREG[0]
#define	GPREG1			DIR_GPREG[1]
#define	GPREG2			DIR_GPREG[2]
#define	GPREG3			DIR_GPREG[3]
#define	GPREG4			DIR_GPREG[4]

#define	STATUS		0
#define	LATITUDE	1
#define LONGITUDE	2
#define	NUMBER		3
#define	RADIUS		4

#define	SZ_DATA_LOADED		0x12345678
/******************************************************************************/


/*********************************PROTOTYPES***********************************/
void nvm_init(void);
uint32_t nvm_get(uint8_t item);
void nvm_set(uint8_t item, uint32_t value);
/******************************************************************************/


/*********************************VARIABLES************************************/
extern double sz_latitude;
extern double sz_longitude;
extern uint32_t sz_radius;
extern uint32_t phone_number;
extern uint32_t device_status;
/******************************************************************************/

void nvm_init(void)
{
	uint32_t aux_reg;

	PCONP |= 0x01<<9;

	device_status = nvm_get(STATUS);

	if (device_status == SZ_DATA_LOADED)
	{
		aux_reg = nvm_get(LATITUDE);
		sz_latitude = (double)aux_reg;
		sz_latitude /= 10000;
		sz_latitude *= (-1);

		aux_reg = nvm_get(LONGITUDE);
		sz_longitude = (double)aux_reg;
		sz_longitude /= 10000;
		sz_longitude *= (-1);

		sz_radius = nvm_get(RADIUS);
		phone_number = nvm_get(NUMBER);
	}
}


uint32_t nvm_get(uint8_t item)
{
	static uint32_t reg;

	switch(item)
	{
		case STATUS:
			reg = GPREG0;
			break;

		case LATITUDE:
			reg = GPREG1;
			break;

		case LONGITUDE:
			reg = GPREG2;
			break;

		case NUMBER:
			reg = GPREG3;
			break;

		case RADIUS:
			reg = GPREG4;
			break;
	}

	return reg;
}


void nvm_set(uint8_t item, uint32_t value)
{
	switch(item)
	{
		case STATUS:
			GPREG0 = value;
			break;

		case LATITUDE:
			GPREG1 = value;
			break;

		case LONGITUDE:
			GPREG2 = value;
			break;

		case NUMBER:
			GPREG3 = value;
			break;

		case RADIUS:
			GPREG4 = value;
			break;
	}
}
