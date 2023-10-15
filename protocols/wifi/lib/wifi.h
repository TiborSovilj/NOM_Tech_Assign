////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT (c) 2023
// NOMNIO d.o.o.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////
/**
 *	@file		wifi.h
 *	@author   	Tibor Sovilj
 *	@date		12.10.2023
 *	@version	V0.0.1

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
#define WIFI_AP_SSID "ESP32_AP_NOM_TA" // AP name
#define WIFI_AP_PASSWORD "password123"     // AP password
#define WIFI_AP_CHANNEL 1              // AP channel
#define WIFI_AP_SSID_HIDDEN 0          // AP visibility
#define WIFI_AP_MAX_CONNECTIONS 5      // AP max clients
#define WIFI_AP_BEACON_INTERVAL 100    // AP beacon: 100 milliseconds recommended

// LIB: default staticly define IPs for init, later modified by DHCP
#define WIFI_AP_IP "192.168.0.1"        // AP default IP
#define WIFI_AP_GATEWAY "192.168.0.1"   // AP default Gateway (should be the same as the IP)
#define WIFI_AP_NETMASK "255.255.255.0" // AP netmask

#define WIFI_AP_BANDWIDTH WIFI_BW_HT20   // AP bandwidth 20 MHz (40 MHz is the other option)
#define WIFI_STA_POWER_SAVE WIFI_PS_NONE // Power save not used
#define MAX_SSID_LENGTH 32               // IEEE standard maximum
#define MAX_PASSWORD_LENGTH 64           // IEEE standard maximum
#define MAX_CONNECTION_RETRIES 5         // Retry number on disconnect

// TO project_config.h
#define WIFI_APP_TASK_STACK_SIZE 4096
#define WIFI_APP_TASK_PRIORITY 5
#define WIFI_APP_TASK_CORE_ID 0

////////////////////////////////////////////////////////////////////////////////
// API Typedefs
////////////////////////////////////////////////////////////////////////////////
/**
 *  Message enums for the WiFi application task
 */
typedef enum wifi_app_message
{
    WIFI_APP_MSG_START_HTTP_SERVER = 0,
    WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER,
    WIFI_APP_MSG_STA_CONNECTED_GOT_IP,
} wifi_app_message_e;

/**
 * Structure for the message queue
 */
typedef struct wifi_app_queue_message
{
    wifi_app_message_e msgID;
} wifi_app_queue_message_t;

////////////////////////////////////////////////////////////////////////////////
// API Variables
////////////////////////////////////////////////////////////////////////////////
extern esp_netif_t *esp_netif_sta;
extern esp_netif_t *esp_netif_ap;

////////////////////////////////////////////////////////////////////////////////
// API Function Prototypes
////////////////////////////////////////////////////////////////////////////////
void wifi_app_start(void);
BaseType_t wifi_app_send_message(wifi_app_message_e msgID);

#endif /* __WIFI_MODULE_H */

////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
///////////////////////////////////////////////////////////////////////////////