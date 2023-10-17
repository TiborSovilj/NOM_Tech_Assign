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

static const wifi_app_events_log_t g_wifi_app_events_table[] = 
{
    [ WIFI_EVENT_AP_START           ] = {.eventMsg = "WIFI_EVENT_AP_START\n\n"           },
    [ WIFI_EVENT_AP_STOP            ] = {.eventMsg = "WIFI_EVENT_AP_STOP\n\n"            },
    [ WIFI_EVENT_AP_STACONNECTED    ] = {.eventMsg = "WIFI_EVENT_AP_STACONNECTED\n\n"    },
    [ WIFI_EVENT_AP_STADISCONNECTED ] = {.eventMsg = "WIFI_EVENT_AP_STADISCONNECTED\n\n" },
    [ WIFI_EVENT_STA_START          ] = {.eventMsg = "WIFI_EVENT_STA_START\n\n"          },
    [ WIFI_EVENT_STA_CONNECTED      ] = {.eventMsg = "WIFI_EVENT_STA_CONNECTED\n\n"      },
    [ WIFI_EVENT_STA_DISCONNECTED   ] = {.eventMsg = "WIFI_EVENT_STA_DISCONNECTED\n\n"   },
//    [ IP_EVENT_STA_GOT_IP           ] = {.eventMsg = "IP_EVENT_STA_GOT_IP\n\n"           }
};

static const wifi_app_queue_message_t g_wifi_app_queue_msg[eWIFI_APP_MSG_NUM_OF]= 
{
    [ eWIFI_APP_MSG_START_HTTP_SERVER           ] =  {.msgID = eWIFI_APP_MSG_START_HTTP_SERVER           , .msgContent = "WIFI_APP_MSG_START_HTTP_SERVER\n\n"           },
    [ eWIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER ] =  {.msgID = eWIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER , .msgContent = "WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER\n\n" },
    [ eWIFI_APP_MSG_STA_CONNECTED_GOT_IP        ] =  {.msgID = eWIFI_APP_MSG_STA_CONNECTED_GOT_IP        , .msgContent = "WIFI_APP_MSG_STA_CONNECTED_GOT_IP\n\n"        }
};
////////////////////////////////////////////////////////////////////////////////
// CONFIG Functions definitions
////////////////////////////////////////////////////////////////////////////////
const wifi_app_events_log_t* wifi_app_get_events_table(void)
{
    return &g_wifi_app_events_table[0];
}

const wifi_app_queue_message_t* wifi_app_get_queue_message(void)
{
    return g_wifi_app_queue_msg;
}
////////////////////////////////////////////////////////////////////////////////
/**
 *  @} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////
