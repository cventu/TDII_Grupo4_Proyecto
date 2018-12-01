
/**********************************INCLUDES************************************/
#include <string.h>
#include "../inc/FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "../inc/PET_BOARD.h"
#include "../inc/RGB.h"
#include "../inc/BUZZER.h"
#include "../inc/UART.h"
#include "../inc/NVM.h"
//#include "../inc/ADC.h"
/******************************************************************************/


/*********************************DATA TYPES***********************************/
typedef struct response_tag
{
	uint8_t data[150];
	uint8_t size;
}response_t;
/******************************************************************************/


/**********************************DEFINES*************************************/
#define DISABLE_ECHO			0
#define WAIT_ECHO					1
#define	SMS_READY					2
#define	TEXT_MODE					3
#define	WAIT_TEXT_MODE		4
#define NETWORK_REG				5
#define	WAIT_NETWORK_REG	6
#define CHANGE_BAND				7
#define WAIT_CHANGE_BAND	8
#define NEXT							9

#define	WAIT_GSM_INIT			0
#define	POWER_ON					1
#define WAIT_POWER_ON			2
#define CHECK_SIGNAL			3
#define WAIT_CHECK_SIGNAL	4
#define	GET_COORDINATE		5
#define	WAIT_COORDINATE		6
/******************************************************************************/


/*********************************PROTOTYPES***********************************/
static void initHardware(void);
static void task(void * a);
void task_rgb(void * a);
void task_buzzer(void * a);
//static void task_adc(void * a);
static void task_gsm(void * a);
static void task_sim808_receive(void * a);
static void task_gps(void * a);
/******************************************************************************/


/*********************************VARIABLES************************************/
const uint32_t OscRateIn = 12000000;
const uint32_t RTCOscRateIn = 32768;

xQueueHandle queue_rgb;
xQueueHandle queue_buzzer;
xQueueHandle queue_uart_0_tx;
xQueueHandle queue_uart_0_rx;
xQueueHandle queue_uart_1_tx;
xQueueHandle queue_uart_1_rx;
xQueueHandle queue_uart_2_tx;
xQueueHandle queue_uart_2_rx;
xQueueHandle queue_adc;
xQueueHandle queue_sim808;

xSemaphoreHandle mutex_sim808;
xSemaphoreHandle binary_sim808;
/******************************************************************************/

int main(void)
{
	queue_rgb = xQueueCreate(1, sizeof(uint8_t));
	queue_buzzer = xQueueCreate(1, sizeof(uint8_t));
	queue_uart_0_tx = xQueueCreate(100, sizeof(uint8_t));
	queue_uart_0_rx = xQueueCreate(100, sizeof(uint8_t));
	queue_uart_1_tx = xQueueCreate(100, sizeof(uint8_t));
	queue_uart_1_rx = xQueueCreate(100, sizeof(uint8_t));
	queue_uart_2_tx = xQueueCreate(100, sizeof(uint8_t));
	queue_uart_2_rx = xQueueCreate(100, sizeof(uint8_t));
	//queue_adc = xQueueCreate(1, sizeof(uint16_t));
	queue_sim808 = xQueueCreate(5, sizeof(response_t));

	mutex_sim808 = xSemaphoreCreateMutex();
	binary_sim808 = xSemaphoreCreateCounting(1,0);

	initHardware();

	xTaskCreate(task, (const char *)"task", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_rgb, (const char *)"task_rgb", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_buzzer, (const char *)"task_buzzer", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	//xTaskCreate(task_adc, (const char *)"task_adc", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_gsm, (const char *)"task_gsm", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_sim808_receive, (const char *)"task_sim808_receive", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_gps, (const char *)"task_gps", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);

	vTaskStartScheduler();

	while (1)
	{
	}
}


static void initHardware(void)
{
	rgb_config_t rgb_cfg;
	buzzer_config_t buzzer_cfg;
	uart_config_t uart_cfg;

	SystemCoreClockUpdate();

	nvm_init();

	rgb_cfg.red_port = RGB_RED_PORT;
	rgb_cfg.red_pin = RGB_RED_PIN;
	rgb_cfg.green_port = RGB_GREEN_PORT;
	rgb_cfg.green_pin = RGB_GREEN_PIN;
	rgb_cfg.blue_port = RGB_BLUE_PORT;
	rgb_cfg.blue_pin = RGB_BLUE_PIN;
	rgb_init(&rgb_cfg);

	buzzer_cfg.port = BUZZER_PORT;
	buzzer_cfg.pin = BUZZER_PIN;
	buzzer_init(&buzzer_cfg);

	uart_cfg.uart_number=UART_DEBUG;
	uart_cfg.baudrate=9600;
	uart_init(&uart_cfg);

	uart_cfg.uart_number=UART_COMM;
	uart_cfg.baudrate=9600;
	uart_init(&uart_cfg);

	//adc_init();

	rgb_set(OFF);
	buzzer_set(BUZZER_OFF);
}


static void task(void * a)
{
	//static uint8_t colour=0;
	//static uint8_t data;

	/*
	static char aux[50];
	static uint32_t reg;
	*/

	while (1)
	{
		vTaskDelay(5000 / portTICK_RATE_MS);
		//uart_send_data(UART_COMM, (uint8_t*)"ATE0\r", 5);
		//vTaskDelay(3000 / portTICK_RATE_MS);

		/*
		vTaskDelay(5000 / portTICK_RATE_MS);
		adc_soc();
		*/

		/*
		nvm_set(STATUS, 21122112);
		nvm_set(LATITUDE, 604604);
		nvm_set(LONGITUDE, 15101510);
		nvm_set(NUMBER, 27052705);
		nvm_set(RADIUS, 12041204);
		uart_send_data(UART_DEBUG, (uint8_t*)"WROTE!\n", strlen("WROTE!\n"));
		*/

		/*
		reg = nvm_get(STATUS);
		sprintf(aux, "Status: %lu\n", reg);
		uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));

		reg = nvm_get(LATITUDE);
		sprintf(aux, "Latitude: %lu\n", reg);
		uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));

		reg = nvm_get(LONGITUDE);
		sprintf(aux, "Longitude: %lu\n", reg);
		uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));

		reg = nvm_get(NUMBER);
		sprintf(aux, "Number: %lu\n", reg);
		uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));

		reg = nvm_get(RADIUS);
		sprintf(aux, "Radius: %lu\n\n", reg);
		uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));
		*/

		/*
		buzzer_set(LOW_BAT);
		vTaskDelay(1000 / portTICK_RATE_MS);
		*/

		/*
		xQueueReceive(queue_uart_1_rx, &data, portMAX_DELAY);
		uart_send_data(UART_COMM, &data, 1);
		*/

		/*
		rgb_set(colour);

		colour++;

		if (colour == 8)
		{
			colour=0;
		}

		if (colour%2)
		{
			buzzer_set(BUZZER_ON);
		}
		else
		{
			buzzer_set(BUZZER_OFF);
		}

		vTaskDelay(2000 / portTICK_RATE_MS);
		*/
	}
}


static void task_gsm(void * a)
{
	static uint8_t status=0;
	static response_t last_response;
	static uint8_t reg_retries=0;

	while (1)
	{
		switch(status)
		{
			case DISABLE_ECHO:
				vTaskDelay(10 / portTICK_RATE_MS);
				xQueueReset(queue_sim808);
				uart_send_data(UART_COMM, (uint8_t*)"ATE0\r", 5);
				status = WAIT_ECHO;
				break;

			case WAIT_ECHO:
				if (xQueueReceive(queue_sim808, &last_response, 5000) == pdTRUE)
				{
					if (!memcmp(last_response.data, "OK", 2))
					{
						uart_send_data(UART_DEBUG, (uint8_t*)"Echo Deshabilitado!\n", strlen("Echo Deshabilitado!\n"));
						status = SMS_READY;
					}
				}
				else
				{
					status = DISABLE_ECHO;
				}
				break;

			case SMS_READY:
				xQueueReceive(queue_sim808, &last_response, portMAX_DELAY);

				if (!memcmp(last_response.data, "SMS Ready", 9))
				{
					uart_send_data(UART_DEBUG, (uint8_t*)"SMS Ready Recibido!\n", strlen("SMS Ready Recibido!\n"));
					status = TEXT_MODE;
					xSemaphoreGive(binary_sim808);
				}
				break;

			case TEXT_MODE:
				xSemaphoreTake(mutex_sim808, portMAX_DELAY);
				vTaskDelay(10 / portTICK_RATE_MS);
				xQueueReset(queue_sim808);
				uart_send_data(UART_COMM, (uint8_t*)"AT+CMGF=1\r", strlen("AT+CMGF=1\r"));
				status = WAIT_TEXT_MODE;
				break;

			case WAIT_TEXT_MODE:
				if (xQueueReceive(queue_sim808, &last_response, 5000) == pdTRUE)
				{
					if (!memcmp(last_response.data, "OK", 2))
					{
						uart_send_data(UART_DEBUG, (uint8_t*)"Configurado Modo Texto!\n", strlen("Configurado Modo Texto!\n"));
						status = NETWORK_REG;
					}
					else
					{
						status = TEXT_MODE;
					}
				}
				else
				{
					status = TEXT_MODE;
				}
				xSemaphoreGive(mutex_sim808);
				break;

			case NETWORK_REG:
				xSemaphoreTake(mutex_sim808, portMAX_DELAY);
				vTaskDelay(10 / portTICK_RATE_MS);
				xQueueReset(queue_sim808);
				uart_send_data(UART_COMM, (uint8_t*)"AT+CREG?\r", strlen("AT+CREG?\r"));
				//uart_send_data(UART_COMM, (uint8_t*)"AT+CSQ\r", strlen("AT+CSQ\r"));
				status = WAIT_NETWORK_REG;
				break;

			case WAIT_NETWORK_REG:
				if (xQueueReceive(queue_sim808, &last_response, 5000) == pdTRUE)
				{
					uart_send_data(UART_DEBUG, (uint8_t*)"RTA: ", 5);
					uart_send_data(UART_DEBUG, last_response.data, last_response.size);
					uart_send_data(UART_DEBUG, (uint8_t*)"\n", 1);

					if (!memcmp(last_response.data, "+CREG: 0,1", 10))
					{
						reg_retries=0;
						uart_send_data(UART_DEBUG, (uint8_t*)"Registrado a la Red!\n", strlen("Registrado a la Red!\n"));
						status = NEXT;
					}
					else
					{
						vTaskDelay(5000 / portTICK_RATE_MS);
						reg_retries++;
						if (reg_retries > 10)
						{
							status=CHANGE_BAND;
							reg_retries=0;
						}
						else
						{
							status = NETWORK_REG;
						}
					}
				}
				else
				{
					vTaskDelay(5000 / portTICK_RATE_MS);
					status = NETWORK_REG;
				}
				xSemaphoreGive(mutex_sim808);
				break;

			case CHANGE_BAND:
				xSemaphoreTake(mutex_sim808, portMAX_DELAY);
				vTaskDelay(10 / portTICK_RATE_MS);
				xQueueReset(queue_sim808);
				uart_send_data(UART_COMM, (uint8_t*)"AT+CBAND=\"GSM850_MODE\"\r", strlen("AT+CBAND=\"GSM850_MODE\"\r"));
				status = WAIT_CHANGE_BAND;
				break;

			case WAIT_CHANGE_BAND:
				if (xQueueReceive(queue_sim808, &last_response, 5000) == pdTRUE)
				{
					if (!memcmp(last_response.data, "OK", 2))
					{
						uart_send_data(UART_DEBUG, (uint8_t*)"Cambio de banda!\n", strlen("Cambio de banda!\n"));
						status = NETWORK_REG;
					}
					else
					{
						status = CHANGE_BAND;
					}
				}
				else
				{
					status = CHANGE_BAND;
				}
				xSemaphoreGive(mutex_sim808);
				break;

			case NEXT:
				vTaskDelay(5000 / portTICK_RATE_MS);
				break;
		}
	}
}



static void task_gps(void * a)
{
	static uint8_t status=0;
	static response_t last_response;

	while(1)
	{
		switch(status)
		{
			case WAIT_GSM_INIT:
				xSemaphoreTake(binary_sim808, portMAX_DELAY);
				status = POWER_ON;
				break;

			case POWER_ON:
				xSemaphoreTake(mutex_sim808, portMAX_DELAY);
				vTaskDelay(10 / portTICK_RATE_MS);
				xQueueReset(queue_sim808);
				uart_send_data(UART_COMM, (uint8_t*)"AT+CGPSPWR=1\r", strlen("AT+CGPSPWR=1\r"));
				status = WAIT_POWER_ON;
				break;

			case WAIT_POWER_ON:
				if (xQueueReceive(queue_sim808, &last_response, 5000) == pdTRUE)
				{
					if (!memcmp(last_response.data, "OK", 2))
					{
						uart_send_data(UART_DEBUG, (uint8_t*)"GPS Encendido!\n", strlen("GPS Encendido!\n"));
						status = CHECK_SIGNAL;
					}
					else
					{
						status = POWER_ON;
					}
				}
				else
				{
					status = POWER_ON;
				}
				xSemaphoreGive(mutex_sim808);
				break;

			case CHECK_SIGNAL:
				xSemaphoreTake(mutex_sim808, portMAX_DELAY);
				vTaskDelay(10 / portTICK_RATE_MS);
				xQueueReset(queue_sim808);
				uart_send_data(UART_COMM, (uint8_t*)"AT+CGPSSTATUS?\r", strlen("AT+CGPSSTATUS?\r"));
				status = WAIT_CHECK_SIGNAL;
				break;

			case WAIT_CHECK_SIGNAL:
				if (xQueueReceive(queue_sim808, &last_response, 5000) == pdTRUE)
				{
					if (!memcmp(last_response.data, "+CGPSSTATUS: Location 3D Fix", strlen("+CGPSSTATUS: Location 3D Fix")))
					{
						uart_send_data(UART_DEBUG, (uint8_t*)"GPS Obtenido!\n", strlen("GPS Obtenido!\n"));
						status = GET_COORDINATE;
					}
					else
					{
						status = CHECK_SIGNAL;
					}
				}
				else
				{
					status = CHECK_SIGNAL;
				}
				xSemaphoreGive(mutex_sim808);
				break;

			case GET_COORDINATE:
				xSemaphoreTake(mutex_sim808, portMAX_DELAY);
				vTaskDelay(10 / portTICK_RATE_MS);
				xQueueReset(queue_sim808);
				uart_send_data(UART_COMM, (uint8_t*)"AT+CGPSINF=0\r", strlen("AT+CGPSINF=0\r"));
				status = WAIT_COORDINATE;
				break;

			case WAIT_COORDINATE:
				if (xQueueReceive(queue_sim808, &last_response, 5000) == pdTRUE)
				{
					if (!memcmp(last_response.data, "+CGPSINF:", strlen("+CGPSINF:")))
					{
						uart_send_data(UART_DEBUG, (uint8_t*)"COORD: ", strlen("COORD: "));
						uart_send_data(UART_DEBUG, last_response.data, last_response.size);
						uart_send_data(UART_DEBUG, (uint8_t*)"\n", 1);
						status = GET_COORDINATE;
					}
					else
					{
						status = GET_COORDINATE;
					}
				}
				else
				{
					status = GET_COORDINATE;
				}
				xSemaphoreGive(mutex_sim808);
				vTaskDelay(3000 / portTICK_RATE_MS);
				break;
		}
	}
}


static void task_sim808_receive(void * a)
{
	static uint8_t data;
	static uint8_t index=0;
	static uint8_t status=0;
	static response_t last_response;

	while(1)
	{
		xQueueReceive(queue_uart_1_rx, &data, portMAX_DELAY);

		switch(status)
		{
			case 0:
				if (data == 0x0D)
				{
					status=1;
				}
				break;

			case 1:
				if (data == 0x0A)
				{
					status=2;
					last_response.size=0;
				}
				break;

			case 2:
				if (data != 0x0D)
				{
					last_response.data[index++]=data;
					last_response.size++;
				}
				else
				{
					xQueueSend(queue_sim808, &last_response, portMAX_DELAY);
					status=3;
				}
				break;

			case 3:
				if (data == 0x0A)
				{
					index=0;
					status=0;
				}
				break;
		}
	}
}


/*
static void task_adc(void * a)
{
	static uint16_t data;
	static char aux[50];

	while (1)
	{
		xQueueReceive(queue_adc, &data, portMAX_DELAY);
		sprintf(aux, "Value: %u\n", data);
		uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));
	}
}
*/

