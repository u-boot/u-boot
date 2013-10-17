/*
 * (C) Copyright 2003
 * Martin Winistoerfer, martinwinistoerfer@gmx.ch.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * File:		cmi.c
 *
 * Discription:		For generic board specific functions
 *
 */


#include <common.h>
#include <mpc5xx.h>

#define SRAM_SIZE	1024000L	/* 1M RAM available*/

#if defined(__APPLE__)
/* Leading underscore on symbols */
#  define SYM_CHAR "_"
#else /* No leading character on symbols */
#  define SYM_CHAR
#endif

/*
 * Macros to generate global absolutes.
 */
#define GEN_SYMNAME(str) SYM_CHAR #str
#define GEN_VALUE(str) #str
#define GEN_ABS(name, value) \
		asm (".globl " GEN_SYMNAME(name)); \
		asm (GEN_SYMNAME(name) " = " GEN_VALUE(value))

/*
 * Check the board
 */
int checkboard(void)
{
    puts ("Board: ### No HW ID - assuming CMI board\n");
    return (0);
}

/*
 * Get RAM size.
 */
phys_size_t initdram(int board_type)
{
	return (SRAM_SIZE);		/* We currently have a static size adapted for cmi board. */
}

/*
 * Absolute environment address for linker file.
 */
GEN_ABS(env_start, CONFIG_ENV_OFFSET + CONFIG_SYS_FLASH_BASE);
