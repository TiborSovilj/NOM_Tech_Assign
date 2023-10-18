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

// HTTP server task handle
static httpd_handle_t http_server_handle = NULL;

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
//static esp_err_t http_server_jquery_handler(httpd_req_t *req);

typedef struct 
{
	const uint8_t* data
}containerStruct;

typedef enum{
	eONE,
	eTWO,

	eCONTAINER_NUM_OF
}containtermembers_e;

containerStruct containerTable[eCONTAINER_NUM_OF] = {
	[ eONE		] = {.data = index_html_start 	},
	[ eTWO		] = {.data = index_html_end		},
};
////////////////////////////////////////////////////////////////////////////////
// Static Function Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * @brief 		Jquery get handler is requested when accessing the web page.
 * 
 * @param[in]	req		HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
////////////////////////////////////////////////////////////////////////////////
static esp_err_t http_server_jquery_handler(httpd_req_t *req)
{
	printf("Jquery requested\n");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)jquery_3_3_1_min_js_start, jquery_3_3_1_min_js_end - jquery_3_3_1_min_js_start);

	return ESP_OK;
}

static esp_err_t http_server_index_html_handler(httpd_req_t *req)
{
	printf("index.html requested\n");

	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, (const char *)containerTable[eONE].data, containerTable[eTWO].data - containerTable[eONE].data);

	return ESP_OK;
}

static esp_err_t http_server_app_css_handler(httpd_req_t *req)
{
	printf("app.css requested\n");

	httpd_resp_set_type(req, "text/css");
	httpd_resp_send(req, (const char *)app_css_start, app_css_end - app_css_start);

	return ESP_OK;
}

static esp_err_t http_server_app_js_handler(httpd_req_t *req)
{
	printf("app.js requested\n");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)app_js_start, app_js_end - app_js_start);

	return ESP_OK;
}

static esp_err_t http_server_favicon_ico_handler(httpd_req_t *req)
{
	printf("favicon.ico requested\n");

	httpd_resp_set_type(req, "image/x-icon");
	httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_end - favicon_ico_start);

	return ESP_OK;
}

static httpd_handle_t http_server_configure(void)
{
    // Generate the default configuration
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    //TODO: Create HTTP server monitor task

    //TODO: Create the message queue

	// Adjust the default priority to 1 less than the wifi application task
	config.task_priority = HTTP_SERVER_TASK_PRIORITY;

	// Bump up the stack size (default is 4096)
	config.stack_size = HTTP_SERVER_TASK_STACK_SIZE;

	// Increase uri handlers
	config.max_uri_handlers = 20;

	// Increase the timeout limits
	config.recv_wait_timeout = 10;
	config.send_wait_timeout = 10;

	printf(	"http_server_configure: Starting server on port: '%d' with task priority: '%d'",
			config.server_port,
			config.task_priority);
	// Start the httpd server
	if (httpd_start(&http_server_handle, &config) == ESP_OK)
	{
		printf("ENTERED THE HTTPD START");
		printf("http_server_configure: Registering URI handlers");

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
		printf("http_server_stop: stopping HTTP server");
		http_server_handle = NULL;
	}
}
////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////