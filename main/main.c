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
#include "../protocols/mqtt/lib/mqtt.h"
#include "../drivers/devices/DHT22/lib/DHT22.h"
#include "../drivers/devices/irq_button/lib/irq_button.h"


// proto
void wifi_connected_event(void);


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
 * @return voID
 */
void wifi_connected_event(void)
{
	printf("WiFi Application Connected!!");
	sntp_task_start();
	mqtt_app_start();
    irq_button_config();
}
