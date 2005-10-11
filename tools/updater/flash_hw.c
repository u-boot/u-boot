/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
 *
 * (C) Copyright 2002
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <flash.h>
#include <asm/io.h>
#include <memio.h>

/*---------------------------------------------------------------------*/
#undef DEBUG_FLASH

#ifdef DEBUG_FLASH
#define DEBUGF(fmt,args...) printf(fmt ,##args)
#else
#define DEBUGF(fmt,args...)
#endif
/*---------------------------------------------------------------------*/

flash_info_t	flash_info[];

static ulong flash_get_size (ulong addr, flash_info_t *info);
static int flash_get_offsets (ulong base, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);
static void flash_reset (ulong addr);

int flash_xd_nest;

static void flash_to_xd(void)
{
    unsigned char x;

    flash_xd_nest ++;

    if (flash_xd_nest == 1)
    {
	DEBUGF("Flash on XD\n");
	x = pci_read_cfg_byte(0, 0, 0x74);
	pci_write_cfg_byte(0, 0, 0x74, x|1);
    }
}

static void flash_to_mem(void)
{
    unsigned char x;

    flash_xd_nest --;

    if (flash_xd_nest == 0)
    {
	DEBUGF("Flash on memory bus\n");
	x = pci_read_cfg_byte(0, 0, 0x74);
	pci_write_cfg_byte(0, 0, 0x74, x&0xFE);
    }
}

unsigned long flash_init_old(void)
{
    int i;

    for (i = 0; i < CFG_MAX_FLASH_BANKS; i++)
    {
	flash_info[i].flash_id = FLASH_UNKNOWN;
	flash_info[i].sector_count = 0;
	flash_info[i].size = 0;
    }


    return 1;
}

unsigned long flash_init (void)
{
	unsigned int i;
	unsigned long flash_size = 0;

	flash_xd_nest = 0;

	flash_to_xd();

	/* Init: no FLASHes known */
	for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
		flash_info[i].sector_count = 0;
		flash_info[i].size = 0;
	}

	DEBUGF("\n## Get flash size @ 0x%08x\n", CFG_FLASH_BASE);

	flash_size = flash_get_size (CFG_FLASH_BASE, flash_info);

	DEBUGF("## Flash bank size: %08lx\n", flash_size);

	if (flash_size) {
#if CFG_MONITOR_BASE >= CFG_FLASH_BASE && \
    CFG_MONITOR_BASE < CFG_FLASH_BASE + CFG_FLASH_MAX_SIZE
		/* monitor protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CFG_MONITOR_BASE,
			      CFG_MONITOR_BASE + CFG_MONITOR_LEN - 1,
			      &flash_info[0]);
#endif

#ifdef CFG_ENV_IS_IN_FLASH
		/* ENV protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CFG_ENV_ADDR,
			      CFG_ENV_ADDR + CFG_ENV_SECT_SIZE - 1,
			      &flash_info[0]);
#endif

	} else {
		printf ("Warning: the BOOT Flash is not initialised !");
	}

	flash_to_mem();

	return flash_size;
}

/*
 * The following code cannot be run from FLASH!
 */
static ulong flash_get_size (ulong addr, flash_info_t *info)
{
	short i;
	uchar value;
	uchar *x = (uchar *)addr;

	flash_to_xd();

	/* Write auto select command: read Manufacturer ID */
	x[0x0555] =  0xAA;
	__asm volatile ("sync\n eieio");
	x[0x02AA] =  0x55;
	__asm volatile ("sync\n eieio");
	x[0x0555] =  0x90;
	__asm volatile ("sync\n eieio");

	value = x[0];
	__asm volatile ("sync\n eieio");

	DEBUGF("Manuf. ID @ 0x%08lx: 0x%08x\n", (ulong)addr, value);

	switch (value | (value << 16)) {
		case AMD_MANUFACT:
			info->flash_id = FLASH_MAN_AMD;
			break;

		case FUJ_MANUFACT:
			info->flash_id = FLASH_MAN_FUJ;
			break;

		case STM_MANUFACT:
			info->flash_id = FLASH_MAN_STM;
			break;

		default:
			info->flash_id = FLASH_UNKNOWN;
			info->sector_count = 0;
			info->size = 0;
			flash_reset (addr);
			return 0;
	}

	value = x[1];
	__asm volatile ("sync\n eieio");

	DEBUGF("Device ID @ 0x%08lx: 0x%08x\n", addr+1, value);

	switch (value) {
		case AMD_ID_F040B:
			DEBUGF("Am29F040B\n");
			info->flash_id += FLASH_AM040;
			info->sector_count = 8;
			info->size = 0x00080000;
			break;			/* => 512 kB		*/

		case AMD_ID_LV040B:
			DEBUGF("Am29LV040B\n");
			info->flash_id += FLASH_AM040;
			info->sector_count = 8;
			info->size = 0x00080000;
			break;			/* => 512 kB		*/

		case AMD_ID_LV400T:
			DEBUGF("Am29LV400T\n");
			info->flash_id += FLASH_AM400T;
			info->sector_count = 11;
			info->size = 0x00100000;
			break;			/* => 1 MB		*/

		case AMD_ID_LV400B:
			DEBUGF("Am29LV400B\n");
			info->flash_id += FLASH_AM400B;
			info->sector_count = 11;
			info->size = 0x00100000;
			break;			/* => 1 MB		*/

		case AMD_ID_LV800T:
			DEBUGF("Am29LV800T\n");
			info->flash_id += FLASH_AM800T;
			info->sector_count = 19;
			info->size = 0x00200000;
			break;			/* => 2 MB		*/

		case AMD_ID_LV800B:
			DEBUGF("Am29LV400B\n");
			info->flash_id += FLASH_AM800B;
			info->sector_count = 19;
			info->size = 0x00200000;
			break;			/* => 2 MB		*/

		case AMD_ID_LV160T:
			DEBUGF("Am29LV160T\n");
			info->flash_id += FLASH_AM160T;
			info->sector_count = 35;
			info->size = 0x00400000;
			break;			/* => 4 MB		*/

		case AMD_ID_LV160B:
			DEBUGF("Am29LV160B\n");
			info->flash_id += FLASH_AM160B;
			info->sector_count = 35;
			info->size = 0x00400000;
			break;			/* => 4 MB		*/

		case AMD_ID_LV320T:
			DEBUGF("Am29LV320T\n");
			info->flash_id += FLASH_AM320T;
			info->sector_count = 67;
			info->size = 0x00800000;
			break;			/* => 8 MB		*/

#if 0
		/* Has the same ID as AMD_ID_LV320T, to be fixed */
		case AMD_ID_LV320B:
			DEBUGF("Am29LV320B\n");
			info->flash_id += FLASH_AM320B;
			info->sector_count = 67;
			info->size = 0x00800000;
			break;			/* => 8 MB		*/
#endif

		case AMD_ID_LV033C:
			DEBUGF("Am29LV033C\n");
			info->flash_id += FLASH_AM033C;
			info->sector_count = 64;
			info->size = 0x01000000;
			break;			/* => 16Mb		*/

		case STM_ID_F040B:
			DEBUGF("M29F040B\n");
			info->flash_id += FLASH_AM040;
			info->sector_count = 8;
			info->size = 0x00080000;
			break;			/* => 512 kB		*/

		default:
			info->flash_id = FLASH_UNKNOWN;
			flash_reset (addr);
			flash_to_mem();
			return (0);		/* => no or unknown flash */

	}

	if (info->sector_count > CFG_MAX_FLASH_SECT) {
		printf ("** ERROR: sector count %d > max (%d) **\n",
			info->sector_count, CFG_MAX_FLASH_SECT);
		info->sector_count = CFG_MAX_FLASH_SECT;
	}

	if (! flash_get_offsets (addr, info)) {
		flash_reset (addr);
		flash_to_mem();
		return 0;
	}

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		value = in8(info->start[i] + 2);
		iobarrier_rw();
		info->protect[i] = (value & 1) != 0;
	}

	/*
	 * Reset bank to read mode
	 */
	flash_reset (addr);

	flash_to_mem();

	return (info->size);
}

static int flash_get_offsets (ulong base, flash_info_t *info)
{
	unsigned int i;

	switch (info->flash_id & FLASH_TYPEMASK) {
		case FLASH_AM040:
			/* set sector offsets for uniform sector type	*/
			for (i = 0; i < info->sector_count; i++) {
				info->start[i] = base + i * info->size /
							    info->sector_count;
			}
			break;
		default:
			return 0;
	}

	return 1;
}

int flash_erase (flash_info_t *info, int s_first, int s_last)
{
	volatile ulong addr = info->start[0];
	int flag, prot, sect, l_sect;
	ulong start, now, last;

	flash_to_xd();

	if (s_first < 0 || s_first > s_last) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		flash_to_mem();
		return 1;
	}

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("Can't erase unknown flash type %08lx - aborted\n",
			info->flash_id);
		flash_to_mem();
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
		printf ("");
	}

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	out8(addr + 0x555, 0xAA);
	iobarrier_rw();
	out8(addr + 0x2AA, 0x55);
	iobarrier_rw();
	out8(addr + 0x555, 0x80);
	iobarrier_rw();
	out8(addr + 0x555, 0xAA);
	iobarrier_rw();
	out8(addr + 0x2AA, 0x55);
	iobarrier_rw();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = info->start[sect];
			out8(addr, 0x30);
			iobarrier_rw();
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
	addr = info->start[l_sect];

	DEBUGF ("Start erase timeout: %d\n", CFG_FLASH_ERASE_TOUT);

	while ((in8(addr) & 0x80) != 0x80) {
		if ((now = get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
			printf ("Timeout\n");
			flash_reset (info->start[0]);
			flash_to_mem();
			return 1;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) {	/* every second */
			putc ('.');
			last = now;
		}
		iobarrier_rw();
	}

DONE:
	/* reset to read mode */
	flash_reset (info->start[0]);
	flash_to_mem();

	printf (" done\n");
	return 0;
}

/*
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int i, l, rc;
	ulong out_cnt = 0;

	flash_to_xd();

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
			flash_to_mem();
			return (rc);
		}
		wp += 4;
	}

	putc(219);

	/*
	 * handle word aligned part
	 */
	while (cnt >= 4) {
	    if (out_cnt>26214)
	    {
		putc(219);
		out_cnt = 0;
	    }
	    data = 0;
	    for (i=0; i<4; ++i) {
		data = (data << 8) | *src++;
	    }
	    if ((rc = write_word(info, wp, data)) != 0) {
		flash_to_mem();
		return (rc);
	    }
	    wp  += 4;
	    cnt -= 4;
	    out_cnt += 4;
	}

	if (cnt == 0) {
		flash_to_mem();
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

	flash_to_mem();
	return (write_word(info, wp, data));
}

/*
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word (flash_info_t *info, ulong dest, ulong data)
{
	volatile ulong addr = info->start[0];
	ulong start;
	int i;

	flash_to_xd();

	/* Check if Flash is (sufficiently) erased */
	if ((in32(dest) & data) != data) {
		flash_to_mem();
		return (2);
	}

	/* write each byte out */
	for (i = 0; i < 4; i++) {
		char *data_ch = (char *)&data;
		int flag = disable_interrupts();

		out8(addr + 0x555, 0xAA);
		iobarrier_rw();
		out8(addr + 0x2AA, 0x55);
		iobarrier_rw();
		out8(addr + 0x555, 0xA0);
		iobarrier_rw();
		out8(dest+i, data_ch[i]);
		iobarrier_rw();

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts();

		/* data polling for D7 */
		start = get_timer (0);
		while ((in8(dest+i) & 0x80) != (data_ch[i] & 0x80)) {
			if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
				flash_reset (addr);
				flash_to_mem();
				return (1);
			}
			iobarrier_rw();
		}
	}

	flash_reset (addr);
	flash_to_mem();
	return (0);
}

/*
 * Reset bank to read mode
 */
static void flash_reset (ulong addr)
{
	flash_to_xd();
	out8(addr, 0xF0);	/* reset bank */
	iobarrier_rw();
	flash_to_mem();
}

void flash_print_info (flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:	printf ("AMD ");		break;
	case FLASH_MAN_FUJ:	printf ("FUJITSU ");		break;
	case FLASH_MAN_BM:	printf ("BRIGHT MICRO ");	break;
	case FLASH_MAN_STM:	printf ("SGS THOMSON ");	break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM040:	printf ("29F040 or 29LV040 (4 Mbit, uniform sectors)\n");
				break;
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

	if (info->size % 0x100000 == 0) {
		printf ("  Size: %ld MB in %d Sectors\n",
			info->size / 0x100000, info->sector_count);
	} else if (info->size % 0x400 == 0) {
		printf ("  Size: %ld KB in %d Sectors\n",
			info->size / 0x400, info->sector_count);
	} else {
		printf ("  Size: %ld B in %d Sectors\n",
			info->size, info->sector_count);
	}

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
