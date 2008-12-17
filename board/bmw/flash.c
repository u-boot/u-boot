/*
 * (C) Copyright 2000
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
#include <mpc824x.h>
#include <asm/processor.h>
#include <asm/pci_io.h>

#define ROM_CS0_START	0xFF800000
#define ROM_CS1_START	0xFF000000

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];	/* info for FLASH chips    */

#if defined(CONFIG_ENV_IS_IN_FLASH)
# ifndef  CONFIG_ENV_ADDR
#  define CONFIG_ENV_ADDR  (CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
# endif
# ifndef  CONFIG_ENV_SIZE
#  define CONFIG_ENV_SIZE  CONFIG_ENV_SECT_SIZE
# endif
# ifndef  CONFIG_ENV_SECT_SIZE
#  define CONFIG_ENV_SECT_SIZE  CONFIG_ENV_SIZE
# endif
#endif

/*-----------------------------------------------------------------------
 * Functions
 */
static int write_word (flash_info_t * info, ulong dest, ulong data);

#if 0
static void flash_get_offsets (ulong base, flash_info_t * info);
#endif /* 0 */

/*flash command address offsets*/

#if 0
#define ADDR0           (0x555)
#define ADDR1           (0x2AA)
#define ADDR3           (0x001)
#else
#define ADDR0		(0xAAA)
#define ADDR1		(0x555)
#define ADDR3		(0x001)
#endif

#define FLASH_WORD_SIZE unsigned char

/*-----------------------------------------------------------------------
 */

#if 0
static int byte_parity_odd (unsigned char x) __attribute__ ((const));
#endif /* 0 */
static unsigned long flash_id (unsigned char mfct, unsigned char chip)
	__attribute__ ((const));

typedef struct {
	FLASH_WORD_SIZE extval;
	unsigned short intval;
} map_entry;

#if 0
static int byte_parity_odd (unsigned char x)
{
	x ^= x >> 4;
	x ^= x >> 2;
	x ^= x >> 1;
	return (x & 0x1) != 0;
}
#endif /* 0 */


static unsigned long flash_id (unsigned char mfct, unsigned char chip)
{
	static const map_entry mfct_map[] = {
		{(FLASH_WORD_SIZE) AMD_MANUFACT,
		 (unsigned short) ((unsigned long) FLASH_MAN_AMD >> 16)},
		{(FLASH_WORD_SIZE) FUJ_MANUFACT,
		 (unsigned short) ((unsigned long) FLASH_MAN_FUJ >> 16)},
		{(FLASH_WORD_SIZE) STM_MANUFACT,
		 (unsigned short) ((unsigned long) FLASH_MAN_STM >> 16)},
		{(FLASH_WORD_SIZE) MT_MANUFACT,
		 (unsigned short) ((unsigned long) FLASH_MAN_MT >> 16)},
		{(FLASH_WORD_SIZE) INTEL_MANUFACT,
		 (unsigned short) ((unsigned long) FLASH_MAN_INTEL >> 16)},
		{(FLASH_WORD_SIZE) INTEL_ALT_MANU,
		 (unsigned short) ((unsigned long) FLASH_MAN_INTEL >> 16)}
	};

	static const map_entry chip_map[] = {
		{AMD_ID_F040B, FLASH_AM040},
		{(FLASH_WORD_SIZE) STM_ID_x800AB, FLASH_STM800AB}
	};

	const map_entry *p;
	unsigned long result = FLASH_UNKNOWN;

	/* find chip id */
	for (p = &chip_map[0];
	     p < &chip_map[sizeof chip_map / sizeof chip_map[0]]; p++)
		if (p->extval == chip) {
			result = FLASH_VENDMASK | p->intval;
			break;
		}

	/* find vendor id */
	for (p = &mfct_map[0];
	     p < &mfct_map[sizeof mfct_map / sizeof mfct_map[0]]; p++)
		if (p->extval == mfct) {
			result &= ~FLASH_VENDMASK;
			result |= (unsigned long) p->intval << 16;
			break;
		}

	return result;
}


unsigned long flash_init (void)
{
	unsigned long i;
	unsigned char j;
	static const ulong flash_banks[] = CONFIG_SYS_FLASH_BANKS;

	/* Init: no FLASHes known */
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		flash_info_t *const pflinfo = &flash_info[i];

		pflinfo->flash_id = FLASH_UNKNOWN;
		pflinfo->size = 0;
		pflinfo->sector_count = 0;
	}

	for (i = 0; i < sizeof flash_banks / sizeof flash_banks[0]; i++) {
		flash_info_t *const pflinfo = &flash_info[i];
		const unsigned long base_address = flash_banks[i];
		volatile FLASH_WORD_SIZE *const flash =
			(FLASH_WORD_SIZE *) base_address;
#if 0
		volatile FLASH_WORD_SIZE *addr2;
#endif
#if 0
		/* write autoselect sequence */
		flash[0x5555] = 0xaa;
		flash[0x2aaa] = 0x55;
		flash[0x5555] = 0x90;
#else
		flash[0xAAA << (3 * i)] = 0xaa;
		flash[0x555 << (3 * i)] = 0x55;
		flash[0xAAA << (3 * i)] = 0x90;
#endif
		__asm__ __volatile__ ("sync");

#if 0
		pflinfo->flash_id = flash_id (flash[0x0], flash[0x1]);
#else
		pflinfo->flash_id =
			flash_id (flash[0x0], flash[0x2 + 14 * i]);
#endif

		switch (pflinfo->flash_id & FLASH_TYPEMASK) {
		case FLASH_AM040:
			pflinfo->size = 0x00080000;
			pflinfo->sector_count = 8;
			for (j = 0; j < 8; j++) {
				pflinfo->start[j] =
					base_address + 0x00010000 * j;
				pflinfo->protect[j] = flash[(j << 16) | 0x2];
			}
			break;
		case FLASH_STM800AB:
			pflinfo->size = 0x00100000;
			pflinfo->sector_count = 19;
			pflinfo->start[0] = base_address;
			pflinfo->start[1] = base_address + 0x4000;
			pflinfo->start[2] = base_address + 0x6000;
			pflinfo->start[3] = base_address + 0x8000;
			for (j = 1; j < 16; j++) {
				pflinfo->start[j + 3] =
					base_address + 0x00010000 * j;
			}
#if 0
			/* check for protected sectors */
			for (j = 0; j < pflinfo->sector_count; j++) {
				/* read sector protection at sector address, (A7 .. A0) = 0x02 */
				/* D0 = 1 if protected */
				addr2 = (volatile FLASH_WORD_SIZE
					 *) (pflinfo->start[j]);
				if (pflinfo->flash_id & FLASH_MAN_SST)
					pflinfo->protect[j] = 0;
				else
					pflinfo->protect[j] = addr2[2] & 1;
			}
#endif
			break;
		}
		/* Protect monitor and environment sectors
		 */
#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
		flash_protect (FLAG_PROTECT_SET,
			       CONFIG_SYS_MONITOR_BASE,
			       CONFIG_SYS_MONITOR_BASE + monitor_flash_len - 1,
			       &flash_info[0]);
#endif

#if defined(CONFIG_ENV_IS_IN_FLASH) && defined(CONFIG_ENV_ADDR)
		flash_protect (FLAG_PROTECT_SET,
			       CONFIG_ENV_ADDR,
			       CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1,
			       &flash_info[0]);
#endif

		/* reset device to read mode */
		flash[0x0000] = 0xf0;
		__asm__ __volatile__ ("sync");
	}

	return flash_info[0].size + flash_info[1].size;
}

#if 0
static void flash_get_offsets (ulong base, flash_info_t * info)
{
	int i;

	/* set up sector start address table */
	if (info->flash_id & FLASH_MAN_SST) {
		for (i = 0; i < info->sector_count; i++)
			info->start[i] = base + (i * 0x00010000);
	} else if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block type    */
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00004000;
		info->start[2] = base + 0x00006000;
		info->start[3] = base + 0x00008000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00010000) - 0x00030000;
		}
	} else {
		/* set sector offsets for top boot block type       */
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00004000;
		info->start[i--] = base + info->size - 0x00006000;
		info->start[i--] = base + info->size - 0x00008000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00010000;
		}
	}

}
#endif /* 0 */

/*-----------------------------------------------------------------------
 */
void flash_print_info (flash_info_t * info)
{
	static const char unk[] = "Unknown";
	const char *mfct = unk, *type = unk;
	unsigned int i;

	if (info->flash_id != FLASH_UNKNOWN) {
		switch (info->flash_id & FLASH_VENDMASK) {
		case FLASH_MAN_AMD:
			mfct = "AMD";
			break;
		case FLASH_MAN_FUJ:
			mfct = "FUJITSU";
			break;
		case FLASH_MAN_STM:
			mfct = "STM";
			break;
		case FLASH_MAN_SST:
			mfct = "SST";
			break;
		case FLASH_MAN_BM:
			mfct = "Bright Microelectonics";
			break;
		case FLASH_MAN_INTEL:
			mfct = "Intel";
			break;
		}

		switch (info->flash_id & FLASH_TYPEMASK) {
		case FLASH_AM040:
			type = "AM29F040B (512K * 8, uniform sector size)";
			break;
		case FLASH_AM400B:
			type = "AM29LV400B (4 Mbit, bottom boot sect)";
			break;
		case FLASH_AM400T:
			type = "AM29LV400T (4 Mbit, top boot sector)";
			break;
		case FLASH_AM800B:
			type = "AM29LV800B (8 Mbit, bottom boot sect)";
			break;
		case FLASH_AM800T:
			type = "AM29LV800T (8 Mbit, top boot sector)";
			break;
		case FLASH_AM160T:
			type = "AM29LV160T (16 Mbit, top boot sector)";
			break;
		case FLASH_AM320B:
			type = "AM29LV320B (32 Mbit, bottom boot sect)";
			break;
		case FLASH_AM320T:
			type = "AM29LV320T (32 Mbit, top boot sector)";
			break;
		case FLASH_STM800AB:
			type = "M29W800AB (8 Mbit, bottom boot sect)";
			break;
		case FLASH_SST800A:
			type = "SST39LF/VF800 (8 Mbit, uniform sector size)";
			break;
		case FLASH_SST160A:
			type = "SST39LF/VF160 (16 Mbit, uniform sector size)";
			break;
		}
	}

	printf ("\n  Brand: %s Type: %s\n"
		"  Size: %lu KB in %d Sectors\n",
		mfct, type, info->size >> 10, info->sector_count);

	printf ("  Sector Start Addresses:");

	for (i = 0; i < info->sector_count; i++) {
		unsigned long size;
		unsigned int erased;
		unsigned long *flash = (unsigned long *) info->start[i];

		/*
		 * Check if whole sector is erased
		 */
		size = (i != (info->sector_count - 1)) ?
			(info->start[i + 1] - info->start[i]) >> 2 :
			(info->start[0] + info->size - info->start[i]) >> 2;

		for (flash = (unsigned long *) info->start[i], erased = 1;
		     (flash != (unsigned long *) info->start[i] + size)
		     && erased; flash++)
			erased = *flash == ~0x0UL;

		printf ("%s %08lX %s %s",
			(i % 5) ? "" : "\n   ",
			info->start[i],
			erased ? "E" : " ", info->protect[i] ? "RO" : "  ");
	}

	puts ("\n");
	return;
}

#if 0

/*
 * The following code cannot be run from FLASH!
 */
ulong flash_get_size (vu_long * addr, flash_info_t * info)
{
	short i;
	FLASH_WORD_SIZE value;
	ulong base = (ulong) addr;
	volatile FLASH_WORD_SIZE *addr2 = (FLASH_WORD_SIZE *) addr;

	printf ("flash_get_size: \n");
	/* Write auto select command: read Manufacturer ID */
	eieio ();
	addr2[ADDR0] = (FLASH_WORD_SIZE) 0xAA;
	addr2[ADDR1] = (FLASH_WORD_SIZE) 0x55;
	addr2[ADDR0] = (FLASH_WORD_SIZE) 0x90;
	value = addr2[0];

	switch (value) {
	case (FLASH_WORD_SIZE) AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
		break;
	case (FLASH_WORD_SIZE) FUJ_MANUFACT:
		info->flash_id = FLASH_MAN_FUJ;
		break;
	case (FLASH_WORD_SIZE) SST_MANUFACT:
		info->flash_id = FLASH_MAN_SST;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);	/* no or unknown flash  */
	}
	printf ("recognised manufacturer");

	value = addr2[ADDR3];	/* device ID        */
	debug ("\ndev_code=%x\n", value);

	switch (value) {
	case (FLASH_WORD_SIZE) AMD_ID_LV400T:
		info->flash_id += FLASH_AM400T;
		info->sector_count = 11;
		info->size = 0x00080000;
		break;		/* => 0.5 MB        */

	case (FLASH_WORD_SIZE) AMD_ID_LV400B:
		info->flash_id += FLASH_AM400B;
		info->sector_count = 11;
		info->size = 0x00080000;
		break;		/* => 0.5 MB        */

	case (FLASH_WORD_SIZE) AMD_ID_LV800T:
		info->flash_id += FLASH_AM800T;
		info->sector_count = 19;
		info->size = 0x00100000;
		break;		/* => 1 MB      */

	case (FLASH_WORD_SIZE) AMD_ID_LV800B:
		info->flash_id += FLASH_AM800B;
		info->sector_count = 19;
		info->size = 0x00100000;
		break;		/* => 1 MB      */

	case (FLASH_WORD_SIZE) AMD_ID_LV160T:
		info->flash_id += FLASH_AM160T;
		info->sector_count = 35;
		info->size = 0x00200000;
		break;		/* => 2 MB      */

	case (FLASH_WORD_SIZE) AMD_ID_LV160B:
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00200000;
		break;		/* => 2 MB      */

	case (FLASH_WORD_SIZE) SST_ID_xF800A:
		info->flash_id += FLASH_SST800A;
		info->sector_count = 16;
		info->size = 0x00100000;
		break;		/* => 1 MB      */

	case (FLASH_WORD_SIZE) SST_ID_xF160A:
		info->flash_id += FLASH_SST160A;
		info->sector_count = 32;
		info->size = 0x00200000;
		break;		/* => 2 MB      */

	case (FLASH_WORD_SIZE) AMD_ID_F040B:
		info->flash_id += FLASH_AM040;
		info->sector_count = 8;
		info->size = 0x00080000;
		break;		/* => 0.5 MB      */

	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);	/* => no or unknown flash */

	}

	printf ("flash id %lx; sector count %x, size %lx\n", info->flash_id,
		info->sector_count, info->size);
	/* set up sector start address table */
	if (info->flash_id & FLASH_MAN_SST) {
		for (i = 0; i < info->sector_count; i++)
			info->start[i] = base + (i * 0x00010000);
	} else if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block type    */
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00004000;
		info->start[2] = base + 0x00006000;
		info->start[3] = base + 0x00008000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00010000) - 0x00030000;
		}
	} else {
		/* set sector offsets for top boot block type       */
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00004000;
		info->start[i--] = base + info->size - 0x00006000;
		info->start[i--] = base + info->size - 0x00008000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00010000;
		}
	}

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr2 = (volatile FLASH_WORD_SIZE *) (info->start[i]);
		if (info->flash_id & FLASH_MAN_SST)
			info->protect[i] = 0;
		else
			info->protect[i] = addr2[2] & 1;
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr2 = (FLASH_WORD_SIZE *) info->start[0];
		*addr2 = (FLASH_WORD_SIZE) 0x00F000F0;	/* reset bank */
	}

	return (info->size);
}

#endif


int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	volatile FLASH_WORD_SIZE *addr = (FLASH_WORD_SIZE *) (info->start[0]);
	int flag, prot, sect, l_sect;
	ulong start, now, last;
	unsigned char sh8b;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	if ((info->flash_id == FLASH_UNKNOWN) ||
	    (info->flash_id > (FLASH_MAN_STM | FLASH_AMD_COMP))) {
		printf ("Can't erase unknown flash type - aborted\n");
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf ("- Warning: %d protected sectors will not be erased!\n", prot);
	} else {
		printf ("\n");
	}

	l_sect = -1;

	/* Check the ROM CS */
	if ((info->start[0] >= ROM_CS1_START)
	    && (info->start[0] < ROM_CS0_START))
		sh8b = 3;
	else
		sh8b = 0;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	addr[ADDR0 << sh8b] = (FLASH_WORD_SIZE) 0x00AA00AA;
	addr[ADDR1 << sh8b] = (FLASH_WORD_SIZE) 0x00550055;
	addr[ADDR0 << sh8b] = (FLASH_WORD_SIZE) 0x00800080;
	addr[ADDR0 << sh8b] = (FLASH_WORD_SIZE) 0x00AA00AA;
	addr[ADDR1 << sh8b] = (FLASH_WORD_SIZE) 0x00550055;

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = (FLASH_WORD_SIZE *) (info->start[0] + ((info->
								       start
								       [sect]
								       -
								       info->
								       start
								       [0]) <<
								      sh8b));
			if (info->flash_id & FLASH_MAN_SST) {
				addr[ADDR0 << sh8b] =
					(FLASH_WORD_SIZE) 0x00AA00AA;
				addr[ADDR1 << sh8b] =
					(FLASH_WORD_SIZE) 0x00550055;
				addr[ADDR0 << sh8b] =
					(FLASH_WORD_SIZE) 0x00800080;
				addr[ADDR0 << sh8b] =
					(FLASH_WORD_SIZE) 0x00AA00AA;
				addr[ADDR1 << sh8b] =
					(FLASH_WORD_SIZE) 0x00550055;
				addr[0] = (FLASH_WORD_SIZE) 0x00500050;	/* block erase */
				udelay (30000);	/* wait 30 ms */
			} else
				addr[0] = (FLASH_WORD_SIZE) 0x00300030;	/* sector erase */
			l_sect = sect;
		}
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();

	/* wait at least 80us - let's wait 1 ms */
	udelay (1000);

	/*
	 * We wait for the last triggered sector
	 */
	if (l_sect < 0)
		goto DONE;

	start = get_timer (0);
	last = start;
	addr = (FLASH_WORD_SIZE *) (info->start[0] + ((info->start[l_sect] -
						       info->
						       start[0]) << sh8b));
	while ((addr[0] & (FLASH_WORD_SIZE) 0x00800080) !=
	       (FLASH_WORD_SIZE) 0x00800080) {
		if ((now = get_timer (start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
			printf ("Timeout\n");
			return 1;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) {	/* every second */
			serial_putc ('.');
			last = now;
		}
	}

      DONE:
	/* reset to read mode */
	addr = (FLASH_WORD_SIZE *) info->start[0];
	addr[0] = (FLASH_WORD_SIZE) 0x00F000F0;	/* reset bank */

	printf (" done\n");
	return 0;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int i, l, rc;

	wp = (addr & ~3);	/* get lower word aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i = 0, cp = wp; i < l; ++i, ++cp) {
			data = (data << 8) | (*(uchar *) cp);
		}
		for (; i < 4 && cnt > 0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt == 0 && i < 4; ++i, ++cp) {
			data = (data << 8) | (*(uchar *) cp);
		}

		if ((rc = write_word (info, wp, data)) != 0) {
			return (rc);
		}
		wp += 4;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= 4) {
		data = 0;
		for (i = 0; i < 4; ++i) {
			data = (data << 8) | *src++;
		}
		if ((rc = write_word (info, wp, data)) != 0) {
			return (rc);
		}
		wp += 4;
		cnt -= 4;
	}

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i = 0, cp = wp; i < 4 && cnt > 0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i < 4; ++i, ++cp) {
		data = (data << 8) | (*(uchar *) cp);
	}

	return (write_word (info, wp, data));
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word (flash_info_t * info, ulong dest, ulong data)
{
	volatile FLASH_WORD_SIZE *addr2 = (FLASH_WORD_SIZE *) info->start[0];
	volatile FLASH_WORD_SIZE *dest2;
	volatile FLASH_WORD_SIZE *data2 = (FLASH_WORD_SIZE *) & data;
	ulong start;
	int flag;
	int i;
	unsigned char sh8b;

	/* Check the ROM CS */
	if ((info->start[0] >= ROM_CS1_START)
	    && (info->start[0] < ROM_CS0_START))
		sh8b = 3;
	else
		sh8b = 0;

	dest2 = (FLASH_WORD_SIZE *) (((dest - info->start[0]) << sh8b) +
				     info->start[0]);

	/* Check if Flash is (sufficiently) erased */
	if ((*dest2 & (FLASH_WORD_SIZE) data) != (FLASH_WORD_SIZE) data) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	for (i = 0; i < 4 / sizeof (FLASH_WORD_SIZE); i++) {
		addr2[ADDR0 << sh8b] = (FLASH_WORD_SIZE) 0x00AA00AA;
		addr2[ADDR1 << sh8b] = (FLASH_WORD_SIZE) 0x00550055;
		addr2[ADDR0 << sh8b] = (FLASH_WORD_SIZE) 0x00A000A0;

		dest2[i << sh8b] = data2[i];

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts ();

		/* data polling for D7 */
		start = get_timer (0);
		while ((dest2[i << sh8b] & (FLASH_WORD_SIZE) 0x00800080) !=
		       (data2[i] & (FLASH_WORD_SIZE) 0x00800080)) {
			if (get_timer (start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
				return (1);
			}
		}
	}

	return (0);
}

/*-----------------------------------------------------------------------
 */
