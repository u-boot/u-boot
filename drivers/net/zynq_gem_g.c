/*
 * (C) Copyright 2012 Xilinx
 *
 * This file contains a configuration table that specifies the configuration of
 * ethernet devices in the system.
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


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/

/*
 * The configuration table for emacps device
 */

XEmacPss_Config XEmacPss_ConfigTable[2] = {
	{
	 0,  /* Device ID */
	 0xe000b000    /* Device base address */
	},
	{
	 1,  /* Device ID */
	 0xe000c000    /* Device base address */
	}
};
