////////////////////////////////////////////////////////////////////////////////
// COPYRIGHT (c) 2023
// NOMNIO d.o.o.
// All rights reserved.
////////////////////////////////////////////////////////////////////////////////
/**
 *	@file		sntp_cfg.h
 *	@author   	Tibor Sovilj
 *	@date		29.10.2023
 *	@version	V0.0.1

 *	@brief 		Configuration Header file for the SNTP protocol.
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
 * 	@addtogroup SNTP_CONFIG
 * 	@{ <!-- BEGIN GROUP -->
 */
////////////////////////////////////////////////////////////////////////////////
#ifndef __SNTP_CFG_H
#define __SNTP_CFG_H

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>

#include "../lib/sntp.h"


////////////////////////////////////////////////////////////////////////////////
// CONFIG Definitions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// CONFIG Typedefs
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// CONFIG Function Prototypes
////////////////////////////////////////////////////////////////////////////////
sntp_config_t* sntp_get_config(void);

#endif /* __SNTP_CFG_H  */

////////////////////////////////////////////////////////////////////////////////
/*!
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////