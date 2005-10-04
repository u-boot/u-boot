/*
 * (C) Copyright 2002
 * Robert Schwebel, Pengutronix, <r.schwebel@pengutronix.de>
 *
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

#include <common.h>
#include <linux/byteorder/swab.h>

#define SWAP(x)               __swab32(x)

flash_info_t	flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips */

/* Functions */
static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);
static void flash_get_offsets (ulong base, flash_info_t *info);

/*-----------------------------------------------------------------------
 */
unsigned long flash_init (void)
{
	int i;
	ulong size = 0;

	for (i = 0; i < CFG_MAX_FLASH_BANKS; i++) {
		switch (i) {
		case 0:
			flash_get_size ((long *) PHYS_FLASH_1, &flash_info[i]);
			flash_get_offsets (PHYS_FLASH_1, &flash_info[i]);
			break;
		case 1:
			flash_get_size ((long *) PHYS_FLASH_2, &flash_info[i]);
			flash_get_offsets (PHYS_FLASH_2, &flash_info[i]);
			break;
		default:
			panic ("configured too many flash banks!\n");
			break;
		}
		size += flash_info[i].size;
	}

	/* Protect monitor and environment sectors */
	flash_protect ( FLAG_PROTECT_SET,CFG_FLASH_BASE,CFG_FLASH_BASE + monitor_flash_len - 1,&flash_info[0] );
	flash_protect ( FLAG_PROTECT_SET,CFG_ENV_ADDR,CFG_ENV_ADDR + CFG_ENV_SIZE - 1, &flash_info[0] );

	return size;
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets (ulong base, flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) return;

	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_AMD) {
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base + (i * PHYS_FLASH_SECT_SIZE);
			info->protect[i] = 0;
		}
	}
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
	case FLASH_AMLV640U:	printf ("AM29LV640ML (64Mbit, uniform sector size)\n");
				break;
	case FLASH_S29GL064M:	printf ("S29GL064M (64Mbit, top boot sector size)\n");
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

	case AMD_ID_MIRROR:
		debug ("Mirror Bit flash: addr[14] = %08lX  addr[15] = %08lX\n",
			addr[14], addr[15]);
		switch(addr[14]) {
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
		case AMD_ID_GL064MT_2:
			if (addr[15] != AMD_ID_GL064MT_3) {
				debug ("Chip: S29GL064M-R3 -> unknown\n");
				info->flash_id = FLASH_UNKNOWN;
			} else {
				debug ("Chip: S29GL064M-R3\n");
				info->flash_id += FLASH_S29GL064M;
				info->sector_count = 128;
				info->size = 0x01000000;
			}
			break;				/* => 16 MB	*/
		default:
			debug ("Chip: *** unknown ***\n");
			info->flash_id = FLASH_UNKNOWN;
			break;
		}
		break;

	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */
	}

	/* set up sector start address table */
	switch (value) {
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
		if ((now - last) > 100000) {	/* every second */
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

		if ((rc = write_word(info, wp, SWAP(data))) != 0) {
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
		if ((rc = write_word(info, wp, SWAP(data))) != 0) {
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

	return (write_word(info, wp, SWAP(data)));
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
