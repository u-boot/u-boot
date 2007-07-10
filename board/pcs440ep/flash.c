/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#include <asm/processor.h>

#ifndef CFG_FLASH_READ0
#define CFG_FLASH_READ0		0x0000	/* 0 is standard			*/
#define CFG_FLASH_READ1		0x0001	/* 1 is standard			*/
#define CFG_FLASH_READ2		0x0002	/* 2 is standard			*/
#endif

flash_info_t	flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/*
 * Functions
 */
static int write_word(flash_info_t *info, ulong dest, ulong data);
static ulong flash_get_size(vu_long *addr, flash_info_t *info);

unsigned long flash_init(void)
{
	unsigned long size_b0, size_b1;
	int i;
	unsigned long base_b0, base_b1;

	/* Init: no FLASHes known */
	for (i = 0; i < CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Static FLASH Bank configuration here - FIXME XXX */

	base_b0 = FLASH_BASE0_PRELIM;
	size_b0 = flash_get_size ((vu_long *) base_b0, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
				size_b0, size_b0 << 20);
	}

	base_b1 = FLASH_BASE1_PRELIM;
	size_b1 = flash_get_size ((vu_long *) base_b1, &flash_info[1]);

	return (size_b0 + size_b1);
}

void flash_print_info(flash_info_t *info)
{
	int i;
	int k;
	int size;
	int erased;
	volatile unsigned long *flash;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:	printf ("AMD ");		break;
	case FLASH_MAN_FUJ:	printf ("FUJITSU ");		break;
	case FLASH_MAN_SST:	printf ("SST ");		break;
	case FLASH_MAN_EXCEL:	printf ("Excel Semiconductor "); break;
	case FLASH_MAN_MX:	printf ("MXIC "); break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM400B:	printf ("AM29LV400B (4 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM400T:	printf ("AM29LV400T (4 Mbit, top boot sector)\n");
		break;
	case FLASH_AM040:	printf ("AM29LV040B (4 Mbit, uniform sector size)\n");
		break;
	case FLASH_AM800B:	printf ("AM29LV800B (8 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM800T:	printf ("AM29LV800T (8 Mbit, top boot sector)\n");
		break;
	case FLASH_AM160B:	printf ("AM29LV160B (16 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM160T:	printf ("AM29LV160T (16 Mbit, top boot sector)\n");
		break;
	case FLASH_AM320T:	printf ("AM29LV320T (32 M, top sector)\n");
		break;
	case FLASH_AM320B:	printf ("AM29LV320B (32 M, bottom sector)\n");
		break;
	case FLASH_AMDL322T:	printf ("AM29DL322T (32 M, top sector)\n");
		break;
	case FLASH_AMDL322B:	printf ("AM29DL322B (32 M, bottom sector)\n");
		break;
	case FLASH_AMDL323T:	printf ("AM29DL323T (32 M, top sector)\n");
		break;
	case FLASH_AMDL323B:	printf ("AM29DL323B (32 M, bottom sector)\n");
		break;
	case FLASH_SST020:	printf ("SST39LF/VF020 (2 Mbit, uniform sector size)\n");
		break;
	case FLASH_SST040:	printf ("SST39LF/VF040 (4 Mbit, uniform sector size)\n");
		break;
	default:		printf ("Unknown Chip Type\n");
		break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
#ifdef CFG_FLASH_EMPTY_INFO
		/*
		 * Check if whole sector is erased
		 */
		if (i != (info->sector_count-1))
			size = info->start[i+1] - info->start[i];
		else
			size = info->start[0] + info->size - info->start[i];
		erased = 1;
		flash = (volatile unsigned long *)info->start[i];
		size = size >> 2;	/* divide by 4 for longword access */
		for (k=0; k<size; k++) {
			if (*flash++ != 0xffffffff) {
				erased = 0;
				break;
			}
		}

		if ((i % 5) == 0)
			printf ("\n   ");
		/* print empty and read-only info */
		printf (" %08lX%s%s",
			info->start[i],
			erased ? " E" : "  ",
			info->protect[i] ? "RO " : "   ");
#else
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     ");
#endif

	}
	printf ("\n");
	return;
}

/*
 * The following code cannot be run from FLASH!
 */
static ulong flash_get_size(vu_long *addr, flash_info_t *info)
{
	short i;
	short n;
	volatile CFG_FLASH_WORD_SIZE value;
	ulong base = (ulong)addr;
	volatile CFG_FLASH_WORD_SIZE *addr2 = (volatile CFG_FLASH_WORD_SIZE *)addr;

	/* Write auto select command: read Manufacturer ID */
	addr2[CFG_FLASH_ADDR0] = (CFG_FLASH_WORD_SIZE)0x00AA00AA;
	addr2[CFG_FLASH_ADDR1] = (CFG_FLASH_WORD_SIZE)0x00550055;
	addr2[CFG_FLASH_ADDR0] = (CFG_FLASH_WORD_SIZE)0x00900090;

	value = addr2[CFG_FLASH_READ0];

	switch (value) {
	case (CFG_FLASH_WORD_SIZE)AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
		break;
	case (CFG_FLASH_WORD_SIZE)FUJ_MANUFACT:
		info->flash_id = FLASH_MAN_FUJ;
		break;
	case (CFG_FLASH_WORD_SIZE)SST_MANUFACT:
		info->flash_id = FLASH_MAN_SST;
		break;
	case (CFG_FLASH_WORD_SIZE)EXCEL_MANUFACT:
		info->flash_id = FLASH_MAN_EXCEL;
		break;
	case (CFG_FLASH_WORD_SIZE)MX_MANUFACT:
		info->flash_id = FLASH_MAN_MX;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);			/* no or unknown flash	*/
	}

	value = addr2[CFG_FLASH_READ1];		/* device ID	*/

	switch (value) {
	case (CFG_FLASH_WORD_SIZE)AMD_ID_LV400T:
		info->flash_id += FLASH_AM400T;
		info->sector_count = 11;
		info->size = 0x00080000;
		break;				/* => 0.5 MB	*/

	case (CFG_FLASH_WORD_SIZE)AMD_ID_LV400B:
		info->flash_id += FLASH_AM400B;
		info->sector_count = 11;
		info->size = 0x00080000;
		break;				/* => 0.5 MB	*/

	case (CFG_FLASH_WORD_SIZE)AMD_ID_LV040B:
		info->flash_id += FLASH_AM040;
		info->sector_count = 8;
		info->size = 0x0080000;		/* => 0.5 MB	*/
		break;

	case (CFG_FLASH_WORD_SIZE)AMD_ID_LV800T:
		info->flash_id += FLASH_AM800T;
		info->sector_count = 19;
		info->size = 0x00100000;
		break;				/* => 1 MB	*/

	case (CFG_FLASH_WORD_SIZE)AMD_ID_LV800B:
		info->flash_id += FLASH_AM800B;
		info->sector_count = 19;
		info->size = 0x00100000;
		break;				/* => 1 MB	*/

	case (CFG_FLASH_WORD_SIZE)AMD_ID_LV160T:
		info->flash_id += FLASH_AM160T;
		info->sector_count = 35;
		info->size = 0x00200000;
		break;				/* => 2 MB	*/

	case (CFG_FLASH_WORD_SIZE)AMD_ID_LV160B:
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00200000;
		break;				/* => 2 MB	*/

	case (CFG_FLASH_WORD_SIZE)AMD_ID_LV320T:
		info->flash_id += FLASH_AM320T;
		info->sector_count = 71;
		info->size = 0x00400000;
		break;				/* => 4 MB	*/

	case (CFG_FLASH_WORD_SIZE)AMD_ID_LV320B:
		info->flash_id += FLASH_AM320B;
		info->sector_count = 71;
		info->size = 0x00400000;
		break;				/* => 4 MB	*/

	case (CFG_FLASH_WORD_SIZE)AMD_ID_DL322T:
		info->flash_id += FLASH_AMDL322T;
		info->sector_count = 71;
		info->size = 0x00400000;
		break;				/* => 4 MB	*/

	case (CFG_FLASH_WORD_SIZE)AMD_ID_DL322B:
		info->flash_id += FLASH_AMDL322B;
		info->sector_count = 71;
		info->size = 0x00400000;
		break;				/* => 4 MB	*/

	case (CFG_FLASH_WORD_SIZE)AMD_ID_DL323T:
		info->flash_id += FLASH_AMDL323T;
		info->sector_count = 71;
		info->size = 0x00400000;
		break;				/* => 4 MB	*/

	case (CFG_FLASH_WORD_SIZE)AMD_ID_DL323B:
		info->flash_id += FLASH_AMDL323B;
		info->sector_count = 71;
		info->size = 0x00400000;
		break;				/* => 4 MB	*/

	case (CFG_FLASH_WORD_SIZE)SST_ID_xF020:
		info->flash_id += FLASH_SST020;
		info->sector_count = 64;
		info->size = 0x00040000;
		break;				/* => 256 kB	*/

	case (CFG_FLASH_WORD_SIZE)SST_ID_xF040:
		info->flash_id += FLASH_SST040;
		info->sector_count = 128;
		info->size = 0x00080000;
		break;				/* => 512 kB	*/

	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */

	}

	/* set up sector start address table */
	if (((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_SST) ||
	    ((info->flash_id & FLASH_TYPEMASK) == FLASH_AM640U)) {
		for (i = 0; i < info->sector_count; i++)
			info->start[i] = base + (i * 0x00001000);
	} else if ((info->flash_id & FLASH_TYPEMASK) == FLASH_AM040) {
		for (i = 0; i < info->sector_count; i++)
			info->start[i] = base + (i * 0x00010000);
	} else if (((info->flash_id & FLASH_TYPEMASK) == FLASH_AMDL322B) ||
		   ((info->flash_id & FLASH_TYPEMASK) == FLASH_AMDL323B) ||
		   ((info->flash_id & FLASH_TYPEMASK) == FLASH_AM320B) ||
		   ((info->flash_id & FLASH_TYPEMASK) == FLASH_AMDL324B)) {
		/* set sector offsets for bottom boot block type	*/
		for (i=0; i<8; ++i) {		/*  8 x 8k boot sectors	*/
			info->start[i] = base;
			base += 8 << 10;
		}
		while (i < info->sector_count) {	/* 64k regular sectors	*/
			info->start[i] = base;
			base += 64 << 10;
			++i;
		}
	} else if (((info->flash_id & FLASH_TYPEMASK) == FLASH_AMDL322T) ||
		   ((info->flash_id & FLASH_TYPEMASK) == FLASH_AMDL323T) ||
		   ((info->flash_id & FLASH_TYPEMASK) == FLASH_AM320T) ||
		   ((info->flash_id & FLASH_TYPEMASK) == FLASH_AMDL324T)) {
		/* set sector offsets for top boot block type		*/
		base += info->size;
		i = info->sector_count;
		for (n=0; n<8; ++n) {		/*  8 x 8k boot sectors	*/
			base -= 8 << 10;
			--i;
			info->start[i] = base;
		}
		while (i > 0) {			/* 64k regular sectors	*/
			base -= 64 << 10;
			--i;
			info->start[i] = base;
		}
	} else {
		if (info->flash_id & FLASH_BTYPE) {
			/* set sector offsets for bottom boot block type	*/
			info->start[0] = base + 0x00000000;
			info->start[1] = base + 0x00004000;
			info->start[2] = base + 0x00006000;
			info->start[3] = base + 0x00008000;
			for (i = 4; i < info->sector_count; i++) {
				info->start[i] = base + (i * 0x00010000) - 0x00030000;
			}
		} else {
			/* set sector offsets for top boot block type		*/
			i = info->sector_count - 1;
			info->start[i--] = base + info->size - 0x00004000;
			info->start[i--] = base + info->size - 0x00006000;
			info->start[i--] = base + info->size - 0x00008000;
			for (; i >= 0; i--) {
				info->start[i] = base + i * 0x00010000;
			}
		}
	}

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr2 = (volatile CFG_FLASH_WORD_SIZE *)(info->start[i]);
		if ((info->flash_id & FLASH_VENDMASK) != FLASH_MAN_AMD)
			info->protect[i] = 0;
		else
			info->protect[i] = addr2[CFG_FLASH_READ2] & 1;
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr2 = (CFG_FLASH_WORD_SIZE *)info->start[0];
		*addr2 = (CFG_FLASH_WORD_SIZE)0x00F000F0;	/* reset bank */
	}

	return (info->size);
}


int flash_erase(flash_info_t *info, int s_first, int s_last)
{
	volatile CFG_FLASH_WORD_SIZE *addr = (CFG_FLASH_WORD_SIZE *)(info->start[0]);
	volatile CFG_FLASH_WORD_SIZE *addr2;
	int flag, prot, sect, l_sect;
	ulong start, now, last;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN)
			printf ("- missing\n");
		else
			printf ("- no sectors to erase\n");
		return 1;
	}

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("Can't erase unknown flash type - aborted\n");
		return 1;
	}

	prot = 0;
	for (sect=s_first; sect<=s_last; ++sect)
		if (info->protect[sect])
			prot++;

	if (prot)
		printf ("- Warning: %d protected sectors will not be erased!\n", prot);
	else
		printf ("\n");

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr2 = (CFG_FLASH_WORD_SIZE *)(info->start[sect]);
			if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_SST) {
				addr[CFG_FLASH_ADDR0] = (CFG_FLASH_WORD_SIZE)0x00AA00AA;
				addr[CFG_FLASH_ADDR1] = (CFG_FLASH_WORD_SIZE)0x00550055;
				addr[CFG_FLASH_ADDR0] = (CFG_FLASH_WORD_SIZE)0x00800080;
				addr[CFG_FLASH_ADDR0] = (CFG_FLASH_WORD_SIZE)0x00AA00AA;
				addr[CFG_FLASH_ADDR1] = (CFG_FLASH_WORD_SIZE)0x00550055;
				addr2[0] = (CFG_FLASH_WORD_SIZE)0x00300030;  /* sector erase */

				/* re-enable interrupts if necessary */
				if (flag) {
					enable_interrupts();
					flag = 0;
				}

				/* data polling for D7 */
				start = get_timer (0);
				while ((addr2[0] & (CFG_FLASH_WORD_SIZE)0x00800080) !=
				       (CFG_FLASH_WORD_SIZE)0x00800080) {
					if (get_timer(start) > CFG_FLASH_WRITE_TOUT)
						return (1);
				}
			} else {
				if (sect == s_first) {
					addr[CFG_FLASH_ADDR0] = (CFG_FLASH_WORD_SIZE)0x00AA00AA;
					addr[CFG_FLASH_ADDR1] = (CFG_FLASH_WORD_SIZE)0x00550055;
					addr[CFG_FLASH_ADDR0] = (CFG_FLASH_WORD_SIZE)0x00800080;
					addr[CFG_FLASH_ADDR0] = (CFG_FLASH_WORD_SIZE)0x00AA00AA;
					addr[CFG_FLASH_ADDR1] = (CFG_FLASH_WORD_SIZE)0x00550055;
				}
				addr2[0] = (CFG_FLASH_WORD_SIZE)0x00300030;  /* sector erase */
			}
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
	addr = (CFG_FLASH_WORD_SIZE *)(info->start[l_sect]);
	while ((addr[0] & (CFG_FLASH_WORD_SIZE)0x00800080) != (CFG_FLASH_WORD_SIZE)0x00800080) {
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
	addr = (CFG_FLASH_WORD_SIZE *)info->start[0];
	addr[0] = (CFG_FLASH_WORD_SIZE)0x00F000F0;	/* reset bank */

	printf (" done\n");
	return 0;
}

/*
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
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
		for (i=0; i<4; ++i)
			data = (data << 8) | *src++;
		if ((rc = write_word(info, wp, data)) != 0)
			return (rc);
		wp  += 4;
		cnt -= 4;
	}

	if (cnt == 0)
		return (0);

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i=0, cp=wp; i<4 && cnt>0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i<4; ++i, ++cp)
		data = (data << 8) | (*(uchar *)cp);

	return (write_word(info, wp, data));
}

/*
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word(flash_info_t *info, ulong dest, ulong data)
{
	volatile CFG_FLASH_WORD_SIZE *addr2 = (CFG_FLASH_WORD_SIZE *)(info->start[0]);
	volatile CFG_FLASH_WORD_SIZE *dest2 = (CFG_FLASH_WORD_SIZE *)dest;
	volatile CFG_FLASH_WORD_SIZE *data2 = (CFG_FLASH_WORD_SIZE *)&data;
	ulong start;
	int flag;
	int i;

	/* Check if Flash is (sufficiently) erased */
	if ((*((vu_long *)dest) & data) != data)
		return (2);

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	for (i=0; i<4/sizeof(CFG_FLASH_WORD_SIZE); i++) {
		addr2[CFG_FLASH_ADDR0] = (CFG_FLASH_WORD_SIZE)0x00AA00AA;
		addr2[CFG_FLASH_ADDR1] = (CFG_FLASH_WORD_SIZE)0x00550055;
		addr2[CFG_FLASH_ADDR0] = (CFG_FLASH_WORD_SIZE)0x00A000A0;

		dest2[i] = data2[i];

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts();

		/* data polling for D7 */
		start = get_timer (0);
		while ((dest2[i] & (CFG_FLASH_WORD_SIZE)0x00800080) !=
		       (data2[i] & (CFG_FLASH_WORD_SIZE)0x00800080)) {
			if (get_timer(start) > CFG_FLASH_WRITE_TOUT)
				return (1);
		}
	}

	return (0);
}
