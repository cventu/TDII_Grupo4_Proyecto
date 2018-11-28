
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
/******************************************************************************/


/*********************************PROTOTYPES***********************************/
void nvm_init(void);
uint32_t nvm_get(uint8_t item);
void nvm_set(uint8_t item, uint32_t value);
/******************************************************************************/

void nvm_init(void)
{
	PCONP |= 0x01<<9;
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
