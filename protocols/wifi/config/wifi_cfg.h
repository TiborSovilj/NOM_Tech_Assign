////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT (c) 2023
// NOMNIO d.o.o.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////
/**
 *	@file		wifi_cfg.h
 *	@author   	Tibor Sovilj
 *	@date		12.10.2023
 *	@version	V1.0.0

 *	@brief 		Configuration Header file for the WiFi protocol.
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * 	@addtogroup WIFI_CONFIG
 * 	@{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////
#ifndef __WIFI_CFG_H
#define __WIFI_CFG_H

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "../lib/wifi.h"

////////////////////////////////////////////////////////////////////////////////
// CONFIG Definitions
////////////////////////////////////////////////////////////////////////////////
#define WIFI_AP_SSID                "ESP32_AP_NOM_TA"   // AP name
#define WIFI_AP_PASSWORD            "password123"       // AP password
#define WIFI_AP_CHANNEL             ( 1 )               // AP channel
#define WIFI_AP_SSID_HIDDEN         ( 0 )               // AP visibility
#define WIFI_AP_MAX_CONNECTIONS     ( 5 )               // AP max clients
#define WIFI_AP_BEACON_INTERVAL     ( 100 )             // AP beacon: 100 milliseconds recommended

#define WIFI_AP_IP                  "192.168.0.1"       // AP default IP
#define WIFI_AP_GATEWAY             "192.168.0.1"       // AP default Gateway (should be the same as the IP)
#define WIFI_AP_NETMASK             "255.255.255.0"     // AP netmask

#define WIFI_AP_BANDWIDTH           ( WIFI_BW_HT20 )    // AP bandwidth 20 MHz (40 MHz is the other option)
#define WIFI_STA_POWER_SAVE         ( WIFI_PS_NONE )    // Power save not used

#define WIFI_CONNECTION_RETRIES     ( 5 )               // Retry number on disconnect

#define WIFI_APP_TASK_STACK_SIZE    ( 4096 )            // Allocated stack space for WIFI app
#define WIFI_APP_TASK_PRIORITY      ( 5 )               // Priority of the WIFI app task

////////////////////////////////////////////////////////////////////////////////
// CONFIG Typedefs
////////////////////////////////////////////////////////////////////////////////

/**
 *  Forward declaration for events log structure.
 */
typedef struct wifi_app_events_log_s wifi_app_events_log_t;

/**
 *  Forward declaration for message queue structure.
 */
typedef struct wifi_app_queue_message_s wifi_app_queue_message_t;

/**
 *  Enum with wifi app events
 *  @ref    g_wifi_app_events_table[]
 *  @see    wifi_app_event_handler
 */
typedef enum
{
    eWIFI_EVENT_AP_START = 0 ,
    eWIFI_EVENT_AP_STOP,
    eWIFI_EVENT_AP_STACONNECTED,
    eWIFI_EVENT_AP_STADISCONNECTED,
    eWIFI_EVENT_STA_START,
    eWIFI_EVENT_STA_CONNECTED,
    eWIFI_EVENT_STA_DISCONNECTED,

    eWIFI_EVENT_STA_NUM_OF,
}wifi_app_events_e;

////////////////////////////////////////////////////////////////////////////////
// CONFIG Function Prototypes
////////////////////////////////////////////////////////////////////////////////
const wifi_app_events_log_t*    wifi_app_get_events_table   (void);
const wifi_app_queue_message_t* wifi_app_get_queue_message  (void);


#endif /* __WIFI_CFG_H  */

////////////////////////////////////////////////////////////////////////////////
/*!
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////