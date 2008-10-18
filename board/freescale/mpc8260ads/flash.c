/*
 * (C) Copyright 2000, 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001, Stuart Hughes, Lineo Inc, stuarth@lineo.com
 * Add support the Sharp chips on the mpc8260ads.
 * I started with board/ip860/flash.c and made changes I found in
 * the MTD project by David Schleef.
 *
 * (C) Copyright 2003 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 * Re-written to support multi-bank flash SIMMs.
 * Added support for real protection and JFFS2.
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

/* Intel-compatible flash ID */
#define INTEL_COMPAT  0x89898989
#define INTEL_ALT     0xB0B0B0B0

/* Intel-compatible flash commands */
#define INTEL_PROGRAM 0x10101010
#define INTEL_ERASE   0x20202020
#define INTEL_CLEAR   0x50505050
#define INTEL_LOCKBIT 0x60606060
#define INTEL_PROTECT 0x01010101
#define INTEL_STATUS  0x70707070
#define INTEL_READID  0x90909090
#define INTEL_CONFIRM 0xD0D0D0D0
#define INTEL_RESET   0xFFFFFFFF

/* Intel-compatible flash status bits */
#define INTEL_FINISHED 0x80808080
#define INTEL_OK       0x80808080

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips */

/*-----------------------------------------------------------------------
 * This board supports 32-bit wide flash SIMMs (4x8-bit configuration.)
 * Up to 32MB of flash supported (up to 4 banks.)
 * BCSR is used for flash presence detect (page 4-65 of the User's Manual)
 *
 * The following code can not run from flash!
 */
unsigned long flash_init (void)
{
	ulong size = 0, sect_start, sect_size = 0, bank_size;
	ushort sect_count = 0;
	int i, j, nbanks;
	vu_long *addr = (vu_long *)CONFIG_SYS_FLASH_BASE;
	vu_long *bcsr = (vu_long *)CONFIG_SYS_BCSR;

	switch (bcsr[2] & 0xF) {
	case 0:
		nbanks = 4;
		break;
	case 1:
		nbanks = 2;
		break;
	case 2:
		nbanks = 1;
		break;
	default:		/* Unsupported configurations */
		nbanks = CONFIG_SYS_MAX_FLASH_BANKS;
	}

	if (nbanks > CONFIG_SYS_MAX_FLASH_BANKS)
		nbanks = CONFIG_SYS_MAX_FLASH_BANKS;

	for (i = 0; i < nbanks; i++) {
		*addr = INTEL_READID;	/* Read Intelligent Identifier */
		if ((addr[0] == INTEL_COMPAT) || (addr[0] == INTEL_ALT)) {
			switch (addr[1]) {
			case SHARP_ID_28F016SCL:
			case SHARP_ID_28F016SCZ:
				flash_info[i].flash_id = FLASH_MAN_SHARP | FLASH_LH28F016SCT;
				sect_count = 32;
				sect_size = 0x40000;
				break;
			default:
				flash_info[i].flash_id = FLASH_UNKNOWN;
				sect_count = CONFIG_SYS_MAX_FLASH_SECT;
				sect_size =
				   CONFIG_SYS_FLASH_SIZE / CONFIG_SYS_MAX_FLASH_BANKS / CONFIG_SYS_MAX_FLASH_SECT;
			}
		}
		else
			flash_info[i].flash_id = FLASH_UNKNOWN;
		if (flash_info[i].flash_id == FLASH_UNKNOWN) {
			printf("### Unknown flash ID %08lX %08lX at address %08lX ###\n",
			       addr[0], addr[1], (ulong)addr);
			size = 0;
			*addr = INTEL_RESET; /* Reset bank to Read Array mode */
			break;
		}
		flash_info[i].sector_count = sect_count;
		flash_info[i].size = bank_size = sect_size * sect_count;
		size += bank_size;
		sect_start = (ulong)addr;
		for (j = 0; j < sect_count; j++) {
			addr = (vu_long *)sect_start;
			flash_info[i].start[j]   = sect_start;
			flash_info[i].protect[j] = (addr[2] == 0x01010101);
			sect_start += sect_size;
		}
		*addr = INTEL_RESET; /* Reset bank to Read Array mode */
		addr = (vu_long *)sect_start;
	}

	if (size == 0) {	/* Unknown flash, fill with hard-coded values */
		sect_start = CONFIG_SYS_FLASH_BASE;
		for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
			flash_info[i].flash_id = FLASH_UNKNOWN;
			flash_info[i].size = CONFIG_SYS_FLASH_SIZE / CONFIG_SYS_MAX_FLASH_BANKS;
			flash_info[i].sector_count = sect_count;
			for (j = 0; j < sect_count; j++) {
				flash_info[i].start[j]   = sect_start;
				flash_info[i].protect[j] = 0;
				sect_start += sect_size;
			}
		}
		size = CONFIG_SYS_FLASH_SIZE;
	}
	else
		for (i = nbanks; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
			flash_info[i].flash_id = FLASH_UNKNOWN;
			flash_info[i].size = 0;
			flash_info[i].sector_count = 0;
		}

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE+monitor_flash_len-1,
		      &flash_info[0]);
#endif

#ifdef	CONFIG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_ENV_ADDR,
		      CONFIG_ENV_ADDR+CONFIG_ENV_SECT_SIZE-1,
		      &flash_info[0]);
#endif
	return (size);
}

/*-----------------------------------------------------------------------
 */
void flash_print_info  (flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_INTEL:	printf ("Intel ");		break;
	case FLASH_MAN_SHARP:   printf ("Sharp ");		break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F016SV:	printf ("28F016SV (16 Mbit, 32 x 64k)\n");
				break;
	case FLASH_28F160S3:	printf ("28F160S3 (16 Mbit, 32 x 512K)\n");
				break;
	case FLASH_28F320S3:	printf ("28F320S3 (32 Mbit, 64 x 512K)\n");
				break;
	case FLASH_LH28F016SCT: printf ("28F016SC (16 Mbit, 32 x 64K)\n");
				break;
	default:		printf ("Unknown Chip Type\n");
				break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);

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

/*-----------------------------------------------------------------------
 */
int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	int flag, prot, sect;
	ulong start, now, last;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	if (    ((info->flash_id & FLASH_VENDMASK) != FLASH_MAN_INTEL)
	     && ((info->flash_id & FLASH_VENDMASK) != FLASH_MAN_SHARP) ) {
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

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			vu_long *addr = (vu_long *)(info->start[sect]);

			last = start = get_timer (0);

			/* Disable interrupts which might cause a timeout here */
			flag = disable_interrupts();

			/* Clear Status Register */
			*addr = INTEL_CLEAR;
			/* Single Block Erase Command */
			*addr = INTEL_ERASE;
			/* Confirm */
			*addr = INTEL_CONFIRM;

			if((info->flash_id & FLASH_TYPEMASK) != FLASH_LH28F016SCT) {
			    /* Resume Command, as per errata update */
			    *addr = INTEL_CONFIRM;
			}

			/* re-enable interrupts if necessary */
			if (flag)
				enable_interrupts();

			while ((*addr & INTEL_FINISHED) != INTEL_FINISHED) {
				if ((now=get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
					printf ("Timeout\n");
					*addr = INTEL_RESET;	/* reset bank */
					return 1;
				}
				/* show that we're waiting */
				if ((now - last) > 1000) {	/* every second */
					putc ('.');
					last = now;
				}
			}

			if (*addr != INTEL_OK) {
				printf("Block erase failed at %08X, CSR=%08X\n",
				       (uint)addr, (uint)*addr);
				*addr = INTEL_RESET;	/* reset bank */
				return 1;
			}

			/* reset to read mode */
			*addr = INTEL_RESET;
		}
	}

	printf (" done\n");
	return 0;
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word (flash_info_t *info, ulong dest, ulong data)
{
	ulong start;
	int rc = 0;
	int flag;
	vu_long *addr = (vu_long *)dest;

	/* Check if Flash is (sufficiently) erased */
	if ((*addr & data) != data) {
		return (2);
	}

	*addr = INTEL_CLEAR; /* Clear status register */

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	/* Write Command */
	*addr = INTEL_PROGRAM;

	/* Write Data */
	*addr = data;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer (0);
	while ((*addr & INTEL_FINISHED) != INTEL_FINISHED) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			printf("Write timed out\n");
			rc = 1;
			break;
		}
	}
	if (*addr != INTEL_OK) {
		printf ("Write failed at %08X, CSR=%08X\n", (uint)addr, (uint)*addr);
		rc = 1;
	}

	*addr = INTEL_RESET; /* Reset to read array mode */

	return rc;
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

	*(vu_long *)wp = INTEL_RESET; /* Reset to read array mode */

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

	rc = write_word(info, wp, data);

	return rc;
}

/*-----------------------------------------------------------------------
 * Set/Clear sector's lock bit, returns:
 * 0 - OK
 * 1 - Error (timeout, voltage problems, etc.)
 */
int flash_real_protect(flash_info_t *info, long sector, int prot)
{
	ulong start;
	int i;
	int rc = 0;
	vu_long *addr = (vu_long *)(info->start[sector]);
	int flag = disable_interrupts();

	*addr = INTEL_CLEAR;	/* Clear status register */
	if (prot) {			/* Set sector lock bit */
		*addr = INTEL_LOCKBIT;	/* Sector lock bit */
		*addr = INTEL_PROTECT;	/* set */
	}
	else {				/* Clear sector lock bit */
		*addr = INTEL_LOCKBIT;	/* All sectors lock bits */
		*addr = INTEL_CONFIRM;	/* clear */
	}

	start = get_timer(0);
	while ((*addr & INTEL_FINISHED) != INTEL_FINISHED) {
		if (get_timer(start) > CONFIG_SYS_FLASH_UNLOCK_TOUT) {
			printf("Flash lock bit operation timed out\n");
			rc = 1;
			break;
		}
	}

	if (*addr != INTEL_OK) {
		printf("Flash lock bit operation failed at %08X, CSR=%08X\n",
		       (uint)addr, (uint)*addr);
		rc = 1;
	}

	if (!rc)
		info->protect[sector] = prot;

	/*
	 * Clear lock bit command clears all sectors lock bits, so
	 * we have to restore lock bits of protected sectors.
	 */
	if (!prot)
		for (i = 0; i < info->sector_count; i++)
			if (info->protect[i]) {
				addr = (vu_long *)(info->start[i]);
				*addr = INTEL_LOCKBIT;	/* Sector lock bit */
				*addr = INTEL_PROTECT;	/* set */
				udelay(CONFIG_SYS_FLASH_LOCK_TOUT * 1000);
			}

	if (flag)
		enable_interrupts();

	*addr = INTEL_RESET;		/* Reset to read array mode */

	return rc;
}
