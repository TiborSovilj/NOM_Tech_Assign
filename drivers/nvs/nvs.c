////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT (c) 2024
// NOMNIO d.o.o.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////
/**
 *	@file		nvs.c
 *	@author   	Tibor Sovilj
 *	@date		23.1.2024
 *	@version	V0.0.1

 *	@brief 		Source file for the NVS.
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *	@addtogroup NVS_STATIC
 *	@{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "nvs.h"
#include "../protocols/wifi/lib/wifi.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#define AZURE_DPS_PRIMARY_SYMETRIC_KEY_MAX_LEN		( 128 )
#define	AZURE_DPS_PRIMARY_SYMETRIC_KEY		"keSMVxML0+nzZ3SIWb1qDVQbV9JTzYDfgWmC+eabATE+6irrMkWad+/DrXNqofcoBafZZthZL6DolKQjhIV1VA=="

#define AZURE_DPS_SERVICE_HOST_ENDPOINT_MAX_LEN		( 64 )
#define AZURE_DPS_SERVICE_HOST_ENDPOINT		"tsoviljdps.azure-devices-provisioning.net"

#define AZURE_DPS_ID_SCOPE_MAX_LEN					( 32 )
#define AZURE_DPS_ID_SCOPE 					"0ne00BD8C68"

////////////////////////////////////////////////////////////////////////////////
// Static Typedefs
////////////////////////////////////////////////////////////////////////////////
/**
 *	@brief Basic DPS credentials
 */
typedef struct
{
    uint8_t* symetricKey;
    uint8_t* hostEndPoint;
    uint8_t* idScope;
}azure_dps_reg_data;

////////////////////////////////////////////////////////////////////////////////
// Static Global Variables
////////////////////////////////////////////////////////////////////////////////

static const char TAG[] = "nvs";
const char g_nvs_azure_dps_creds[] = "AZURE_DPS_NVS:";
azure_dps_reg_data* azure_dps_creds = NULL;

////////////////////////////////////////////////////////////////////////////////
// Static Function Prototypes
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Static Function Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *	@addtogroup NVS_API
 *	@{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API Function Definitions
////////////////////////////////////////////////////////////////////////////////

/**
 * Set all 3 required strings 
 * 		1. Primary key (symetricKey) (function AzureIoTProvisioningClient_SetSymmetricKey)
 * 		2. Service endpoint (provisioning host) from the resource summary
 * 		3. ID Scope from the resource summary
 */

esp_err_t nvs_save_dps_creds (void)
{
    nvs_handle handle;
    esp_err_t esp_err;
    ESP_LOGI(TAG, "Saving provisioning credentials");
    
    // Opening handle for NVS
    esp_err = nvs_open(g_nvs_azure_dps_creds, NVS_READWRITE, &handle);
    if (esp_err != ESP_OK)
    {
        printf("Error opening NVS handle, PRIM_KEY!\n");
        return esp_err;
    }
    
    // PRIMARY_KEY
    esp_err = nvs_set_blob(handle, "symetricKey", AZURE_DPS_PRIMARY_SYMETRIC_KEY, AZURE_DPS_PRIMARY_SYMETRIC_KEY_MAX_LEN);
    if (esp_err != ESP_OK)
    {
        printf("Error setting PRIMARY_KEY to NVS!\n");
        return esp_err;
    }

    // HOST_ENDPOINT
    esp_err = nvs_set_blob(handle, "hostEndpoint", AZURE_DPS_SERVICE_HOST_ENDPOINT, AZURE_DPS_SERVICE_HOST_ENDPOINT_MAX_LEN);
    if (esp_err != ESP_OK)
    {
        printf("Error setting HOST_ENDPOINT to NVS!\n");
        return esp_err;
    }

    // ID_SCOPE
    esp_err = nvs_set_blob(handle, "idScope", AZURE_DPS_ID_SCOPE, AZURE_DPS_ID_SCOPE_MAX_LEN);
    if (esp_err != ESP_OK)
    {
        printf("Error setting ID_SCOPE to NVS!\n");
        return esp_err;
    }

    // Commit credentials to NVS
    esp_err = nvs_commit(handle);
    if (esp_err != ESP_OK)
    {
        printf("Error comitting credentials to NVS!\n");
        return esp_err;
    }

    nvs_close(handle);
    ESP_LOGI(TAG, "NVS_SUCCESS: Wrote DPS credentials to NVS");
    return ESP_OK;
}

bool nvs_load_dps_creds (void)
{
    nvs_handle handle;
    esp_err_t esp_err;

    ESP_LOGI(TAG, "nvs_load_sta_creds: Loading Wifi credentials from flash");

    if (nvs_open(g_nvs_azure_dps_creds, NVS_READONLY, &handle) == ESP_OK)
    {
        // Initialize struct instance
        //azure_dps_reg_data* azure_dps_creds;
        memset(azure_dps_creds, 0x00, sizeof(azure_dps_reg_data));

        // Define size, allocate memory for that size, then initiaize memory space with zeros
        size_t azure_dps_reg_data_size = sizeof(azure_dps_reg_data);
        uint8_t* azure_dps_reg_data_buff = (uint8_t*)malloc(sizeof(uint8_t) * azure_dps_reg_data_size);
        memset(azure_dps_reg_data_buff, 0x00, sizeof(azure_dps_reg_data_size));

        // Redefine size, request blob, check if the readout was successful, then copy to an appropriate location
        // Load Symetric Key
        azure_dps_reg_data_size = sizeof(azure_dps_creds->symetricKey);
        esp_err = nvs_get_blob(handle, "symetricKey", azure_dps_reg_data_buff, &azure_dps_reg_data_size);
        if (esp_err != ESP_OK)
        {
            free(azure_dps_reg_data_buff);
            printf("Azure DPS ERROR: Symetric Key can't be loaded");
            return false;
        }
        memcpy(azure_dps_creds->symetricKey, azure_dps_reg_data_buff, azure_dps_reg_data_size);

        // Load Host Endpoint 
        azure_dps_reg_data_size = sizeof(azure_dps_creds->hostEndPoint);
        esp_err = nvs_get_blob(handle, "hostEndpoint", azure_dps_reg_data_buff, &azure_dps_reg_data_size);
        if (esp_err != ESP_OK)
        {
            free(azure_dps_reg_data_buff);
            printf("Azure DPS ERROR:  Host Endpoint can't be loaded");
            return false;
        }
        memcpy(azure_dps_creds->hostEndPoint, azure_dps_reg_data_buff, azure_dps_reg_data_size);

        // Load ID Scope 
        azure_dps_reg_data_size = sizeof(azure_dps_creds->idScope);
        esp_err = nvs_get_blob(handle, "idScope", azure_dps_reg_data_buff, &azure_dps_reg_data_size);
        if (esp_err != ESP_OK)
        {
            free(azure_dps_reg_data_buff);
            printf("Azure DPS ERROR: ID Scope can't be loaded");
            return false;
        }
        memcpy(azure_dps_creds->idScope, azure_dps_reg_data_buff, azure_dps_reg_data_size);

        free(azure_dps_reg_data_buff);
        nvs_close(handle);

        printf("Azure DPS creds: Symetric Key: %s\nHost Endpoint: %s\n ID Scope: %s\n", azure_dps_creds->symetricKey, azure_dps_creds->hostEndPoint, azure_dps_creds->idScope);
        return true;
    }
    else
    {
        return false;
    }
}

esp_err_t nvs_clear_dps_creds (void)
{
    nvs_handle handle;
    esp_err_t esp_err;
    ESP_LOGI(TAG, "Clearing Azure DPS credentials");

    esp_err = nvs_open(g_nvs_azure_dps_creds, NVS_READWRITE, &handle);
    if (esp_err != ESP_OK)
    {
        printf("Error opening NVS section for Azure DPS credentials.\n");
        return esp_err;
    }

    // Erase credentials
    esp_err = nvs_erase_all(handle);
    if (esp_err != ESP_OK)
    {
        printf("Error erasing Azure DPS credentials.\n");
        return esp_err;
    }

    // Commit clearing credentials from NVS
    esp_err = nvs_commit(handle);
    if (esp_err != ESP_OK)
    {
        printf("Error NVS erase_commit.\n");
        return esp_err;
    }
    nvs_close(handle);

    printf("Clearing NVS: Azure DPD credentials returned ESP_OK\n");
    return ESP_OK;
}

////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////