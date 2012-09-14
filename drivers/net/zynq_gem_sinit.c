/*
 * (C) Copyright 2012 Xilinx
 *
 * This file contains lookup method by device ID when success, it returns
 * pointer to config table to be used to initialize the device.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/***************************** Include Files *********************************/

#include "zynq_gem.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/*****************************************************************************/
/**
* Lookup the device configuration based on the unique device ID.  The table
* contains the configuration info for each device in the system.
*
* @param DeviceId is the unique device ID of the device being looked up.
*
* @return
* A pointer to the configuration table entry corresponding to the given
* device ID, or NULL if no match is found.
*
******************************************************************************/
XEmacPss_Config *XEmacPss_LookupConfig(u16 DeviceId)
{
	extern XEmacPss_Config XEmacPss_ConfigTable[];
	XEmacPss_Config *CfgPtr = NULL;
	int i;

	for (i = 0; i < XPAR_XEMACPSS_NUM_INSTANCES; i++) {
		if (XEmacPss_ConfigTable[i].DeviceId == DeviceId) {
			CfgPtr = &XEmacPss_ConfigTable[i];
			break;
		}
	}

	return (CfgPtr);
}
