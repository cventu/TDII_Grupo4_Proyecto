
/**********************************INCLUDES************************************/
#include <stdint.h>
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
