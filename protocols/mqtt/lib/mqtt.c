////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT (c) 2023
// NOMNIO d.o.o.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////
/**
 *	@file		mqtt.c
 *	@author   	Tibor Sovilj
 *	@date		26.10.2023
 *	@version	V0.0.1

 *	@brief 		Source file for the MQTT protocol.
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *	@addtogroup MQTT_STATIC
 *	@{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_tls.h"

#include "mqtt.h"
#include "../config/mqtt_cert.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Static Typedefs
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Static Global Variables
////////////////////////////////////////////////////////////////////////////////

//  Tag designator for log messages
static const char *TAG = "MQTTS";

//  Binaries assigned to their .rodata representative variables
const uint8_t mqtt_eclipseprojects_io_pem_start[]   asm("_binary_mqtt_eclipseprojects_io_pem_start");
const uint8_t mqtt_eclipseprojects_io_pem_end[]     asm("_binary_mqtt_eclipseprojects_io_pem_end");

//  Global container for certificate string.
const char* gp_mqtt_cert = NULL;


////////////////////////////////////////////////////////////////////////////////
// Static Function Prototypes
////////////////////////////////////////////////////////////////////////////////
static void mqtt_broker_req     (esp_mqtt_client_handle_t client);
static void mqtt_event_handler  (void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

////////////////////////////////////////////////////////////////////////////////
// Static Function Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief       Any predefined message from the broker will be handled with own
 *              series of actions. 
 * 
 * @param[in]   client  Pointer to an client handle structure
 * @return      void
 */
////////////////////////////////////////////////////////////////////////////////
static void mqtt_broker_req(esp_mqtt_client_handle_t client)
{
    int msg_id = esp_mqtt_client_publish(client, "nom_ta_test", "hello from BROKER", 0, 0, 0);
    ESP_LOGI(TAG, "binary sent with msg_id=%d", msg_id);
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief       General MQTT event handler.
 * 
 * @param[in]   handler_args    Arguments of the handler
 * @param[in]   base            Pointer to a subsystem that exposes events
 * @param[in]   event_id        Event unique ID
 * @param[in]   event_data      Pointer to an event handler structure
 * @return      void
 */
////////////////////////////////////////////////////////////////////////////////
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id) 
	{
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_subscribe(client, "nom_ta_test", 0);
		esp_mqtt_client_publish(client, "nom_ta_test", "Hi to all from ESP32 !!!---!!!", 0, 1, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
        
    case MQTT_EVENT_DATA:
        if (strncmp(event->data, "BROKER_REQ", event->data_len) == 0) 
		{
            ESP_LOGI(TAG, "Sending by BROKER_REQ");
            mqtt_broker_req(client);
        }
        else
        {
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        } else {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}
////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *	@addtogroup MQTT_API
 *	@{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API Function Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief   Start funciton for th MQTT protocol.
 * 
 * @return  void
 */
////////////////////////////////////////////////////////////////////////////////

void mqtt_app_start(void)
{
    // Fetshing the certificate from mqtt_cert.c
    gp_mqtt_cert = mqtt_get_certificate();

    esp_mqtt_client_config_t mqtt_cfg = {
		.broker = {
        	.address.uri = "mqtts://test.mosquitto.org:8883",
        	.verification.certificate = gp_mqtt_cert,
		}
    };

    // Init of MQTT server
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event  (client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start           (client);
}
////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////





