////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT (c) 2024
// NOMNIO d.o.o.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////
/**
 *	@file		nvs.h
 *	@author   	Tibor Sovilj
 *	@date		23.1.2024
 *	@version	V0.0.1

 *	@brief 		Header file for the NVS.
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 *	@addtogroup NVS_API
 *	@{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////
#ifndef __NVS_MODULE_H
#define __NVS_MODULE_H

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
// API Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API Typedefs
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API Variables
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// API Function Prototypes
////////////////////////////////////////////////////////////////////////////////
esp_err_t   nvs_save_dps_creds  (void);
bool        nvs_load_dps_creds  (void);
esp_err_t   nvs_clear_dps_creds (void);

#endif /* __NVS_MODULE_H */

////////////////////////////////////////////////////////////////////////////////
/**
 *	@} <!-- END GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////