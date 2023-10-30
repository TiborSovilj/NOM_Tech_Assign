////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT (c) 2023
// NOMNIO d.o.o.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////
/**
 *	@file		wifi_cfg.c
 *	@author   	Tibor Sovilj
 *	@date		12.10.2023
 *	@version	V0.0.1

 *	@brief 		Configuration source file for the WiFi protocol.
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *  @addtogroup WIFI_CONFIG
 *  @{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include "wifi_cfg.h"
#include "esp_wifi_types.h"
#include "esp_netif_types.h"


////////////////////////////////////////////////////////////////////////////////
// CONFIG Variables and Tables
////////////////////////////////////////////////////////////////////////////////

// Configuration table for events logging
static const wifi_app_events_log_t g_wifi_app_events_table[] = 
{
    [ WIFI_EVENT_AP_START           ] = {.eventMsg = "WIFI_EVENT_AP_START"           },
    [ WIFI_EVENT_AP_STOP            ] = {.eventMsg = "WIFI_EVENT_AP_STOP"            },
    [ WIFI_EVENT_AP_STACONNECTED    ] = {.eventMsg = "WIFI_EVENT_AP_STACONNECTED"    },
    [ WIFI_EVENT_AP_STADISCONNECTED ] = {.eventMsg = "WIFI_EVENT_AP_STADISCONNECTED" },
    [ WIFI_EVENT_STA_START          ] = {.eventMsg = "WIFI_EVENT_STA_START"          },
    [ WIFI_EVENT_STA_CONNECTED      ] = {.eventMsg = "WIFI_EVENT_STA_CONNECTED"      },
    [ WIFI_EVENT_STA_DISCONNECTED   ] = {.eventMsg = "WIFI_EVENT_STA_DISCONNECTED"   },
//    [ IP_EVENT_STA_GOT_IP           ] = {.eventMsg = "IP_EVENT_STA_GOT_IP\n\n"           }
};

// Configuraiton table for message queue
static const wifi_app_queue_message_t g_wifi_app_queue_msg[eWIFI_APP_MSG_NUM_OF]= 
{
    [ eWIFI_APP_MSG_START_HTTP_SERVER           ] =  {.msgID = eWIFI_APP_MSG_START_HTTP_SERVER           , .msgContent = "WIFI_APP_MSG_START_HTTP_SERVER\n\n"           },
    [ eWIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER ] =  {.msgID = eWIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER , .msgContent = "WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER\n\n" },
    [ eWIFI_APP_MSG_STA_CONNECTED_GOT_IP        ] =  {.msgID = eWIFI_APP_MSG_STA_CONNECTED_GOT_IP        , .msgContent = "WIFI_APP_MSG_STA_CONNECTED_GOT_IP\n\n"        }
};


////////////////////////////////////////////////////////////////////////////////
// CONFIG Functions definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief		Getter function for configuration table that contains
 *              event logging messages.
 * 
 * @return		Pointer to a table.
 */
////////////////////////////////////////////////////////////////////////////////
const wifi_app_events_log_t* wifi_app_get_events_table(void)
{
    return &g_wifi_app_events_table[0];
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief		Getter function for configuration table that contains
 *              queue messages.
 * 
 * @return		Pointer to a table.
 */
////////////////////////////////////////////////////////////////////////////////
const wifi_app_queue_message_t* wifi_app_get_queue_message(void)
{
    return g_wifi_app_queue_msg;
}


////////////////////////////////////////////////////////////////////////////////
/**
 *  @} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////
