
/**********************************INCLUDES************************************/
#include <stdint.h>
/******************************************************************************/


/**********************************DEFINES*************************************/
#define	PINSEL			((volatile uint32_t  *) 0x4002C000UL)
#define	PINMODE			((volatile uint32_t  *) 0x4002C040UL)
#define	GPIO				((volatile uint32_t  *) 0x2009C000UL)
#define	PINMODE_OD	((volatile uint32_t  *) 0x4002C068UL)
/******************************************************************************/


/*********************************PROTOTYPES***********************************/
void set_pinsel(uint8_t port, uint8_t pin, uint8_t function);
void set_pinmode(uint8_t port, uint8_t pin, uint8_t mode);
void set_mode_od( uint8_t port , uint8_t pin , uint8_t dir);
void set_dir(uint8_t port, uint8_t pin, uint8_t dir);
void set_pin(uint8_t port, uint8_t pin, uint8_t state);
uint8_t get_pin(uint8_t port, uint8_t pin);
void toggle_pin (uint8_t port, uint8_t pin);
/******************************************************************************/

void set_pinsel(uint8_t port, uint8_t pin, uint8_t function)
{
	port = port * 2 + pin / 16;
	pin = ( pin % 16 ) * 2;
	PINSEL[port] = PINSEL[port] & ( ~ ( 3 << pin ) );
	PINSEL[port] = PINSEL[port] | ( function << pin );
}


void set_pinmode(uint8_t port, uint8_t pin, uint8_t mode)
{
	port = port * 2 + pin / 16;
	pin = ( pin % 16 ) * 2;
	PINMODE[port] = PINMODE[port] & ( ~ ( 3 << pin ) );
	PINMODE[port] = PINMODE[port] | ( mode << pin );
}


void set_mode_od( uint8_t port , uint8_t pin , uint8_t dir)
{
	PINMODE_OD[ port ] = PINMODE_OD[ port ] & ( ~ ( 1 << pin ) );
	PINMODE_OD[ port ] = PINMODE_OD[ port ] | ( dir << pin );
}


void set_dir(uint8_t port, uint8_t pin, uint8_t dir)
{
	port = port * 8;

	GPIO[port] = GPIO[port] & ( ~ ( 1 << pin ) );
	GPIO[port] = GPIO[port] | ( dir << pin );
}


void set_pin(uint8_t port, uint8_t pin, uint8_t state)
{
	if (state == 1)
	{
		port = port * 8 + 6;	//FIOSET
		GPIO[port] |= ( 0x01 << pin );
	}
	else
	{
		port = port * 8 + 7;	//FIOCLR
		GPIO[port] |= ( 0x01 << pin );
	}
}


uint8_t get_pin(uint8_t port, uint8_t pin)
{
	port = port * 8 + 5;

	return (GPIO[port] >> pin) & 1;
}


void toggle_pin (uint8_t port, uint8_t pin)
{
	uint8_t aux;

	aux = get_pin (port, pin);

	if (aux == 1)
	{
		set_pin(port, pin, 0);
	}
	else
	{
		set_pin(port, pin, 1);
	}
}
