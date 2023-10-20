////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT (c) 2023
// NOMNIO d.o.o.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////
/**
 *	@file		DHT22.c
 *	@author   	Tibor Sovilj
 *	@date		20.10.2023
 *	@version	V0.0.1

 *	@brief 		Source file for the DHT22 temperature and humidty sensor.
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *	@addtogroup DHT22_STATIC
 *	@{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>

#include "esp_log.h"
#include "esp_system.h"
#include "rom/ets_sys.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "DHT22.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#define DHT22_REGISTER_BYTE_LENGHT 		( 5  )	//  40 = 5 * 8 Bits
#define DHT22_DATA_REGISTER_LENGHT		( 40 )

////////////////////////////////////////////////////////////////////////////////
// Static Typedefs
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Static Global Variables
////////////////////////////////////////////////////////////////////////////////

// Humidity value
static float g_dht22_humidity 		= 0.0f;

// Temperature value
static float g_dht22_temperature 	= 0.0f;


////////////////////////////////////////////////////////////////////////////////
// Static Function Prototypes
////////////////////////////////////////////////////////////////////////////////

static void				dht22_task				(void *p_arg);
static void 			dht22_errorHandler		(dht22_status_e status);
static uint8_t 			dht22_get_signal_level	(int usTimeOut, bool state );
static dht22_status_e 	dht22_read_register		(void);


////////////////////////////////////////////////////////////////////////////////
// Static Function Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 		DHT22 task which updates temperature and humidity within time 
 * 				interval defined as task delay. If the error occurs, function 
 * 				will not stop dispaching new temperature and humidity values
 * 				since none of the timeout intervals are predictable.
 * 
 * @param[in]	p_arg	void pointer for task arguments
 * @return 		void
 */
////////////////////////////////////////////////////////////////////////////////
static void dht22_task(void *p_arg)
{
	printf("Starting DHT task\n\n");

	for (;;)
	{
		dht22_errorHandler(dht22_read_register());

		// Manufacturer demands at least 2 seconds before reading again
		vTaskDelay(4000 / portTICK_PERIOD_MS);
	}
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 		Error handler for DHT22
 * 
 * @param[in]	response	Error state
 * @return 		void
 */
////////////////////////////////////////////////////////////////////////////////

static void dht22_errorHandler(dht22_status_e status)
{
	switch(status) {
	
		case eDHT22_TIMEOUT_ERROR :
			printf("Sensor Timeout\n");
			break;

		case eDHT22_CHECKSUM_ERROR:
			printf("CheckSum error\n");
			break;

		case eDHT22_OK:
			break;

		default :
			printf("Unknown error\n" );
	}
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 		Get next GPIO state on the DATA pin. Function counts the elapsed
 * 				time within which is expeced for the sensor to hold the particular
 * 				state. Function then returns the intiger value of the elapsed time
 * 				or -1 as an error code for the timeout error
 * 
 * @param[in]	usTimeout	Sensor timeout in microseconds
 * @param[in]	state		
 * @return 		Time in microseconds / error value -1
 */
////////////////////////////////////////////////////////////////////////////////
static uint8_t dht22_get_signal_level( int usTimeOut, bool state )
{
	int uSec = 0;
	
	while( gpio_get_level(DHT22_GPIO) == state ) {

		if( uSec > usTimeOut ) return -1;
		
		// adding i microsecond delay
		++uSec;
		ets_delay_us(1);		
	}
	
	return uSec;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 		Reads and parses data within 40-bit register, then updates 
 * 				global variables
 * 
 * @return 		Error code
 */
////////////////////////////////////////////////////////////////////////////////
static dht22_status_e dht22_read_register(void)
{
	dht22_status_e status;
	uint8_t checksum;
	int uSec = 0;

	uint8_t dhtData[DHT22_REGISTER_BYTE_LENGHT];
	uint8_t byteIndex = 0;
	uint8_t bitIndex = 7;

	// Erase internal buffer
	for (uint8_t i = 0; i < DHT22_REGISTER_BYTE_LENGHT; i++) 
	{
		dhtData[i]= 0;
	}

	// Send start signal to DHT22 sensor: set to LOW state, then wait 3ms for sensor to wake up
	gpio_set_direction( DHT22_GPIO, GPIO_MODE_OUTPUT );
	gpio_set_level( DHT22_GPIO, 0 );
	ets_delay_us( 3000 );			

	// Proceed with holding HIGH stata for 25 microseconds
	gpio_set_level( DHT22_GPIO, 1 );
	ets_delay_us( 25 );

	gpio_set_direction( DHT22_GPIO, GPIO_MODE_INPUT );		// change to input mode
  
	// DHT22 keeps the line in LOW state for 80us and then HIGH for additional 80us
	if( dht22_get_signal_level( 85, 0 ) < 0 ) 
	{
		status =  eDHT22_TIMEOUT_ERROR; 
		return status;
	}
	if( dht22_get_signal_level( 85, 1 ) < 0 )
	{
		status =  eDHT22_TIMEOUT_ERROR; 
		return status;
	}

	// If there are no errors, read the 40 data bits
	for( uint8_t i = 0; i < DHT22_DATA_REGISTER_LENGHT; i++ ) {

		// starts new data transmission with >50us low signal
		uSec = dht22_get_signal_level( 56, 0 );
		if( uSec<0 )
		{
			status =  eDHT22_TIMEOUT_ERROR; 
			return status;
		}

		// check to see if after >70us rx data is a 0 or a 1
		uSec = dht22_get_signal_level( 75, 1 );
		if( uSec<0 )
		{
			status =  eDHT22_TIMEOUT_ERROR; 
			return status;
		}

		// Add the current read to the output data
		if (uSec > 40) 
		{
			dhtData[ byteIndex ] |= (1 << bitIndex);
		}
	
		// Increment index to next byte
		if (bitIndex == 0) 
		{ 
			bitIndex = 7; 
			byteIndex++; 
		}
		else bitIndex--;
	}

	// Get humidity from Data[0] and Data[1].
	g_dht22_humidity = dhtData[0];
	g_dht22_humidity *= 0x100;					
	g_dht22_humidity += dhtData[1];
	g_dht22_humidity /= 10;						

	// Get temp from Data[2] and Data[3].
	g_dht22_temperature = dhtData[2] & 0x7F;	
	g_dht22_temperature *= 0x100;				
	g_dht22_temperature += dhtData[3];
	g_dht22_temperature /= 10;

	// In case of negative temperature, data is multiplied by -1.
	if( dhtData[2] & 0x80 )
	{
		g_dht22_temperature *= -1;
	} 			

	// Verify the checksum
	checksum = (dhtData[0] + dhtData[1] + dhtData[2] + dhtData[3]) & 0xFF;
	status = ( dhtData[4] == checksum ) ? eDHT22_OK : eDHT22_CHECKSUM_ERROR;

	return status;
}


////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *	@addtogroup DHT22_API
 *	@{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API Function Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 		Starting DHT22 task.
 *
 * @return 		void
 */
////////////////////////////////////////////////////////////////////////////////
void dht22_task_start (void)
{
	xTaskCreate(&dht22_task, "DHT22_task", DHT22_TASK_STACK_SIZE, NULL, DHT22_TASK_PRIORITY, NULL);
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 		Getter function for the humidity.

 * @return 		Float humidity value
 */
////////////////////////////////////////////////////////////////////////////////
float dht22_get_humidity(void) 
{
	 return g_dht22_humidity; 
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 		Getter function for the temperature.

 * @return 		Float temperature value
 */
////////////////////////////////////////////////////////////////////////////////
float dht22_get_temperature(void) 
{
	return g_dht22_temperature;
}

////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////







