/*
 * (C) Copyright 2006
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
#include <mpc5xxx.h>
#include <asm/processor.h>

#if defined(CONFIG_CAM5200) && defined(CONFIG_CAM5200_NIOSFLASH)

#if 0
#define DEBUGF(x...) printf(x)
#else
#define DEBUGF(x...)
#endif

#define swap16(x) __swab16(x)

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];	/* info for FLASH chips */

/*
 * CAM5200 is a TQM5200B based board. Additionally it also features
 * a NIOS cpu. The NIOS CPU peripherals are accessible through MPC5xxx
 * Local Bus on CS5. This includes 32 bit wide RAM and SRAM as well as
 * 16 bit wide flash device. Big Endian order on a 32 bit CS5 makes
 * access to flash chip slightly more complicated as additional byte
 * swapping is necessary within each 16 bit wide flash 'word'.
 *
 * This driver's task is to handle both flash devices: 32 bit TQM5200B
 * flash chip and 16 bit NIOS cpu flash chip. In the below
 * flash_addr_table table we use least significant address bit to mark
 * 16 bit flash bank and two sets of routines *_32 and *_16 to handle
 * specifics of both flashes.
 */
static unsigned long flash_addr_table[][CONFIG_SYS_MAX_FLASH_BANKS] = {
	{CONFIG_SYS_BOOTCS_START, CONFIG_SYS_CS5_START | 1}
};

/*-----------------------------------------------------------------------
 * Functions
 */
static int write_word(flash_info_t * info, ulong dest, ulong data);
#ifdef CONFIG_SYS_FLASH_2ND_16BIT_DEV
static int write_word_32(flash_info_t * info, ulong dest, ulong data);
static int write_word_16(flash_info_t * info, ulong dest, ulong data);
static int flash_erase_32(flash_info_t * info, int s_first, int s_last);
static int flash_erase_16(flash_info_t * info, int s_first, int s_last);
static ulong flash_get_size_32(vu_long * addr, flash_info_t * info);
static ulong flash_get_size_16(vu_long * addr, flash_info_t * info);
#endif

void flash_print_info(flash_info_t * info)
{
	int i, k;
	int size, erased;
	volatile unsigned long *flash;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
		case FLASH_MAN_AMD:
			printf("AMD ");
			break;
		case FLASH_MAN_FUJ:
			printf("FUJITSU ");
			break;
		default:
			printf("Unknown Vendor ");
			break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
		case FLASH_S29GL128N:
			printf ("S29GL128N (256 Mbit, uniform sector size)\n");
			break;
		case FLASH_AM320B:
			printf ("29LV320B (32 Mbit, bottom boot sect)\n");
			break;
		case FLASH_AM320T:
			printf ("29LV320T (32 Mbit, top boot sect)\n");
			break;
		default:
			printf("Unknown Chip Type\n");
			break;
	}

	printf("  Size: %ld KB in %d Sectors\n",
			info->size >> 10, info->sector_count);

	printf("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
		/*
		 * Check if whole sector is erased
		 */
		if (i != (info->sector_count - 1))
			size = info->start[i + 1] - info->start[i];
		else
			size = info->start[0] + info->size - info->start[i];

		erased = 1;
		flash = (volatile unsigned long *)info->start[i];
		size = size >> 2;	/* divide by 4 for longword access */

		for (k = 0; k < size; k++) {
			if (*flash++ != 0xffffffff) {
				erased = 0;
				break;
			}
		}

		if ((i % 5) == 0)
			printf("\n   ");

		printf(" %08lX%s%s", info->start[i],
				erased ? " E" : "  ",
				info->protect[i] ? "RO " : "   ");
	}
	printf("\n");
	return;
}


/*
 * The following code cannot be run from FLASH!
 */
#ifdef CONFIG_SYS_FLASH_2ND_16BIT_DEV
static ulong flash_get_size(vu_long * addr, flash_info_t * info)
{

	DEBUGF("get_size: FLASH ADDR %08lx\n", addr);

	/* bit 0 used for big flash marking */
	if ((ulong)addr & 0x1)
		return flash_get_size_16((vu_long *)((ulong)addr & 0xfffffffe), info);
	else
		return flash_get_size_32(addr, info);
}

static ulong flash_get_size_32(vu_long * addr, flash_info_t * info)
#else
static ulong flash_get_size(vu_long * addr, flash_info_t * info)
#endif
{
	short i;
	CONFIG_SYS_FLASH_WORD_SIZE value;
	ulong base = (ulong) addr;
	volatile CONFIG_SYS_FLASH_WORD_SIZE *addr2 = (CONFIG_SYS_FLASH_WORD_SIZE *) addr;

	DEBUGF("get_size32: FLASH ADDR: %08x\n", (unsigned)addr);

	/* Write auto select command: read Manufacturer ID */
	addr2[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00AA00AA;
	addr2[CONFIG_SYS_FLASH_ADDR1] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00550055;
	addr2[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00900090;
	udelay(1000);

	value = addr2[0];
	DEBUGF("FLASH MANUFACT: %x\n", value);

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
	DEBUGF("\nFLASH DEVICEID: %x\n", value);

	switch (value) {
		case AMD_ID_MIRROR:
			DEBUGF("Mirror Bit flash: addr[14] = %08lX  addr[15] = %08lX\n",
					addr[14], addr[15]);
			switch(addr[14]) {
				case AMD_ID_GL128N_2:
					if (addr[15] != AMD_ID_GL128N_3) {
						DEBUGF("Chip: S29GL128N -> unknown\n");
						info->flash_id = FLASH_UNKNOWN;
					} else {
						DEBUGF("Chip: S29GL128N\n");
						info->flash_id += FLASH_S29GL128N;
						info->sector_count = 128;
						info->size = 0x02000000;
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
		info->start[i] = base + (i * 0x00040000);

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

#ifdef CONFIG_SYS_FLASH_2ND_16BIT_DEV
int flash_erase(flash_info_t * info, int s_first, int s_last)
{
	if ((info->flash_id & FLASH_TYPEMASK) == FLASH_AM320B) {
		return flash_erase_16(info, s_first, s_last);
	} else {
		return flash_erase_32(info, s_first, s_last);
	}
}

static int flash_erase_32(flash_info_t * info, int s_first, int s_last)
#else
int flash_erase(flash_info_t * info, int s_first, int s_last)
#endif
{
	volatile CONFIG_SYS_FLASH_WORD_SIZE *addr = (CONFIG_SYS_FLASH_WORD_SIZE *) (info->start[0]);
	volatile CONFIG_SYS_FLASH_WORD_SIZE *addr2;
	int flag, prot, sect, l_sect;

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
			wait_for_DQ7_32(info, sect);
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

	printf(" done\n");
	return 0;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff(flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int i, l, rc;

	wp = (addr & ~3);	/* get lower word aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i = 0, cp = wp; i < l; ++i, ++cp)
			data = (data << 8) | (*(uchar *) cp);

		for (; i < 4 && cnt > 0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}

		for (; cnt == 0 && i < 4; ++i, ++cp)
			data = (data << 8) | (*(uchar *) cp);

		if ((rc = write_word(info, wp, data)) != 0)
			return (rc);

		wp += 4;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= 4) {
		data = 0;
		for (i = 0; i < 4; ++i)
			data = (data << 8) | *src++;

		if ((rc = write_word(info, wp, data)) != 0)
			return (rc);

		wp += 4;
		cnt -= 4;
	}

	if (cnt == 0)
		return (0);

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i = 0, cp = wp; i < 4 && cnt > 0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i < 4; ++i, ++cp)
		data = (data << 8) | (*(uchar *) cp);

	return (write_word(info, wp, data));
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
#ifdef CONFIG_SYS_FLASH_2ND_16BIT_DEV
static int write_word(flash_info_t * info, ulong dest, ulong data)
{
	if ((info->flash_id & FLASH_TYPEMASK) == FLASH_AM320B) {
		return write_word_16(info, dest, data);
	} else {
		return write_word_32(info, dest, data);
	}
}

static int write_word_32(flash_info_t * info, ulong dest, ulong data)
#else
static int write_word(flash_info_t * info, ulong dest, ulong data)
#endif
{
	volatile CONFIG_SYS_FLASH_WORD_SIZE *addr2 = (CONFIG_SYS_FLASH_WORD_SIZE *) (info->start[0]);
	volatile CONFIG_SYS_FLASH_WORD_SIZE *dest2 = (CONFIG_SYS_FLASH_WORD_SIZE *) dest;
	ulong *datap = &data;
	volatile CONFIG_SYS_FLASH_WORD_SIZE *data2 =
			(volatile CONFIG_SYS_FLASH_WORD_SIZE *)datap;
	ulong start;
	int i, flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*((vu_long *)dest) & data) != data)
		return (2);

	for (i = 0; i < 4 / sizeof(CONFIG_SYS_FLASH_WORD_SIZE); i++) {
		/* Disable interrupts which might cause a timeout here */
		flag = disable_interrupts();

		addr2[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00AA00AA;
		addr2[CONFIG_SYS_FLASH_ADDR1] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00550055;
		addr2[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x00A000A0;

		dest2[i] = data2[i];

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts();

		/* data polling for D7 */
		start = get_timer(0);
		while ((dest2[i] & (CONFIG_SYS_FLASH_WORD_SIZE) 0x00800080) !=
				(data2[i] & (CONFIG_SYS_FLASH_WORD_SIZE) 0x00800080)) {

			if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT)
				return (1);
		}
	}

	return (0);
}

#ifdef CONFIG_SYS_FLASH_2ND_16BIT_DEV

#undef  CONFIG_SYS_FLASH_WORD_SIZE
#define CONFIG_SYS_FLASH_WORD_SIZE unsigned short

/*
 * The following code cannot be run from FLASH!
 */
static ulong flash_get_size_16(vu_long * addr, flash_info_t * info)
{
	short i;
	CONFIG_SYS_FLASH_WORD_SIZE value;
	ulong base = (ulong) addr;
	volatile CONFIG_SYS_FLASH_WORD_SIZE *addr2 = (CONFIG_SYS_FLASH_WORD_SIZE *) addr;

	DEBUGF("get_size16: FLASH ADDR: %08x\n", (unsigned)addr);

	/* issue bank reset to return to read mode */
	addr2[0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0xF000F000;

	/* Write auto select command: read Manufacturer ID */
	addr2[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0xAA00AA00;
	addr2[CONFIG_SYS_FLASH_ADDR1] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x55005500;
	addr2[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x90009000;
	udelay(1000);

	value = swap16(addr2[0]);
	DEBUGF("FLASH MANUFACT: %x\n", value);

	switch (value) {
		case (CONFIG_SYS_FLASH_WORD_SIZE) AMD_MANUFACT:
			info->flash_id = FLASH_MAN_AMD;
			break;
		case (CONFIG_SYS_FLASH_WORD_SIZE) FUJ_MANUFACT:
			info->flash_id = FLASH_MAN_FUJ;
			break;
		default:
			info->flash_id = FLASH_UNKNOWN;
			info->sector_count = 0;
			info->size = 0;
			return (0);	/* no or unknown flash  */
	}

	value = swap16(addr2[1]);	/* device ID            */
	DEBUGF("\nFLASH DEVICEID: %x\n", value);

	switch (value) {
		case (CONFIG_SYS_FLASH_WORD_SIZE)AMD_ID_LV320B:
			info->flash_id += FLASH_AM320B;
			info->sector_count = 71;
			info->size = 0x00400000;
			break;	/* => 4 MB	*/
		case (CONFIG_SYS_FLASH_WORD_SIZE)AMD_ID_LV320T:
			info->flash_id += FLASH_AM320T;
			info->sector_count = 71;
			info->size = 0x00400000;
			break;	/* => 4 MB	*/
		default:
			info->flash_id = FLASH_UNKNOWN;
			return (0);	/* => no or unknown flash */
	}

	if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block type        */
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00002000;
		info->start[2] = base + 0x00004000;
		info->start[3] = base + 0x00006000;
		info->start[4] = base + 0x00008000;
		info->start[5] = base + 0x0000a000;
		info->start[6] = base + 0x0000c000;
		info->start[7] = base + 0x0000e000;

		for (i = 8; i < info->sector_count; i++)
			info->start[i] = base + (i * 0x00010000) - 0x00070000;
	} else {
		/* set sector offsets for top boot block type           */
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00002000;
		info->start[i--] = base + info->size - 0x00004000;
		info->start[i--] = base + info->size - 0x00006000;
		info->start[i--] = base + info->size - 0x00008000;
		info->start[i--] = base + info->size - 0x0000a000;
		info->start[i--] = base + info->size - 0x0000c000;
		info->start[i--] = base + info->size - 0x0000e000;

		for (; i >= 0; i--)
			info->start[i] = base + i * 0x00010000;
	}

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr2 = (volatile CONFIG_SYS_FLASH_WORD_SIZE *)(info->start[i]);

		info->protect[i] = addr2[2] & 1;
	}

	/* issue bank reset to return to read mode */
	addr2[0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0xF000F000;

	return (info->size);
}

static int wait_for_DQ7_16(flash_info_t * info, int sect)
{
	ulong start, now, last;
	volatile CONFIG_SYS_FLASH_WORD_SIZE *addr =
		(CONFIG_SYS_FLASH_WORD_SIZE *) (info->start[sect]);

	start = get_timer(0);
	last = start;
	while ((addr[0] & (CONFIG_SYS_FLASH_WORD_SIZE) 0x80008000) !=
			(CONFIG_SYS_FLASH_WORD_SIZE) 0x80008000) {
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

static int flash_erase_16(flash_info_t * info, int s_first, int s_last)
{
	volatile CONFIG_SYS_FLASH_WORD_SIZE *addr = (CONFIG_SYS_FLASH_WORD_SIZE *) (info->start[0]);
	volatile CONFIG_SYS_FLASH_WORD_SIZE *addr2;
	int flag, prot, sect, l_sect;

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
		printf("- Warning: %d protected sectors will not be erased!",	prot);

	printf("\n");

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr2 = (CONFIG_SYS_FLASH_WORD_SIZE *) (info->start[sect]);

			addr[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0xAA00AA00;
			addr[CONFIG_SYS_FLASH_ADDR1] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x55005500;
			addr[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x80008000;
			addr[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0xAA00AA00;
			addr[CONFIG_SYS_FLASH_ADDR1] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x55005500;
			addr2[0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x30003000;	/* sector erase */

			l_sect = sect;
			/*
			 * Wait for each sector to complete, it's more
			 * reliable.  According to AMD Spec, you must
			 * issue all erase commands within a specified
			 * timeout.  This has been seen to fail, especially
			 * if printf()s are included (for debug)!!
			 */
			wait_for_DQ7_16(info, sect);
		}
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* wait at least 80us - let's wait 1 ms */
	udelay(1000);

	/* reset to read mode */
	addr = (CONFIG_SYS_FLASH_WORD_SIZE *) info->start[0];
	addr[0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0xF000F000;	/* reset bank */

	printf(" done\n");
	return 0;
}

static int write_word_16(flash_info_t * info, ulong dest, ulong data)
{
	volatile CONFIG_SYS_FLASH_WORD_SIZE *addr2 = (CONFIG_SYS_FLASH_WORD_SIZE *) (info->start[0]);
	volatile CONFIG_SYS_FLASH_WORD_SIZE *dest2 = (CONFIG_SYS_FLASH_WORD_SIZE *) dest;
	ulong *datap = &data;
	volatile CONFIG_SYS_FLASH_WORD_SIZE *data2 =
			(volatile CONFIG_SYS_FLASH_WORD_SIZE *)datap;
	ulong start;
	int i;

	/* Check if Flash is (sufficiently) erased */
	for (i = 0; i < 4 / sizeof(CONFIG_SYS_FLASH_WORD_SIZE); i++) {
		if ((dest2[i] & swap16(data2[i])) != swap16(data2[i]))
			return (2);
	}

	for (i = 0; i < 4 / sizeof(CONFIG_SYS_FLASH_WORD_SIZE); i++) {
		int flag;

		/* Disable interrupts which might cause a timeout here */
		flag = disable_interrupts();

		addr2[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0xAA00AA00;
		addr2[CONFIG_SYS_FLASH_ADDR1] = (CONFIG_SYS_FLASH_WORD_SIZE) 0x55005500;
		addr2[CONFIG_SYS_FLASH_ADDR0] = (CONFIG_SYS_FLASH_WORD_SIZE) 0xA000A000;

		dest2[i] = swap16(data2[i]);

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts();

		/* data polling for D7 */
		start = get_timer(0);
		while ((dest2[i] & (CONFIG_SYS_FLASH_WORD_SIZE) 0x80008000) !=
				(swap16(data2[i]) & (CONFIG_SYS_FLASH_WORD_SIZE) 0x80008000)) {

			if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
				return (1);
			}
		}
	}

	return (0);
}
#endif /* CONFIG_SYS_FLASH_2ND_16BIT_DEV */

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size(vu_long * addr, flash_info_t * info);
static int write_word(flash_info_t * info, ulong dest, ulong data);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init(void)
{
	unsigned long total_b = 0;
	unsigned long size_b[CONFIG_SYS_MAX_FLASH_BANKS];
	unsigned short index = 0;
	int i;

	DEBUGF("\n");
	DEBUGF("FLASH: Index: %d\n", index);

	/* Init: no FLASHes known */
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
		flash_info[i].sector_count = -1;
		flash_info[i].size = 0;

		/* check whether the address is 0 */
		if (flash_addr_table[index][i] == 0)
			continue;

		/* call flash_get_size() to initialize sector address */
		size_b[i] = flash_get_size((vu_long *) flash_addr_table[index][i],
				&flash_info[i]);

		flash_info[i].size = size_b[i];

		if (flash_info[i].flash_id == FLASH_UNKNOWN) {
			printf("## Unknown FLASH on Bank %d - Size = 0x%08lx = %ld MB\n",
					i+1, size_b[i], size_b[i] << 20);
			flash_info[i].sector_count = -1;
			flash_info[i].size = 0;
		}

		/* Monitor protection ON by default */
		(void)flash_protect(FLAG_PROTECT_SET, CONFIG_SYS_MONITOR_BASE,
				    CONFIG_SYS_MONITOR_BASE + CONFIG_SYS_MONITOR_LEN - 1,
				    &flash_info[i]);
#if defined(CONFIG_ENV_IS_IN_FLASH)
		(void)flash_protect(FLAG_PROTECT_SET, CONFIG_ENV_ADDR,
				    CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1,
				    &flash_info[i]);
#if defined(CONFIG_ENV_ADDR_REDUND)
		(void)flash_protect(FLAG_PROTECT_SET, CONFIG_ENV_ADDR_REDUND,
				    CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SECT_SIZE - 1,
				    &flash_info[i]);
#endif
#endif
		total_b += flash_info[i].size;
	}

	return total_b;
}
#endif /* if defined(CONFIG_CAM5200) && defined(CONFIG_CAM5200_NIOSFLASH) */
