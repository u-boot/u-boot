/*
 * (C) Copyright 2003
 * Texas Instruments <www.ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <arm925t.h>

#define MIF_CONFIG_REG 0xFFFECC0C
#define FLASH_GLOBAL_CTRL_NWP 1

void archflashwp (void *archdata, int wp)
{
	ulong *fgc = (ulong *) MIF_CONFIG_REG;

	if (wp == 1)
		*fgc &= ~FLASH_GLOBAL_CTRL_NWP;
	else
		*fgc |= FLASH_GLOBAL_CTRL_NWP;
}
