////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT (c) 2023
// NOMNIO d.o.o.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////
/**
 *	@file		http_server.c
 *	@author   	Tibor Sovilj
 *	@date		12.10.2023
 *	@version	V0.0.1

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

#include "http_server.h"
#include "../../wifi/lib/wifi.h"
////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Static Typedefs
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Static Global Variables
////////////////////////////////////////////////////////////////////////////////

// Firmware update status
static int g_fw_update_status = OTA_UPDATE_PENDING;

// HTTP server task handle
static httpd_handle_t http_server_handle = NULL;

// HTTP server monitor task handle
static TaskHandle_t task_http_server_monitor = NULL;

// Queue handle used to manipulate the main queue of events
static QueueHandle_t http_server_monitor_queue_handle;

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

/**
 * Checks the g_fw_update_status and creates the fw_update_reset timer if g_fw_update_status is true.
 */
static void http_server_fw_update_reset_timer(void)
{
	if (g_fw_update_status == OTA_UPDATE_SUCCESSFUL)
	{
		printf("http_server_fw_update_reset_timer: FW updated successful starting FW update reset timer");

		// Give the web page a chance to receive an acknowledge back and initialize the timer
		ESP_ERROR_CHECK(esp_timer_create(&fw_update_reset_args, &fw_update_reset));
		ESP_ERROR_CHECK(esp_timer_start_once(fw_update_reset, 8000000));
	}
	else
	{
		printf("http_server_fw_update_reset_timer: FW update unsuccessful");
	}
}

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief		HTTP server monitor task used to track events of the HTTP server
 * 
 * @param[in] 	pvParameters 	Parameter which can be passed to the task.
 */
////////////////////////////////////////////////////////////////////////////////
static void http_server_monitor(void *parameter)
{
	http_server_queue_message_t msg;

	for (;;)
	{
		if (xQueueReceive(http_server_monitor_queue_handle, &msg, portMAX_DELAY))
		{
			switch (msg.msgID)
			{
				case HTTP_MSG_WIFI_CONNECT_INIT:
					printf("HTTP_MSG_WIFI_CONNECT_INIT\n");
					break;

				case HTTP_MSG_WIFI_CONNECT_SUCCESS:
					printf("HTTP_MSG_WIFI_CONNECT_SUCCESS\n");
					break;

				case HTTP_MSG_WIFI_CONNECT_FAIL:
					printf("HTTP_MSG_WIFI_CONNECT_FAIL\n");
					break;

				case HTTP_MSG_OTA_UPDATE_SUCCESSFUL:
					printf("HTTP_MSG_OTA_UPDATE_SUCCESSFUL\n");

					g_fw_update_status = OTA_UPDATE_SUCCESSFUL;
					http_server_fw_update_reset_timer();

					break;

				case HTTP_MSG_OTA_UPDATE_FAILED:
					printf("HTTP_MSG_OTA_UPDATE_FAILED\n");

					g_fw_update_status = OTA_UPDATE_FAILED;

					break;
				
				case HTTP_MSG_OTA_UPATE_INITIALIZED:
					printf("HTTP_MSG_OTA_UPDATE_INITIALIZED\n");

					g_fw_update_status = OTA_UPDATE_FAILED;

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
	printf("Jquery requested\n");

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
	printf("index.html requested\n");

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
	printf("app.css requested\n");

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
	printf("app.js requested\n");

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
	printf("favicon.ico requested\n");

	httpd_resp_set_type(req, "image/x-icon");
	httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_end - favicon_ico_start);

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
				printf("http_server_OTA_update_handler: Socket Timeout\n");
				continue;
			}
						
			printf("http_server_OTA_update_handler: OTA other Error %d\n", recv_len);

			return ESP_FAIL;
		}

		printf("http_server_OTA_update_handler: OTA RX: %d of %d\n", content_received, content_length);

		// Verify weather if this is the first occurence of any data = .bin file header,
		// else proceed with writing OTA data to a selected partition over esp_ota_write()
		if (!is_req_body_started)
		{
			is_req_body_started = true;

			// Get the location of the .bin file content (offset by web form data lenght)
			char *body_start_p = strstr(ota_buff, "\r\n\r\n") + 4;				

			// Calculating lenght of .bin body, without header or any metadata
			int body_part_len = recv_len - (body_start_p - ota_buff);

			printf("http_server_OTA_update_handler: OTA file size: %d\n", content_length);

			// Begin the update, OTA_SIZE_UNKNOWN provided so the whole partition can be erased before the update
			esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
			
			if (err != ESP_OK)
			{
				printf("http_server_OTA_update_handler: ERROR with OTA begin, cancelling OTA\r\n");
				return ESP_FAIL;
			}
			else
			{
				printf("http_server_OTA_update_handler: Writing to partition subtype %d at offset 0x%lx\r\n", update_partition->subtype, update_partition->address);
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
			printf("http_server_OTA_update_handler: Next boot partition subtype %d at offset 0x%lx\n", boot_partition->subtype, boot_partition->address);
			flash_successful = true;
		}
		else
		{
			printf("http_server_OTA_update_handler: FLASHED ERROR!!!\n");
		}
	}
	else
	{
		printf("http_server_OTA_update_handler: esp_ota_end ERROR!!!\n");
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

	printf("OTAstatus requested");

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
	xTaskCreate(&http_server_monitor, "http_server_monitor", HTTP_SERVER_MONITOR_STACK_SIZE, NULL, HTTP_SERVER_MONITOR_PRIORITY, &task_http_server_monitor);

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

	printf(	"http_server_configure: Starting server on port: '%d' with task priority: '%d' \n", config.server_port, config.task_priority);

	// Start the httpd server
	if (httpd_start(&http_server_handle, &config) == ESP_OK)
	{
		printf("ENTERED THE HTTPD START\n");
		printf("http_server_configure: Registering URI handlers\n");

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
		printf("http_server_stop: stopping HTTP server\n");
		http_server_handle = NULL;
	}

	if (task_http_server_monitor)
	{
		vTaskDelete(task_http_server_monitor);
		printf("http_server_stop: stopping HTTP server monitor\n");
		task_http_server_monitor = NULL;
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
	printf("http_server_fw_update_reset_callback: Timer timed-out, restarting the device\n");
	esp_restart();
}

////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////