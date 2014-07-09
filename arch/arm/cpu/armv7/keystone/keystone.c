/*
 * Keystone EVM : Board initialization
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>

/**
 * cpu_to_bus - swap bytes of the 32-bit data if the device is BE
 * @ptr - array of data
 * @length - lenght of data array
 */
int cpu_to_bus(u32 *ptr, u32 length)
{
	u32 i;

	if (!(readl(K2HK_DEVSTAT) & 0x1))
		for (i = 0; i < length; i++, ptr++)
			*ptr = cpu_to_be32(*ptr);

	return 0;
}
