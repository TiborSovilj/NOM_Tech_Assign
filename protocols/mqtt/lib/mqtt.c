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

#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_tls.h"

#include "../../project_config.h"
#include "../../drivers/devices/DHT22/lib/DHT22.h"

#include "mqtt.h"
#include "../config/mqtt_cfg.h"
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
static const char* gp_mqtt_tag = "MQTTS";

//  Global container for certificate string.
const char* gp_mqtt_cert = NULL;

//  Queue handle for signal messages
static QueueHandle_t g_mqtt_queue_handle = NULL;

// Global MQTT client
esp_mqtt_client_handle_t g_mqtt_client = NULL;


////////////////////////////////////////////////////////////////////////////////
// Static Function Prototypes
////////////////////////////////////////////////////////////////////////////////
static void mqtt_event_handler          (void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void mqtt_publisher_task         (void *p_args);
static void mqtt_button_publish_task    (void *p_args);
static void mqtt_payload_builder        (mqtt_opmode_e opmode);


////////////////////////////////////////////////////////////////////////////////
// Static Function Definitions
////////////////////////////////////////////////////////////////////////////////

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
    ESP_LOGD(gp_mqtt_tag, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id) 
	{
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(gp_mqtt_tag, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, MQTT_USER_TOPIC, 0);
	    	esp_mqtt_client_publish(client, MQTT_USER_TOPIC, "MQTT_HELLO_MESSAGE", 0, 1, 0);

            // Create publisher and button interrupt tasks
            // Purposely not defined wihin mqtt_cfg.c file
            xTaskCreate(&mqtt_publisher_task, "mqtt_publisher_task", MQTT_PUBLISHER_TASK_STACK_SIZE, NULL, MQTT_PUBLISHER_TASK_PRIORITY, NULL);
            xTaskCreate(&mqtt_button_publish_task, "mqtt_button_publish_task", MQTT_BUTTON_PUBLISH_TASK_STACK_SIZE, NULL, MQTT_BUTTON_PUBLISH_TASK_PRIORITY, NULL);

            ESP_LOGI(gp_mqtt_tag, "sent subscribe successful, msg_id=%d", msg_id);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(gp_mqtt_tag, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(gp_mqtt_tag, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(gp_mqtt_tag, "sent publish successful, msg_id=%d", msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(gp_mqtt_tag, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(gp_mqtt_tag, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        
        case MQTT_EVENT_DATA:
            if (strncmp(event->data, "BROKER_REQ", event->data_len) == 0) 
	    	{
                ESP_LOGI(gp_mqtt_tag, "Sending by BROKER_REQ");
                mqtt_payload_builder(eMQTT_OPERATION_DHT22_BROKER_REQUEST);
            }
            else
            {
                ESP_LOGI(gp_mqtt_tag, "MQTT_EVENT_DATA");
                printf("TOPIC = %.*s\r\n", event->topic_len, event->topic);
                printf("DATA = %.*s\r\n", event->data_len, event->data);
            }
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(gp_mqtt_tag, "MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                ESP_LOGI(gp_mqtt_tag, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
                ESP_LOGI(gp_mqtt_tag, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
                ESP_LOGI(gp_mqtt_tag, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                         strerror(event->error_handle->esp_transport_sock_errno));
            } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                ESP_LOGI(gp_mqtt_tag, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
            } else {
                ESP_LOGW(gp_mqtt_tag, "Unknown error type: 0x%x", event->error_handle->error_type);
            }
            break;

        default:
            ESP_LOGI(gp_mqtt_tag, "Other event id:%d", event->event_id);
            break;
    }
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief       Publisher task which is called upon a button press. After the
 *              button is pressed a queue message is sent from button library
 *              which activated new publish.    
 * 
 * @param[in]   p_arg  Parameter passed to the task
 * @return      void
 */
////////////////////////////////////////////////////////////////////////////////
static void mqtt_button_publish_task(void *p_args)
{
    mqtt_signal_message_t msg;

    while(1)
    {
        if (xQueueReceive(g_mqtt_queue_handle, &msg, portMAX_DELAY))
        {
            switch(msg.msgID)
            {
                case eMQTT_SIGNAL_MSG_BUTTON_IT:
                    mqtt_payload_builder(eMQTT_OPERATION_DHT22_BUTTON_INTERRUPT);

                    break;
                
                default:
                    break;
            }
        }       
    }
    vTaskDelete(NULL);
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief       Publisher task which is periodically called immediately after
 *              the device is connected to a WiFi      
 * 
 * @param[in]   p_arg  Parameter passed to the task
 * @return      void
 */
////////////////////////////////////////////////////////////////////////////////
static void mqtt_publisher_task(void *p_arg)
{
    while(1)
    {
        mqtt_payload_builder(eMQTT_OPERATION_DHT22_INTERVAL_5S);
        vTaskDelay(5000 / portTICK_PERIOD_MS);      
    }
    vTaskDelete(NULL);
}


////////////////////////////////////////////////////////////////////////////////
/**
 * @brief   Builds payload as defined by user in mwtt_cfg.c table
 * 
 * @return  void
 */
////////////////////////////////////////////////////////////////////////////////
static void mqtt_payload_builder(mqtt_opmode_e opmode)
{
    char payload[100] = "";
    mqtt_get_payload_string(payload, opmode);
    esp_mqtt_client_publish(g_mqtt_client, MQTT_USER_TOPIC, payload, 0, 0, 0);  
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
 * @brief   Start funciton for the MQTT protocol.
 * 
 * @return  void
 */
////////////////////////////////////////////////////////////////////////////////
void mqtt_app_start(void)
{
    // Fetshing the certificate from mqtt_cert.c
    gp_mqtt_cert = mqtt_get_certificate();

    g_mqtt_queue_handle = xQueueCreate(eMQTT_SIGNAL_NUM_OF, sizeof(mqtt_signal_message_t)); 

    esp_mqtt_client_config_t mqtt_cfg = {
		.broker = {
        	.address.uri =              MQTT_BROKER_ADRESS_URI,
        	.verification.certificate = gp_mqtt_cert,
		}
    };

    // Init of MQTT server
    g_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event  (g_mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, g_mqtt_client);
    esp_mqtt_client_start           (g_mqtt_client);
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief       Receives message from outside components to take appropriate 
 *              actions within queue state machine. Function will execute only
 *              if there are any external queue messsages. 
 * 
 * @param[in]   msgID   Enum ID of an action.
 * @return      BaseType intiger
 */
////////////////////////////////////////////////////////////////////////////////
BaseType_t  mqtt_send_signal_message(mqtt_signal_message_e msgID)
{
    if ( eMQTT_SIGNAL_NUM_OF != 0 )
    {
        mqtt_signal_message_t msg;
	    msg.msgID = msgID;
    
        // Sends structure with enum 
	    return xQueueSend(g_mqtt_queue_handle, &msg, portMAX_DELAY);
    }
}


////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////





