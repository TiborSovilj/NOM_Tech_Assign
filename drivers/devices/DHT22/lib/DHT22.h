////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT (c) 2023
// NOMNIO d.o.o.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////
/**
 *	@file		DHT22.h
 *	@author   	Tibor Sovilj
 *	@date		20.10.2023
 *	@version	V0.0.1

 *	@brief 		Header file for the DHT22 temperature and humidty sensor.
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *	@addtogroup DHT22_API
 *	@{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////
#ifndef		__DHT22_MODULE_H
#define 	__DHT22_MODULE_H

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// API Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API Typedefs
////////////////////////////////////////////////////////////////////////////////

/**
 * 	Error states for the DHT22 semsor
 */
typedef enum
{
	eDHT22_OK 				= 0, 
 	eDHT22_CHECKSUM_ERROR,
 	eDHT22_TIMEOUT_ERROR,
}dht22_status_e;


////////////////////////////////////////////////////////////////////////////////
// API Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API Function Prototypes
////////////////////////////////////////////////////////////////////////////////
void 	dht22_task_start 		(void);
float 	dht22_get_humidity		(void);
float 	dht22_get_temperature	(void);


#endif /* __DHT22_MODULE_H */

////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
///////////////////////////////////////////////////////////////////////////////




