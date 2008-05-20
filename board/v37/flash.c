/*
 * (C) Copyright 2003
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

/*
 * Yoo. Jonghoon, IPone, yooth@ipone.co.kr
 * U-Boot port on RPXlite board
 *
 * Some of flash control words are modified. (from 2x16bit device
 * to 4x8bit device)
 * RPXLite board I tested has only 4 AM29LV800BB devices. Other devices
 * are not tested.
 *
 * (?) Does an RPXLite board which
 *	does not use AM29LV800 flash memory exist ?
 *	I don't know...
 */

#include <common.h>
#include <mpc8xx.h>

flash_info_t	flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size ( short manu, short dev_id, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);
static void flash_get_offsets (ulong base, flash_info_t *info, int two_chips);
static void flash_get_id_word( void *ptr,  short *ptr_manuf, short *ptr_dev_id);
static void flash_get_id_long( void *ptr,  short *ptr_manuf, short *ptr_dev_id);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	volatile immap_t     *immap  = (immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	unsigned long size_b0, size_b1;
	short manu, dev_id;
	int i;

	/* Init: no FLASHes known */
	for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Do sizing to get full correct info */

	flash_get_id_word((void*)CFG_FLASH_BASE0,&manu,&dev_id);

	size_b0 = flash_get_size(manu, dev_id, &flash_info[0]);

	flash_get_offsets (CFG_FLASH_BASE0, &flash_info[0],0);

	memctl->memc_or0 = CFG_OR_TIMING_FLASH | (0 - size_b0);

#if CFG_MONITOR_BASE >= CFG_FLASH_BASE0
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CFG_MONITOR_BASE,
		      CFG_MONITOR_BASE+monitor_flash_len-1,
		      &flash_info[0]);
#endif

	flash_get_id_long((void*)CFG_FLASH_BASE1,&manu,&dev_id);

	size_b1 = 2 * flash_get_size(manu, dev_id, &flash_info[1]);

	flash_get_offsets(CFG_FLASH_BASE1, &flash_info[1],1);

	memctl->memc_or1 = CFG_OR_TIMING_FLASH | (0 - size_b1);

	flash_info[0].size = size_b0;
	flash_info[1].size = size_b1;

	return (size_b0+size_b1);
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets (ulong base, flash_info_t *info, int two_chips)
{
	int i, addr_shift;
	vu_short *addr = (vu_short*)base;

	addr[0x555] = 0x00AA ;
	addr[0xAAA] = 0x0055 ;
	addr[0x555] = 0x0090 ;

	addr_shift = (two_chips ? 2 : 1 );

	/* set up sector start address table */
	if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block type	*/
		info->start[0] = base + (0x00000000<<addr_shift);
		info->start[1] = base + (0x00002000<<addr_shift);
		info->start[2] = base + (0x00003000<<addr_shift);
		info->start[3] = base + (0x00004000<<addr_shift);
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + ((i-3) * (0x00008000<<addr_shift)) ;
		}
	} else {
		/* set sector offsets for top boot block type		*/
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - (0x00002000<<addr_shift);
		info->start[i--] = base + info->size - (0x00003000<<addr_shift);
		info->start[i--] = base + info->size - (0x00004000<<addr_shift);
		for (; i >= 0; i--) {
			info->start[i] = base + i * (0x00008000<<addr_shift);
		}
	}

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr = (vu_short *)(info->start[i]);
		info->protect[i] = addr[1<<addr_shift] & 1 ;
	}

	addr = (vu_short *)info->start[0];
	*addr = 0xF0F0;	/* reset bank */
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
	case FLASH_MAN_TOSH:	printf ("TOSHIBA ");		break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM400B:	printf ("AM29LV400B (4 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM400T:	printf ("AM29LV400T (4 Mbit, top boot sector)\n");
				break;
	case FLASH_AM800B:	printf ("AM29LV800B (8 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM800T:	printf ("AM29LV800T (8 Mbit, top boot sector)\n");
				break;
	case FLASH_AM160B:	printf ("AM29LV160B (16 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM160T:	printf ("AM29LV160T (16 Mbit, top boot sector)\n");
				break;
	case FLASH_AM320B:	printf ("AM29LV320B (32 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM320T:	printf ("AM29LV320T (32 Mbit, top boot sector)\n");
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
}

/*-----------------------------------------------------------------------
 */


/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */

static void flash_get_id_word( void *ptr,  short *ptr_manuf, short *ptr_dev_id)
{
	vu_short *addr = (vu_short*)ptr;

	addr[0x555] = 0x00AA ;
	addr[0xAAA] = 0x0055 ;
	addr[0x555] = 0x0090 ;

	*ptr_manuf  = addr[0];
	*ptr_dev_id = addr[1];

	addr[0] = 0xf0f0;       /* return to normal */
}

static void flash_get_id_long( void *ptr,  short *ptr_manuf, short *ptr_dev_id)
{
	vu_short *addr = (vu_short*)ptr;
	vu_short *addr1, *addr2, *addr3;

	addr1 = (vu_short*) ( ((int)ptr) + (0x5555<<2) );
	addr2 = (vu_short*) ( ((int)ptr) + (0x2AAA<<2) );
	addr3 = (vu_short*) ( ((int)ptr) + (0x5555<<2) );

	*addr1 = 0xAAAA;
	*addr2 = 0x5555;
	*addr3 = 0x9090;

	*ptr_manuf  = addr[0];
	*ptr_dev_id = addr[2];

	addr[0] = 0xf0f0;       /* return to normal */
}

static ulong flash_get_size ( short manu, short dev_id, flash_info_t *info)
{
	switch (manu) {
	case ((short)AMD_MANUFACT):
		info->flash_id = FLASH_MAN_AMD;
		break;
	case ((short)FUJ_MANUFACT):
		info->flash_id = FLASH_MAN_FUJ;
		break;
	case ((short)TOSH_MANUFACT):
		info->flash_id = FLASH_MAN_TOSH;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);			/* no or unknown flash	*/
	}


	switch (dev_id) {
	case ((short)TOSH_ID_FVT160):
		info->flash_id += FLASH_AM160T;
		info->sector_count = 35;
		info->size = 0x00200000;
		break;				/* => 1 MB		*/

	case ((short)TOSH_ID_FVB160):
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00200000;
		break;				/* => 1 MB		*/

	case ((short)AMD_ID_LV400T):
		info->flash_id += FLASH_AM400T;
		info->sector_count = 11;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/

	case ((short)AMD_ID_LV400B):
		info->flash_id += FLASH_AM400B;
		info->sector_count = 11;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/

	case ((short)AMD_ID_LV800T):
		info->flash_id += FLASH_AM800T;
		info->sector_count = 19;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/

	case ((short)AMD_ID_LV800B):
		info->flash_id += FLASH_AM800B;
		info->sector_count = 19;
		info->size = 0x00400000;	/*%%% Size doubled by yooth */
		break;				/* => 4 MB		*/

	case ((short)AMD_ID_LV160T):
		info->flash_id += FLASH_AM160T;
		info->sector_count = 35;
		info->size = 0x00200000;
		break;				/* => 4 MB		*/

	case ((short)AMD_ID_LV160B):
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00200000;
		break;				/* => 4 MB		*/
	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */

	}

	return(info->size);
}


/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	vu_short *addr = (vu_short*)(info->start[0]);
	int flag, prot, sect, l_sect;
	ulong start, now, last;

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

	addr[0x555] = (vu_short)0xAAAAAAAA;
	addr[0xAAA] = (vu_short)0x55555555;
	addr[0x555] = (vu_short)0x80808080;
	addr[0x555] = (vu_short)0xAAAAAAAA;
	addr[0xAAA] = (vu_short)0x55555555;

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = (vu_short *)(info->start[sect]) ;
			addr[0] = (vu_short)0x30303030 ;
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
	addr = (vu_short *)(info->start[l_sect]);
	while ((addr[0] & 0x8080) != 0x8080) {
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
	addr = (vu_short *)info->start[0];
	addr[0] = (vu_short)0xF0F0F0F0;	/* reset bank */

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
	vu_short *addr = (vu_short *)(info->start[0]);
	vu_short sdata;

	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*((vu_long *)dest) & data) != data) {
		return (2);
	}

	/* First write upper 16 bits */
	sdata = (short)(data>>16);

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	addr[0x555] = 0xAAAA;
	addr[0xAAA] = 0x5555;
	addr[0x555] = 0xA0A0;

	*((vu_short *)dest) = sdata;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer (0);
	while ((*((vu_short *)dest) & 0x8080) != (sdata & 0x8080)) {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			return (1);
		}
	}

	/* Now write lower 16 bits */
	sdata = (short)(data&0xffff);

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	addr[0x555] = 0xAAAA;
	addr[0xAAA] = 0x5555;
	addr[0x555] = 0xA0A0;

	*((vu_short *)dest + 1) = sdata;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer (0);
	while ((*((vu_short *)dest + 1) & 0x8080) != (sdata & 0x8080)) {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			return (1);
		}
	}
	return (0);
}

/*-----------------------------------------------------------------------
 */
