/*
 * (C) Copyright 2006 Embedded Artists AB <www.embeddedartists.com>
 *
 * (C) Copyright 2007 Gary Jennejohn garyj@denx.de
 * Modified to use the routines in cpu/arm720t/lpc2292/flash.c.
 * Heavily modified to support the SMN42 board from Siemens
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
#include <asm/byteorder.h>
#include <asm/arch/hardware.h>

static unsigned long flash_addr_table[CONFIG_SYS_MAX_FLASH_BANKS] = CONFIG_SYS_FLASH_BANKS_LIST;
flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

extern int lpc2292_copy_buffer_to_flash(flash_info_t *, ulong);
extern int lpc2292_flash_erase(flash_info_t *, int, int);
extern int lpc2292_write_buff (flash_info_t *, uchar *, ulong, ulong);
static unsigned long ext_flash_init(void);
static int ext_flash_erase(flash_info_t *, int, int);
static int ext_write_buff(flash_info_t *, uchar *, ulong, ulong);

/*-----------------------------------------------------------------------
 */

ulong flash_init (void)
{
	int j, k;
	ulong size = 0;
	ulong flashbase = 0;

	flash_info[0].flash_id = PHILIPS_LPC2292;
	flash_info[0].size = 0x003E000;	/* 256 - 8 KB */
	flash_info[0].sector_count = 17;
	memset (flash_info[0].protect, 0, 17);
	flashbase = 0x00000000;
	for (j = 0, k = 0; j < 8; j++, k++) {
		flash_info[0].start[k] = flashbase;
		flashbase += 0x00002000;
	}
	for (j = 0; j < 2; j++, k++) {
		flash_info[0].start[k] = flashbase;
		flashbase += 0x00010000;
	}
	for (j = 0; j < 7; j++, k++) {
		flash_info[0].start[k] = flashbase;
		flashbase += 0x00002000;
	}
	size += flash_info[0].size;

	/* Protect monitor and environment sectors */
	flash_protect (FLAG_PROTECT_SET,
		 0x0,
		 0x0 + monitor_flash_len - 1,
		 &flash_info[0]);

	flash_protect (FLAG_PROTECT_SET,
		 CONFIG_ENV_ADDR,
		 CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1,
		 &flash_info[0]);

	size += ext_flash_init();

	return size;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info (flash_info_t * info)
{
	int i;
	int erased = 0;
	unsigned long j;
	unsigned long count;
	unsigned char *p;

	switch (info->flash_id & FLASH_VENDMASK) {
	case (PHILIPS_LPC2292 & FLASH_VENDMASK):
		printf("Philips: ");
		break;
	case FLASH_MAN_AMD:
		printf("AMD: ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case (PHILIPS_LPC2292 & FLASH_TYPEMASK):
		printf("LPC2292 internal flash\n");
		break;
	case FLASH_S29GL128N:
		printf ("S29GL128N (128 Mbit, uniform sector size)\n");
		break;
	default:
		printf("Unknown Chip Type\n");
		return;
	}

	printf ("  Size: %ld KB in %d Sectors\n",
	  info->size >> 10, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; i++) {
		if ((i % 5) == 0) {
			printf ("\n   ");
		}
		if (i < (info->sector_count - 1)) {
			count = info->start[i+1] - info->start[i];
		}
		else {
			count = info->start[0] + info->size - info->start[i];
		}
		p = (unsigned char*)(info->start[i]);
		erased = 1;
		for (j = 0; j < count; j++) {
			if (*p != 0xFF) {
				erased = 0;
				break;
			}
			p++;
		}
		printf (" %08lX%s%s", info->start[i], info->protect[i] ? " RO" : "   ",
			erased ? " E" : "  ");
	}
	printf ("\n");
}

int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	switch (info->flash_id & FLASH_TYPEMASK) {
		case (PHILIPS_LPC2292 & FLASH_TYPEMASK):
			return lpc2292_flash_erase(info, s_first, s_last);
		case FLASH_S29GL128N:
			return ext_flash_erase(info, s_first, s_last);
		default:
			return ERR_PROTECTED;
	}
	return ERR_PROTECTED;
}

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	switch (info->flash_id & FLASH_TYPEMASK) {
		case (PHILIPS_LPC2292 & FLASH_TYPEMASK):
			return lpc2292_write_buff(info, src, addr, cnt);
		case FLASH_S29GL128N:
			return ext_write_buff(info, src, addr, cnt);
		default:
			return ERR_PROG_ERROR;
	}
	return ERR_PROG_ERROR;
}

/*--------------------------------------------------------------------------
 * From here on is code for the external S29GL128N taken from cam5200_flash.c
 */

#define CONFIG_SYS_FLASH_WORD_SIZE unsigned short

static int wait_for_DQ7_32(flash_info_t * info, int sect)
{
	ulong start, now, last;
	volatile CONFIG_SYS_FLASH_WORD_SIZE *addr =
		(CONFIG_SYS_FLASH_WORD_SIZE *) (info->start[sect]);

	start = get_timer(0);
	last = start;
	while ((addr[0] & (CONFIG_SYS_FLASH_WORD_SIZE) 0x00800080) !=
			(CONFIG_SYS_FLASH_WORD_SIZE) 0x00800080) {
		if ((now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
			printf("Timeout\n");
			return -1;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) {	/* every second */
			putc('.');
			last = now;
		}
	}
	return 0;
}

int ext_flash_erase(flash_info_t * info, int s_first, int s_last)
{
	volatile CONFIG_SYS_FLASH_WORD_SIZE *addr = (CONFIG_SYS_FLASH_WORD_SIZE *) (info->start[0]);
	volatile CONFIG_SYS_FLASH_WORD_SIZE *addr2;
	int flag, prot, sect, l_sect, ret;

	ret = 0;
	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN)
			printf("- missing\n");
		else
			printf("- no sectors to erase\n");
		return 1;
	}

	if (info->flash_id == FLASH_UNKNOWN) {
		printf("Can't erase unknown flash type - aborted\n");
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect])
			prot++;
	}

	if (prot)
		printf("- Warning: %d protected sectors will not be erased!", prot);

	printf("\n");

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr2 = (CONFIG_SYS_FLASH_WORD_SIZE *) (info->start[sect]);

			addr[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00AA00AA;
			addr[CONFIG_SYS_FLASH_ADDR1] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00550055;
			addr[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00800080;
			addr[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00AA00AA;
			addr[CONFIG_SYS_FLASH_ADDR1] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00550055;
			addr2[0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00300030;	/* sector erase */

			l_sect = sect;
			/*
			 * Wait for each sector to complete, it's more
			 * reliable.  According to AMD Spec, you must
			 * issue all erase commands within a specified
			 * timeout.  This has been seen to fail, especially
			 * if printf()s are included (for debug)!!
			 */
			ret = wait_for_DQ7_32(info, sect);
			if (ret) {
				ret = ERR_PROTECTED;
				break;
			}
		}
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* wait at least 80us - let's wait 1 ms */
	udelay(1000);

	/* reset to read mode */
	addr = (CONFIG_SYS_FLASH_WORD_SIZE *) info->start[0];
	addr[0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00F000F0;	/* reset bank */

	if (ret)
		printf(" error\n");
	else
		printf(" done\n");
	return ret;
}

static ulong flash_get_size(vu_long * addr, flash_info_t * info)
{
	short i;
	CONFIG_SYS_FLASH_WORD_SIZE value;
	ulong base = (ulong) addr;
	volatile CONFIG_SYS_FLASH_WORD_SIZE *addr2 = (CONFIG_SYS_FLASH_WORD_SIZE *) addr;

	/* Write auto select command: read Manufacturer ID */
	addr2[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00AA00AA;
	addr2[CONFIG_SYS_FLASH_ADDR1] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00550055;
	addr2[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00900090;
	udelay(1000);

	value = addr2[0];

	switch (value) {
		case (CONFIG_SYS_FLASH_WORD_SIZE) AMD_MANUFACT:
			info->flash_id = FLASH_MAN_AMD;
			break;
		default:
			info->flash_id = FLASH_UNKNOWN;
			info->sector_count = 0;
			info->size = 0;
			return (0);	/* no or unknown flash  */
	}

	value = addr2[1];	/* device ID            */

	switch (value) {
		case (CONFIG_SYS_FLASH_WORD_SIZE)AMD_ID_MIRROR:
			value = addr2[14];
			switch(value) {
				case (CONFIG_SYS_FLASH_WORD_SIZE)AMD_ID_GL128N_2:
					value = addr2[15];
					if (value != (CONFIG_SYS_FLASH_WORD_SIZE)AMD_ID_GL128N_3) {
						info->flash_id = FLASH_UNKNOWN;
					} else {
						info->flash_id += FLASH_S29GL128N;
						info->sector_count = 128;
						info->size = 0x01000000;
					}
					break;
				default:
					info->flash_id = FLASH_UNKNOWN;
					return(0);
			}
			break;

		default:
			info->flash_id = FLASH_UNKNOWN;
			return (0);	/* => no or unknown flash */
	}

	/* set up sector start address table */
	for (i = 0; i < info->sector_count; i++)
		info->start[i] = base + (i * 0x00020000);

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr2 = (volatile CONFIG_SYS_FLASH_WORD_SIZE *)(info->start[i]);

		info->protect[i] = addr2[2] & 1;
	}

	/* issue bank reset to return to read mode */
	addr2[0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00F000F0;

	return (info->size);
}

static unsigned long ext_flash_init(void)
{
	unsigned long total_b = 0;
	unsigned long size_b[CONFIG_SYS_MAX_FLASH_BANKS];
	int i;

	/* Init: no FLASHes known */
	for (i = 1; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
		flash_info[i].sector_count = -1;
		flash_info[i].size = 0;

		/* call flash_get_size() to initialize sector address */
		size_b[i] = flash_get_size((vu_long *) flash_addr_table[i],
				&flash_info[i]);

		flash_info[i].size = size_b[i];

		if (flash_info[i].flash_id == FLASH_UNKNOWN) {
			printf("## Unknown FLASH on Bank %d - Size = 0x%08lx = %ld MB\n",
					i+1, size_b[i], size_b[i] << 20);
			flash_info[i].sector_count = -1;
			flash_info[i].size = 0;
		}

		total_b += flash_info[i].size;
	}

	return total_b;
}

static int write_word(flash_info_t * info, ulong dest, ushort data)
{
	volatile CONFIG_SYS_FLASH_WORD_SIZE *addr2 = (CONFIG_SYS_FLASH_WORD_SIZE *) (info->start[0]);
	volatile CONFIG_SYS_FLASH_WORD_SIZE *dest2 = (CONFIG_SYS_FLASH_WORD_SIZE *) dest;
	volatile CONFIG_SYS_FLASH_WORD_SIZE *data2 = (CONFIG_SYS_FLASH_WORD_SIZE *) &data;
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*dest2 & *data2) != *data2) {
		return (2);
	}

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	addr2[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00AA00AA;
	addr2[CONFIG_SYS_FLASH_ADDR1] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00550055;
	addr2[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00A000A0;
	*dest2 = *data2;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer(0);
	while ((*dest2 & (CONFIG_SYS_FLASH_WORD_SIZE) 0x00800080) !=
			(*data2 & (CONFIG_SYS_FLASH_WORD_SIZE) 0x00800080)) {

		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			printf("WRITE_TOUT\n");
			return (1);
		}
	}
	return (0);
}

/*-----------------------------------------------------------------------
 * This is taken from the original flash.c for the LPC2292 SODIMM board
 * and modified to suit.
 */

int ext_write_buff(flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ushort tmp;
	ulong i;
	uchar* src_org;
	uchar* dst_org;
	ulong cnt_org = cnt;
	int ret = ERR_OK;

	src_org = src;
	dst_org = (uchar*)addr;

	if (addr & 1) {		/* if odd address */
		tmp = *((uchar*)(addr - 1)); /* little endian */
		tmp |= (*src << 8);
		if (write_word(info, addr - 1, tmp))
			return ERR_PROG_ERROR;
		addr += 1;
		cnt -= 1;
		src++;
	}
	while (cnt > 1) {
		tmp = ((*(src+1)) << 8) + (*src); /* little endian */
		if (write_word(info, addr, tmp))
			return ERR_PROG_ERROR;
		addr += 2;
		src += 2;
		cnt -= 2;
	}
	if (cnt > 0) {
		tmp = (*((uchar*)(addr + 1))) << 8;
		tmp |= *src;
		if (write_word(info, addr, tmp))
			return ERR_PROG_ERROR;
	}

	for (i = 0; i < cnt_org; i++) {
		if (*dst_org != *src_org) {
			printf("Write failed. Byte %lX differs\n", i);
			ret = ERR_PROG_ERROR;
			break;
		}
		dst_org++;
		src_org++;
	}

	return ret;
}
