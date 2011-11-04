/*
 * (C) Copyright 2000-2011
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
#include <mpc8xx.h>

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size(vu_long *addr, flash_info_t *info);
static int write_word(flash_info_t *info, ulong dest, ulong data);
static void flash_get_offsets(ulong base, flash_info_t *info);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init(void)
{
	unsigned long size_b0;
	int i;

	/* Init: no FLASHes known */
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i)
		flash_info[i].flash_id = FLASH_UNKNOWN;

	/* Detect size */
	size_b0 = flash_get_size((vu_long *)CONFIG_SYS_FLASH_BASE,
			&flash_info[0]);

	/* Setup offsets */
	flash_get_offsets(CONFIG_SYS_FLASH_BASE, &flash_info[0]);

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
	/* Monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE+monitor_flash_len-1,
		      &flash_info[0]);
#endif

	flash_info[0].size = size_b0;

	return size_b0;
}

/*-----------------------------------------------------------------------
 * Fix this to support variable sector sizes
*/
static void flash_get_offsets(ulong base, flash_info_t *info)
{
	int i;

	/* set up sector start address table */
	if ((info->flash_id & FLASH_TYPEMASK) == FLASH_AM040) {
		/* set sector offsets for bottom boot block type	*/
		for (i = 0; i < info->sector_count; i++)
			info->start[i] = base + (i * 0x00010000);
	}
}

/*-----------------------------------------------------------------------
 */
void flash_print_info(flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		puts("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:
		printf("AMD ");
		break;
	case FLASH_MAN_FUJ:
		printf("FUJITSU ");
		break;
	case FLASH_MAN_BM:
		printf("BRIGHT MICRO ");
		break;
	default:
		printf("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM040:
		printf("29F040 or 29LV040 (4 Mbit, uniform sectors)\n");
		break;
	case FLASH_AM400B:
		printf("AM29LV400B (4 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM400T:
		printf("AM29LV400T (4 Mbit, top boot sector)\n");
		break;
	case FLASH_AM800B:
		printf("AM29LV800B (8 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM800T:
		printf("AM29LV800T (8 Mbit, top boot sector)\n");
		break;
	case FLASH_AM160B:
		printf("AM29LV160B (16 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM160T:
		printf("AM29LV160T (16 Mbit, top boot sector)\n");
		break;
	case FLASH_AM320B:
		printf("AM29LV320B (32 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM320T:
		printf("AM29LV320T (32 Mbit, top boot sector)\n");
		break;
	default:
		printf("Unknown Chip Type\n");
		break;
	}

	if (info->size >> 20) {
		printf("  Size: %ld MB in %d Sectors\n",
			info->size >> 20,
			info->sector_count);
	} else {
		printf("  Size: %ld KB in %d Sectors\n",
			info->size >> 10,
			info->sector_count);
	}

	puts("  Sector Start Addresses:");

	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			puts("\n   ");

		printf(" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     ");
	}

	putc('\n');
	return;
}
/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */

static ulong flash_get_size(vu_long *addr, flash_info_t *info)
{
	short i;
	volatile unsigned char *caddr;
	char value;

	caddr = (volatile unsigned char *)addr ;

	/* Write auto select command: read Manufacturer ID */

	debug("Base address is: %8p\n", caddr);

	caddr[0x0555] = 0xAA;
	caddr[0x02AA] = 0x55;
	caddr[0x0555] = 0x90;

	value = caddr[0];

	debug("Manufact ID: %02x\n", value);

	switch (value) {
	case 0x1: /* AMD_MANUFACT */
		info->flash_id = FLASH_MAN_AMD;
		break;
	case 0x4: /* FUJ_MANUFACT */
		info->flash_id = FLASH_MAN_FUJ;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		break;
	}

	value = caddr[1];			/* device ID		*/

	debug("Device ID: %02x\n", value);

	switch (value) {
	case AMD_ID_LV040B:
		info->flash_id += FLASH_AM040;
		info->sector_count = 8;
		info->size = 0x00080000;
		break;				/* => 512Kb		*/

	default:
		info->flash_id = FLASH_UNKNOWN;
		return 0;			/* => no or unknown flash */
	}

	flash_get_offsets((ulong)addr, &flash_info[0]);

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/*
		 * read sector protection at sector address,
		 * (A7 .. A0) = 0x02
		 * D0 = 1 if protected
		 */
		caddr = (volatile unsigned char *)(info->start[i]);
		info->protect[i] = caddr[2] & 1;
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		caddr = (volatile unsigned char *)info->start[0];
		*caddr = 0xF0;	/* reset bank */
	}

	return info->size;
}

int	flash_erase(flash_info_t *info, int s_first, int s_last)
{
	volatile unsigned char *addr =
		(volatile unsigned char *)(info->start[0]);
	int flag, prot, sect, l_sect;
	ulong start, now, last;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN)
			printf("- missing\n");
		else
			printf("- no sectors to erase\n");

		return 1;
	}

	if ((info->flash_id == FLASH_UNKNOWN) ||
	    (info->flash_id > FLASH_AMD_COMP)) {
		printf("Can't erase unknown flash type - aborted\n");
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect])
			prot++;
	}

	if (prot) {
		printf("- Warning: %d protected sectors will not be erased!\n",
			prot);
	} else {
		printf("\n");
	}

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	addr[0x0555] = 0xAA;
	addr[0x02AA] = 0x55;
	addr[0x0555] = 0x80;
	addr[0x0555] = 0xAA;
	addr[0x02AA] = 0x55;

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = (volatile unsigned char *)(info->start[sect]);
			addr[0] = 0x30;
			l_sect = sect;
		}
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* wait at least 80us - let's wait 1 ms */
	udelay(1000);

	/*
	 * We wait for the last triggered sector
	 */
	if (l_sect < 0)
		goto DONE;

	start = get_timer(0);
	last  = start;
	addr = (volatile unsigned char *)(info->start[l_sect]);

	while ((addr[0] & 0xFF) != 0xFF) {

		now = get_timer(start);

		if (now > CONFIG_SYS_FLASH_ERASE_TOUT) {
			printf("Timeout\n");
			return 1;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) {	/* every second */
			putc('.');
			last = now;
		}
	}

DONE:
	/* reset to read mode */
	addr = (volatile unsigned char *)info->start[0];

	addr[0] = 0xF0;	/* reset bank */

	printf(" done\n");
	return 0;
}

/*-----------------------------------------------------------------------
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
	l = addr - wp;

	if (l != 0) {
		data = 0;
		for (i = 0, cp = wp; i < l; ++i, ++cp)
			data = (data << 8) | (*(uchar *)cp);

		for (; i < 4 && cnt > 0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt == 0 && i < 4; ++i, ++cp)
			data = (data << 8) | (*(uchar *)cp);

		rc = write_word(info, wp, data);

		if (rc != 0)
			return rc;

		wp += 4;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= 4) {
		data = 0;
		for (i = 0; i < 4; ++i)
			data = (data << 8) | *src++;

		rc = write_word(info, wp, data);

		if (rc != 0)
			return rc;

		wp  += 4;
		cnt -= 4;
	}

	if (cnt == 0)
		return 0;

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i = 0, cp = wp; i < 4 && cnt > 0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i < 4; ++i, ++cp)
		data = (data << 8) | (*(uchar *)cp);

	return write_word(info, wp, data);
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word(flash_info_t *info, ulong dest, ulong data)
{
	volatile unsigned char *cdest, *cdata;
	volatile unsigned char *addr =
		(volatile unsigned char *)(info->start[0]);
	ulong start;
	int flag, count = 4 ;

	cdest = (volatile unsigned char *)dest ;
	cdata = (volatile unsigned char *)&data ;

	/* Check if Flash is (sufficiently) erased */
	if ((*((vu_long *)dest) & data) != data)
		return 2;

	while (count--) {

		/* Disable interrupts which might cause a timeout here */
		flag = disable_interrupts();

		addr[0x0555] = 0xAA;
		addr[0x02AA] = 0x55;
		addr[0x0555] = 0xA0;

		*cdest = *cdata;

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts();

		/* data polling for D7 */
		start = get_timer(0);
		while ((*cdest ^ *cdata) & 0x80) {
			if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT)
				return 1;
		}

		cdata++ ;
		cdest++ ;
	}
	return 0;
}
