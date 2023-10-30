////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT (c) 2023
// NOMNIO d.o.o.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////
/**
 *	@file		wifi.h
 *	@author   	Tibor Sovilj
 *	@date		12.10.2023
 *	@version	V1.0.0

 *	@brief 		Header file for the WiFi protocol.
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *	@addtogroup WIFI_API
 *	@{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////
#ifndef __WIFI_MODULE_H
#define __WIFI_MODULE_H

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "esp_netif.h"
#include "esp_wifi_types.h"
#include "freertos/FreeRTOS.h"

////////////////////////////////////////////////////////////////////////////////
// API Definitions
////////////////////////////////////////////////////////////////////////////////
#define WIFI_MAX_SSID_LENGTH        ( 32 )              // IEEE standard maximum
#define WIFI_MAX_PASSWORD_LENGTH    ( 64 )              // IEEE standard maximum
#define WIFI_MIN_PASSWORD_LENGTH    ( 8 )               // Less than that causes runtime errors
#define MAX_CONNECTION_RETRIES		( 5 )				// Retry number on disconnect

////////////////////////////////////////////////////////////////////////////////
// API Typedefs
////////////////////////////////////////////////////////////////////////////////

/**
 *  Function pointer typedef for an event when the WiFi is connected.
 */
typedef void (*wifi_connected_event_callback_t)(void);

/**
 *  Message enums for the WiFi application task
 */
typedef enum
{
    eWIFI_APP_MSG_START_HTTP_SERVER = 0,
    eWIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER,
    eWIFI_APP_MSG_STA_CONNECTED_GOT_IP,
    eWIFI_APP_MSG_STA_DISCONNECTED,

    eWIFI_APP_MSG_NUM_OF
} wifi_app_message_e;

/**
 *  Structure for the message queue
 */
typedef struct wifi_app_queue_message_s
{
    wifi_app_message_e  msgID;
    char*               msgContent;
};

/**
 *  Structure for event messages.
 */
typedef struct wifi_app_events_log_s
{
    char* eventMsg;
};

////////////////////////////////////////////////////////////////////////////////
// API Variables
////////////////////////////////////////////////////////////////////////////////
extern esp_netif_t *esp_netif_sta;
extern esp_netif_t *esp_netif_ap;

////////////////////////////////////////////////////////////////////////////////
// API Function Prototypes
////////////////////////////////////////////////////////////////////////////////
void            wifi_app_start              (void);
BaseType_t      wifi_app_send_message       (wifi_app_message_e msgID);
wifi_config_t*  wifi_app_get_wifi_config    (void);
void            wifi_app_set_callback       (wifi_connected_event_callback_t callback);
void            wifi_app_call_callback      (void);

#endif /* __WIFI_MODULE_H */

////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
///////////////////////////////////////////////////////////////////////////////