/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004, Li-Pro.Net <www.li-pro.net>
 * Stephan Linz <linz@li-pro.net>
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


#include <common.h>
#include <nios.h>

/*
 * include common flash code (for ssv boards)
 */
#include "../common/flash.c"

/*---------------------------------------------------------------------*/
#define	BANKSZ	(8 * 1024 * 1024)
#define	SECTSZ	(64 * 1024)
#define	UBOOTSECS ((CFG_MONITOR_LEN + CFG_ENV_SIZE) / SECTSZ)
#define	UBOOTAREA (UBOOTSECS * 64 * 1024)	/* monitor / env area */

/*---------------------------------------------------------------------*/
unsigned long flash_init (void)
{
	int i;
	unsigned long addr;
	flash_info_t *fli = &flash_info[0];

	fli->size = BANKSZ;
	fli->sector_count = CFG_MAX_FLASH_SECT;
	fli->flash_id = FLASH_MAN_AMD + FLASH_AMLV640U;

	addr = CFG_FLASH_BASE;
	for (i = 0; i < fli->sector_count; ++i) {
		fli->start[i] = addr;
		addr += SECTSZ;

		/* Protect monitor / environment area */
		if (addr <= (CFG_FLASH_BASE + UBOOTAREA))
			fli->protect[i] = 1;
		else
			fli->protect[i] = 0;
	}

	return (BANKSZ);
}
