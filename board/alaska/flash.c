/*
 * (C) Copyright 2001
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2001-2004
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
#include <linux/byteorder/swab.h>


flash_info_t flash_info[CFG_MAX_FLASH_BANKS];	/* info for FLASH chips    */

/* Board support for 1 or 2 flash devices */
#define FLASH_PORT_WIDTH8

typedef unsigned char FLASH_PORT_WIDTH;
typedef volatile unsigned char FLASH_PORT_WIDTHV;

#define SWAP(x)         (x)

/* Intel-compatible flash ID */
#define INTEL_COMPAT    0x89
#define INTEL_ALT       0xB0

/* Intel-compatible flash commands */
#define INTEL_PROGRAM   0x10
#define INTEL_ERASE     0x20
#define INTEL_CLEAR     0x50
#define INTEL_LOCKBIT   0x60
#define INTEL_PROTECT   0x01
#define INTEL_STATUS    0x70
#define INTEL_READID    0x90
#define INTEL_CONFIRM   0xD0
#define INTEL_RESET     0xFF

/* Intel-compatible flash status bits */
#define INTEL_FINISHED  0x80
#define INTEL_OK        0x80

#define FPW             FLASH_PORT_WIDTH
#define FPWV            FLASH_PORT_WIDTHV

#define FLASH_CYCLE1    0x0555
#define FLASH_CYCLE2    0x02aa

#define WR_BLOCK        0x20
/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (FPW * addr, flash_info_t * info);
static int write_data (flash_info_t * info, ulong dest, FPW data);
static int write_data_block (flash_info_t * info, ulong src, ulong dest);
static int write_word_amd (flash_info_t * info, FPWV * dest, FPW data);
static void flash_get_offsets (ulong base, flash_info_t * info);
void inline spin_wheel (void);
static void flash_sync_real_protect (flash_info_t * info);
static unsigned char intel_sector_protected (flash_info_t *info, ushort sector);
static unsigned char same_chip_banks (int bank1, int bank2);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	int i;
	ulong size = 0;
	ulong fsize = 0;

	for (i = 0; i < CFG_MAX_FLASH_BANKS; i++) {
		memset (&flash_info[i], 0, sizeof (flash_info_t));

		switch (i) {
		case 0:
			flash_get_size ((FPW *) CFG_FLASH1_BASE,
					&flash_info[i]);
			flash_get_offsets (CFG_FLASH1_BASE, &flash_info[i]);
			break;
		case 1:
			flash_get_size ((FPW *) CFG_FLASH1_BASE,
					&flash_info[i]);
			fsize = CFG_FLASH1_BASE + flash_info[i - 1].size;
			flash_get_offsets (fsize, &flash_info[i]);
			break;
		case 2:
			flash_get_size ((FPW *) CFG_FLASH0_BASE,
					&flash_info[i]);
			flash_get_offsets (CFG_FLASH0_BASE, &flash_info[i]);
			break;
		case 3:
			flash_get_size ((FPW *) CFG_FLASH0_BASE,
					&flash_info[i]);
			fsize = CFG_FLASH0_BASE + flash_info[i - 1].size;
			flash_get_offsets (fsize, &flash_info[i]);
			break;
		default:
			panic ("configured to many flash banks!\n");
			break;
		}
		size += flash_info[i].size;

		/* get the h/w and s/w protection status in sync */
		flash_sync_real_protect(&flash_info[i]);
	}

	/* Protect monitor and environment sectors
	 */
#if defined (CFG_AMD_BOOT)
	flash_protect (FLAG_PROTECT_SET,
		       CFG_MONITOR_BASE,
		       CFG_MONITOR_BASE + monitor_flash_len - 1,
		       &flash_info[2]);
	flash_protect (FLAG_PROTECT_SET,
		       CFG_INTEL_BASE,
		       CFG_INTEL_BASE + monitor_flash_len - 1,
		       &flash_info[1]);
#else
	flash_protect (FLAG_PROTECT_SET,
		       CFG_MONITOR_BASE,
		       CFG_MONITOR_BASE + monitor_flash_len - 1,
		       &flash_info[3]);
	flash_protect (FLAG_PROTECT_SET,
		       CFG_AMD_BASE,
		       CFG_AMD_BASE + monitor_flash_len - 1, &flash_info[0]);
#endif

	flash_protect (FLAG_PROTECT_SET,
		       CFG_ENV1_ADDR,
		       CFG_ENV1_ADDR + CFG_ENV1_SIZE - 1, &flash_info[1]);
	flash_protect (FLAG_PROTECT_SET,
		       CFG_ENV_ADDR,
		       CFG_ENV_ADDR + CFG_ENV_SIZE - 1, &flash_info[3]);

	return size;
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets (ulong base, flash_info_t * info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN)
		return;

	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_AMD) {
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base + (i * PHYS_AMD_SECT_SIZE);
			info->protect[i] = 0;
		}
	}

	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL) {
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base + (i * PHYS_INTEL_SECT_SIZE);
		}
	}
}

/*-----------------------------------------------------------------------
 */
void flash_print_info (flash_info_t * info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_INTEL:
		printf ("INTEL ");
		break;
	case FLASH_MAN_AMD:
		printf ("AMD ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F128J3A:
		printf ("28F128J3A\n");
		break;

	case FLASH_AM040:
		printf ("AMD29F040B\n");
		break;

	default:
		printf ("Unknown Chip Type\n");
		break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s",
			info->start[i], info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");
	return;
}

/*
 * The following code cannot be run from FLASH!
 */
static ulong flash_get_size (FPW * addr, flash_info_t * info)
{
	FPWV value;
	static int amd = 0;

	/* Write auto select command: read Manufacturer ID */
	/* Write auto select command sequence and test FLASH answer */
	addr[FLASH_CYCLE1] = (FPW) 0x00AA00AA;	/* for AMD, Intel ignores this */
	__asm__ ("sync");
	addr[FLASH_CYCLE2] = (FPW) 0x00550055;	/* for AMD, Intel ignores this */
	__asm__ ("sync");
	addr[FLASH_CYCLE1] = (FPW) 0x00900090;	/* selects Intel or AMD */
	__asm__ ("sync");

	udelay (100);

	switch (addr[0] & 0xff) {

	case (uchar) AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
		value = addr[1];
		break;

	case (uchar) INTEL_MANUFACT:
		info->flash_id = FLASH_MAN_INTEL;
		value = addr[2];
		break;

	default:
		printf ("unknown\n");
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		addr[0] = (FPW) 0x00FF00FF;	/* restore read mode */
		return (0);	/* no or unknown flash  */
	}

	switch (value) {

	case (FPW) INTEL_ID_28F128J3A:
		info->flash_id += FLASH_28F128J3A;
		info->sector_count = 64;
		info->size = 0x00800000;	/* => 16 MB     */
		break;

	case (FPW) AMD_ID_LV040B:
		info->flash_id += FLASH_AM040;
		if (amd == 0) {
			info->sector_count = 7;
			info->size = 0x00070000;	/* => 448 KB     */
			amd = 1;
		} else {
			/* for Environment settings */
			info->sector_count = 1;
			info->size = PHYS_AMD_SECT_SIZE;	/* => 64 KB     */
			amd = 0;
		}
		break;

	default:
		info->flash_id = FLASH_UNKNOWN;
		break;
	}

	if (info->sector_count > CFG_MAX_FLASH_SECT) {
		printf ("** ERROR: sector count %d > max (%d) **\n",
			info->sector_count, CFG_MAX_FLASH_SECT);
		info->sector_count = CFG_MAX_FLASH_SECT;
	}

	if (value == (FPW) INTEL_ID_28F128J3A)
		addr[0] = (FPW) 0x00FF00FF;	/* restore read mode */
	else
		addr[0] = (FPW) 0x00F000F0;	/* restore read mode */

	return (info->size);
}


/*
 * This function gets the u-boot flash sector protection status
 * (flash_info_t.protect[]) in sync with the sector protection
 * status stored in hardware.
 */
static void flash_sync_real_protect (flash_info_t * info)
{
	int i;

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F128J3A:
		for (i = 0; i < info->sector_count; ++i) {
			info->protect[i] = intel_sector_protected(info, i);
		}
		break;
	case FLASH_AM040:
	default:
		/* no h/w protect support */
		break;
	}
}


/*
 * checks if "sector" in bank "info" is protected. Should work on intel
 * strata flash chips 28FxxxJ3x in 8-bit mode.
 * Returns 1 if sector is protected (or timed-out while trying to read
 * protection status), 0 if it is not.
 */
static unsigned char intel_sector_protected (flash_info_t *info, ushort sector)
{
	FPWV *addr;
	FPWV *lock_conf_addr;
	ulong start;
	unsigned char ret;

	/*
	 * first, wait for the WSM to be finished. The rationale for
	 * waiting for the WSM to become idle for at most
	 * CFG_FLASH_ERASE_TOUT is as follows. The WSM can be busy
	 * because of: (1) erase, (2) program or (3) lock bit
	 * configuration. So we just wait for the longest timeout of
	 * the (1)-(3), i.e. the erase timeout.
	 */

	/* wait at least 35ns (W12) before issuing Read Status Register */
	udelay(1);
	addr = (FPWV *) info->start[sector];
	*addr = (FPW) INTEL_STATUS;

	start = get_timer (0);
	while ((*addr & (FPW) INTEL_FINISHED) != (FPW) INTEL_FINISHED) {
		if (get_timer (start) > CFG_FLASH_ERASE_TOUT) {
			*addr = (FPW) INTEL_RESET; /* restore read mode */
			printf("WSM busy too long, can't get prot status\n");
			return 1;
		}
	}

	/* issue the Read Identifier Codes command */
	*addr = (FPW) INTEL_READID;

	/* wait at least 35ns (W12) before reading */
	udelay(1);

	/* Intel example code uses offset of 4 for 8-bit flash */
	lock_conf_addr = (FPWV *) info->start[sector] + 4;
	ret = (*lock_conf_addr & (FPW) INTEL_PROTECT) ? 1 : 0;

	/* put flash back in read mode */
	*addr = (FPW) INTEL_RESET;

	return ret;
}


/*
 * Checks if "bank1" and "bank2" are on the same chip.  Returns 1 if they
 * are and 0 otherwise.
 */
static unsigned char same_chip_banks (int bank1, int bank2)
{
	unsigned char same_chip[CFG_MAX_FLASH_BANKS][CFG_MAX_FLASH_BANKS] = {
		{1, 1, 0, 0},
		{1, 1, 0, 0},
		{0, 0, 1, 1},
		{0, 0, 1, 1}
	};
	return same_chip[bank1][bank2];
}


/*-----------------------------------------------------------------------
 */
int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int flag, prot, sect;
	ulong type, start, last;
	int rcode = 0, intel = 0;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN)
			printf ("- missing\n");
		else
			printf ("- no sectors to erase\n");
		return 1;
	}

	type = (info->flash_id & FLASH_VENDMASK);
	if ((type != FLASH_MAN_INTEL)) {
		type = (info->flash_id & FLASH_VENDMASK);
		if ((type != FLASH_MAN_AMD)) {
			printf ("Can't erase unknown flash type %08lx - aborted\n",
				info->flash_id);
			return 1;
		}
	}

	if (type == FLASH_MAN_INTEL)
		intel = 1;

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

	start = get_timer (0);
	last = start;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			FPWV *addr = (FPWV *) (info->start[sect]);
			FPW status;

			printf ("Erasing sector %2d ... ", sect);

			/* arm simple, non interrupt dependent timer */
			start = get_timer (0);

			if (intel) {
				*addr = (FPW) 0x00500050;	/* clear status register */
				*addr = (FPW) 0x00200020;	/* erase setup */
				*addr = (FPW) 0x00D000D0;	/* erase confirm */
			} else {
				FPWV *base;	/* first address in bank */

				base = (FPWV *) (CFG_AMD_BASE);
				base[FLASH_CYCLE1] = (FPW) 0x00AA00AA;	/* unlock */
				base[FLASH_CYCLE2] = (FPW) 0x00550055;	/* unlock */
				base[FLASH_CYCLE1] = (FPW) 0x00800080;	/* erase mode */
				base[FLASH_CYCLE1] = (FPW) 0x00AA00AA;	/* unlock */
				base[FLASH_CYCLE2] = (FPW) 0x00550055;	/* unlock */
				*addr = (FPW) 0x00300030;	/* erase sector */
			}

			while (((status =
				 *addr) & (FPW) 0x00800080) !=
			       (FPW) 0x00800080) {
				if (get_timer (start) > CFG_FLASH_ERASE_TOUT) {
					printf ("Timeout\n");
					if (intel) {
						*addr = (FPW) 0x00B000B0;	/* suspend erase     */
						*addr = (FPW) 0x00FF00FF;	/* reset to read mode */
					} else
						*addr = (FPW) 0x00F000F0;	/* reset to read mode */

					rcode = 1;
					break;
				}
			}

			if (intel) {
				*addr = (FPW) 0x00500050;	/* clear status register cmd.   */
				*addr = (FPW) 0x00FF00FF;	/* resest to read mode          */
			} else
				*addr = (FPW) 0x00F000F0;	/* reset to read mode */

			printf (" done\n");
		}
	}
	return rcode;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 4 - Flash not identified
 */

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	if (info->flash_id == FLASH_UNKNOWN) {
		return 4;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:
	    {
		FPW data = 0;	/* 16 or 32 bit word, matches flash bus width */
		int bytes;	/* number of bytes to program in current word */
		int left;	/* number of bytes left to program */
		int i, res;

		for (left = cnt, res = 0;
		     left > 0 && res == 0;
		     addr += sizeof (data), left -=
		     sizeof (data) - bytes) {

			bytes = addr & (sizeof (data) - 1);
			addr &= ~(sizeof (data) - 1);

			/* combine source and destination data so can program
			 * an entire word of 16 or 32 bits
			 */
			for (i = 0; i < sizeof (data); i++) {
				data <<= 8;
				if (i < bytes || i - bytes >= left)
					data += *((uchar *) addr + i);
				else
					data += *src++;
			}

			res = write_word_amd (info, (FPWV *) addr,
					      data);
		}
		return res;
	    }		/* case FLASH_MAN_AMD */

	case FLASH_MAN_INTEL:
	    {
		ulong cp, wp;
		FPW data;
		int count, i, l, rc, port_width;

		/* get lower word aligned address */
		wp = addr;
		port_width = 1;

		/*
		 * handle unaligned start bytes
		 */
		if ((l = addr - wp) != 0) {
			data = 0;
			for (i = 0, cp = wp; i < l; ++i, ++cp) {
				data = (data << 8) | (*(uchar *) cp);
			}

			for (; i < port_width && cnt > 0; ++i) {
				data = (data << 8) | *src++;
				--cnt;
				++cp;
			}

			for (; cnt == 0 && i < port_width; ++i, ++cp)
				data = (data << 8) | (*(uchar *) cp);

			if ((rc =
			     write_data (info, wp, SWAP (data))) != 0)
				return (rc);
			wp += port_width;
		}

		if (cnt > WR_BLOCK) {
			/*
			 * handle word aligned part
			 */
			count = 0;
			while (cnt >= WR_BLOCK) {

				if ((rc =
				     write_data_block (info,
						       (ulong) src,
						       wp)) != 0)
					return (rc);

				wp += WR_BLOCK;
				src += WR_BLOCK;
				cnt -= WR_BLOCK;

				if (count++ > 0x800) {
					spin_wheel ();
					count = 0;
				}
			}
		}

		if (cnt < WR_BLOCK) {
			/*
			 * handle word aligned part
			 */
			count = 0;
			while (cnt >= port_width) {
				data = 0;
				for (i = 0; i < port_width; ++i)
					data = (data << 8) | *src++;

				if ((rc =
				     write_data (info, wp,
						 SWAP (data))) != 0)
					return (rc);

				wp += port_width;
				cnt -= port_width;
				if (count++ > 0x800) {
					spin_wheel ();
					count = 0;
				}
			}
		}

		if (cnt == 0)
			return (0);

		/*
		 * handle unaligned tail bytes
		 */
		data = 0;
		for (i = 0, cp = wp; i < port_width && cnt > 0;
		     ++i, ++cp) {
			data = (data << 8) | *src++;
			--cnt;
		}

		for (; i < port_width; ++i, ++cp)
			data = (data << 8) | (*(uchar *) cp);

		return (write_data (info, wp, SWAP (data)));
	    }		/* case FLASH_MAN_INTEL */

	}			/* switch */
	return (0);
}

/*-----------------------------------------------------------------------
 * Write a word or halfword to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_data (flash_info_t * info, ulong dest, FPW data)
{
	FPWV *addr = (FPWV *) dest;
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*addr & data) != data) {
		printf ("not erased at %08lx (%lx)\n", (ulong)addr, (ulong)*addr);
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	*addr = (FPW) 0x00400040;	/* write setup */
	*addr = data;

	/* arm simple, non interrupt dependent timer */
	start = get_timer (0);

	/* wait while polling the status register */
	while ((*addr & (FPW) 0x00800080) != (FPW) 0x00800080) {
		if (get_timer (start) > CFG_FLASH_WRITE_TOUT) {
			*addr = (FPW) 0x00FF00FF;	/* restore read mode */
			return (1);
		}
	}

	*addr = (FPW) 0x00FF00FF;	/* restore read mode */

	return (0);
}

/*-----------------------------------------------------------------------
 * Write a word or halfword to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_data_block (flash_info_t * info, ulong src, ulong dest)
{
	FPWV *srcaddr = (FPWV *) src;
	FPWV *dstaddr = (FPWV *) dest;
	ulong start;
	int flag, i;

	/* Check if Flash is (sufficiently) erased */
	for (i = 0; i < WR_BLOCK; i++)
		if ((*dstaddr++ & 0xff) != 0xff) {
			printf ("not erased at %08lx (%lx)\n",
				(ulong)dstaddr, (ulong)*dstaddr);
			return (2);
		}

	dstaddr = (FPWV *) dest;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	*dstaddr = (FPW) 0x00e800e8;	/* write block setup */

	/* arm simple, non interrupt dependent timer */
	start = get_timer (0);

	/* wait while polling the status register */
	while ((*dstaddr & (FPW) 0x00800080) != (FPW) 0x00800080) {
		if (get_timer (start) > CFG_FLASH_WRITE_TOUT) {
			*dstaddr = (FPW) 0x00FF00FF;	/* restore read mode */
			return (1);
		}
	}

	*dstaddr = (FPW) 0x001f001f;	/* write 32 to buffer */
	for (i = 0; i < WR_BLOCK; i++)
		*dstaddr++ = *srcaddr++;

	dstaddr -= 1;
	*dstaddr = (FPW) 0x00d000d0;	/* write 32 to buffer */

	/* arm simple, non interrupt dependent timer */
	start = get_timer (0);

	/* wait while polling the status register */
	while ((*dstaddr & (FPW) 0x00800080) != (FPW) 0x00800080) {
		if (get_timer (start) > CFG_FLASH_WRITE_TOUT) {
			*dstaddr = (FPW) 0x00FF00FF;	/* restore read mode */
			return (1);
		}
	}

	*dstaddr = (FPW) 0x00FF00FF;	/* restore read mode */

	return (0);
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
static int write_word_amd (flash_info_t * info, FPWV * dest, FPW data)
{
	ulong start;
	int flag;
	int res = 0;		/* result, assume success */
	FPWV *base;		/* first address in flash bank */

	/* Check if Flash is (sufficiently) erased */
	if ((*dest & data) != data) {
		return (2);
	}

	base = (FPWV *) (CFG_AMD_BASE);

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	base[FLASH_CYCLE1] = (FPW) 0x00AA00AA;	/* unlock */
	base[FLASH_CYCLE2] = (FPW) 0x00550055;	/* unlock */
	base[FLASH_CYCLE1] = (FPW) 0x00A000A0;	/* selects program mode */

	*dest = data;		/* start programming the data */

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();

	start = get_timer (0);

	/* data polling for D7 */
	while (res == 0
	       && (*dest & (FPW) 0x00800080) != (data & (FPW) 0x00800080)) {
		if (get_timer (start) > CFG_FLASH_WRITE_TOUT) {
			*dest = (FPW) 0x00F000F0;	/* reset bank */
			res = 1;
		}
	}

	return (res);
}

void inline spin_wheel (void)
{
	static int p = 0;
	static char w[] = "\\/-";

	printf ("\010%c", w[p]);
	(++p == 3) ? (p = 0) : 0;
}

/*-----------------------------------------------------------------------
 * Set/Clear sector's lock bit, returns:
 * 0 - OK
 * 1 - Error (timeout, voltage problems, etc.)
 */
int flash_real_protect (flash_info_t * info, long sector, int prot)
{
	ulong start;
	int i, j;
	int curr_bank;
	int bank;
	int rc = 0;
	FPWV *addr = (FPWV *) (info->start[sector]);
	int flag = disable_interrupts ();

	/*
	 * 29F040B AMD flash does not support software protection/unprotection,
	 * the only way to protect the AMD flash is marked it as prot bit.
	 * This flash only support hardware protection, by supply or not supply
	 * 12vpp to the flash
	 */
	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_AMD) {
		info->protect[sector] = prot;

		return 0;
	}

	*addr = INTEL_CLEAR;	/* Clear status register    */
	if (prot) {		/* Set sector lock bit      */
		*addr = INTEL_LOCKBIT;	/* Sector lock bit          */
		*addr = INTEL_PROTECT;	/* set                      */
	} else {		/* Clear sector lock bit    */
		*addr = INTEL_LOCKBIT;	/* All sectors lock bits    */
		*addr = INTEL_CONFIRM;	/* clear                    */
	}

	start = get_timer (0);

	while ((*addr & INTEL_FINISHED) != INTEL_FINISHED) {
		if (get_timer (start) > CFG_FLASH_UNLOCK_TOUT) {
			printf ("Flash lock bit operation timed out\n");
			rc = 1;
			break;
		}
	}

	if (*addr != INTEL_OK) {
		printf ("Flash lock bit operation failed at %08X, CSR=%08X\n",
			(uint) addr, (uint) * addr);
		rc = 1;
	}

	if (!rc)
		info->protect[sector] = prot;

	/*
	 * Clear lock bit command clears all sectors lock bits, so
	 * we have to restore lock bits of protected sectors.
	 */
	if (!prot) {
		/*
		 * re-locking must be done for all banks that belong on one
		 * FLASH chip, as all the sectors on the chip were unlocked
		 * by INTEL_LOCKBIT/INTEL_CONFIRM commands. (let's hope
		 * that banks never span chips, in particular chips which
		 * support h/w protection differently).
		 */

		/* find the current bank number */
		curr_bank = CFG_MAX_FLASH_BANKS + 1;
		for (j = 0; j < CFG_MAX_FLASH_BANKS; ++j) {
			if (&flash_info[j] == info) {
				curr_bank = j;
			}
		}
		if (curr_bank == CFG_MAX_FLASH_BANKS + 1) {
			printf("Error: can't determine bank number!\n");
		}

		for (bank = 0; bank < CFG_MAX_FLASH_BANKS; ++bank) {
			if (!same_chip_banks(curr_bank, bank)) {
				continue;
			}
			info = &flash_info[bank];
			for (i = 0; i < info->sector_count; i++) {
				if (info->protect[i]) {
					start = get_timer (0);
					addr = (FPWV *) (info->start[i]);
					*addr = INTEL_LOCKBIT;	/* Sector lock bit  */
					*addr = INTEL_PROTECT;	/* set              */
					while ((*addr & INTEL_FINISHED) !=
					       INTEL_FINISHED) {
						if (get_timer (start) >
						    CFG_FLASH_UNLOCK_TOUT) {
							printf ("Flash lock bit operation timed out\n");
							rc = 1;
							break;
						}
					}
				}
			}
		}

		/*
		 * get the s/w sector protection status in sync with the h/w,
		 * in case something went wrong during the re-locking.
		 */
		flash_sync_real_protect(info); /* resets flash to read  mode */
	}

	if (flag)
		enable_interrupts ();

	*addr = INTEL_RESET;	/* Reset to read array mode */

	return rc;
}
