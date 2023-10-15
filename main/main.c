#include <stdio.h>
#include "nvs_flash.h"

#include "../protocols/wifi/lib/wifi.h"


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
}
