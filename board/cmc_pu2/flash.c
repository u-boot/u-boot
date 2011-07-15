/*
 * (C) Copyright 2003-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Martin Krause, TQ-Systems GmbH, martin.krause@tqs.de
 *
 * Modified for the CMC PU2 by (C) Copyright 2004 Gary Jennejohn
 * garyj@denx.de
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

#ifndef	CONFIG_ENV_ADDR
#define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#endif

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips */

#define FLASH_CYCLE1	0x0555
#define FLASH_CYCLE2	0x02AA

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size(vu_short *addr, flash_info_t *info);
static void flash_reset(flash_info_t *info);
static int write_word_amd(flash_info_t *info, vu_short *dest, ushort data);
static flash_info_t *flash_get_info(ulong base);

/*-----------------------------------------------------------------------
 * flash_init()
 *
 * sets up flash_info and returns size of FLASH (bytes)
 */
unsigned long flash_init (void)
{
	unsigned long size = 0;
	ulong flashbase = CONFIG_SYS_FLASH_BASE;

	/* Init: no FLASHes known */
	memset(&flash_info[0], 0, sizeof(flash_info_t));

	flash_info[0].size = flash_get_size((vu_short *)flashbase, &flash_info[0]);

	size = flash_info[0].size;

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE+monitor_flash_len-1,
		      flash_get_info(CONFIG_SYS_MONITOR_BASE));
#endif

#ifdef	CONFIG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_ENV_ADDR,
		      CONFIG_ENV_ADDR+CONFIG_ENV_SIZE-1,
		      flash_get_info(CONFIG_ENV_ADDR));
#endif

	return size ? size : 1;
}

/*-----------------------------------------------------------------------
 */
static void flash_reset(flash_info_t *info)
{
	vu_short *base = (vu_short *)(info->start[0]);

	/* Put FLASH back in read mode */
	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL)
		*base = 0x00FF;	/* Intel Read Mode */
	else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_AMD)
		*base = 0x00F0;	/* AMD Read Mode */
}

/*-----------------------------------------------------------------------
 */

static flash_info_t *flash_get_info(ulong base)
{
	int i;
	flash_info_t * info;

	info = NULL;
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i ++) {
		info = & flash_info[i];
		if (info->size && info->start[0] <= base &&
		    base <= info->start[0] + info->size - 1)
			break;
	}

	return i == CONFIG_SYS_MAX_FLASH_BANKS ? 0 : info;
}

/*-----------------------------------------------------------------------
 */

void flash_print_info (flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:	printf ("AMD ");		break;
	case FLASH_MAN_BM:	printf ("BRIGHT MICRO ");	break;
	case FLASH_MAN_FUJ:	printf ("FUJITSU ");		break;
	case FLASH_MAN_SST:	printf ("SST ");		break;
	case FLASH_MAN_STM:	printf ("STM ");		break;
	case FLASH_MAN_INTEL:	printf ("INTEL ");		break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_S29GL064M:
		printf ("S29GL064M-R6 (64Mbit, uniform sector size)\n");
		break;
	default:
		printf ("Unknown Chip Type\n");
		break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20,
		info->sector_count);

	printf ("  Sector Start Addresses:");

	for (i=0; i<info->sector_count; ++i) {
		if ((i % 5) == 0) {
			printf ("\n   ");
		}
		printf (" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");
	return;
}

/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */

ulong flash_get_size (vu_short *addr, flash_info_t *info)
{
	int i;
	ushort value;
	ulong base = (ulong)addr;

	/* Write auto select command sequence */
	addr[FLASH_CYCLE1] = 0x00AA;	/* for AMD, Intel ignores this */
	addr[FLASH_CYCLE2] = 0x0055;	/* for AMD, Intel ignores this */
	addr[FLASH_CYCLE1] = 0x0090;	/* selects Intel or AMD */

	/* read Manufacturer ID */
	udelay(100);
	value = addr[0];
	debug ("Manufacturer ID: %04X\n", value);

	switch (value) {

	case (AMD_MANUFACT & 0xFFFF):
		debug ("Manufacturer: AMD (Spansion)\n");
		info->flash_id = FLASH_MAN_AMD;
		break;

	case (INTEL_MANUFACT & 0xFFFF):
		debug ("Manufacturer: Intel (not supported yet)\n");
		info->flash_id = FLASH_MAN_INTEL;
		break;

	default:
		printf ("Unknown Manufacturer ID: %04X\n", value);
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		goto out;
	}

	value = addr[1];
	debug ("Device ID: %04X\n", value);

	switch (addr[1]) {

	case (AMD_ID_MIRROR & 0xFFFF):
		debug ("Mirror Bit flash: addr[14] = %08X  addr[15] = %08X\n",
			addr[14], addr[15]);

		switch(addr[14]) {
		case (AMD_ID_GL064M_2 & 0xFFFF):
			if (addr[15] != (AMD_ID_GL064M_3 & 0xffff)) {
				printf ("Chip: S29GLxxxM -> unknown\n");
				info->flash_id = FLASH_UNKNOWN;
				info->sector_count = 0;
				info->size = 0;
			} else {
				debug ("Chip: S29GL064M-R6\n");
				info->flash_id += FLASH_S29GL064M;
				info->sector_count = 128;
				info->size = 0x00800000;
				for (i = 0; i < info->sector_count; i++) {
					info->start[i] = base;
					base += 0x10000;
				}
			}
			break;	/* => 16 MB	*/
		default:
			printf ("Chip: *** unknown ***\n");
			info->flash_id = FLASH_UNKNOWN;
			info->sector_count = 0;
			info->size = 0;
			break;
		}
		break;

	default:
		printf ("Unknown Device ID: %04X\n", value);
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		break;
	}

out:
	/* Put FLASH back in read mode */
	flash_reset(info);

	return (info->size);
}

/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	vu_short *addr = (vu_short *)(info->start[0]);
	int flag, prot, sect, ssect, l_sect;
	ulong now, last, start;

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

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	/*
	 * Start erase on unprotected sectors.
	 * Since the flash can erase multiple sectors with one command
	 * we take advantage of that by doing the erase in chunks of
	 * 3 sectors.
	 */
	for (sect = s_first; sect <= s_last; ) {
		l_sect = -1;

		addr[FLASH_CYCLE1] = 0x00AA;
		addr[FLASH_CYCLE2] = 0x0055;
		addr[FLASH_CYCLE1] = 0x0080;
		addr[FLASH_CYCLE1] = 0x00AA;
		addr[FLASH_CYCLE2] = 0x0055;

		/* do the erase in chunks of at most 3 sectors */
		for (ssect = 0; ssect < 3; ssect++) {
			if ((sect + ssect) > s_last)
				break;
			if (info->protect[sect + ssect] == 0) {	/* not protected */
				addr = (vu_short *)(info->start[sect + ssect]);
				addr[0] = 0x0030;
				l_sect = sect + ssect;
			}
		}
		/* wait at least 80us - let's wait 1 ms */
		udelay (1000);

		/*
		 * We wait for the last triggered sector
		 */
		if (l_sect < 0)
			goto DONE;

		start = get_timer(0);
		last  = 0;
		addr = (vu_short *)(info->start[l_sect]);
		while ((addr[0] & 0x0080) != 0x0080) {
			if ((now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
				printf ("Timeout\n");
				return 1;
			}
			/* show that we're waiting */
			if ((now - last) > 1000) {	/* every second */
				putc ('.');
				last = now;
			}
		}
		addr = (vu_short *)info->start[0];
		addr[0] = 0x00F0;	/* reset bank */
		sect += ssect;
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

DONE:
	/* reset to read mode */
	addr = (vu_short *)info->start[0];
	addr[0] = 0x00F0;	/* reset bank */

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
	ulong wp, data;
	int rc;

	if (addr & 1) {
		printf ("unaligned destination not supported\n");
		return ERR_ALIGN;
	};

	if ((int) src & 1) {
		printf ("unaligned source not supported\n");
		return ERR_ALIGN;
	};

	wp = addr;

	while (cnt >= 2) {
		data = *((vu_short *)src);
		if ((rc = write_word_amd(info, (vu_short *)wp, data)) != 0) {
printf ("write_buff 1: write_word_amd() rc=%d\n", rc);
			return (rc);
		}
		src += 2;
		wp += 2;
		cnt -= 2;
	}

	if (cnt == 0) {
		return (ERR_OK);
	}

	if (cnt == 1) {
		data = (*((volatile u8 *) src)) | (*((volatile u8 *) (wp + 1)) << 8);
		if ((rc = write_word_amd(info, (vu_short *)wp, data)) != 0) {
printf ("write_buff 1: write_word_amd() rc=%d\n", rc);
			return (rc);
		}
		src += 1;
		wp += 1;
		cnt -= 1;
	}

	return ERR_OK;
}

/*-----------------------------------------------------------------------
 * Write a word to Flash for AMD FLASH
 * A word is 16 or 32 bits, whichever the bus width of the flash bank
 * (not an individual chip) is.
 *
 * returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word_amd (flash_info_t *info, vu_short *dest, ushort data)
{
	int flag;
	vu_short *base;		/* first address in flash bank	*/
	ulong start;

	/* Check if Flash is (sufficiently) erased */
	if ((*dest & data) != data) {
		return (2);
	}

	base = (vu_short *)(info->start[0]);

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	base[FLASH_CYCLE1] = 0x00AA;	/* unlock */
	base[FLASH_CYCLE2] = 0x0055;	/* unlock */
	base[FLASH_CYCLE1] = 0x00A0;	/* selects program mode */

	*dest = data;		/* start programming the data	*/

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	start = get_timer(0);

	/* data polling for D7 */
	while ((*dest & 0x0080) != (data & 0x0080)) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			*dest = 0x00F0;	/* reset bank */
			return (1);
		}
	}
	return (0);
}
