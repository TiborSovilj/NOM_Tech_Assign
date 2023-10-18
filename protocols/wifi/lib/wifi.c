////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT (c) 2023
// NOMNIO d.o.o.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////
/**
 *	@file		wifi.c
 *	@author   	Tibor Sovilj
 *	@date		12.10.2023
 *	@version	V0.0.1

 *	@brief 		Source file for the WiFi protocol.
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *	@addtogroup WIFI_STATIC
 *	@{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "lwip/netdb.h"

#include "wifi.h"
#include "../config/wifi_cfg.h"
#include "../../http/lib/http_server.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Static Typedefs
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Static Global Variables
////////////////////////////////////////////////////////////////////////////////
// Queue handler for the WiFi app
static QueueHandle_t g_wifi_app_queue_handle = NULL;

// Global carrier for events logging table
static const wifi_app_events_log_t* g_wifi_app_events_table;

// Global carrier for queue message table
static const wifi_app_queue_message_t* g_wifi_app_queue_msg;

// Extern esp-netif container for default station init
esp_netif_t *esp_netif_sta = NULL;

// Extern esp-netif container for default AP init
esp_netif_t *esp_netif_ap = NULL;


////////////////////////////////////////////////////////////////////////////////
// Static Function Prototypes
////////////////////////////////////////////////////////////////////////////////
static void wifi_app_task				(void *p_arg);
static void wifi_app_event_handler_init	(void);
static void wifi_app_default_wifi_init	(void);
static void wifi_app_soft_ap_config		(void);
static void wifi_app_event_handler		(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);


////////////////////////////////////////////////////////////////////////////////
// Static Function Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief	Main task for the WiFi application.
 * 
 * @param	p_arg	Parameter which can be passed to the task
 * @return	void
 */
////////////////////////////////////////////////////////////////////////////////
static void wifi_app_task(void *p_arg)
{
	wifi_app_queue_message_t msg;
   	g_wifi_app_queue_msg = wifi_app_get_queue_message();
	
	wifi_app_event_handler_init();		// Initialize the event handler
    
	wifi_app_default_wifi_init();		// Initialize the TCP/IP stack and WiFi config
    
	wifi_app_soft_ap_config();			// SoftAP config
	
	ESP_ERROR_CHECK(esp_wifi_start());	// Start WiFi

	// Send first event message
	wifi_app_send_message(eWIFI_APP_MSG_START_HTTP_SERVER);

    printf("FINISHED: esp_wifi_start()\n\n");

    while(1)
	{
		if (xQueueReceive(g_wifi_app_queue_handle, &msg, portMAX_DELAY))
		{
			// TODO:  test when HTTP is implemented, if successfull -> delete switch statement
			// printf ("%s", g_wifi_app_queue_msg[msg.msgID].msgContent);
			
			switch (msg.msgID)
			{
				case eWIFI_APP_MSG_START_HTTP_SERVER:
					printf("WIFI_APP_MSG_START_HTTP_SERVER\n\n");
					http_server_start();
					break;

				case eWIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER:
					printf("WIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER\n\n");
					break;

				case eWIFI_APP_MSG_STA_CONNECTED_GOT_IP:
					printf("WIFI_APP_MSG_STA_CONNECTED_GOT_IP\n\n");
					break;

				default:
					printf("DEFAULT QUEUE MSG");
					break;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief	Initializes the WiFi application event handler for WiFi and IP events.
 * 
 * @return	void
 */
////////////////////////////////////////////////////////////////////////////////
static void wifi_app_event_handler_init(void)
{
	// Event loop for the WiFi driver
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	// Event handler for the connection events (WIFI and IP);
	esp_event_handler_instance_t instance_wifi_event;
	esp_event_handler_instance_t instance_ip_event;

	// Handler instance creation with abstraction layer functions.
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_wifi_event));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_app_event_handler, NULL, &instance_ip_event));

	printf("FINISHED: wifi event handler init\n\n");
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief		Initializes the TCP stack and default WiFi configuration.
 * 				
 * @param[in]	arg			Event data.
 * @param[in]	event_base	Base ID of the event for which a handler will be registerd
 * @param[in]	event_id	ID of the event for which a handler will be registerd.
 * @param[in]	event_data	Event data.
 * @return		void
 * 
 * @see			esp_event_handler_instance_register	
 */
////////////////////////////////////////////////////////////////////////////////
static void wifi_app_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	if (event_base == WIFI_EVENT)
	{
		printf ("%s", g_wifi_app_events_table[event_id].eventMsg);
	}
	else if (event_base == IP_EVENT)
	{
		// TODO: test if same approach works with STA mode only, then delete if statements, leave just print/log
		// printf ("%s", g_wifi_app_events_table[event_id].eventMsg);

		switch (event_id)
		{
			case IP_EVENT_STA_GOT_IP:
				printf("IP_EVENT_STA_GOT_IP\n\n");
				break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief		Initializes the TCP stack and default WiFi configuration.
 * 
 * @return		void
 */
////////////////////////////////////////////////////////////////////////////////
static void wifi_app_default_wifi_init(void)
{
	// Initialize the TCP stack
	ESP_ERROR_CHECK(esp_netif_init());

	// Default WiFi config
	// @note FUNCTIONS MUST EXECUTE IN THIS PARTICULAR ORDER.
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

	esp_netif_sta = esp_netif_create_default_wifi_sta();
	esp_netif_ap  = esp_netif_create_default_wifi_ap();

	printf("FINISHED: default wifi init\n\n");
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief		Configures the WiFi access point settings and assigns the static 
 * 				IP to the SoftAP.
 * 
 * @return		void
 */
////////////////////////////////////////////////////////////////////////////////
static void wifi_app_soft_ap_config(void)
{
	// SoftAP - WiFi access point configuration
	wifi_config_t ap_config =
	{
		.ap = {
				.ssid 			 = WIFI_AP_SSID,
				.ssid_len 		 = strlen(WIFI_AP_SSID),
				.password 		 = WIFI_AP_PASSWORD,
				.channel 		 = WIFI_AP_CHANNEL,
				.ssid_hidden 	 = WIFI_AP_SSID_HIDDEN,
				.authmode 		 = WIFI_AUTH_WPA2_PSK,
				.max_connection  = WIFI_AP_MAX_CONNECTIONS,
				.beacon_interval = WIFI_AP_BEACON_INTERVAL,
		},
	};

	// Configure DHCP for the AP
	esp_netif_ip_info_t ap_ip_info;
	memset(&ap_ip_info, 0x00, sizeof(ap_ip_info));

	esp_netif_dhcps_stop(esp_netif_ap);											// Stop DHCP instance for AP

	inet_pton(AF_INET, WIFI_AP_IP, &ap_ip_info.ip);								// Assign access point's static IP, gateway, and netmask
	inet_pton(AF_INET, WIFI_AP_GATEWAY, &ap_ip_info.gw);
	inet_pton(AF_INET, WIFI_AP_NETMASK, &ap_ip_info.netmask);

	ESP_ERROR_CHECK(esp_netif_set_ip_info(esp_netif_ap, &ap_ip_info));			// Static configuration of the network interface

	ESP_ERROR_CHECK(esp_netif_dhcps_start(esp_netif_ap));						// Start the AP DHCP server (for connecting stations e.g. your mobile device)

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));						// Setting the mode as Access Point / Station Mode
	
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));			// Apply configuration from wifi_config_t struct

	ESP_ERROR_CHECK(esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_AP_BANDWIDTH));		//  Setting BW and power saving mode
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_STA_POWER_SAVE));			

	printf("FINISHED: softAP init\n\n");			
}


////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *	@addtogroup WIFI_API
 *	@{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API Function Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief		Starts a RTOS task for the WiFi app
 * 
 * @return		void
 */
////////////////////////////////////////////////////////////////////////////////
void wifi_app_start(void)
{
    printf ("__WIFI_APP_START__\n\n");

	g_wifi_app_events_table = wifi_app_get_events_table();

    // Create message queue
    g_wifi_app_queue_handle = xQueueCreate(eWIFI_APP_MSG_NUM_OF, sizeof(wifi_app_queue_message_t));

    // Start the task
    xTaskCreate(&wifi_app_task, "wifi_app_task", WIFI_APP_TASK_STACK_SIZE, NULL, WIFI_APP_TASK_PRIORITY, NULL);
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief		Sends a message to the queue. Returns pdTRUE if an item was 
 * 				successfully sent to the queue, otherwise pdFALSE.	
 * 
 * @param[in]	msgID	Message ID from the wifi_app_message_e enum.
 * @return		pdTRUE/pdFALSE.	
 */
////////////////////////////////////////////////////////////////////////////////
BaseType_t  wifi_app_send_message(wifi_app_message_e msgID)
{
    wifi_app_queue_message_t msg;
	msg.msgID = msgID;

	return xQueueSend(g_wifi_app_queue_handle, &msg, portMAX_DELAY);
}

////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////