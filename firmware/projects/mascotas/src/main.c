
/**********************************INCLUDES************************************/
#include <string.h>
#include <stdlib.h>
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
#include "../inc/GPS.h"
#include "../inc/ADC.h"
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
#define	READ_SMS					7
#define	WAIT_READ_SMS			8
#define DELETE_SMS				9
#define	WAIT_DELETE_SMS		10
#define SEND_SMS_1				11
#define WAIT_SEND_SMS_1		12
#define	SEND_SMS_2				13
#define	WAIT_SEND_SMS_2		14
#define NEXT							15

#define	WAIT_GSM_INIT			0
#define	POWER_ON					1
#define WAIT_POWER_ON			2
#define CHECK_SIGNAL			3
#define WAIT_CHECK_SIGNAL	4
#define	GET_COORDINATE		5
#define	WAIT_COORDINATE		6

#define	OFFLINE						0
#define	ONLINE						1

#define	SZ_DATA_LOADED		0x12345678
/******************************************************************************/


/*********************************PROTOTYPES***********************************/
static void initHardware(void);
void task_rgb(void * a);
void task_buzzer(void * a);
static void task_adc(void * a);
static void task_gsm(void * a);
static void task_sim808_receive(void * a);
static void task_gps(void * a);
static void task_coordinates(void * a);
static void task_led(void * a);

static void store_phone_number(uint8_t * data);
static void store_sms_data(response_t response);
/******************************************************************************/


/*********************************VARIABLES************************************/
const uint32_t OscRateIn = 12000000;
const uint32_t RTCOscRateIn = 32768;

double sz_latitude;
double sz_longitude;
uint32_t sz_radius;
uint8_t phone_number[14];
uint32_t device_status;

uint8_t sms_check;
uint8_t gps_status;
uint8_t gsm_status;
uint8_t sms_hold;

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
xQueueHandle queue_coordinates;
xQueueHandle queue_sms;

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
	queue_adc = xQueueCreate(1, sizeof(uint16_t));
	queue_sim808 = xQueueCreate(5, sizeof(response_t));
	queue_coordinates = xQueueCreate(1, sizeof(coordinate_t));
	queue_sms = xQueueCreate(1, sizeof(coordinate_t));

	mutex_sim808 = xSemaphoreCreateMutex();
	binary_sim808 = xSemaphoreCreateCounting(1,0);

	initHardware();

	xTaskCreate(task_rgb, (const char *)"task_rgb", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_buzzer, (const char *)"task_buzzer", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_adc, (const char *)"task_adc", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_gsm, (const char *)"task_gsm", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_sim808_receive, (const char *)"task_sim808_receive", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_gps, (const char *)"task_gps", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_coordinates, (const char *)"task_coordinates", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
	xTaskCreate(task_led, (const char *)"task_led", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);

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

	adc_init();

	rgb_set(OFF);
	buzzer_set(BUZZER_OFF);
}


static void task_coordinates(void * a)
{
	static coordinate_t new_coordinate;
	static double distance;
	static char aux[50];

	while (1)
	{
		xQueueReceive(queue_coordinates, &new_coordinate, portMAX_DELAY);

		if (device_status == SZ_DATA_LOADED)
		{
			sprintf(aux, "Latitud: %lf\n", new_coordinate.latitude);
			uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));

			sprintf(aux, "Longitud: %lf\n", new_coordinate.longitude);
			uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));

			distance = calculate_distance(sz_latitude, sz_longitude, new_coordinate.latitude, new_coordinate.longitude);

			sprintf(aux, "Distancia: %lf\n\n", distance);
			uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));

			if (distance > sz_radius && gsm_status == ONLINE && sms_hold == 0)
			{
				sms_hold=1;
				xQueueSend(queue_sms, &new_coordinate, 0);
			}
		}
	}
}


static void task_gsm(void * a)
{
	static uint8_t status=0;
	static response_t last_response;
	static coordinate_t coord;
	static char aux[100];

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
						gsm_status=ONLINE;
						uart_send_data(UART_DEBUG, (uint8_t*)"Registrado a la Red!\n", strlen("Registrado a la Red!\n"));
						status = READ_SMS;
					}
					else
					{
						vTaskDelay(5000 / portTICK_RATE_MS);
						status = NETWORK_REG;
					}
				}
				else
				{
					vTaskDelay(5000 / portTICK_RATE_MS);
					status = NETWORK_REG;
				}
				xSemaphoreGive(mutex_sim808);
				break;

			case READ_SMS:
				xSemaphoreTake(mutex_sim808, portMAX_DELAY);
				vTaskDelay(10 / portTICK_RATE_MS);
				xQueueReset(queue_sim808);
				sms_check=1;
				uart_send_data(UART_COMM, (uint8_t*)"AT+CMGL=\"ALL\"\r", strlen("AT+CMGL=\"ALL\"\r"));
				status = WAIT_READ_SMS;
				break;

			case WAIT_READ_SMS:
				if (xQueueReceive(queue_sim808, &last_response, 5000) == pdTRUE)
				{
					if (!memcmp(last_response.data, "+CMGL: 1", strlen("+CMGL: 1")))
					{
						store_phone_number(last_response.data);

						xQueueReceive(queue_sim808, &last_response, portMAX_DELAY);
						store_sms_data(last_response);
						status = DELETE_SMS;
					}
					else
					{
						status = SEND_SMS_1;
					}
				}
				else
				{
					status = READ_SMS;
				}
				xSemaphoreGive(mutex_sim808);
				break;

			case DELETE_SMS:
				xSemaphoreTake(mutex_sim808, portMAX_DELAY);
				vTaskDelay(10 / portTICK_RATE_MS);
				xQueueReset(queue_sim808);
				uart_send_data(UART_COMM, (uint8_t*)"AT+CMGDA=\"DEL ALL\"\r", strlen("AT+CMGDA=\"DEL ALL\"\r"));
				status = WAIT_DELETE_SMS;
				break;

			case WAIT_DELETE_SMS:
				if (xQueueReceive(queue_sim808, &last_response, 5000) == pdTRUE)
				{
					if (!memcmp(last_response.data, "OK", 2))
					{
						uart_send_data(UART_DEBUG, (uint8_t*)"SMS Borrado!\n\n", strlen("SMS Borrado!\n\n"));
						status = SEND_SMS_1;
					}
					else
					{
						status = DELETE_SMS;
					}
				}
				else
				{
					status = DELETE_SMS;
				}
				xSemaphoreGive(mutex_sim808);
				break;

			case SEND_SMS_1:
				if (xQueueReceive(queue_sms, &coord, 0) == pdTRUE)
				{

					xSemaphoreTake(mutex_sim808, portMAX_DELAY);
					vTaskDelay(10 / portTICK_RATE_MS);
					xQueueReset(queue_sim808);
					sprintf(aux, "AT+CMGS=\"%s\"\r",phone_number);
					uart_send_data(UART_COMM, (uint8_t*)aux, strlen(aux));
					status = WAIT_SEND_SMS_1;


					/*
					uart_send_data(UART_DEBUG, (uint8_t*)"MANDA!\n\n", strlen("MANDA!\n\n"));
					status=NEXT;
					*/
				}
				else
				{
					vTaskDelay(3000 / portTICK_RATE_MS);
					status = READ_SMS;
				}
				break;

			case WAIT_SEND_SMS_1:
				if (xQueueReceive(queue_sim808, &last_response, 10000) == pdTRUE)
				{
					if (!memcmp(last_response.data, ">", 1))
					{
						status = SEND_SMS_2;
					}
				}
				xSemaphoreGive(mutex_sim808);
				break;

			case SEND_SMS_2:
				xSemaphoreTake(mutex_sim808, portMAX_DELAY);
				vTaskDelay(10 / portTICK_RATE_MS);
				xQueueReset(queue_sim808);
				sprintf(aux,"Ubicacion mascota:\n\nLatitud: %lf\nLongitud: %lf\x1a", coord.latitude, coord.longitude);
				uart_send_data(UART_COMM, (uint8_t*)aux, strlen(aux));
				status = WAIT_SEND_SMS_2;
				break;

			case WAIT_SEND_SMS_2:
				if (xQueueReceive(queue_sim808, &last_response, 60000) == pdTRUE)
				{
					if (!memcmp(last_response.data, "+CMGS:", 6))
					{
						uart_send_data(UART_DEBUG, (uint8_t*)"SMS Enviado!\n\n", strlen("SMS Enviado!\n\n"));
						status = NEXT;
					}
					else
					{
						uart_send_data(UART_DEBUG, (uint8_t*)"SMS ERROR\n\n", strlen("SMS ERROR\n\n"));
						status = NEXT;
					}
				}
				else
				{
					uart_send_data(UART_DEBUG, (uint8_t*)"SMS TIMEOUT!\n\n", strlen("SMS TIMEOUT!\n\n"));
					status = NEXT;
				}
				xSemaphoreGive(mutex_sim808);
				break;

			case NEXT:
				vTaskDelay(60000 / portTICK_RATE_MS);
				sms_hold=0;
				status = READ_SMS;
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
				vTaskDelay(100 / portTICK_RATE_MS);
				xQueueReset(queue_sim808);
				uart_send_data(UART_COMM, (uint8_t*)"AT+CGPSSTATUS?\r", strlen("AT+CGPSSTATUS?\r"));
				status = WAIT_CHECK_SIGNAL;
				break;

			case WAIT_CHECK_SIGNAL:
				if (xQueueReceive(queue_sim808, &last_response, 5000) == pdTRUE)
				{
					uart_send_data(UART_DEBUG, (uint8_t*)"RTA: ", 5);
					uart_send_data(UART_DEBUG, last_response.data, last_response.size);
					uart_send_data(UART_DEBUG, (uint8_t*)"\n", 1);

					if (!memcmp(last_response.data, "+CGPSSTATUS: Location 3D Fix", strlen("+CGPSSTATUS: Location 3D Fix")))
					{
						uart_send_data(UART_DEBUG, (uint8_t*)"GPS Obtenido!\n\n", strlen("GPS Obtenido!\n\n"));
						gps_status = ONLINE;
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
				vTaskDelay(3000 / portTICK_RATE_MS);
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
						if (parse_frame(last_response.data, last_response.size))
						{
							status = GET_COORDINATE;
						}
						else
						{
							gps_status = OFFLINE;
							status = CHECK_SIGNAL;
						}
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

					if (data == '>')
					{
						xQueueSend(queue_sim808, &last_response, portMAX_DELAY);
						index=0;
						status=0;
					}
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

					if (sms_check)
					{
						if (last_response.size != 2)
						{
							status=2;
							last_response.size=0;
						}
						else
						{
							status=0;
						}
					}
					else
					{
						status=0;
					}

					sms_check=0;
				}
				break;
		}
	}
}


static void task_adc(void * a)
{
	static uint16_t data;
	static char aux[50];
	static double counts;
	static double tension;

	while (1)
	{
		adc_soc();

		xQueueReceive(queue_adc, &data, portMAX_DELAY);
		counts = data;
		tension = counts * 0.000967441860;
		sprintf(aux, "Tension: %.2lf V\n", tension);
		uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));

		if (tension < 2)
		{
			buzzer_set(LOW_BAT);
		}

		vTaskDelay(5000 / portTICK_RATE_MS);
	}
}


static void store_phone_number(uint8_t * data)
{
	static uint8_t index=0;
	static uint8_t i=0;
	static uint32_t phone_number_nvm;

	/* index=1 para que no capture el '+' inicial */
	index=1;

	while(data[index] != '+')
	{
		index++;
	}

	for(i=0 ; i<13 ; i++)
	{
		phone_number[i]=data[index];
		index++;
	}

	phone_number[i]='\0';

	uart_send_data(UART_DEBUG, (uint8_t*)"\n", 1);
	uart_send_data(UART_DEBUG, (uint8_t*)"Numero: ", strlen("Numero: "));
	uart_send_data(UART_DEBUG, phone_number, strlen((char*)phone_number));
	uart_send_data(UART_DEBUG, (uint8_t*)"\n", 1);

	phone_number_nvm = strtol((char*)(phone_number + 3), NULL, 10);
	nvm_set(NUMBER, phone_number_nvm);
}


static void store_sms_data(response_t response)
{
	static uint8_t i=0;
	static uint8_t index=0;
	static uint8_t commas=0;
	static uint8_t latitude_string[20];
	static uint8_t longitude_string[20];
	static uint8_t radius_string[20];
	static char aux[50];
	static double aux_double;
	static uint32_t aux_reg;

	while(i < response.size)
	{
		if (response.data[i]==',')
		{
			commas++;
		}

		i++;
	}

	if (commas==2)
	{
		i=0;

		while(response.data[i] != ',')
		{
			latitude_string[index]=response.data[i];
			index++;
			i++;
		}

		latitude_string[index]='\0';
		index=0;
		i++;

		while(response.data[i] != ',')
		{
			longitude_string[index]=response.data[i];
			index++;
			i++;
		}

		longitude_string[index]='\0';
		index=0;
		i++;

		while(i < response.size)
		{
			radius_string[index]=response.data[i];
			index++;
			i++;
		}

		radius_string[index]='\0';
	}

	i=0;
	index=0;
	commas=0;

	sz_radius = strtol((char*)radius_string, NULL, 10);
	nvm_set(RADIUS, sz_radius);

	sprintf(aux, "Radio: %lu\n", sz_radius);
	uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));

	aux_double = strtod((char*)latitude_string, NULL);
	sz_latitude = aux_double;

	if(aux_double < 0)
	{
		aux_double *= (-1);
	}

	aux_double *= 10000;
	aux_reg = (uint32_t) aux_double;
	nvm_set(LATITUDE, aux_reg);

	sprintf(aux, "Latitud SZ: %lf\n", sz_latitude);
	uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));

	aux_double = strtod((char*)longitude_string, NULL);
	sz_longitude = aux_double;

	if(aux_double < 0)
	{
		aux_double *= (-1);
	}

	aux_double *= 10000;
	aux_reg = (uint32_t) aux_double;
	nvm_set(LONGITUDE, aux_reg);

	sprintf(aux, "Longitud SZ: %lf\n", sz_longitude);
	uart_send_data(UART_DEBUG, (uint8_t*)aux, strlen(aux));

	device_status = SZ_DATA_LOADED;
	nvm_set(STATUS, device_status);
}



static void task_led(void * a)
{
	while (1)
	{
		vTaskDelay(1000 / portTICK_RATE_MS);

		if (gps_status == OFFLINE && gsm_status == OFFLINE)
		{
			rgb_set(RED);
		}
		else if (gps_status == OFFLINE)
		{
			rgb_set(BLUE);
		}
		else if (gsm_status == OFFLINE)
		{
			rgb_set(YELLOW);
		}
		else if (device_status != SZ_DATA_LOADED)
		{
			rgb_set(MAGENTA);
		}
		else
		{
			rgb_set(GREEN);
		}
	}
}
