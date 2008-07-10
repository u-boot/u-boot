/*
 * (C) Copyright 2003
 * Martin Winistoerfer, martinwinistoerfer@gmx.ch.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
GEN_ABS(env_start, CFG_ENV_OFFSET + CFG_FLASH_BASE);
