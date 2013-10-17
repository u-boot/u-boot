/*
 * (C) Copyright 2008
 * Ulf Samuelsson <ulf@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <config.h>
#include <asm/hardware.h>
#include <dataflash.h>

AT91S_DATAFLASH_INFO dataflash_info[CONFIG_SYS_MAX_DATAFLASH_BANKS];

struct dataflash_addr cs[CONFIG_SYS_MAX_DATAFLASH_BANKS] = {
	{CONFIG_SYS_DATAFLASH_LOGIC_ADDR_CS0, 0},	/* Logical adress, CS */
};

/*define the area offsets*/
dataflash_protect_t area_list[NB_DATAFLASH_AREA] = {
	{0x00000000, 0x000041FF, FLAG_PROTECT_SET,   0, "Bootstrap"},
	{0x00004200, 0x000083FF, FLAG_PROTECT_CLEAR, 0, "Environment"},
	{0x00008400, 0x00083FFF, FLAG_PROTECT_SET,   0, "U-Boot"},
	{0x00084000, 0x00293FFF, FLAG_PROTECT_CLEAR, 0,	"Kernel"},
	{0x00294000, 0xFFFFFFFF, FLAG_PROTECT_CLEAR, 0,	"FS"},
};
