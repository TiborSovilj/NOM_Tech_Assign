////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT (c) 2023
// NOMNIO d.o.o.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////
/**
 *	@file		http_server.c
 *	@author   	Tibor Sovilj
 *	@date		12.10.2023
 *	@version	V1.0.0

 *	@brief 		Source file for the HTTP server.
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *	@addtogroup HTTP_SERVER_STATIC
 *	@{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_timer.h"
#include "sys/param.h"

#include "../../../drivers/devices/DHT22/lib/DHT22.h"

#include "http_server.h"
#include "../../wifi/lib/wifi.h"
#include "../../sntp/lib/sntp.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Static Typedefs
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Static Global Variables
////////////////////////////////////////////////////////////////////////////////

// ESP loggging tags
static const char* gp_http_server_tag 			= "HTTP_SERVER";
static const char* gp_http_server_ota_tag 		= "HTTP_SERVER_OTA";
static const char* gp_http_server_webpage_tag 	= "HTTP_SERVER_WEBPAGE";

// Wifi connect status
static int g_wifi_connect_status = NONE;

// Firmware update status
static int g_fw_update_status = OTA_UPDATE_PENDING;

// HTTP server task handle
static httpd_handle_t http_server_handle = NULL;

// HTTP server monitor task handle
static TaskHandle_t g_task_http_server_monitor = NULL;

// Queue handle used to manipulate the main queue of events
static QueueHandle_t http_server_monitor_queue_handle;

// Local time set flag
static bool g_is_local_time_set = false;

// ESP32 timer configuration passed to esp_timer_create. 
const esp_timer_create_args_t fw_update_reset_args = {
		.callback = &http_server_fw_update_reset_callback,
		.arg = NULL,
		.dispatch_method = ESP_TIMER_TASK,
		.name = "fw_update_reset"
};

// Timer reset handler
esp_timer_handle_t fw_update_reset;

// Assign binaries to their .rodata representative variables
extern const uint8_t jquery_3_3_1_min_js_start[]	asm("_binary_jquery_3_3_1_min_js_start");
extern const uint8_t jquery_3_3_1_min_js_end[]		asm("_binary_jquery_3_3_1_min_js_end");
extern const uint8_t index_html_start[]				asm("_binary_index_html_start");
extern const uint8_t index_html_end[]				asm("_binary_index_html_end");
extern const uint8_t app_css_start[]				asm("_binary_app_css_start");
extern const uint8_t app_css_end[]					asm("_binary_app_css_end");
extern const uint8_t app_js_start[]					asm("_binary_app_js_start");
extern const uint8_t app_js_end[]					asm("_binary_app_js_end");
extern const uint8_t favicon_ico_start[]			asm("_binary_favicon_ico_start");
extern const uint8_t favicon_ico_end[]				asm("_binary_favicon_ico_end");


////////////////////////////////////////////////////////////////////////////////
// Static Function Prototypes
////////////////////////////////////////////////////////////////////////////////
static httpd_handle_t http_server_configure(void);

////////////////////////////////////////////////////////////////////////////////
// Static Function Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 	Checks the g_fw_update_status and creates the fw_update_reset timer 
 * 			if g_fw_update_status is true.
 * 
 * @return 	void 
 */
////////////////////////////////////////////////////////////////////////////////
static void http_server_fw_update_reset_timer(void)
{
	if (g_fw_update_status == OTA_UPDATE_SUCCESSFUL)
	{
		ESP_LOGI(gp_http_server_ota_tag, "FW updated SUCCESSFULLY. Starting FW update reset timer");

		// Give the web page a chance to receive an acknowledge back and initialize the timer
		ESP_ERROR_CHECK(esp_timer_create(&fw_update_reset_args, &fw_update_reset));
		ESP_ERROR_CHECK(esp_timer_start_once(fw_update_reset, 8000000));
	}
	else
	{
		ESP_LOGI(gp_http_server_ota_tag, "FW update FAILED.");
	}
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief		HTTP server monitor task used to track events of the HTTP server
 * 
 * @param[in] 	pvParameters 	Parameter which can be passed to the task.
 * @return		void
 */
////////////////////////////////////////////////////////////////////////////////
static void http_server_monitor(void *parameter)
{
	http_server_queue_message_t msg;

	while(1)
	{
		if (xQueueReceive(http_server_monitor_queue_handle, &msg, portMAX_DELAY))
		{
			switch (msg.msgID)
			{
				case HTTP_MSG_WIFI_CONNECT_INIT:
					ESP_LOGI(gp_http_server_tag, "HTTP_MSG_WIFI_CONNECT_INIT");
					g_wifi_connect_status = HTTP_WIFI_STATUS_CONNECTING;
					break;

				case HTTP_MSG_WIFI_CONNECT_SUCCESS:
					ESP_LOGI(gp_http_server_tag, "HTTP_MSG_WIFI_CONNECT_SUCCESS");
					g_wifi_connect_status = HTTP_WIFI_STATUS_CONNECT_SUCCESS;
					break;

				case HTTP_MSG_WIFI_CONNECT_FAIL:
					ESP_LOGI(gp_http_server_tag, "HTTP_MSG_WIFI_CONNECT_FAIL");
					g_wifi_connect_status = HTTP_WIFI_STATUS_CONNECT_FAILED;
					break;

				case HTTP_MSG_OTA_UPDATE_SUCCESSFUL:
					ESP_LOGI(gp_http_server_tag, "HTTP_MSG_OTA_UPDATE_SUCCESSFUL");

					g_fw_update_status = OTA_UPDATE_SUCCESSFUL;
					http_server_fw_update_reset_timer();

					break;

				case HTTP_MSG_OTA_UPDATE_FAILED:
					ESP_LOGI(gp_http_server_ota_tag, "HTTP_MSG_OTA_UPDATE_FAILED");

					g_fw_update_status = OTA_UPDATE_FAILED;

					break;
				
				case HTTP_MSG_OTA_UPATE_INITIALIZED:
					ESP_LOGI(gp_http_server_ota_tag, "HTTP_MSG_OTA_UPDATE_INITIALIZED");

					g_fw_update_status = OTA_UPDATE_FAILED;

					break;

				case HTTP_MSG_TIME_SERVICE_INITIALIZED:
					ESP_LOGI(gp_http_server_tag, "HTTP_MSG_TIME_SERVICE_INITIALIZED");
					g_is_local_time_set = true;

					break;

				default:
					break;
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// Webpage handlers
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 		Jquery get handler is requested when accessing the web page.
 * 
 * @param[in]	req		HTTP request for which the uri needs to be handled.
 * @return 		Error code
 */
////////////////////////////////////////////////////////////////////////////////
static esp_err_t http_server_jquery_handler(httpd_req_t *req)
{
	ESP_LOGI(gp_http_server_webpage_tag, "Jquery requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)jquery_3_3_1_min_js_start, jquery_3_3_1_min_js_end - jquery_3_3_1_min_js_start);

	return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 		HTML get handler 
 * 
 * @param[in]	req		HTTP request for which the uri needs to be handled.
 * @return 		Error code
 */
////////////////////////////////////////////////////////////////////////////////
static esp_err_t http_server_index_html_handler(httpd_req_t *req)
{
	ESP_LOGI(gp_http_server_webpage_tag, "index.html requested");

	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);

	return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 		CSS get handler 
 * 
 * @param[in]	req		HTTP request for which the uri needs to be handled.
 * @return 		Error code
 */
////////////////////////////////////////////////////////////////////////////////
static esp_err_t http_server_app_css_handler(httpd_req_t *req)
{
	ESP_LOGI(gp_http_server_webpage_tag, "app.css requested");

	httpd_resp_set_type(req, "text/css");
	httpd_resp_send(req, (const char *)app_css_start, app_css_end - app_css_start);

	return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 		JavaScript handler 
 * 
 * @param[in]	req		HTTP request for which the uri needs to be handled.
 * @return 		Error code
 */
////////////////////////////////////////////////////////////////////////////////
static esp_err_t http_server_app_js_handler(httpd_req_t *req)
{
	ESP_LOGI(gp_http_server_webpage_tag, "app.js requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)app_js_start, app_js_end - app_js_start);

	return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 		Title icon handler 
 * 
 * @param[in]	req		HTTP request for which the uri needs to be handled.
 * @return 		Error code
 */
////////////////////////////////////////////////////////////////////////////////
static esp_err_t http_server_favicon_ico_handler(httpd_req_t *req)
{
	ESP_LOGI(gp_http_server_webpage_tag, "favicon.ico requested");

	httpd_resp_set_type(req, "image/x-icon");
	httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_end - favicon_ico_start);

	return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 		DHT22 reading handler and JSON builder, later used in app.js
 * 				requests.
 * 
 * @param[in]	req		HTTP request for which the uri needs to be handled.
 * @return 		Error code
 */
////////////////////////////////////////////////////////////////////////////////
static esp_err_t http_server_get_dht_sensor_readings_json_handler(httpd_req_t *req)
{
	ESP_LOGI(gp_http_server_webpage_tag, "dhtSensor.json requested");

	char dhtSensorJSON[100];

	sprintf(dhtSensorJSON, "{\"temp\":\"%.1f\",\"humidity\":\"%.1f\"}", dht22_get_temperature(), dht22_get_humidity());

	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, dhtSensorJSON, strlen(dhtSensorJSON));

	return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief			wifiConnect.json handler is invoked after the connect button 
 * 					is pressed and handles receiving the SSID and password entered 
 * 					by the user.
 * 
 * @param[in]	req	HTTP request for which the uri needs to be handled.
 * @return 		Error code
 */
////////////////////////////////////////////////////////////////////////////////
static esp_err_t http_server_wifi_connect_json_handler(httpd_req_t *req)
{
	ESP_LOGI(gp_http_server_webpage_tag, "wifiConnect.json requested");

	size_t 	len_ssid = 0;
	size_t 	len_pass = 0;
	char* 	ssid_str = NULL;
	char* 	pass_str = NULL;

	// Get SSID header
	len_ssid = httpd_req_get_hdr_value_len(req, "my-connect-ssid") + 1;
	if (len_ssid > 1)
	{
		ssid_str = malloc(len_ssid);
		if (httpd_req_get_hdr_value_str(req, "my-connect-ssid", ssid_str, len_ssid) == ESP_OK)
		{
			ESP_LOGI(gp_http_server_webpage_tag, "Found header => my-connect-ssid: %s\n", ssid_str);
		}
	}

	// Get Password header
	len_pass = httpd_req_get_hdr_value_len(req, "my-connect-pwd") + 1;
	if (len_pass > 1)
	{
		pass_str = malloc(len_pass);
		if (httpd_req_get_hdr_value_str(req, "my-connect-pwd", pass_str, len_pass) == ESP_OK)
		{
			ESP_LOGI(gp_http_server_webpage_tag, "Found header => my-connect-pwd: %s\n", pass_str);
		}
	}

	// Update the Wifi networks configuration and let the wifi application know
	wifi_config_t* wifi_config = wifi_app_get_wifi_config();
	memset(wifi_config, 0x00, sizeof(wifi_config_t));
	memcpy(wifi_config->sta.ssid, ssid_str, len_ssid);
	memcpy(wifi_config->sta.password, pass_str, len_pass);
	wifi_app_send_message(eWIFI_APP_MSG_CONNECTING_FROM_HTTP_SERVER);

	free(ssid_str);
	free(pass_str);

	return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief		wifiConnectStatus handler updates the connection status for the web page.
 * 
 * @param[in]	req	HTTP request for which the uri needs to be handled.
 * @return 		Error code
 */
////////////////////////////////////////////////////////////////////////////////
static esp_err_t http_server_wifi_connect_status_json_handler(httpd_req_t *req)
{
	ESP_LOGI(gp_http_server_webpage_tag, "wifiConnectStatus requested");

	char statusJSON[100];

	sprintf(statusJSON, "{\"wifi_connect_status\":%d}", g_wifi_connect_status);

	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, statusJSON, strlen(statusJSON));

	return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 	local Time JSON handler responds by sending the local time.
 * 
 * @param	req 	HTTP request for which the uri needs to be handled.
 * @return 	eRROR CODE
 */
////////////////////////////////////////////////////////////////////////////////
static esp_err_t http_server_get_local_time_json_handler(httpd_req_t *req)
{
	ESP_LOGI(gp_http_server_webpage_tag, "localTime.json requested");

	char localTimeJSON[100] = {0};

	if (g_is_local_time_set)
	{
		sprintf(localTimeJSON, "{\"time\":\"%s\"}", sntp_get_time());
	}

	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, localTimeJSON, strlen(localTimeJSON));

	return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////
// OTA handlers
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief	Receives the .bin file fia the web page and handles the firmware update.
 * 			Returns ESP_OK if the update was successful or ESP_FAIL if one of three
 * 			errors occur:
 * 				HTTPD_SOCK_ERR_FAIL      -1		(Invalid arguments )
				HTTPD_SOCK_ERR_INVALID   -2		(Timeout/interrupted while calling socket recv() )
		 		HTTPD_SOCK_ERR_TIMEOUT   -3		(Unrecoverable error while calling socket recv() )
 * 			
 * 
 * @param 	req		HTTP request for which the uri needs to be handled.
 * @return 	Error code 
 */
////////////////////////////////////////////////////////////////////////////////
esp_err_t http_server_OTA_update_handler(httpd_req_t *req)
{
	esp_ota_handle_t ota_handle;					

	char ota_buff[1024];							// Data buffer for data received from the web page
	int content_length = req->content_len;			// lenght of the content
	int content_received = 0;						// Current content to be flashed, increments with body part lenght
	int recv_len;									// Receives data from each HTTP request function call
	bool is_req_body_started = false;				// Flag that checks if the actuall update material was found or not
	bool flash_successful = false;					// Flag to indicate successful update/flash status

	// ESP partition instance, finds the OTA APP partition that can be later passed to esp_ota_begin()
	const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

	// loop to receive .bin file for update 
	do
	{
		// Read the data for the request, check for errors
		if ((recv_len = httpd_req_recv(req, ota_buff, MIN(content_length, sizeof(ota_buff)))) < 0)
		{
			// Check if timeout occurred , HTTPD_SOCK_ERR_INVALID = -2
			// try with switch/case for all three errors

			if (recv_len == HTTPD_SOCK_ERR_TIMEOUT)
			{
				ESP_LOGI(gp_http_server_ota_tag, "Socket Timeout");
				continue;
			}
						
			ESP_LOGI(gp_http_server_ota_tag, "OTA other Error %d\n", recv_len);

			return ESP_FAIL;
		}

		ESP_LOGI(gp_http_server_ota_tag, "OTA RX: %d of %d\r", content_received, content_length);

		// Verify weather if this is the first occurence of any data = .bin file header,
		// else proceed with writing OTA data to a selected partition over esp_ota_write()
		if (!is_req_body_started)
		{
			is_req_body_started = true;

			// Get the location of the .bin file content (offset by web form data lenght)
			char *body_start_p = strstr(ota_buff, "\r\n\r\n") + 4;				

			// Calculating lenght of .bin body, without header or any metadata
			int body_part_len = recv_len - (body_start_p - ota_buff);

			ESP_LOGI(gp_http_server_ota_tag, "OTA file size: %d\n", content_length);

			// Begin the update, OTA_SIZE_UNKNOWN provided so the whole partition can be erased before the update
			esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
			
			if (err != ESP_OK)
			{
				ESP_LOGI(gp_http_server_ota_tag, "ERROR with OTA begin, cancelling OTA");
				return ESP_FAIL;
			}
			else
			{
				ESP_LOGI(gp_http_server_ota_tag, "Writing to partition subtype %d at offset 0x%lx\r\n", update_partition->subtype, update_partition->address);
			}

			// Write OTA data
			esp_ota_write(ota_handle, body_start_p, body_part_len);
			content_received += body_part_len;
		}
		else
		{
			// Write OTA data
			esp_ota_write(ota_handle, ota_buff, recv_len);
			content_received += recv_len;
		}
	} while (recv_len > 0 && content_received < content_length);

	// If the update was finished, proceed to initialize partitions
	if (esp_ota_end(ota_handle) == ESP_OK)
	{
		// Declaring update partition as a new boot partition
		if (esp_ota_set_boot_partition(update_partition) == ESP_OK)
		{
			const esp_partition_t *boot_partition = esp_ota_get_boot_partition();
			ESP_LOGI(gp_http_server_ota_tag,  "Next boot partition subtype %d at offset 0x%lx\n", boot_partition->subtype, boot_partition->address);
			flash_successful = true;
		}
		else
		{
			ESP_LOGI(gp_http_server_ota_tag,  "FLASHED ERROR.");
		}
	}
	else
	{
		ESP_LOGI(gp_http_server_ota_tag,  "esp_ota_end ERROR.");
	}

	// Send the message about the status
	if (flash_successful) 
	{ 
		http_server_monitor_send_message(HTTP_MSG_OTA_UPDATE_SUCCESSFUL); 
	}
	else 
	{ 
		http_server_monitor_send_message(HTTP_MSG_OTA_UPDATE_FAILED); 
	}

	return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 	OTA status handler responds with the firmware update status after 
 * 			the OTA update is started and responds with the compile time/date
 * 			when the page is first requested.
 * 
 * @param 	req 	HTTP request for which the uri needs to be handled
 * @return 	Error code
 */
////////////////////////////////////////////////////////////////////////////////
esp_err_t http_server_OTA_status_handler(httpd_req_t *req)
{
	char ota_json[100];

	ESP_LOGI(gp_http_server_ota_tag, "OTAstatus requested");

	sprintf(ota_json, "{\"ota_update_status\":%d,\"compile_time\":\"%s\",\"compile_date\":\"%s\"}", g_fw_update_status, __TIME__, __DATE__);

	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, ota_json, strlen(ota_json));

	return ESP_OK;
}


////////////////////////////////////////////////////////////////////////////////
// HTTP server functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 	Sets up the default httpd server configuration.
 * 
 * @return 	HTTP server instance handle / NULL 
 */
////////////////////////////////////////////////////////////////////////////////
static httpd_handle_t http_server_configure(void)
{
    // Generate the default configuration
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    //TODO: Create HTTP server monitor task
	xTaskCreate(&http_server_monitor, "http_server_monitor", HTTP_SERVER_MONITOR_STACK_SIZE, NULL, HTTP_SERVER_MONITOR_PRIORITY, &g_task_http_server_monitor);

    //TODO: Create the message queue
	http_server_monitor_queue_handle = xQueueCreate(3, sizeof(http_server_queue_message_t));
	// Adjust the default priority to 1 less than the wifi application task
	config.task_priority = HTTP_SERVER_TASK_PRIORITY;

	// Bump up the stack size (default is 4096)
	config.stack_size = HTTP_SERVER_TASK_STACK_SIZE;

	// Increase uri handlers
	config.max_uri_handlers = 20;

	// Increase the timeout limits
	config.recv_wait_timeout = 10;
	config.send_wait_timeout = 10;

	ESP_LOGI(gp_http_server_tag,  "Starting server on port: '%d' with task priority: '%d' \n", config.server_port, config.task_priority);

	// Start the httpd server
	if (httpd_start(&http_server_handle, &config) == ESP_OK)
	{
		ESP_LOGI(gp_http_server_tag, "ENTERED THE HTTPD START");
		ESP_LOGI(gp_http_server_tag, "Registering URI handlers");

		// register query handler
		httpd_uri_t jquery_js = {
				.uri = "/jquery-3.3.1.min.js",
				.method = HTTP_GET,
				.handler = http_server_jquery_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &jquery_js);

		// register index.html handler
		httpd_uri_t index_html = {
				.uri = "/",
				.method = HTTP_GET,
				.handler = http_server_index_html_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &index_html);

		// register app.css handler
		httpd_uri_t app_css = {
				.uri = "/app.css",
				.method = HTTP_GET,
				.handler = http_server_app_css_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &app_css);

		// register app.js handler
		httpd_uri_t app_js = {
				.uri = "/app.js",
				.method = HTTP_GET,
				.handler = http_server_app_js_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &app_js);

		// register favicon.ico handler
		httpd_uri_t favicon_ico = {
				.uri = "/favicon.ico",
				.method = HTTP_GET,
				.handler = http_server_favicon_ico_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &favicon_ico);

		// register OTA_update handler
		httpd_uri_t OTA_update = {
				.uri = "/OTAupdate",
				.method = HTTP_POST,
				.handler = http_server_OTA_update_handler,
				.user_ctx = NULL
		};

		httpd_register_uri_handler(http_server_handle, &OTA_update);

		// register OTA_status handler
		httpd_uri_t OTA_status = {
				.uri = "/OTAstatus",
				.method = HTTP_POST,
				.handler = http_server_OTA_status_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &OTA_status);

				// register dhtSensor.json handler
		httpd_uri_t dht_sensor_json = {
				.uri = "/dhtSensor.json",
				.method = HTTP_GET,
				.handler = http_server_get_dht_sensor_readings_json_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &dht_sensor_json);
		
		// register wifiConnect.json handler
		httpd_uri_t wifi_connect_json = {
				.uri = "/wifiConnect.json",
				.method = HTTP_POST,
				.handler = http_server_wifi_connect_json_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &wifi_connect_json);

		// register wifiConnectStatus.json handler
		httpd_uri_t wifi_connect_status_json = {
				.uri = "/wifiConnectStatus",
				.method = HTTP_POST,
				.handler = http_server_wifi_connect_status_json_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &wifi_connect_status_json);

		// register localTime.json handler
		httpd_uri_t local_time_json = {
				.uri = "/localTime.json",
				.method = HTTP_GET,
				.handler = http_server_get_local_time_json_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &local_time_json);

		return http_server_handle;
	}

    return NULL;
}


////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *	@addtogroup HTTP_SERVER_API
 *	@{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API Function Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 	Starts the HTTP server.
 * 
 * @return	void
 */
////////////////////////////////////////////////////////////////////////////////
void http_server_start(void)
{
	if (http_server_handle == NULL)
	{
		http_server_handle = http_server_configure();
	}
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 	Stops the HTTP server.
 * 
 * @return	void
 */
////////////////////////////////////////////////////////////////////////////////
void http_server_stop(void)
{
	if (http_server_handle)
	{
		httpd_stop(http_server_handle);
		ESP_LOGI(gp_http_server_tag, "Stopping HTTP server");
		http_server_handle = NULL;
	}

	if (g_task_http_server_monitor)
	{
		vTaskDelete(g_task_http_server_monitor);
		ESP_LOGI(gp_http_server_tag, "stopping HTTP server monitor");
		g_task_http_server_monitor = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief		Sends a message to the queue. Returns pdTRUE if an item was 
 * 				successfully sent to the queue, otherwise pdFALSE.
 * 
 * @param[in] 	msgID 	Message ID from the http_server_message_e enum.
 * @return 		pdTRUE / pdFALSE
 */
////////////////////////////////////////////////////////////////////////////////
BaseType_t http_server_monitor_send_message(http_server_message_e msgID)
{
	http_server_queue_message_t msg;
	msg.msgID = msgID;
	return xQueueSend(http_server_monitor_queue_handle, &msg, portMAX_DELAY);
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief		Timer callback function which calls esp_restart upon successful 
 * 				firmware update.
 * 
 * @param[in]	arg		Void pointer for an argument
 * @return		void
 */
////////////////////////////////////////////////////////////////////////////////
void http_server_fw_update_reset_callback(void *arg)
{
	ESP_LOGI(gp_http_server_ota_tag, "Timer timed-out, restarting the device");
	esp_restart();
}

////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////