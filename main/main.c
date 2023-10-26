#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "mqtt_client.h"
#include "esp_tls.h"

#include "../protocols/wifi/lib/wifi.h"
#include "../protocols/sntp/lib/sntp.h"
// #include "../protocols/mqtt/lib/mqtt.h"
#include "../drivers/devices/DHT22/lib/DHT22.h"

static const char *TAG = "MQTTS_EXAMPLE";


const uint8_t mqtt_eclipseprojects_io_pem_start[]   asm("_binary_mqtt_eclipseprojects_io_pem_start");
const uint8_t mqtt_eclipseprojects_io_pem_end[]   asm("_binary_mqtt_eclipseprojects_io_pem_end");


// proto
void wifi_connected_event(void);
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event);
void mqtt_app_start(void);
static void broker_req(esp_mqtt_client_handle_t client);

void app_main(void)
{
    printf("__helloWorld__\n\n");
        // Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Start Wifi
	wifi_app_start();

	dht22_task_start();

	wifi_app_set_callback(&wifi_connected_event);
}

/**
 * @brief 	Function that starts SNTP task and is used as callback function
 * 			to an eWIFI_APP_MSG_STA_CONNECTED_GOT_IP event
 * 
 * @return vodi 
 */
void wifi_connected_event(void)
{
	printf("WiFi Application Connected!!");
	sntp_task_start();
	mqtt_app_start();
}

static void broker_req(esp_mqtt_client_handle_t client)
{
    int msg_id = esp_mqtt_client_publish(client, "nom_ta_test", "hello from BROKER", 0, 0, 0);
    ESP_LOGI(TAG, "binary sent with msg_id=%d", msg_id);
}

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
            broker_req(client);
        }
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
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

const char* cert = "-----BEGIN RSA PRIVATE KEY-----\n\
MIIEowIBAAKCAQEAsgyMKg+og7h7AhVoMvQDXDyA1LrWHO2BTrJ6HdARLqsh9QP1\n\
OatY05b40092Uc1BMkOTlj+6bug6qIRYpZi6VrGmtCwQ+RASWiYIhbfTEidPvu8F\n\
a4/gQmQZbcmV4qwvDB/8AUUN68Me5eRQHQNFbHMmqJhrVJ140bSrrH9B9GlXCR39\n\
fbHk2rlYe4vzr98jSuvWp1GItSLtMGWidvTguEJFsSRcUoFo0sJ+68IOyOXcEDGk\n\
r4vJFGogNB8QyaVLIXa7JWKgIIgmVjJvYDaePQwUXePvGjaZzRfR89fPrr0S/CVh\n\
LDGrb0WHYiC+RivlCJZWEoODbKfX+8moh7r60wIDAQABAoIBAFu1EsznW8jc0J72\n\
H8F+5ewwTbtEsNwdiSjbzQJmFTOQeeEVtM2LcCkr7eYJW8wuiJI3NGGDWaeeffgm\n\
kvJYhEH7Jv5OZD+lA47jYChf0pvbG7wgqQ4KAVyw6lgeKNGkFmeMYeTViKIS0mte\n\
+dS3xp5hgVv9hesDYSPCh1pGAda9P1alRRlWC2OJk3769vs4nwOu03IGQOSWBl4Y\n\
/QSrU5lLgU2v1CtieZQRENkpYyBZiOBO88thskjok99dqKAhDAgd9N3/sa1Nw7Gw\n\
tMxqeQAoSG0PCk4jIv5gjE8LHROrv4urpdfqIBOXGttbK6dg6IPqbwSOhaYgtSR6\n\
5Qj6VqECgYEA2kCROapJS3iIsQrMHZkOyNj+NCZfDW809g24biPkVOtpZJv/ngEi\n\
c9UhSmGNcOMtI+NGxWvAl9xmTw7b8xzOk+ITnCOBoKjDFQ0DSehaKGMbNW5RbGN2\n\
gpRZVOJ270FyIzAXyDCCGKJQW5fDlTFCXqEr1oUVC3kMsqlyCVh9IWUCgYEA0Nfs\n\
3ogP5KIwuNlELK4RpfpNUa8TCiGQ0D6ezUNB3ozgcYAJZa8WS+meYIogvrT/e4Vo\n\
L8mXdTX9trGNMeUwao0PmbbTMqLR5E19HqJd1HKEmXzwZ0Be8scFNdAxMEdeBgmM\n\
ih9E+Z/a8ViPhlAu5xtXyhgVLMaSDPvTWSQkw9cCgYAmqsfP+p5vs5QsIaiWGdbn\n\
uKIY5S9z9t7gNQAW6175uJd8jrLT8ImFEh6KygvAE0+dCxgvw+5kOVUa7pwDT3g9\n\
9RDaWeQObbfaU+rgPj0y6JQafEgKtvh5HAVTp6fArcyl9VBRVF7INIGeKJ4rIYYL\n\
s+xLXlqjJLgeMy0UAMxyjQKBgGYQsQ3Ml1/YuFEOtdfUNoHUg0chdf+kid6MTBXr\n\
Ad0fIm218mHEoPP2t9VcjEZHtPiMKW/5aND60wUfXu78oJ3iVLZ9+Fet5UBbcoOv\n\
PIYgdZeBzQfZGM4z3+L93ZxHtLbkoc+7Gn2Y12rOKk6tD08ZON1myap5XVWFGTRe\n\
iq0/AoGBAKQ5LGbR0ioZUv859OyjHyyP3qRHvvxzShe5jX65aDI1mKWjL7DpmwEb\n\
tj+oALX1pGStlSsOp3dmdYY7hcqPsMi+y9gOS2Go6dwUuSOCt2ldjLa5+7/ZF4DD\n\
68v7hYBw4OpVCvby/apMCu9yAMOj9mbHpCXMpMgUMQN0lr4e5cOn\n\
-----END RSA PRIVATE KEY-----\n\
-----BEGIN CERTIFICATE-----\n\
MIIDVDCCAjygAwIBAgIBADANBgkqhkiG9w0BAQsFADCBkDELMAkGA1UEBhMCR0Ix\n\
FzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTESMBAGA1UE\n\
CgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVpdHRvLm9y\n\
ZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzAeFw0yMzEwMjYxNzE2\n\
NTdaFw0yNDAxMjQxNzE2NTdaMC4xCzAJBgNVBAYTAlNJMQ8wDQYDVQQKDAZOb21u\n\
aW8xDjAMBgNVBAMMBVRJQk9SMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC\n\
AQEAsgyMKg+og7h7AhVoMvQDXDyA1LrWHO2BTrJ6HdARLqsh9QP1OatY05b40092\n\
Uc1BMkOTlj+6bug6qIRYpZi6VrGmtCwQ+RASWiYIhbfTEidPvu8Fa4/gQmQZbcmV\n\
4qwvDB/8AUUN68Me5eRQHQNFbHMmqJhrVJ140bSrrH9B9GlXCR39fbHk2rlYe4vz\n\
r98jSuvWp1GItSLtMGWidvTguEJFsSRcUoFo0sJ+68IOyOXcEDGkr4vJFGogNB8Q\n\
yaVLIXa7JWKgIIgmVjJvYDaePQwUXePvGjaZzRfR89fPrr0S/CVhLDGrb0WHYiC+\n\
RivlCJZWEoODbKfX+8moh7r60wIDAQABoxowGDAJBgNVHRMEAjAAMAsGA1UdDwQE\n\
AwIF4DANBgkqhkiG9w0BAQsFAAOCAQEAkrsJrzSl9zK3qU3bYC3GCQMK0TSezU5A\n\
kttxOJ3EXCjjAtWbHGXGbSI+gqbLWFpSpm7ssJEgtnYMH1+73xOmPJ5kClk6GB7a\n\
Oki7dppM+AMrPBRpDIYWqYxusI/jZDZk4RTk9EVoUAv4IOP1AMk975qEmOqKg2E4\n\
IUWZJ7GcmTZdrudGlap/FbKaFvO7vqwWMMmAP03q6i2kWR90EBtRVmhEoCCTaMkc\n\
Qzw4i2lTApTcxtk22YArbbGsAX3xSjB3pF599VQoVtCvMKHrhx7T9VclIGTVlPSW\n\
jLdIr+tUehOybEfcyQ2y8pDkPUXtOEyvtDYaN+LalVV+Mn+KDReGZQ==\n\
-----END CERTIFICATE-----\n\
-----BEGIN CERTIFICATE-----\n\
MIIEAzCCAuugAwIBAgIUBY1hlCGvdj4NhBXkZ/uLUZNILAwwDQYJKoZIhvcNAQEL\n\
BQAwgZAxCzAJBgNVBAYTAkdCMRcwFQYDVQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwG\n\
A1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1vc3F1aXR0bzELMAkGA1UECwwCQ0ExFjAU\n\
BgNVBAMMDW1vc3F1aXR0by5vcmcxHzAdBgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hv\n\
by5vcmcwHhcNMjAwNjA5MTEwNjM5WhcNMzAwNjA3MTEwNjM5WjCBkDELMAkGA1UE\n\
BhMCR0IxFzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTES\n\
MBAGA1UECgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVp\n\
dHRvLm9yZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzCCASIwDQYJ\n\
KoZIhvcNAQEBBQADggEPADCCAQoCggEBAME0HKmIzfTOwkKLT3THHe+ObdizamPg\n\
UZmD64Tf3zJdNeYGYn4CEXbyP6fy3tWc8S2boW6dzrH8SdFf9uo320GJA9B7U1FW\n\
Te3xda/Lm3JFfaHjkWw7jBwcauQZjpGINHapHRlpiCZsquAthOgxW9SgDgYlGzEA\n\
s06pkEFiMw+qDfLo/sxFKB6vQlFekMeCymjLCbNwPJyqyhFmPWwio/PDMruBTzPH\n\
3cioBnrJWKXc3OjXdLGFJOfj7pP0j/dr2LH72eSvv3PQQFl90CZPFhrCUcRHSSxo\n\
E6yjGOdnz7f6PveLIB574kQORwt8ePn0yidrTC1ictikED3nHYhMUOUCAwEAAaNT\n\
MFEwHQYDVR0OBBYEFPVV6xBUFPiGKDyo5V3+Hbh4N9YSMB8GA1UdIwQYMBaAFPVV\n\
6xBUFPiGKDyo5V3+Hbh4N9YSMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL\n\
BQADggEBAGa9kS21N70ThM6/Hj9D7mbVxKLBjVWe2TPsGfbl3rEDfZ+OKRZ2j6AC\n\
6r7jb4TZO3dzF2p6dgbrlU71Y/4K0TdzIjRj3cQ3KSm41JvUQ0hZ/c04iGDg/xWf\n\
+pp58nfPAYwuerruPNWmlStWAXf0UTqRtg4hQDWBuUFDJTuWuuBvEXudz74eh/wK\n\
sMwfu1HFvjy5Z0iMDU8PUDepjVolOCue9ashlS4EB5IECdSR2TItnAIiIwimx839\n\
LdUdRudafMu5T5Xma182OC0/u/xRlEm+tvKGGmfFcN0piqVl8OrSPBgIlb+1IKJE\n\
m/XriWr/Cq4h/JfB7NTsezVslgkBaoU=\n\
-----END CERTIFICATE-----\n";

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
		.broker = {
        	.address.uri = "mqtts://test.mosquitto.org:8883",
        	.verification.certificate = cert,
		}
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    printf("___MQTT INIT ERROR____\n");
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}