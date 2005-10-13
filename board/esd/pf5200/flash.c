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

#include <common.h>

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];	/* info for FLASH chips */

typedef unsigned short FLASH_PORT_WIDTH;
typedef volatile unsigned short FLASH_PORT_WIDTHV;

#define FLASH_ID_MASK           0x00FF

#define FPW                     FLASH_PORT_WIDTH
#define FPWV                    FLASH_PORT_WIDTHV

#define FLASH_CYCLE1            0x0555
#define FLASH_CYCLE2            0x0aaa
#define FLASH_ID1               0x00
#define FLASH_ID2               0x01
#define FLASH_ID3               0x0E
#define FLASH_ID4               0x0F

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size(FPWV * addr, flash_info_t * info);
static void flash_reset(flash_info_t * info);
static int write_word_amd(flash_info_t * info, FPWV * dest, FPW data);
static flash_info_t *flash_get_info(ulong base);

/*-----------------------------------------------------------------------
 * flash_init()
 *
 * sets up flash_info and returns size of FLASH (bytes)
 */
unsigned long flash_init(void)
{
	unsigned long size = 0;
	int i = 0;
	extern void flash_preinit(void);
	extern void flash_afterinit(uint, ulong, ulong);

	ulong flashbase = CFG_FLASH_BASE;

	flash_preinit();

	/* There is only ONE FLASH device */
	memset(&flash_info[i], 0, sizeof(flash_info_t));
	flash_info[i].size = flash_get_size((FPW *) flashbase, &flash_info[i]);
	size += flash_info[i].size;

#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET, CFG_MONITOR_BASE,
		      CFG_MONITOR_BASE + monitor_flash_len - 1,
		      flash_get_info(CFG_MONITOR_BASE));
#endif

#ifdef  CFG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET, CFG_ENV_ADDR,
		      CFG_ENV_ADDR + CFG_ENV_SIZE - 1,
		      flash_get_info(CFG_ENV_ADDR));
#endif

	flash_afterinit(i, flash_info[i].start[0], flash_info[i].size);
	return size ? size : 1;
}

/*-----------------------------------------------------------------------
 */
static void flash_reset(flash_info_t * info) {
	FPWV *base = (FPWV *) (info->start[0]);

	/* Put FLASH back in read mode */
	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL) {
		*base = (FPW) 0x00FF00FF;	/* Intel Read Mode */
	} else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_AMD) {
		*base = (FPW) 0x00F000F0;	/* AMD Read Mode */
	}
}

/*-----------------------------------------------------------------------
 */

static flash_info_t *flash_get_info(ulong base) {
	int i;
	flash_info_t *info;

	for (i = 0; i < CFG_MAX_FLASH_BANKS; i++) {
		info = &flash_info[i];
		if ((info->size) && (info->start[0] <= base)
		    && (base <= info->start[0] + info->size - 1)) {
			break;
		}
	}
	return (i == CFG_MAX_FLASH_BANKS ? 0 : info);
}

/*-----------------------------------------------------------------------
 */

void flash_print_info(flash_info_t * info) {
	int i;
	char *fmt;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:
		printf("AMD ");
		break;
	default:
		printf("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AMLV256U:
		fmt = "29LV256M (256 Mbit)\n";
		break;
	default:
		fmt = "Unknown Chip Type\n";
		break;
	}

	printf(fmt);
	printf("  Size: %ld MB in %d Sectors\n", info->size >> 20,
	       info->sector_count);
	printf("  Sector Start Addresses:");

	for (i = 0; i < info->sector_count; ++i) {
		ulong size;
		int erased;
		ulong *flash = (unsigned long *)info->start[i];

		if ((i % 5) == 0) {
			printf("\n   ");
		}

		/*
		 * Check if whole sector is erased
		 */
		size =
		    (i !=
		     (info->sector_count - 1)) ? (info->start[i + 1] -
						  info->start[i]) >> 2 : (info->
									  start
									  [0] +
									  info->
									  size -
									  info->
									  start
									  [i])
		    >> 2;

		for (flash = (unsigned long *)info->start[i], erased = 1;
		     (flash != (unsigned long *)info->start[i] + size)
		     && erased; flash++) {
			erased = *flash == ~0x0UL;
		}
		printf(" %08lX %s %s", info->start[i], erased ? "E" : " ",
		       info->protect[i] ? "(RO)" : "    ");
	}

	printf("\n");
}

/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */

ulong flash_get_size(FPWV * addr, flash_info_t * info) {
	int i;

	/* Write auto select command: read Manufacturer ID                     */
	/* Write auto select command sequence and test FLASH answer            */
	addr[FLASH_CYCLE1] = (FPW) 0x00AA00AA;	/* for AMD, Intel ignores this */
	addr[FLASH_CYCLE2] = (FPW) 0x00550055;	/* for AMD, Intel ignores this */
	addr[FLASH_CYCLE1] = (FPW) 0x00900090;	/* selects Intel or AMD        */

	/* The manufacturer codes are only 1 byte, so just use 1 byte.         */
	/* This works for any bus width and any FLASH device width.            */
	udelay(100);
	switch (addr[FLASH_ID1] & 0x00ff) {
	case (uchar) AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
		break;
	default:
		printf("unknown vendor=%x ", addr[FLASH_ID1] & 0xff);
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		break;
	}

	/* Check 16 bits or 32 bits of ID so work on 32 or 16 bit bus.     */
	if (info->flash_id != FLASH_UNKNOWN) {
		switch ((FPW) addr[FLASH_ID2]) {
		case (FPW) AMD_ID_MIRROR:
			/* MIRROR BIT FLASH, read more ID bytes */
			if ((FPW) addr[FLASH_ID3] == (FPW) AMD_ID_LV256U_2
			    && (FPW) addr[FLASH_ID4] == (FPW) AMD_ID_LV256U_3) {
				/* attention: only the first 16 MB will be used in u-boot */
				info->flash_id += FLASH_AMLV256U;
				info->sector_count = 512;
				info->size = 0x02000000;
				for (i = 0; i < info->sector_count; i++) {
					info->start[i] =
					    (ulong) addr + 0x10000 * i;
				}
				break;
			}
			/* fall thru to here ! */
		default:
			printf("unknown AMD device=%x %x %x",
			       (FPW) addr[FLASH_ID2], (FPW) addr[FLASH_ID3],
			       (FPW) addr[FLASH_ID4]);
			info->flash_id = FLASH_UNKNOWN;
			info->sector_count = 0;
			info->size = 0x800000;
			break;
		}

		/* Put FLASH back in read mode */
		flash_reset(info);
	}
	return (info->size);
}

/*-----------------------------------------------------------------------
 */

int flash_erase(flash_info_t * info, int s_first, int s_last) {
	FPWV *addr;
	int flag, prot, sect;
	int intel = (info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL;
	ulong start, now, last;
	int rcode = 0;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf("- missing\n");
		} else {
			printf("- no sectors to erase\n");
		}
		return 1;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AMLV256U:
		break;
	case FLASH_UNKNOWN:
	default:
		printf("Can't erase unknown flash type %08lx - aborted\n",
		       info->flash_id);
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf("- Warning: %d protected sectors will not be erased!\n",
		       prot);
	} else {
		printf("\n");
	}

	last = get_timer(0);

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last && rcode == 0; sect++) {
		if (info->protect[sect] != 0) {	/* protected, skip it */
			continue;
		}
		/* Disable interrupts which might cause a timeout here */
		flag = disable_interrupts();

		addr = (FPWV *) (info->start[sect]);
		if (intel) {
			*addr = (FPW) 0x00500050;	/* clear status register */
			*addr = (FPW) 0x00200020;	/* erase setup */
			*addr = (FPW) 0x00D000D0;	/* erase confirm */
		} else {
			/* must be AMD style if not Intel */
			FPWV *base;	/* first address in bank */

			base = (FPWV *) (info->start[0]);
			base[FLASH_CYCLE1] = (FPW) 0x00AA00AA;	/* unlock */
			base[FLASH_CYCLE2] = (FPW) 0x00550055;	/* unlock */
			base[FLASH_CYCLE1] = (FPW) 0x00800080;	/* erase mode */
			base[FLASH_CYCLE1] = (FPW) 0x00AA00AA;	/* unlock */
			base[FLASH_CYCLE2] = (FPW) 0x00550055;	/* unlock */
			*addr = (FPW) 0x00300030;	/* erase sector */
		}

		/* re-enable interrupts if necessary */
		if (flag) {
			enable_interrupts();
		}
		start = get_timer(0);

		/* wait at least 50us for AMD, 80us for Intel. */
		/* Let's wait 1 ms.                            */
		udelay(1000);

		while ((*addr & (FPW) 0x00800080) != (FPW) 0x00800080) {
			if ((now = get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
				printf("Timeout\n");
				if (intel) {
					/* suspend erase        */
					*addr = (FPW) 0x00B000B0;
				}
				flash_reset(info);	/* reset to read mode */
				rcode = 1;	/* failed */
				break;
			}
			/* show that we're waiting */
			if ((get_timer(last)) > CFG_HZ) {
				/* every second */
				putc('.');
				last = get_timer(0);
			}
		}
		/* show that we're waiting */
		if ((get_timer(last)) > CFG_HZ) {
			/* every second */
			putc('.');
			last = get_timer(0);
		}
		flash_reset(info);	/* reset to read mode */
	}
	printf(" done\n");
	return (rcode);
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff(flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	FPW data = 0;		/* 16 or 32 bit word, matches flash bus width on MPC8XX */
	int bytes;		/* number of bytes to program in current word         */
	int left;		/* number of bytes left to program                    */
	int i, res;

	for (left = cnt, res = 0;
	     left > 0 && res == 0;
	     addr += sizeof(data), left -= sizeof(data) - bytes) {

		bytes = addr & (sizeof(data) - 1);
		addr &= ~(sizeof(data) - 1);

		/* combine source and destination data so can program
		 * an entire word of 16 or 32 bits
		 */
		for (i = 0; i < sizeof(data); i++) {
			data <<= 8;
			if (i < bytes || i - bytes >= left)
				data += *((uchar *) addr + i);
			else
				data += *src++;
		}

		/* write one word to the flash */
		switch (info->flash_id & FLASH_VENDMASK) {
		case FLASH_MAN_AMD:
			res = write_word_amd(info, (FPWV *) addr, data);
			break;
		default:
			/* unknown flash type, error! */
			printf("missing or unknown FLASH type\n");
			res = 1;	/* not really a timeout, but gives error */
			break;
		}
	}
	return (res);
}

/*-----------------------------------------------------------------------
 * Write a word to Flash for AMD FLASH
 * A word is 16 or 32 bits, whichever the bus width of the flash bank
 * (not an individual chip) is.
 *
 * returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word_amd(flash_info_t * info, FPWV * dest, FPW data) {
	ulong start;
	int flag;
	int res = 0;		/* result, assume success       */
	FPWV *base;		/* first address in flash bank  */

	/* Check if Flash is (sufficiently) erased */
	if ((*dest & data) != data) {
		return (2);
	}

	base = (FPWV *) (info->start[0]);

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	base[FLASH_CYCLE1] = (FPW) 0x00AA00AA;	/* unlock */
	base[FLASH_CYCLE2] = (FPW) 0x00550055;	/* unlock */
	base[FLASH_CYCLE1] = (FPW) 0x00A000A0;	/* selects program mode */

	*dest = data;		/* start programming the data   */

	/* re-enable interrupts if necessary */
	if (flag) {
		enable_interrupts();
	}
	start = get_timer(0);

	/* data polling for D7 */
	while (res == 0
	       && (*dest & (FPW) 0x00800080) != (data & (FPW) 0x00800080)) {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			*dest = (FPW) 0x00F000F0;	/* reset bank */
			res = 1;
		}
	}
	return (res);
}
