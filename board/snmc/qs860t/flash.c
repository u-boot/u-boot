/*
 * (C) Copyright 2003
 * MuLogic B.V.
 *
 * (C) Copyright 2001
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

#include <common.h>
#include <asm/ppc4xx.h>
#include <asm/u-boot.h>
#include <asm/processor.h>

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips */


#ifdef CONFIG_SYS_FLASH_16BIT
#define FLASH_WORD_SIZE	unsigned short
#define FLASH_ID_MASK	0xFFFF
#else
#define FLASH_WORD_SIZE unsigned long
#define FLASH_ID_MASK	0xFFFFFFFF
#endif

/*-----------------------------------------------------------------------
 * Functions
 */
/* stolen from esteem192e/flash.c */
ulong flash_get_size (volatile FLASH_WORD_SIZE *addr, flash_info_t *info);

#ifndef CONFIG_SYS_FLASH_16BIT
static int write_word (flash_info_t *info, ulong dest, ulong data);
#else
static int write_short (flash_info_t *info, ulong dest, ushort data);
#endif
static void flash_get_offsets (ulong base, flash_info_t *info);


/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	unsigned long size_b0, size_b1;
	int i;
	uint pbcr;
	unsigned long base_b0, base_b1;

	/* Init: no FLASHes known */
	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Static FLASH Bank configuration here - FIXME XXX */

	size_b0 = flash_get_size((volatile FLASH_WORD_SIZE *)FLASH_BASE1_PRELIM, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
			size_b0, size_b0<<20);
	}

	/* Only one bank */
	if (CONFIG_SYS_MAX_FLASH_BANKS == 1) {
		/* Setup offsets */
		flash_get_offsets (FLASH_BASE1_PRELIM, &flash_info[0]);

		/* Monitor protection ON by default */
#if 0	/* sand: */
		(void)flash_protect(FLAG_PROTECT_SET,
			FLASH_BASE1_PRELIM-CONFIG_SYS_MONITOR_LEN+size_b0,
			FLASH_BASE1_PRELIM-1+size_b0,
			&flash_info[0]);
#else
		(void)flash_protect(FLAG_PROTECT_SET,
			CONFIG_SYS_MONITOR_BASE,
			CONFIG_SYS_MONITOR_BASE+CONFIG_SYS_MONITOR_LEN-1,
			&flash_info[0]);
#endif
		size_b1 = 0 ;
		flash_info[0].size = size_b0;
	} else {	/* 2 banks */
		size_b1 = flash_get_size((volatile FLASH_WORD_SIZE *)FLASH_BASE1_PRELIM, &flash_info[1]);

		/* Re-do sizing to get full correct info */
		if (size_b1) {
			mtdcr(EBC0_CFGADDR, PB0CR);
			pbcr = mfdcr(EBC0_CFGDATA);
			mtdcr(EBC0_CFGADDR, PB0CR);
			base_b1 = -size_b1;
			pbcr = (pbcr & 0x0001ffff) | base_b1 | (((size_b1/1024/1024)-1)<<17);
			mtdcr(EBC0_CFGDATA, pbcr);
		}

		if (size_b0) {
			mtdcr(EBC0_CFGADDR, PB1CR);
			pbcr = mfdcr(EBC0_CFGDATA);
			mtdcr(EBC0_CFGADDR, PB1CR);
			base_b0 = base_b1 - size_b0;
			pbcr = (pbcr & 0x0001ffff) | base_b0 | (((size_b0/1024/1024)-1)<<17);
			mtdcr(EBC0_CFGDATA, pbcr);
		}

		size_b0 = flash_get_size((volatile FLASH_WORD_SIZE *)base_b0, &flash_info[0]);

		flash_get_offsets (base_b0, &flash_info[0]);

		/* monitor protection ON by default */
#if 0	/* sand: */
		(void)flash_protect(FLAG_PROTECT_SET,
			FLASH_BASE1_PRELIM-CONFIG_SYS_MONITOR_LEN+size_b0,
			FLASH_BASE1_PRELIM-1+size_b0,
			&flash_info[0]);
#else
		(void)flash_protect(FLAG_PROTECT_SET,
			CONFIG_SYS_MONITOR_BASE,
			CONFIG_SYS_MONITOR_BASE+CONFIG_SYS_MONITOR_LEN-1,
			&flash_info[0]);
#endif

		if (size_b1) {
			/* Re-do sizing to get full correct info */
			size_b1 = flash_get_size((volatile FLASH_WORD_SIZE *)base_b1, &flash_info[1]);

			flash_get_offsets (base_b1, &flash_info[1]);

			/* monitor protection ON by default */
			(void)flash_protect(FLAG_PROTECT_SET,
				base_b1+size_b1-CONFIG_SYS_MONITOR_LEN,
				base_b1+size_b1-1,
				&flash_info[1]);
			/* monitor protection OFF by default (one is enough) */
			(void)flash_protect(FLAG_PROTECT_CLEAR,
				base_b0+size_b0-CONFIG_SYS_MONITOR_LEN,
				base_b0+size_b0-1,
				&flash_info[0]);
		} else {
			flash_info[1].flash_id = FLASH_UNKNOWN;
			flash_info[1].sector_count = -1;
		}

		flash_info[0].size = size_b0;
		flash_info[1].size = size_b1;
	}/* else 2 banks */
	return (size_b0 + size_b1);
}


/*-----------------------------------------------------------------------
 */

static void flash_get_offsets (ulong base, flash_info_t *info)
{
	int i;

	/* set up sector start adress table */
	if ((info->flash_id & FLASH_TYPEMASK) == INTEL_ID_28F320J3A ||
		(info->flash_id & FLASH_TYPEMASK) == INTEL_ID_28F640J3A ||
		(info->flash_id & FLASH_TYPEMASK) == INTEL_ID_28F128J3A) {
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base + (i * info->size/info->sector_count);
		}
	}
	else if (info->flash_id & FLASH_BTYPE) {
		if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL) {

#ifndef CONFIG_SYS_FLASH_16BIT
			/* set sector offsets for bottom boot block type */
			info->start[0] = base + 0x00000000;
			info->start[1] = base + 0x00004000;
			info->start[2] = base + 0x00008000;
			info->start[3] = base + 0x0000C000;
			info->start[4] = base + 0x00010000;
			info->start[5] = base + 0x00014000;
			info->start[6] = base + 0x00018000;
			info->start[7] = base + 0x0001C000;
			for (i = 8; i < info->sector_count; i++) {
				info->start[i] = base + (i * 0x00020000) - 0x000E0000;
			}
		} else {
			/* set sector offsets for bottom boot block type */
			info->start[0] = base + 0x00000000;
			info->start[1] = base + 0x00008000;
			info->start[2] = base + 0x0000C000;
			info->start[3] = base + 0x00010000;
			for (i = 4; i < info->sector_count; i++) {
				info->start[i] = base + (i * 0x00020000) - 0x00060000;
			}
		}
#else
			/* set sector offsets for bottom boot block type */
			info->start[0] = base + 0x00000000;
			info->start[1] = base + 0x00002000;
			info->start[2] = base + 0x00004000;
			info->start[3] = base + 0x00006000;
			info->start[4] = base + 0x00008000;
			info->start[5] = base + 0x0000A000;
			info->start[6] = base + 0x0000C000;
			info->start[7] = base + 0x0000E000;
			for (i = 8; i < info->sector_count; i++) {
				info->start[i] = base + (i * 0x00010000) - 0x00070000;
			}
		} else {
			/* set sector offsets for bottom boot block type */
			info->start[0] = base + 0x00000000;
			info->start[1] = base + 0x00004000;
			info->start[2] = base + 0x00006000;
			info->start[3] = base + 0x00008000;
			for (i = 4; i < info->sector_count; i++) {
				info->start[i] = base + (i * 0x00010000) - 0x00030000;
			}
		}
#endif
	} else {
		/* set sector offsets for top boot block type */
		i = info->sector_count - 1;
		if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL) {

#ifndef CONFIG_SYS_FLASH_16BIT
			info->start[i--] = base + info->size - 0x00004000;
			info->start[i--] = base + info->size - 0x00008000;
			info->start[i--] = base + info->size - 0x0000C000;
			info->start[i--] = base + info->size - 0x00010000;
			info->start[i--] = base + info->size - 0x00014000;
			info->start[i--] = base + info->size - 0x00018000;
			info->start[i--] = base + info->size - 0x0001C000;
			for (; i >= 0; i--) {
				info->start[i] = base + i * 0x00020000;
			}
		} else {

			info->start[i--] = base + info->size - 0x00008000;
			info->start[i--] = base + info->size - 0x0000C000;
			info->start[i--] = base + info->size - 0x00010000;
			for (; i >= 0; i--) {
				info->start[i] = base + i * 0x00020000;
			}
		}
#else
			info->start[i--] = base + info->size - 0x00002000;
			info->start[i--] = base + info->size - 0x00004000;
			info->start[i--] = base + info->size - 0x00006000;
			info->start[i--] = base + info->size - 0x00008000;
			info->start[i--] = base + info->size - 0x0000A000;
			info->start[i--] = base + info->size - 0x0000C000;
			info->start[i--] = base + info->size - 0x0000E000;
			for (; i >= 0; i--) {
				info->start[i] = base + i * 0x00010000;
			}
		} else {
			info->start[i--] = base + info->size - 0x00004000;
			info->start[i--] = base + info->size - 0x00006000;
			info->start[i--] = base + info->size - 0x00008000;
			for (; i >= 0; i--) {
				info->start[i] = base + i * 0x00010000;
			}
		}
#endif
	}
}

/*-----------------------------------------------------------------------
 */

void flash_print_info  (flash_info_t *info)
{
	int i;
	uchar *boottype;
	uchar botboot[]=", bottom boot sect)\n";
	uchar topboot[]=", top boot sector)\n";

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:	printf ("AMD ");		break;
	case FLASH_MAN_FUJ:	printf ("FUJITSU ");		break;
	case FLASH_MAN_SST:	printf ("SST ");		break;
	case FLASH_MAN_STM:	printf ("STM ");		break;
	case FLASH_MAN_INTEL:	printf ("INTEL ");		break;
	default:		printf ("Unknown Vendor ");	break;
	}

	if (info->flash_id & 0x0001 ) {
		boottype = botboot;
	} else {
		boottype = topboot;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM400B:	printf ("AM29LV400B (4 Mbit%s",boottype);
				break;
	case FLASH_AM400T:	printf ("AM29LV400T (4 Mbit%s",boottype);
				break;
	case FLASH_AM800B:	printf ("AM29LV800B (8 Mbit%s",boottype);
				break;
	case FLASH_AM800T:	printf ("AM29LV800T (8 Mbit%s",boottype);
				break;
	case FLASH_AM160B:	printf ("AM29LV160B (16 Mbit%s",boottype);
				break;
	case FLASH_AM160T:	printf ("AM29LV160T (16 Mbit%s",boottype);
				break;
	case FLASH_AM320B:	printf ("AM29LV320B (32 Mbit%s",boottype);
				break;
	case FLASH_AM320T:	printf ("AM29LV320T (32 Mbit%s",boottype);
				break;
	case FLASH_INTEL800B:	printf ("INTEL28F800B (8 Mbit%s",boottype);
				break;
	case FLASH_INTEL800T:	printf ("INTEL28F800T (8 Mbit%s",boottype);
				break;
	case FLASH_INTEL160B:	printf ("INTEL28F160B (16 Mbit%s",boottype);
				break;
	case FLASH_INTEL160T:	printf ("INTEL28F160T (16 Mbit%s",boottype);
				break;
	case FLASH_INTEL320B:	printf ("INTEL28F320B (32 Mbit%s",boottype);
				break;
	case FLASH_INTEL320T:	printf ("INTEL28F320T (32 Mbit%s",boottype);
				break;
	case FLASH_AMDL322T:	printf ("AM29DL322T (32 Mbit%s",boottype);
				break;

#if 0 /* enable when devices are available */

	case FLASH_INTEL640B:	printf ("INTEL28F640B (64 Mbit%s",boottype);
				break;
	case FLASH_INTEL640T:	printf ("INTEL28F640T (64 Mbit%s",boottype);
				break;
#endif
	case INTEL_ID_28F320J3A:	printf ("INTEL28F320JA3 (32 Mbit%s",boottype);
				break;
	case INTEL_ID_28F640J3A:	printf ("INTEL28F640JA3 (64 Mbit%s",boottype);
				break;
	case INTEL_ID_28F128J3A:	printf ("INTEL28F128JA3 (128 Mbit%s",boottype);
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
ulong flash_get_size (volatile FLASH_WORD_SIZE *addr, flash_info_t *info)
{
	short i;
	ulong base = (ulong)addr;
	FLASH_WORD_SIZE value;

	/* Write auto select command: read Manufacturer ID */


#ifndef CONFIG_SYS_FLASH_16BIT

	/*
	 * Note: if it is an AMD flash and the word at addr[0000]
	 * is 0x00890089 this routine will think it is an Intel
	 * flash device and may(most likely) cause trouble.
	 */

	addr[0x0000] = 0x00900090;
	if(addr[0x0000] != 0x00890089){
		addr[0x0555] = 0x00AA00AA;
		addr[0x02AA] = 0x00550055;
		addr[0x0555] = 0x00900090;
#else

	/*
	 * Note: if it is an AMD flash and the word at addr[0000]
	 * is 0x0089 this routine will think it is an Intel
	 * flash device and may(most likely) cause trouble.
	 */

	addr[0x0000] = 0x0090;

	if(addr[0x0000] != 0x0089){
		addr[0x0555] = 0x00AA;
		addr[0x02AA] = 0x0055;
		addr[0x0555] = 0x0090;
#endif
	}
	value = addr[0];

	switch (value) {
	case (AMD_MANUFACT & FLASH_ID_MASK):
		info->flash_id = FLASH_MAN_AMD;
		break;
	case (FUJ_MANUFACT & FLASH_ID_MASK):
		info->flash_id = FLASH_MAN_FUJ;
		break;
	case (STM_MANUFACT & FLASH_ID_MASK):
		info->flash_id = FLASH_MAN_STM;
		break;
	case (SST_MANUFACT & FLASH_ID_MASK):
		info->flash_id = FLASH_MAN_SST;
		break;
	case (INTEL_MANUFACT & FLASH_ID_MASK):
		info->flash_id = FLASH_MAN_INTEL;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0); /* no or unknown flash */

	}

	value = addr[1];			/* device ID		*/

	switch (value) {

	case (AMD_ID_LV400T & FLASH_ID_MASK):
		info->flash_id += FLASH_AM400T;
		info->sector_count = 11;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/

	case (AMD_ID_LV400B & FLASH_ID_MASK):
		info->flash_id += FLASH_AM400B;
		info->sector_count = 11;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/

	case (AMD_ID_LV800T & FLASH_ID_MASK):
		info->flash_id += FLASH_AM800T;
		info->sector_count = 19;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/

	case (AMD_ID_LV800B & FLASH_ID_MASK):
		info->flash_id += FLASH_AM800B;
		info->sector_count = 19;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/

	case (AMD_ID_LV160T & FLASH_ID_MASK):
		info->flash_id += FLASH_AM160T;
		info->sector_count = 35;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/

	case (AMD_ID_LV160B & FLASH_ID_MASK):
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/
#if 0	/* enable when device IDs are available */
	case (AMD_ID_LV320T & FLASH_ID_MASK):
		info->flash_id += FLASH_AM320T;
		info->sector_count = 67;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/

	case (AMD_ID_LV320B & FLASH_ID_MASK):
		info->flash_id += FLASH_AM320B;
		info->sector_count = 67;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/
#endif

	case (AMD_ID_DL322T & FLASH_ID_MASK):
		info->flash_id += FLASH_AMDL322T;
		info->sector_count = 71;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/

	case (INTEL_ID_28F800B3T & FLASH_ID_MASK):
		info->flash_id += FLASH_INTEL800T;
		info->sector_count = 23;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/

	case (INTEL_ID_28F800B3B & FLASH_ID_MASK):
		info->flash_id += FLASH_INTEL800B;
		info->sector_count = 23;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/

	case (INTEL_ID_28F160B3T & FLASH_ID_MASK):
		info->flash_id += FLASH_INTEL160T;
		info->sector_count = 39;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/

	case (INTEL_ID_28F160B3B & FLASH_ID_MASK):
		info->flash_id += FLASH_INTEL160B;
		info->sector_count = 39;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/

	case (INTEL_ID_28F320B3T & FLASH_ID_MASK):
		info->flash_id += FLASH_INTEL320T;
		info->sector_count = 71;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/

	case (INTEL_ID_28F320B3B & FLASH_ID_MASK):
		info->flash_id += FLASH_AM320B;
		info->sector_count = 71;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/

#if 0 /* enable when devices are available */
	case (INTEL_ID_28F320B3T & FLASH_ID_MASK):
		info->flash_id += FLASH_INTEL320T;
		info->sector_count = 135;
		info->size = 0x01000000;
		break;				/* => 16 MB		*/

	case (INTEL_ID_28F320B3B & FLASH_ID_MASK):
		info->flash_id += FLASH_AM320B;
		info->sector_count = 135;
		info->size = 0x01000000;
		break;				/* => 16 MB		*/
#endif
	case (INTEL_ID_28F320J3A & FLASH_ID_MASK):
		info->flash_id += FLASH_28F320J3A;
		info->sector_count = 32;
		info->size = 0x00400000;
		break;				/* => 32 MBit	*/
	case (INTEL_ID_28F640J3A & FLASH_ID_MASK):
		info->flash_id += FLASH_28F640J3A;
		info->sector_count = 64;
		info->size = 0x00800000;
		break;				/* => 64 MBit	*/
	case (INTEL_ID_28F128J3A & FLASH_ID_MASK):
		info->flash_id += FLASH_28F128J3A;
		info->sector_count = 128;
		info->size = 0x01000000;
		break;				/* => 128 MBit	*/

	default:
		/* FIXME*/
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */
	}

	flash_get_offsets(base, info);

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr = (volatile FLASH_WORD_SIZE *)(info->start[i]);
		info->protect[i] = addr[2] & 1;
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr = (volatile FLASH_WORD_SIZE *)info->start[0];
		if( (info->flash_id & 0xFF00) == FLASH_MAN_INTEL){
			*addr = (0x00F000F0 & FLASH_ID_MASK);	/* reset bank */
		} else {
			*addr = (0x00FF00FF & FLASH_ID_MASK);	/* reset bank */
		}
	}

	return (info->size);
}


/*-----------------------------------------------------------------------
 */

int flash_erase (flash_info_t * info, int s_first, int s_last)
{

	volatile FLASH_WORD_SIZE *addr =
		(volatile FLASH_WORD_SIZE *) (info->start[0]);
	int flag, prot, sect, l_sect, barf;
	ulong start, now, last;
	int rcode = 0;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	if ((info->flash_id == FLASH_UNKNOWN) ||
	    ((info->flash_id > FLASH_AMD_COMP) &&
	     ((info->flash_id & FLASH_VENDMASK) != FLASH_MAN_INTEL))) {
		printf ("Can't erase unknown flash type - aborted\n");
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf ("- Warning: %d protected sectors will not be erased!\n", prot);
	} else {
		printf ("\n");
	}

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();
	if (info->flash_id < FLASH_AMD_COMP) {
#ifndef CONFIG_SYS_FLASH_16BIT
		addr[0x0555] = 0x00AA00AA;
		addr[0x02AA] = 0x00550055;
		addr[0x0555] = 0x00800080;
		addr[0x0555] = 0x00AA00AA;
		addr[0x02AA] = 0x00550055;
#else
		addr[0x0555] = 0x00AA;
		addr[0x02AA] = 0x0055;
		addr[0x0555] = 0x0080;
		addr[0x0555] = 0x00AA;
		addr[0x02AA] = 0x0055;
#endif
		/* Start erase on unprotected sectors */
		for (sect = s_first; sect <= s_last; sect++) {
			if (info->protect[sect] == 0) {	/* not protected */
				addr = (volatile FLASH_WORD_SIZE *) (info->start[sect]);
				addr[0] = (0x00300030 & FLASH_ID_MASK);
				l_sect = sect;
			}
		}

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts ();

		/* wait at least 80us - let's wait 1 ms */
		udelay (1000);

		/*
		 * We wait for the last triggered sector
		 */
		if (l_sect < 0)
			goto DONE;

		start = get_timer (0);
		last = start;
		addr = (volatile FLASH_WORD_SIZE *) (info->start[l_sect]);
		while ((addr[0] & (0x00800080 & FLASH_ID_MASK)) !=
		       (0x00800080 & FLASH_ID_MASK)) {
			if ((now = get_timer (start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
				printf ("Timeout\n");
				return 1;
			}
			/* show that we're waiting */
			if ((now - last) > 1000) {	/* every second */
				serial_putc ('.');
				last = now;
			}
		}

	      DONE:
		/* reset to read mode */
		addr = (volatile FLASH_WORD_SIZE *) info->start[0];
		addr[0] = (0x00F000F0 & FLASH_ID_MASK);	/* reset bank */
	} else {


		for (sect = s_first; sect <= s_last; sect++) {
			if (info->protect[sect] == 0) {	/* not protected */
				barf = 0;
#ifndef CONFIG_SYS_FLASH_16BIT
				addr = (vu_long *) (info->start[sect]);
				addr[0] = 0x00200020;
				addr[0] = 0x00D000D0;
				while (!(addr[0] & 0x00800080));	/* wait for error or finish */
				if (addr[0] & 0x003A003A) {	/* check for error */
					barf = addr[0] & 0x003A0000;
					if (barf) {
						barf >>= 16;
					} else {
						barf = addr[0] & 0x0000003A;
					}
				}
#else
				addr = (vu_short *) (info->start[sect]);
				addr[0] = 0x0020;
				addr[0] = 0x00D0;
				while (!(addr[0] & 0x0080));	/* wait for error or finish */
				if (addr[0] & 0x003A)	/* check for error */
					barf = addr[0] & 0x003A;
#endif
				if (barf) {
					printf ("\nFlash error in sector at %lx\n",
						(unsigned long) addr);
					if (barf & 0x0002)
						printf ("Block locked, not erased.\n");
					if ((barf & 0x0030) == 0x0030)
						printf ("Command Sequence error.\n");
					if ((barf & 0x0030) == 0x0020)
						printf ("Block Erase error.\n");
					if (barf & 0x0008)
						printf ("Vpp Low error.\n");
					rcode = 1;
				} else
					printf (".");
				l_sect = sect;
			}
			addr = (volatile FLASH_WORD_SIZE *) info->start[0];
			addr[0] = (0x00FF00FF & FLASH_ID_MASK);	/* reset bank */

		}

	}
	printf (" done\n");
	return rcode;
}

/*-----------------------------------------------------------------------
 */

/*flash_info_t *addr2info (ulong addr)
{
	flash_info_t *info;
	int i;

	for (i=0, info=&flash_info[0]; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i, ++info) {
		if ((addr >= info->start[0]) &&
		    (addr < (info->start[0] + info->size)) ) {
			return (info);
		}
	}

	return (NULL);
}
*/
/*-----------------------------------------------------------------------
 * Copy memory to flash.
 * Make sure all target addresses are within Flash bounds,
 * and no protected sectors are hit.
 * Returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 4 - target range includes protected sectors
 * 8 - target address not in Flash memory
 */

/*int flash_write (uchar *src, ulong addr, ulong cnt)
{
	int i;
	ulong         end        = addr + cnt - 1;
	flash_info_t *info_first = addr2info (addr);
	flash_info_t *info_last  = addr2info (end );
	flash_info_t *info;

	if (cnt == 0) {
		return (0);
	}

	if (!info_first || !info_last) {
		return (8);
	}

	for (info = info_first; info <= info_last; ++info) {
		ulong b_end = info->start[0] + info->size;*/	/* bank end addr */
/*		short s_end = info->sector_count - 1;
		for (i=0; i<info->sector_count; ++i) {
			ulong e_addr = (i == s_end) ? b_end : info->start[i + 1];

			if ((end >= info->start[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				return (4);
			}
		}
	}

*/	/* finally write data to flash */
/*	for (info = info_first; info <= info_last && cnt>0; ++info) {
		ulong len;

		len = info->start[0] + info->size - addr;
		if (len > cnt)
			len = cnt;
		if ((i = write_buff(info, src, addr, len)) != 0) {
			return (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}
	return (0);
}
*/
/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
#ifndef CONFIG_SYS_FLASH_16BIT
	ulong cp, wp, data;
	int l;
#else
	ulong cp, wp;
	ushort data;
#endif
	int i, rc;

#ifndef CONFIG_SYS_FLASH_16BIT


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

#else
	wp = (addr & ~1);	/* get lower word aligned address */

	/*
	 * handle unaligned start byte
	 */
	if (addr - wp) {
		data = 0;
		data = (data << 8) | *src++;
		--cnt;
		if ((rc = write_short(info, wp, data)) != 0) {
			return (rc);
		}
		wp += 2;
	}

	/*
	 * handle word aligned part
	 */
/*	l = 0; used for debuging  */
	while (cnt >= 2) {
		data = 0;
		for (i=0; i<2; ++i) {
			data = (data << 8) | *src++;
		}

/*		if(!l){
			printf("%x",data);
			l = 1;
		}  used for debuging */

		if ((rc = write_short(info, wp, data)) != 0) {
			return (rc);
		}
		wp  += 2;
		cnt -= 2;
	}

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i=0, cp=wp; i<2 && cnt>0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i<2; ++i, ++cp) {
		data = (data << 8) | (*(uchar *)cp);
	}

	return (write_short(info, wp, data));


#endif
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
#ifndef CONFIG_SYS_FLASH_16BIT
static int write_word (flash_info_t *info, ulong dest, ulong data)
{
	vu_long *addr = (vu_long*)(info->start[0]);
	ulong start,barf;
	int flag;


	/* Check if Flash is (sufficiently) erased */
	if ((*((vu_long *)dest) & data) != data) {
		return (2);
	}

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	if(info->flash_id > FLASH_AMD_COMP) {
		/* AMD stuff */
		addr[0x0555] = 0x00AA00AA;
		addr[0x02AA] = 0x00550055;
		addr[0x0555] = 0x00A000A0;
	} else {
		/* intel stuff */
		*addr = 0x00400040;
	}
	*((vu_long *)dest) = data;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer (0);

	if(info->flash_id > FLASH_AMD_COMP) {
		while ((*((vu_long *)dest) & 0x00800080) != (data & 0x00800080)) {
			if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
				return (1);
			}
		}
	} else {
		while(!(addr[0] & 0x00800080)) {	/* wait for error or finish */
			if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
				return (1);
			}

		if( addr[0] & 0x003A003A) {	/* check for error */
			barf = addr[0] & 0x003A0000;
			if( barf ) {
				barf >>=16;
			} else {
				barf = addr[0] & 0x0000003A;
			}
			printf("\nFlash write error at address %lx\n",(unsigned long)dest);
			if(barf & 0x0002) printf("Block locked, not erased.\n");
			if(barf & 0x0010) printf("Programming error.\n");
			if(barf & 0x0008) printf("Vpp Low error.\n");
			return(2);
		}
	}

	return (0);
}

#else

static int write_short (flash_info_t *info, ulong dest, ushort data)
{
	vu_short *addr = (vu_short*)(info->start[0]);
	ulong start,barf;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*((vu_short *)dest) & data) != data) {
		return (2);
	}

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	if(info->flash_id < FLASH_AMD_COMP) {
		/* AMD stuff */
		addr[0x0555] = 0x00AA;
		addr[0x02AA] = 0x0055;
		addr[0x0555] = 0x00A0;
	} else {
		/* intel stuff */
		*addr = 0x00D0;
		*addr = 0x0040;
	}
	*((vu_short *)dest) = data;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer (0);

	if(info->flash_id < FLASH_AMD_COMP) {
		/* AMD stuff */
		while ((*((vu_short *)dest) & 0x0080) != (data & 0x0080)) {
			if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
				return (1);
			}
		}

	} else {
		/* intel stuff */
		while(!(addr[0] & 0x0080)){	/* wait for error or finish */
			if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) return (1);
		}

		if( addr[0] & 0x003A) {	/* check for error */
			barf = addr[0] & 0x003A;
			printf("\nFlash write error at address %lx\n",(unsigned long)dest);
			if(barf & 0x0002) printf("Block locked, not erased.\n");
			if(barf & 0x0010) printf("Programming error.\n");
			if(barf & 0x0008) printf("Vpp Low error.\n");
			return(2);
		}
		*addr = 0x00B0;
		*addr = 0x0070;
		while(!(addr[0] & 0x0080)){	/* wait for error or finish */
			if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) return (1);
		}
		*addr = 0x00FF;
	}
	return (0);
}

#endif

/*-----------------------------------------------------------------------*/
