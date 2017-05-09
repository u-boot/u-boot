/*
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <dm/uclass.h>

/* Firmware access is platform-dependent.  No generic code in uclass */
UCLASS_DRIVER(firmware) = {
	.id		= UCLASS_FIRMWARE,
	.name		= "firmware",
};
