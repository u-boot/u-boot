/*
 * (C) Copyright 2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * flash_real_protect() routine based on boards/alaska/flash.c
 * (C) Copyright 2001
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
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

/* Intel-compatible flash commands */
#define INTEL_ERASE	0x20
#define INTEL_PROGRAM	0x40
#define INTEL_CLEAR	0x50
#define INTEL_LOCKBIT	0x60
#define INTEL_PROTECT	0x01
#define INTEL_STATUS	0x70
#define INTEL_READID	0x90
#define INTEL_READID	0x90
#define INTEL_SUSPEND	0xB0
#define INTEL_CONFIRM	0xD0
#define INTEL_RESET	0xFF

/* Intel-compatible flash status bits */
#define INTEL_FINISHED	0x80
#define INTEL_OK	0x80

typedef unsigned char FLASH_PORT_WIDTH;
typedef volatile unsigned char FLASH_PORT_WIDTHV;
#define FPW		FLASH_PORT_WIDTH
#define FPWV		FLASH_PORT_WIDTHV
#define	FLASH_ID_MASK	0xFF

#define ORMASK(size) ((-size) & OR_AM_MSK)

#define FLASH_CYCLE1	0x0555
#define FLASH_CYCLE2	0x02aa

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips */

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size(FPWV *addr, flash_info_t *info);
static void flash_reset(flash_info_t *info);
static flash_info_t *flash_get_info(ulong base);
static int write_data (flash_info_t *info, FPWV *dest, FPW data); /* O2D */
static void flash_sync_real_protect (flash_info_t * info);
static unsigned char intel_sector_protected (flash_info_t *info, ushort sector);

/*-----------------------------------------------------------------------
 * flash_init()
 *
 * sets up flash_info and returns size of FLASH (bytes)
 */
unsigned long flash_init (void)
{
	unsigned long size = 0;
	int i;
	extern void flash_preinit(void);
	extern void flash_afterinit(ulong);

	flash_preinit();

	/* Init: no FLASHes known */
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		memset(&flash_info[i], 0, sizeof(flash_info_t));
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Query flash chip */
	flash_info[0].size =
		flash_get_size((FPW *)CONFIG_SYS_FLASH_BASE, &flash_info[0]);
	size += flash_info[0].size;

	/* get the h/w and s/w protection status in sync */
	flash_sync_real_protect(&flash_info[0]);

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
			CONFIG_SYS_MONITOR_BASE,
			CONFIG_SYS_MONITOR_BASE+monitor_flash_len-1,
			flash_get_info(CONFIG_SYS_MONITOR_BASE));
#endif

#ifdef	CONFIG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
			CONFIG_ENV_ADDR,
			CONFIG_ENV_ADDR+CONFIG_ENV_SIZE-1,
			flash_get_info(CONFIG_ENV_ADDR));
#endif


	flash_afterinit(size);
	return (size ? size : 1);
}

/*-----------------------------------------------------------------------
 */
static void flash_reset(flash_info_t *info)
{
	FPWV *base = (FPWV *)(info->start[0]);

	/* Put FLASH back in read mode */
	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL)
		*base = (FPW) INTEL_RESET;	/* Intel Read Mode */
}

/*-----------------------------------------------------------------------
 */

static flash_info_t *flash_get_info(ulong base)
{
	int i;
	flash_info_t * info;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i ++) {
		info = & flash_info[i];
		if (info->size &&
				info->start[0] <= base &&
				base <= info->start[0] + info->size - 1)
			break;
	}

	return (i == CONFIG_SYS_MAX_FLASH_BANKS ? 0 : info);
}

/*-----------------------------------------------------------------------
 */

void flash_print_info (flash_info_t *info)
{
	int i;
	uchar *boottype;
	uchar *bootletter;
	char *fmt;
	uchar botbootletter[] = "B";
	uchar topbootletter[] = "T";
	uchar botboottype[] = "bottom boot sector";
	uchar topboottype[] = "top boot sector";

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
		case FLASH_MAN_INTEL:
			printf ("INTEL ");
			break;
		default:
			printf ("Unknown Vendor ");
			break;
	}

	/* check for top or bottom boot, if it applies */
	if (info->flash_id & FLASH_BTYPE) {
		boottype = botboottype;
		bootletter = botbootletter;
	} else {
		boottype = topboottype;
		bootletter = topbootletter;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
		case FLASH_28F128J3A:
			fmt = "28F128J3 (128 Mbit, uniform sectors)\n";
			break;
		default:
			fmt = "Unknown Chip Type\n";
			break;
	}

	printf (fmt, bootletter, boottype);
	printf ("  Size: %ld MB in %d Sectors\n",
			info->size >> 20,
			info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf ("\n   ");

		printf (" %08lX%s", info->start[i],
				info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");
}

/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */
ulong flash_get_size (FPWV *addr, flash_info_t *info)
{
	int i;

	/* Write auto select command: read Manufacturer ID */
	/* Write auto select command sequence and test FLASH answer */
	addr[FLASH_CYCLE1] = (FPW) INTEL_READID;	/* selects Intel or AMD */

	/* The manufacturer codes are only 1 byte, so just use 1 byte.
	 * This works for any bus width and any FLASH device width.
	 */
	udelay(100);
	switch (addr[0] & 0xff) {
		case (uchar)INTEL_MANUFACT:
			info->flash_id = FLASH_MAN_INTEL;
			break;
		default:
			info->flash_id = FLASH_UNKNOWN;
			info->sector_count = 0;
			info->size = 0;
			break;
	}

	/* Strataflash is configurable to 8/16bit Bus,
	 * but the Query-Structure is Word-orientated */
	if (info->flash_id != FLASH_UNKNOWN) {
		switch ((FPW)addr[2]) {
			case (FPW)INTEL_ID_28F128J3:
				info->flash_id += FLASH_28F128J3A;
				info->sector_count = 128;
				info->size = 0x01000000;
				for( i = 0; i < info->sector_count; i++ )
					info->start[i] = (ulong)addr + (i * 0x20000);
				break;				/* => Intel Strataflash 16MB */
			default:
				printf("Flash_id != %xd\n", (FPW)addr[2]);
				info->flash_id = FLASH_UNKNOWN;
				info->sector_count = 0;
				info->size = 0;
				return (0);			/* => no or unknown flash */
		}
	}

	/* Put FLASH back in read mode */
	flash_reset(info);

	return (info->size);
}

/*-----------------------------------------------------------------------
 */

int flash_erase (flash_info_t *info, int s_first, int s_last)
{
	FPWV *addr;
	int flag, prot, sect;
	ulong start, now, last;
	int rcode = 0;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
		case FLASH_28F128J3A:
			break;
		case FLASH_UNKNOWN:
		default:
			printf ("Can't erase unknown flash type %08lx - aborted\n",
					info->flash_id);
			return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect)
		if (info->protect[sect])
			prot++;

	if (prot)
		printf ("- Warning: %d protected sectors will not be erased!",
				prot);

	printf ("\n");
	last = get_timer(0);

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last && rcode == 0; sect++) {

		if (info->protect[sect] != 0)	/* protected, skip it */
			continue;

		/* Disable interrupts which might cause a timeout here */
		flag = disable_interrupts();

		addr = (FPWV *)(info->start[sect]);
		*addr = (FPW) INTEL_CLEAR; /* clear status register */
		*addr = (FPW) INTEL_ERASE; /* erase setup */
		*addr = (FPW) INTEL_CONFIRM; /* erase confirm */

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts();

		start = get_timer(0);

		/* wait at least 80us for Intel - let's wait 1 ms */
		udelay (1000);

		while ((*addr & (FPW) INTEL_FINISHED) != (FPW) INTEL_FINISHED) {
			if ((now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
				printf ("Timeout\n");
				*addr = (FPW) INTEL_SUSPEND;/* suspend erase */
				flash_reset(info);	/* reset to read mode */
				rcode = 1;		/* failed */
				break;
			}

			/* show that we're waiting */
			if ((get_timer(last)) > CONFIG_SYS_HZ) { /* every second */
				putc ('.');
				last = get_timer(0);
			}
		}

		flash_reset(info);	/* reset to read mode */
	}

	printf (" done\n");
	return rcode;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	FPW data = 0;	/* 16 or 32 bit word, matches flash bus width on MPC8XX */
	int bytes;	/* number of bytes to program in current word		*/
	int left;	/* number of bytes left to program			*/
	int i, res;

	for (left = cnt, res = 0;
			left > 0 && res == 0;
			addr += sizeof(data), left -= sizeof(data) - bytes) {

		bytes = addr & (sizeof(data) - 1);
		addr &= ~(sizeof(data) - 1);

		/* combine source and destination data so can program
		 * an entire word of 16 or 32 bits  */
		for (i = 0; i < sizeof(data); i++) {
			data <<= 8;
			if (i < bytes || i - bytes >= left )
				data += *((uchar *)addr + i);
			else
				data += *src++;
		}

		/* write one word to the flash */
		switch (info->flash_id & FLASH_VENDMASK) {
			case FLASH_MAN_INTEL:
				res = write_data(info, (FPWV *)addr, data);
				break;
			default:
				/* unknown flash type, error! */
				printf ("missing or unknown FLASH type\n");
				res = 1;	/* not really a timeout, but gives error */
				break;
		}
	}

	return (res);
}

/*-----------------------------------------------------------------------
 * Write a word or halfword to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_data (flash_info_t *info, FPWV *dest, FPW data)
{
	FPWV *addr = dest;
	ulong status;
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*addr & data) != data) {
		printf ("not erased at %08lx (%x)\n", (ulong) addr, *addr);
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	*addr = (FPW) INTEL_PROGRAM;	/* write setup */
	*addr = data;

	/* arm simple, non interrupt dependent timer */
	start = get_timer(0);

	/* wait while polling the status register */
	while (((status = *addr) & (FPW) INTEL_FINISHED) != (FPW) INTEL_FINISHED) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			*addr = (FPW) INTEL_RESET;	/* restore read mode */
			return (1);
		}
	}

	*addr = (FPW) INTEL_RESET;	/* restore read mode */
	if (flag)
		enable_interrupts();

	return (0);
}


/*-----------------------------------------------------------------------
 * Set/Clear sector's lock bit, returns:
 * 0 - OK
 * 1 - Error (timeout, voltage problems, etc.)
 */
int flash_real_protect (flash_info_t * info, long sector, int prot)
{
	ulong start;
	int i;
	int rc = 0;
	FPWV *addr = (FPWV *) (info->start[sector]);
	int flag = disable_interrupts ();

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
		if (get_timer (start) > CONFIG_SYS_FLASH_UNLOCK_TOUT) {
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
		for (i = 0; i < info->sector_count; i++) {
			if (info->protect[i]) {
				start = get_timer (0);
				addr = (FPWV *) (info->start[i]);
				*addr = INTEL_LOCKBIT;	/* Sector lock bit  */
				*addr = INTEL_PROTECT;	/* set              */
				while ((*addr & INTEL_FINISHED) !=
				       INTEL_FINISHED) {
					if (get_timer (start) >
					    CONFIG_SYS_FLASH_UNLOCK_TOUT) {
						printf ("Flash lock bit operation timed out\n");
						rc = 1;
						break;
					}
				}
			}
		}
	}

	if (flag)
		enable_interrupts ();

	*addr = INTEL_RESET;	/* Reset to read array mode */

	return rc;
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
	 * CONFIG_SYS_FLASH_ERASE_TOUT is as follows. The WSM can be busy
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
		if (get_timer (start) > CONFIG_SYS_FLASH_ERASE_TOUT) {
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
