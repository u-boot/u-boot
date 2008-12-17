/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Gregory E. Allen, gallen@arlut.utexas.edu
 * Matthew E. Karger, karger@arlut.utexas.edu
 * Applied Research Laboratories, The University of Texas at Austin
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

#define ROM_CS0_START	0xFF800000
#define ROM_CS1_START	0xFF000000

#if defined(CONFIG_ENV_IS_IN_FLASH)
# ifndef  CONFIG_ENV_ADDR
#  define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
# endif
# ifndef  CONFIG_ENV_SIZE
#  define CONFIG_ENV_SIZE	CONFIG_ENV_SECT_SIZE
# endif
# ifndef  CONFIG_ENV_SECT_SIZE
#  define CONFIG_ENV_SECT_SIZE  CONFIG_ENV_SIZE
# endif
#endif

#define	FLASH_BANK_SIZE	((uint)(16 * 1024 * 1024))	/* max 16Mbyte */
#define	MAIN_SECT_SIZE	0x10000
#define	SECT_SIZE_32KB	0x8000
#define	SECT_SIZE_8KB	0x2000

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

static int write_word (flash_info_t * info, ulong dest, ulong data);
#if 0
static void write_via_fpu (vu_long * addr, ulong * data);
#endif
static __inline__ unsigned long get_msr (void);
static __inline__ void set_msr (unsigned long msr);

/*flash command address offsets*/
#define ADDR0		(0x555)
#define ADDR1		(0xAAA)
#define ADDR3		(0x001)

#define FLASH_WORD_SIZE unsigned char

/*---------------------------------------------------------------------*/
/*#define	DEBUG_FLASH	1 */

/*---------------------------------------------------------------------*/

unsigned long flash_init (void)
{
	int i;		/* flash bank counter */
	int j;		/* flash device sector counter */
	int k;		/* flash size calculation loop counter */
	int N;		/* pow(2,N) is flash size, but we don't have <math.h> */
	ulong total_size = 0, device_size = 1;
	unsigned char manuf_id, device_id;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		vu_char *addr = (vu_char *) (CONFIG_SYS_FLASH_BASE + i * FLASH_BANK_SIZE);

		addr[0x555] = 0xAA;		/* get manuf/device info command */
		addr[0x2AA] = 0x55;		/* 3-cycle command */
		addr[0x555] = 0x90;

		manuf_id = addr[0];		/* read back manuf/device info */
		device_id = addr[1];

		addr[0x55] = 0x98;		/* CFI command */
		N = addr[0x27];			/* read back device_size = pow(2,N) */

		for (k = 0; k < N; k++)	/* calculate device_size = pow(2,N) */
			device_size *= 2;

		flash_info[i].size = device_size;
		flash_info[i].sector_count = CONFIG_SYS_MAX_FLASH_SECT;

#if defined DEBUG_FLASH
		printf ("manuf_id = %x, device_id = %x\n", manuf_id, device_id);
#endif
		/* find out what kind of flash we are using */
		if ((manuf_id == (uchar) (AMD_MANUFACT))
			&& (device_id == AMD_ID_LV033C)) {
			flash_info[i].flash_id =
					((FLASH_MAN_AMD & FLASH_VENDMASK) << 16) |
					(FLASH_AM033C & FLASH_TYPEMASK);

			/* set individual sector start addresses */
			for (j = 0; j < flash_info[i].sector_count; j++) {
				flash_info[i].start[j] =
						(CONFIG_SYS_FLASH_BASE + i * FLASH_BANK_SIZE +
						 j * MAIN_SECT_SIZE);
			}
		}

		else if ((manuf_id == (uchar) (AMD_MANUFACT)) &&
				 (device_id == AMD_ID_LV116DT)) {
			flash_info[i].flash_id =
					((FLASH_MAN_AMD & FLASH_VENDMASK) << 16) |
					(FLASH_AM160T & FLASH_TYPEMASK);

			/* set individual sector start addresses */
			for (j = 0; j < flash_info[i].sector_count; j++) {
				flash_info[i].start[j] =
						(CONFIG_SYS_FLASH_BASE + i * FLASH_BANK_SIZE +
						 j * MAIN_SECT_SIZE);

				if (j < (CONFIG_SYS_MAX_FLASH_SECT - 3)) {
					flash_info[i].start[j] =
							(CONFIG_SYS_FLASH_BASE + i * FLASH_BANK_SIZE +
							 j * MAIN_SECT_SIZE);
				} else if (j == (CONFIG_SYS_MAX_FLASH_SECT - 3)) {
					flash_info[i].start[j] =
							(flash_info[i].start[j - 1] + SECT_SIZE_32KB);

				} else {
					flash_info[i].start[j] =
							(flash_info[i].start[j - 1] + SECT_SIZE_8KB);
				}
			}
		}

		else {
			flash_info[i].flash_id = FLASH_UNKNOWN;
			addr[0] = 0xFF;
			goto Done;
		}

#if defined DEBUG_FLASH
		printf ("flash_id = 0x%08lX\n", flash_info[i].flash_id);
#endif

		addr[0] = 0xFF;

		memset (flash_info[i].protect, 0, CONFIG_SYS_MAX_FLASH_SECT);

		total_size += flash_info[i].size;
	}

	/* Protect monitor and environment sectors
	 */
#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
	flash_protect (FLAG_PROTECT_SET, CONFIG_SYS_MONITOR_BASE,
				   CONFIG_SYS_MONITOR_BASE + monitor_flash_len - 1,
				   &flash_info[0]);
#endif

#if defined(CONFIG_ENV_IS_IN_FLASH) && defined(CONFIG_ENV_ADDR)
	flash_protect (FLAG_PROTECT_SET, CONFIG_ENV_ADDR,
			CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1, &flash_info[0]);
#endif

  Done:
	return total_size;
}

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
		case FLASH_AM033C:
			type = "AM29LV033C (32 Mbit, uniform sector size)";
			break;
		case FLASH_AM160T:
			type = "AM29LV160T (16 Mbit, top boot sector)";
			break;
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
			 (flash != (unsigned long *) info->start[i] + size) && erased;
			 flash++)
			erased = *flash == ~0x0UL;

		printf ("%s %08lX %s %s",
				(i % 5) ? "" : "\n   ",
				info->start[i],
				erased ? "E" : " ", info->protect[i] ? "RO" : "  ");
	}

	puts ("\n");
	return;
}

/*-----------------------------------------------------------------------
 */

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
		printf ("- Warning: %d protected sectors will not be erased!\n",
				prot);
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
														   start[sect] -
														   info->
														   start[0]) <<
														  sh8b));

			if (info->flash_id & FLASH_MAN_SST) {
				addr[ADDR0 << sh8b] = (FLASH_WORD_SIZE) 0x00AA00AA;
				addr[ADDR1 << sh8b] = (FLASH_WORD_SIZE) 0x00550055;
				addr[ADDR0 << sh8b] = (FLASH_WORD_SIZE) 0x00800080;
				addr[ADDR0 << sh8b] = (FLASH_WORD_SIZE) 0x00AA00AA;
				addr[ADDR1 << sh8b] = (FLASH_WORD_SIZE) 0x00550055;
				addr[0] = (FLASH_WORD_SIZE) 0x00500050;	/* block erase */
				udelay (30000);	/* wait 30 ms */
			} else {
				addr[0] = (FLASH_WORD_SIZE) 0x00300030;	/* sector erase */
			}

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

	wp = (addr & ~3);			/* get lower word aligned address */

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
#if 0
static void write_via_fpu (vu_long * addr, ulong * data)
{
	__asm__ __volatile__ ("lfd  1, 0(%0)"::"r" (data));
	__asm__ __volatile__ ("stfd 1, 0(%0)"::"r" (addr));
}
#endif

/*-----------------------------------------------------------------------
 */
static __inline__ unsigned long get_msr (void)
{
	unsigned long msr;

	__asm__ __volatile__ ("mfmsr %0":"=r" (msr):);

	return msr;
}

static __inline__ void set_msr (unsigned long msr)
{
	__asm__ __volatile__ ("mtmsr %0"::"r" (msr));
}
