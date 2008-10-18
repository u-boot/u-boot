/*
 * (C) Copyright 2004
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

/* Yes,Yoo.They do use other FLASH for the board.
 *
 * Sam Song, IEMC. SHU, samsongshu@yahoo.com.cn
 * U-Boot port on RPXlite DW version board
 *
 * By now,it uses 4 AM29DL323DB90VI devices(4x8bit).
 * The total FLASH has 16MB(4x4MB).
 * I just made some necessary changes on the basis of Wolfgang and Yoo's job.
 *
 * June 8, 2004 */

#include <common.h>
#include <mpc8xx.h>

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/*-----------------------------------------------------------------------
 * Functions   vu_long : volatile unsigned long IN include/common.h
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);
static void flash_get_offsets (ulong base, flash_info_t *info);

unsigned long flash_init (void)
{
	unsigned long size_b0 ;
	int i;

	/* Init: no FLASHes known */
	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	size_b0 = flash_get_size((vu_long *)CONFIG_SYS_FLASH_BASE, &flash_info[0]);
	flash_get_offsets (CONFIG_SYS_FLASH_BASE, &flash_info[0]);

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
	/* If Monitor is in the cope of FLASH,then
	 * protect this area by default in case for
	 * other occupation. [SAM] */

	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE+CONFIG_SYS_MONITOR_LEN-1,
		      &flash_info[0]);
#endif
	flash_info[0].size = size_b0;
	return (size_b0);
}

static void flash_get_offsets (ulong base, flash_info_t *info)
{
	int i;

	/* set up sector start address table */
	if (info->flash_id & FLASH_BTYPE) {
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00008000;
		info->start[2] = base + 0x00010000;
		info->start[3] = base + 0x00018000;
		info->start[4] = base + 0x00020000;
		info->start[5] = base + 0x00028000;
		info->start[6] = base + 0x00030000;
		info->start[7] = base + 0x00038000;

		for (i = 8; i < info->sector_count; i++) {
			info->start[i] = base + ((i-7) * 0x00040000);
		}
	} else {
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00010000;
		info->start[i--] = base + info->size - 0x00018000;
		info->start[i--] = base + info->size - 0x00020000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00040000;
		}
	}

}

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
	case FLASH_AMDL323B:    printf ("AM29DL323B (32 Mbit, bottom boot sector)\n");
				break;
	/* I just add the FLASH_AMDL323B for RPXlite_DW BOARD. [SAM]  */
	default:		printf ("Unknown Chip Type\n");
				break;
	}
	printf ("  Size: %ld MB in %d Sectors\n",info->size >> 20, info->sector_count);
	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s",info->start[i],info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");
	return;
}

static ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
	short i;
	ulong value;
	ulong base = (ulong)addr;

	/* Write auto select command: read Manufacturer ID */
	addr[0xAAA] = 0x00AA00AA ;
	addr[0x555] = 0x00550055 ;
	addr[0xAAA] = 0x00900090 ;

	value = addr[0] ;
	switch (value & 0x00FF00FF) {
	case AMD_MANUFACT:			/* AMD_MANUFACT =0x00010001 in flash.h */
		info->flash_id = FLASH_MAN_AMD; /* FLASH_MAN_AMD=0x00000000 in flash.h */
		break;
	case FUJ_MANUFACT:
		info->flash_id = FLASH_MAN_FUJ;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);			/* no or unknown flash	*/
	}

	value = addr[2] ;			/* device ID		*/
	switch (value & 0x00FF00FF) {
	case (AMD_ID_LV400T & 0x00FF00FF):
		info->flash_id += FLASH_AM400T;
		info->sector_count = 11;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/
	case (AMD_ID_LV400B & 0x00FF00FF):
		info->flash_id += FLASH_AM400B;
		info->sector_count = 11;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/
	case (AMD_ID_LV800T & 0x00FF00FF):
		info->flash_id += FLASH_AM800T;
		info->sector_count = 19;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/
	case (AMD_ID_LV800B & 0x00FF00FF):
		info->flash_id += FLASH_AM800B;
		info->sector_count = 19;
		info->size = 0x00400000;	/* Size doubled by yooth */
		break;				/* => 4 MB		 */
	case (AMD_ID_LV160T & 0x00FF00FF):
		info->flash_id += FLASH_AM160T;
		info->sector_count = 35;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/
	case (AMD_ID_LV160B & 0x00FF00FF):
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/
	case (AMD_ID_DL323B & 0x00FF00FF):
		info->flash_id += FLASH_AMDL323B;
		info->sector_count = 71;
		info->size = 0x01000000;
		break;                          /* => 16 MB(4x4MB)  */
	/* AMD_ID_DL323B= 0x22532253  FLASH_AMDL323B= 0x0013
	 * AMD_ID_DL323B could be found in <flash.h>.[SAM]
	 * So we could get : flash_id = 0x00000013.
	 * The first four-bit represents VEDOR ID,leaving others for FLASH ID. */
	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */

	}
	/* set up sector start address table */
	if (info->flash_id & FLASH_BTYPE) {
	/* FLASH_BTYPE=0x0001 mask for bottom boot sector type.If the last bit equals 1,
	 * it means bottom boot flash. GOOD IDEA! [SAM]
	 */

	/* set sector offsets for bottom boot block type        */
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00008000;
		info->start[2] = base + 0x00010000;
		info->start[3] = base + 0x00018000;
		info->start[4] = base + 0x00020000;
		info->start[5] = base + 0x00028000;
		info->start[6] = base + 0x00030000;
		info->start[7] = base + 0x00038000;

		for (i = 8; i < info->sector_count; i++) {
			info->start[i] = base + ((i-7) * 0x00040000) ;
		}
	} else {
		/* set sector offsets for top boot block type		*/
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00010000;
		info->start[i--] = base + info->size - 0x00018000;
		info->start[i--] = base + info->size - 0x00020000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00040000;
		}
	}

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr = (volatile unsigned long *)(info->start[i]);
		/* info->protect[i] = addr[4] & 1 ; */
		/* Mask it for disorder FLASH protection **[Sam]** */
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr = (volatile unsigned long *)info->start[0];

		*addr = 0xF0F0F0F0;	/* reset bank */
	}
	return (info->size);
}

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	vu_long *addr = (vu_long*)(info->start[0]);
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

	addr[0xAAA] = 0xAAAAAAAA;
	addr[0x555] = 0x55555555;
	addr[0xAAA] = 0x80808080;
	addr[0xAAA] = 0xAAAAAAAA;
	addr[0x555] = 0x55555555;

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = (vu_long *)(info->start[sect]) ;
			addr[0] = 0x30303030 ;
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
	addr = (vu_long *)(info->start[l_sect]);
	while ((addr[0] & 0x80808080) != 0x80808080) {
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

DONE:
	/* reset to read mode */
	addr = (vu_long *)info->start[0];
	addr[0] = 0xF0F0F0F0;	/* reset bank */

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
	vu_long *addr = (vu_long *)(info->start[0]);
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*((vu_long *)dest) & data) != data) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	addr[0xAAA] = 0xAAAAAAAA;
	addr[0x555] = 0x55555555;
	addr[0xAAA] = 0xA0A0A0A0;

	*((vu_long *)dest) = data;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer (0);
	while ((*((vu_long *)dest) & 0x80808080) != (data & 0x80808080)) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			return (1);
		}
	}
	return (0);
}
