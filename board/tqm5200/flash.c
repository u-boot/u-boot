/*
 * (C) Copyright 2003-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Martin Krause, TQ-Systems GmbH, martin.krause@tqs.de
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

flash_info_t	flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips */

/*
 * CPU to flash interface is 32-bit, so make declaration accordingly
 */
typedef unsigned long FLASH_PORT_WIDTH;
typedef volatile unsigned long FLASH_PORT_WIDTHV;

#define FPW	FLASH_PORT_WIDTH
#define FPWV	FLASH_PORT_WIDTHV

#define FLASH_CYCLE1	0x0555
#define FLASH_CYCLE2	0x02aa

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size(FPWV *addr, flash_info_t *info);
static void flash_reset(flash_info_t *info);
static int write_word_amd(flash_info_t *info, FPWV *dest, FPW data);
static flash_info_t *flash_get_info(ulong base);

/*-----------------------------------------------------------------------
 * flash_init()
 *
 * sets up flash_info and returns size of FLASH (bytes)
 */
unsigned long flash_init (void)
{
	unsigned long size = 0;
	extern void flash_preinit(void);
	ulong flashbase = CFG_FLASH_BASE;

	flash_preinit();

	/* Init: no FLASHes known */
	memset(&flash_info[0], 0, sizeof(flash_info_t));

	flash_info[0].size =
		flash_get_size((FPW *)flashbase, &flash_info[0]);

	size = flash_info[0].size;

#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CFG_MONITOR_BASE,
		      CFG_MONITOR_BASE+monitor_flash_len-1,
		      flash_get_info(CFG_MONITOR_BASE));
#endif

#ifdef	CFG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CFG_ENV_ADDR,
		      CFG_ENV_ADDR+CFG_ENV_SIZE-1,
		      flash_get_info(CFG_ENV_ADDR));
#endif

	return size ? size : 1;
}

/*-----------------------------------------------------------------------
 */
static void flash_reset(flash_info_t *info)
{
	FPWV *base = (FPWV *)(info->start[0]);

	/* Put FLASH back in read mode */
	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL)
		*base = (FPW)0x00FF00FF;	/* Intel Read Mode */
	else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_AMD)
		*base = (FPW)0x00F000F0;	/* AMD Read Mode */
}

/*-----------------------------------------------------------------------
 */

static flash_info_t *flash_get_info(ulong base)
{
	int i;
	flash_info_t * info;

	for (i = 0; i < CFG_MAX_FLASH_BANKS; i ++) {
		info = & flash_info[i];
		if (info->size && info->start[0] <= base &&
		    base <= info->start[0] + info->size - 1)
			break;
	}

	return i == CFG_MAX_FLASH_BANKS ? 0 : info;
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
	case FLASH_AMLV128U:
		printf ("AM29LV128ML (128Mbit, uniform sector size)\n");
		break;
	case FLASH_AM160B:
		printf ("AM29LV160B (16 Mbit, bottom boot sect)\n");
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

ulong flash_get_size (FPWV *addr, flash_info_t *info)
{
	int i;
	ulong base = (ulong)addr;

	/* Write auto select command: read Manufacturer ID */
	/* Write auto select command sequence and test FLASH answer */
	addr[FLASH_CYCLE1] = (FPW)0x00AA00AA;	/* for AMD, Intel ignores this */
	addr[FLASH_CYCLE2] = (FPW)0x00550055;	/* for AMD, Intel ignores this */
	addr[FLASH_CYCLE1] = (FPW)0x00900090;	/* selects Intel or AMD */

	/* The manufacturer codes are only 1 byte, so just use 1 byte.
	 * This works for any bus width and any FLASH device width.
	 */
	udelay(100);
	switch (addr[0] & 0xff) {

	case (uchar)AMD_MANUFACT:
		debug ("Manufacturer: AMD (Spansion)\n");
		info->flash_id = FLASH_MAN_AMD;
		break;

	case (uchar)INTEL_MANUFACT:
		debug ("Manufacturer: Intel (not supported yet)\n");
		info->flash_id = FLASH_MAN_INTEL;
		break;

	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		break;
	}

	/* Check 16 bits or 32 bits of ID so work on 32 or 16 bit bus. */
	if (info->flash_id != FLASH_UNKNOWN) switch ((FPW)addr[1]) {

	case (FPW)AMD_ID_LV160B:
		debug ("Chip: AM29LV160MB\n");
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00400000;
		/*
		 * The first 4 sectors are 16 kB, 8 kB, 8 kB and 32 kB, all
		 * the other ones are 64 kB
		 */
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00008000;
		info->start[2] = base + 0x0000C000;
		info->start[3] = base + 0x00010000;
		for( i = 4; i < info->sector_count; i++ )
			info->start[i] =
				base + (i * 2 * (64 << 10)) - 0x00060000;
		break;		/* => 4 MB */

	case AMD_ID_MIRROR:
		debug ("Mirror Bit flash: addr[14] = %08lX  addr[15] = %08lX\n",
			addr[14], addr[15]);

		switch(addr[14]) {
		case AMD_ID_LV128U_2:
			if (addr[15] != AMD_ID_LV128U_3) {
				debug ("Chip: AM29LVxxxM -> unknown\n");
				info->flash_id = FLASH_UNKNOWN;
				info->sector_count = 0;
				info->size = 0;
			} else {
				debug ("Chip: AM29LV128M\n");
				info->flash_id += FLASH_AMLV128U;
				info->sector_count = 256;
				info->size = 0x02000000;
				for (i = 0; i < info->sector_count; i++) {
					info->start[i] = base;
					base += 0x20000;
				}
			}
			break;	/* => 32 MB	*/
		default:
			debug ("Chip: *** unknown ***\n");
			info->flash_id = FLASH_UNKNOWN;
			info->sector_count = 0;
			info->size = 0;
			break;
		}
		break;

	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
	}

	/* Put FLASH back in read mode */
	flash_reset(info);

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

	/*
	 * Get lower word aligned address. Assumes 32 bit flash bus width.
	 */
	wp = (addr & ~3);

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

		if ((rc = write_word_amd(info, (FPW *)wp, data)) != 0) {
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
		if ((rc = write_word_amd(info, (FPW *)wp, data)) != 0) {
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

	return (write_word_amd(info, (FPW *)wp, data));
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
static int write_word_amd (flash_info_t *info, FPWV *dest, FPW data)
{
	ulong start;
	int flag;
	FPWV *base;		/* first address in flash bank	*/

	/* Check if Flash is (sufficiently) erased */
	if ((*dest & data) != data) {
		return (2);
	}

	base = (FPWV *)(info->start[0]);

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	base[FLASH_CYCLE1] = (FPW)0x00AA00AA;	/* unlock */
	base[FLASH_CYCLE2] = (FPW)0x00550055;	/* unlock */
	base[FLASH_CYCLE1] = (FPW)0x00A000A0;	/* selects program mode */

	*dest = data;		/* start programming the data	*/

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	start = get_timer (0);

	/* data polling for D7 */
	while ((*dest & (FPW)0x00800080) != (data & (FPW)0x00800080)) {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			*dest = (FPW)0x00F000F0;	/* reset bank */
			return (1);
		}
	}
	return (0);
}
