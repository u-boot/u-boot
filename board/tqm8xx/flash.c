/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#if 0
#define DEBUG
#endif

#include <common.h>
#include <mpc8xx.h>
#include <environment.h>

#include <asm/processor.h>

#if defined(CONFIG_TQM8xxL) && !defined(CONFIG_TQM866M)
# ifndef CFG_OR_TIMING_FLASH_AT_50MHZ
#  define CFG_OR_TIMING_FLASH_AT_50MHZ	(OR_ACS_DIV1  | OR_TRLX | OR_CSNT_SAM | \
					 OR_SCY_2_CLK | OR_EHTR | OR_BI)
# endif
#endif /* CONFIG_TQM8xxL/M, !TQM866M */

#ifndef	CFG_ENV_ADDR
#define CFG_ENV_ADDR	(CFG_FLASH_BASE + CFG_ENV_OFFSET)
#endif

flash_info_t	flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	volatile immap_t     *immap  = (immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	unsigned long size_b0, size_b1;
	int i;

#ifdef	CFG_OR_TIMING_FLASH_AT_50MHZ
	int scy, trlx, flash_or_timing, clk_diff;

	DECLARE_GLOBAL_DATA_PTR;

	scy = (CFG_OR_TIMING_FLASH_AT_50MHZ & OR_SCY_MSK) >> 4;
	if (CFG_OR_TIMING_FLASH_AT_50MHZ & OR_TRLX) {
		trlx = OR_TRLX;
		scy *= 2;
	} else
		trlx = 0;

		/* We assume that each 10MHz of bus clock require 1-clk SCY
		 * adjustment.
		 */
	clk_diff = (gd->bus_clk / 1000000) - 50;

		/* We need proper rounding here. This is what the "+5" and "-5"
		 * are here for.
		 */
	if (clk_diff >= 0)
		scy += (clk_diff + 5) / 10;
	else
		scy += (clk_diff - 5) / 10;

		/* For bus frequencies above 50MHz, we want to use relaxed timing
		 * (OR_TRLX).
		 */
	if (gd->bus_clk >= 50000000)
		trlx = OR_TRLX;
	else
		trlx = 0;

	if (trlx)
		scy /= 2;

	if (scy > 0xf)
		scy = 0xf;
	if (scy < 1)
		scy = 1;

	flash_or_timing = (scy << 4) | trlx |
	                  (CFG_OR_TIMING_FLASH_AT_50MHZ & ~(OR_TRLX | OR_SCY_MSK));
#endif
	/* Init: no FLASHes known */
	for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Static FLASH Bank configuration here - FIXME XXX */

	debug ("\n## Get flash bank 1 size @ 0x%08x\n",FLASH_BASE0_PRELIM);

	size_b0 = flash_get_size((vu_long *)FLASH_BASE0_PRELIM, &flash_info[0]);

	debug ("## Get flash bank 2 size @ 0x%08x\n",FLASH_BASE1_PRELIM);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
			size_b0, size_b0<<20);
	}

	size_b1 = flash_get_size((vu_long *)FLASH_BASE1_PRELIM, &flash_info[1]);

	debug ("## Prelim. Flash bank sizes: %08lx + 0x%08lx\n",size_b0,size_b1);

	if (size_b1 > size_b0) {
		printf ("## ERROR: "
			"Bank 1 (0x%08lx = %ld MB) > Bank 0 (0x%08lx = %ld MB)\n",
			size_b1, size_b1<<20,
			size_b0, size_b0<<20
		);
		flash_info[0].flash_id	= FLASH_UNKNOWN;
		flash_info[1].flash_id	= FLASH_UNKNOWN;
		flash_info[0].sector_count	= -1;
		flash_info[1].sector_count	= -1;
		flash_info[0].size		= 0;
		flash_info[1].size		= 0;
		return (0);
	}

	debug  ("## Before remap: "
		"BR0: 0x%08x    OR0: 0x%08x    "
		"BR1: 0x%08x    OR1: 0x%08x\n",
		memctl->memc_br0, memctl->memc_or0,
		memctl->memc_br1, memctl->memc_or1);

	/* Remap FLASH according to real size */
#ifndef	CFG_OR_TIMING_FLASH_AT_50MHZ
	memctl->memc_or0 = CFG_OR_TIMING_FLASH | (-size_b0 & OR_AM_MSK);
#else
	memctl->memc_or0 = flash_or_timing | (-size_b0 & OR_AM_MSK);
#endif
	memctl->memc_br0 = (CFG_FLASH_BASE & BR_BA_MSK) | BR_MS_GPCM | BR_V;

	debug ("## BR0: 0x%08x    OR0: 0x%08x\n",
		memctl->memc_br0, memctl->memc_or0);

	/* Re-do sizing to get full correct info */
	size_b0 = flash_get_size((vu_long *)CFG_FLASH_BASE, &flash_info[0]);

#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
	/* monitor protection ON by default */
	debug ("Protect monitor: %08lx ... %08lx\n",
		(ulong)CFG_MONITOR_BASE,
		(ulong)CFG_MONITOR_BASE + monitor_flash_len - 1);

	flash_protect(FLAG_PROTECT_SET,
		      CFG_MONITOR_BASE,
		      CFG_MONITOR_BASE + monitor_flash_len - 1,
		      &flash_info[0]);
#endif

#ifdef	CFG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
# ifdef CFG_ENV_ADDR_REDUND
	debug ("Protect primary   environment: %08lx ... %08lx\n",
		(ulong)CFG_ENV_ADDR,
		(ulong)CFG_ENV_ADDR + CFG_ENV_SECT_SIZE - 1);
# else
	debug ("Protect environment: %08lx ... %08lx\n",
		(ulong)CFG_ENV_ADDR,
		(ulong)CFG_ENV_ADDR + CFG_ENV_SECT_SIZE - 1);
# endif

	flash_protect(FLAG_PROTECT_SET,
		      CFG_ENV_ADDR,
		      CFG_ENV_ADDR + CFG_ENV_SECT_SIZE - 1,
		      &flash_info[0]);
#endif

#ifdef CFG_ENV_ADDR_REDUND
	debug ("Protect redundand environment: %08lx ... %08lx\n",
		(ulong)CFG_ENV_ADDR_REDUND,
		(ulong)CFG_ENV_ADDR_REDUND + CFG_ENV_SECT_SIZE - 1);

	flash_protect(FLAG_PROTECT_SET,
		      CFG_ENV_ADDR_REDUND,
		      CFG_ENV_ADDR_REDUND + CFG_ENV_SECT_SIZE - 1,
		      &flash_info[0]);
#endif

	if (size_b1) {
#ifndef	CFG_OR_TIMING_FLASH_AT_50MHZ
		memctl->memc_or1 = CFG_OR_TIMING_FLASH | (-size_b1 & 0xFFFF8000);
#else
		memctl->memc_or1 = flash_or_timing | (-size_b1 & 0xFFFF8000);
#endif
		memctl->memc_br1 = ((CFG_FLASH_BASE + size_b0) & BR_BA_MSK) |
				    BR_MS_GPCM | BR_V;

		debug ("## BR1: 0x%08x    OR1: 0x%08x\n",
			memctl->memc_br1, memctl->memc_or1);

		/* Re-do sizing to get full correct info */
		size_b1 = flash_get_size((vu_long *)(CFG_FLASH_BASE + size_b0),
					  &flash_info[1]);

#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
		/* monitor protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CFG_MONITOR_BASE,
			      CFG_MONITOR_BASE+monitor_flash_len-1,
			      &flash_info[1]);
#endif

#ifdef	CFG_ENV_IS_IN_FLASH
		/* ENV protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CFG_ENV_ADDR,
			      CFG_ENV_ADDR+CFG_ENV_SIZE-1,
			      &flash_info[1]);
#endif
	} else {
		memctl->memc_br1 = 0;		/* invalidate bank */

		flash_info[1].flash_id = FLASH_UNKNOWN;
		flash_info[1].sector_count = -1;
		flash_info[1].size = 0;

		debug ("## DISABLE BR1: 0x%08x    OR1: 0x%08x\n",
			memctl->memc_br1, memctl->memc_or1);
	}

	debug ("## Final Flash bank sizes: %08lx + 0x%08lx\n",size_b0,size_b1);

	flash_info[0].size = size_b0;
	flash_info[1].size = size_b1;

	return (size_b0 + size_b1);
}

/*-----------------------------------------------------------------------
 */
void flash_print_info  (flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:	printf ("AMD ");		break;
	case FLASH_MAN_FUJ:	printf ("FUJITSU ");		break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
#ifdef CONFIG_TQM8xxM	/* mirror bit flash */
	case FLASH_AMLV128U:	printf ("AM29LV128ML (128Mbit, uniform sector size)\n");
				break;
	case FLASH_AMLV320U:	printf ("AM29LV320ML (32Mbit, uniform sector size)\n");
				break;
	case FLASH_AMLV640U:	printf ("AM29LV640ML (64Mbit, uniform sector size)\n");
				break;
	case FLASH_AMLV320B:	printf ("AM29LV320MB (32Mbit, bottom boot sect)\n");
				break;
# else	/* ! TQM8xxM */
	case FLASH_AM400B:	printf ("AM29LV400B (4 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM400T:	printf ("AM29LV400T (4 Mbit, top boot sector)\n");
				break;
	case FLASH_AM800B:	printf ("AM29LV800B (8 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM800T:	printf ("AM29LV800T (8 Mbit, top boot sector)\n");
				break;
	case FLASH_AM320B:	printf ("AM29LV320B (32 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM320T:	printf ("AM29LV320T (32 Mbit, top boot sector)\n");
				break;
#endif	/* TQM8xxM */
	case FLASH_AM160B:	printf ("AM29LV160B (16 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM160T:	printf ("AM29LV160T (16 Mbit, top boot sector)\n");
				break;
	case FLASH_AMDL163B:	printf ("AM29DL163B (16 Mbit, bottom boot sect)\n");
				break;
	default:		printf ("Unknown Chip Type\n");
				break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     "
		);
	}
	printf ("\n");
	return;
}

/*-----------------------------------------------------------------------
 */


/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */

static ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
	short i;
	ulong value;
	ulong base = (ulong)addr;

	/* Write auto select command: read Manufacturer ID */
	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;
	addr[0x0555] = 0x00900090;

	value = addr[0];

	debug ("Manuf. ID @ 0x%08lx: 0x%08lx\n", (ulong)addr, value);

	switch (value) {
	case AMD_MANUFACT:
		debug ("Manufacturer: AMD\n");
		info->flash_id = FLASH_MAN_AMD;
		break;
	case FUJ_MANUFACT:
		debug ("Manufacturer: FUJITSU\n");
		info->flash_id = FLASH_MAN_FUJ;
		break;
	default:
		debug ("Manufacturer: *** unknown ***\n");
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);			/* no or unknown flash	*/
	}

	value = addr[1];			/* device ID		*/

	debug ("Device ID @ 0x%08lx: 0x%08lx\n", (ulong)(&addr[1]), value);

	switch (value) {
#ifdef CONFIG_TQM8xxM	/* mirror bit flash */
	case AMD_ID_MIRROR:
		debug ("Mirror Bit flash: addr[14] = %08lX  addr[15] = %08lX\n",
			addr[14], addr[15]);
		/* Special case for AMLV320MH/L */
		if ((addr[14] & 0x00ff00ff) == 0x001d001d &&
		    (addr[15] & 0x00ff00ff) == 0x00000000) {
			debug ("Chip: AMLV320MH/L\n");
			info->flash_id += FLASH_AMLV320U;
			info->sector_count = 64;
			info->size = 0x00800000;	/* => 8 MB */
			break;
		}
		switch(addr[14]) {
		case AMD_ID_LV128U_2:
			if (addr[15] != AMD_ID_LV128U_3) {
				debug ("Chip: AMLV128U -> unknown\n");
				info->flash_id = FLASH_UNKNOWN;
			} else {
				debug ("Chip: AMLV128U\n");
				info->flash_id += FLASH_AMLV128U;
				info->sector_count = 256;
				info->size = 0x02000000;
			}
			break;				/* => 32 MB	*/
		case AMD_ID_LV640U_2:
			if (addr[15] != AMD_ID_LV640U_3) {
				debug ("Chip: AMLV640U -> unknown\n");
				info->flash_id = FLASH_UNKNOWN;
			} else {
				debug ("Chip: AMLV640U\n");
				info->flash_id += FLASH_AMLV640U;
				info->sector_count = 128;
				info->size = 0x01000000;
			}
			break;				/* => 16 MB	*/
		case AMD_ID_LV320B_2:
			if (addr[15] != AMD_ID_LV320B_3) {
				debug ("Chip: AMLV320B -> unknown\n");
				info->flash_id = FLASH_UNKNOWN;
			} else {
				debug ("Chip: AMLV320B\n");
				info->flash_id += FLASH_AMLV320B;
				info->sector_count = 71;
				info->size = 0x00800000;
			}
			break;				/* =>  8 MB	*/
		default:
			debug ("Chip: *** unknown ***\n");
			info->flash_id = FLASH_UNKNOWN;
			break;
		}
		break;
# else	/* ! TQM8xxM */
	case AMD_ID_LV400T:
		info->flash_id += FLASH_AM400T;
		info->sector_count = 11;
		info->size = 0x00100000;
		break;					/* => 1 MB		*/

	case AMD_ID_LV400B:
		info->flash_id += FLASH_AM400B;
		info->sector_count = 11;
		info->size = 0x00100000;
		break;					/* => 1 MB		*/

	case AMD_ID_LV800T:
		info->flash_id += FLASH_AM800T;
		info->sector_count = 19;
		info->size = 0x00200000;
		break;					/* => 2 MB	*/

	case AMD_ID_LV800B:
		info->flash_id += FLASH_AM800B;
		info->sector_count = 19;
		info->size = 0x00200000;
		break;					/* => 2 MB	*/

	case AMD_ID_LV320T:
		info->flash_id += FLASH_AM320T;
		info->sector_count = 71;
		info->size = 0x00800000;
		break;					/* => 8 MB	*/

	case AMD_ID_LV320B:
		info->flash_id += FLASH_AM320B;
		info->sector_count = 71;
		info->size = 0x00800000;
		break;					/* => 8 MB	*/
#endif	/* TQM8xxM */

	case AMD_ID_LV160T:
		info->flash_id += FLASH_AM160T;
		info->sector_count = 35;
		info->size = 0x00400000;
		break;					/* => 4 MB	*/

	case AMD_ID_LV160B:
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00400000;
		break;					/* => 4 MB	*/

	case AMD_ID_DL163B:
		info->flash_id += FLASH_AMDL163B;
		info->sector_count = 39;
		info->size = 0x00400000;
		break;					/* => 4 MB	*/

	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */
	}

	/* set up sector start address table */
	switch (value) {
#ifdef CONFIG_TQM8xxM	/* mirror bit flash */
	case AMD_ID_MIRROR:
		switch (info->flash_id & FLASH_TYPEMASK) {
			/* only known types here - no default */
		case FLASH_AMLV128U:
		case FLASH_AMLV640U:
		case FLASH_AMLV320U:
			for (i = 0; i < info->sector_count; i++) {
				info->start[i] = base;
				base += 0x20000;
			}
			break;
		case FLASH_AMLV320B:
			for (i = 0; i < info->sector_count; i++) {
				info->start[i] = base;
				/*
				 * The first 8 sectors are 8 kB,
				 * all the other ones  are 64 kB
				 */
				base += (i < 8)
					?  2 * ( 8 << 10)
					:  2 * (64 << 10);
			}
			break;
		}
		break;
# else	/* ! TQM8xxM */
	case AMD_ID_LV400B:
	case AMD_ID_LV800B:
		/* set sector offsets for bottom boot block type	*/
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00008000;
		info->start[2] = base + 0x0000C000;
		info->start[3] = base + 0x00010000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00020000) - 0x00060000;
		}
		break;
	case AMD_ID_LV400T:
	case AMD_ID_LV800T:
		/* set sector offsets for top boot block type		*/
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00008000;
		info->start[i--] = base + info->size - 0x0000C000;
		info->start[i--] = base + info->size - 0x00010000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00020000;
		}
		break;
	case AMD_ID_LV320B:
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base;
			/*
			 * The first 8 sectors are 8 kB,
			 * all the other ones  are 64 kB
			 */
			base += (i < 8)
				?  2 * ( 8 << 10)
				:  2 * (64 << 10);
		}
		break;
	case AMD_ID_LV320T:
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base;
			/*
			 * The last 8 sectors are 8 kB,
			 * all the other ones  are 64 kB
			 */
			base += (i < (info->sector_count - 8))
				?  2 * (64 << 10)
				:  2 * ( 8 << 10);
		}
		break;
#endif	/* TQM8xxM */
	case AMD_ID_LV160B:
		/* set sector offsets for bottom boot block type	*/
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00008000;
		info->start[2] = base + 0x0000C000;
		info->start[3] = base + 0x00010000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00020000) - 0x00060000;
		}
		break;
	case AMD_ID_LV160T:
		/* set sector offsets for top boot block type		*/
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00008000;
		info->start[i--] = base + info->size - 0x0000C000;
		info->start[i--] = base + info->size - 0x00010000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00020000;
		}
		break;
	case AMD_ID_DL163B:
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base;
			/*
			 * The first 8 sectors are 8 kB,
			 * all the other ones  are 64 kB
			 */
			base += (i < 8)
				?  2 * ( 8 << 10)
				:  2 * (64 << 10);
		}
		break;
	default:
		return (0);
		break;
	}

#if 0
	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr = (volatile unsigned long *)(info->start[i]);
		info->protect[i] = addr[2] & 1;
	}
#endif

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr = (volatile unsigned long *)info->start[0];

		*addr = 0x00F000F0;	/* reset bank */
	}

	return (info->size);
}


/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	vu_long *addr = (vu_long*)(info->start[0]);
	int flag, prot, sect, l_sect;
	ulong start, now, last;

	debug ("flash_erase: first: %d last: %d\n", s_first, s_last);

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	if ((info->flash_id == FLASH_UNKNOWN) ||
	    (info->flash_id > FLASH_AMD_COMP)) {
		printf ("Can't erase unknown flash type %08lx - aborted\n",
			info->flash_id);
		return 1;
	}

	prot = 0;
	for (sect=s_first; sect<=s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf ("- Warning: %d protected sectors will not be erased!\n",
			prot);
	} else {
		printf ("\n");
	}

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;
	addr[0x0555] = 0x00800080;
	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = (vu_long*)(info->start[sect]);
			addr[0] = 0x00300030;
			l_sect = sect;
		}
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* wait at least 80us - let's wait 1 ms */
	udelay (1000);

	/*
	 * We wait for the last triggered sector
	 */
	if (l_sect < 0)
		goto DONE;

	start = get_timer (0);
	last  = start;
	addr = (vu_long*)(info->start[l_sect]);
	while ((addr[0] & 0x00800080) != 0x00800080) {
		if ((now = get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
			printf ("Timeout\n");
			return 1;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) {	/* every second */
			putc ('.');
			last = now;
		}
	}

DONE:
	/* reset to read mode */
	addr = (volatile unsigned long *)info->start[0];
	addr[0] = 0x00F000F0;	/* reset bank */

	printf (" done\n");
	return 0;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int i, l, rc;

	wp = (addr & ~3);	/* get lower word aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i=0, cp=wp; i<l; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}
		for (; i<4 && cnt>0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt==0 && i<4; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}

		if ((rc = write_word(info, wp, data)) != 0) {
			return (rc);
		}
		wp += 4;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= 4) {
		data = 0;
		for (i=0; i<4; ++i) {
			data = (data << 8) | *src++;
		}
		if ((rc = write_word(info, wp, data)) != 0) {
			return (rc);
		}
		wp  += 4;
		cnt -= 4;
	}

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i=0, cp=wp; i<4 && cnt>0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i<4; ++i, ++cp) {
		data = (data << 8) | (*(uchar *)cp);
	}

	return (write_word(info, wp, data));
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word (flash_info_t *info, ulong dest, ulong data)
{
	vu_long *addr = (vu_long*)(info->start[0]);
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*((vu_long *)dest) & data) != data) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;
	addr[0x0555] = 0x00A000A0;

	*((vu_long *)dest) = data;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer (0);
	while ((*((vu_long *)dest) & 0x00800080) != (data & 0x00800080)) {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			return (1);
		}
	}
	return (0);
}

/*-----------------------------------------------------------------------
 */
