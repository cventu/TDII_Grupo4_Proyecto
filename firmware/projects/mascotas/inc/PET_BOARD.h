
/**********************************DEFINES*************************************/
#define	BASEBOARD	0
#define	TDII			1

#define	PET_BOARD	BASEBOARD

#if PET_BOARD == BASEBOARD
	#define	RGB_RED_PORT		2
	#define RGB_RED_PIN			3
	#define	RGB_GREEN_PORT	2
	#define RGB_GREEN_PIN		2
	#define	RGB_BLUE_PORT		2
	#define	RGB_BLUE_PIN		1
	#define BUZZER_PORT			0
	#define BUZZER_PIN			28
	#define	UART_DEBUG			0
	#define	UART_COMM				1
	#define ADC_PORT				1
	#define	ADC_PIN					31
	#define	ADC_CHANNEL			5
#elif PET_BOARD == TDII
	#define	RGB_RED_PORT		0
	#define RGB_RED_PIN			2
	#define	RGB_GREEN_PORT	0
	#define RGB_GREEN_PIN		3
	#define	RGB_BLUE_PORT		0
	#define	RGB_BLUE_PIN		22
	#define BUZZER_PORT			2
	#define	BUZZER_PIN			0
	#define	UART_DEBUG			2
	#define	UART_COMM				1
	#define ADC_PORT				1
	#define	ADC_PIN					31
	#define	ADC_CHANNEL			5
#endif
/******************************************************************************/
