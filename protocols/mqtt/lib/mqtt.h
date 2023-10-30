////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT (c) 2023
// NOMNIO d.o.o.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////
/**
 *	@file		mqtt.h
 *	@author   	Tibor Sovilj
 *	@date		25.10.2023
 *	@version	V0.0.1

 *	@brief 		Header file for the MQTT protocol.
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *	@addtogroup MQTT_API
 *	@{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////
#ifndef __MQTT_MODULE_H
#define __MQTT_MODULE_H

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "../config/mqtt_cfg.h"

////////////////////////////////////////////////////////////////////////////////
// API Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API Typedefs
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API Function Prototypes
////////////////////////////////////////////////////////////////////////////////
void        mqtt_app_start              (void);
BaseType_t  mqtt_send_signal_message    (mqtt_signal_message_e msgID);


#endif /* __MQTT_MODULE_H */

////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
///////////////////////////////////////////////////////////////////////////////




