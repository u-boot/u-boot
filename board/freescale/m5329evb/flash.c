/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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

#ifndef CFG_FLASH_CFI

typedef unsigned short FLASH_PORT_WIDTH;
typedef volatile unsigned short FLASH_PORT_WIDTHV;

#define PHYS_FLASH_1 CFG_FLASH_BASE
#define FLASH_BANK_SIZE 0x200000

#define FPW             FLASH_PORT_WIDTH
#define FPWV            FLASH_PORT_WIDTHV

/* Intel-compatible flash commands */
#define INTEL_PROGRAM   0x00100010
#define INTEL_ERASE     0x00200020
#define INTEL_WRSETUP	0x00400040
#define INTEL_CLEAR     0x00500050
#define INTEL_LOCKBIT   0x00600060
#define INTEL_PROTECT   0x00010001
#define INTEL_STATUS    0x00700070
#define INTEL_READID    0x00900090
#define INTEL_CFIQRY	0x00980098
#define INTEL_SUSERASE	0x00B000B0
#define INTEL_PROTPROG	0x00C000C0
#define INTEL_CONFIRM   0x00D000D0
#define INTEL_RESET     0x00FF00FF

/* Intel-compatible flash status bits */
#define INTEL_FINISHED  0x00800080
#define INTEL_OK        0x00800080
#define INTEL_ERASESUS  0x00600060
#define INTEL_WSM_SUS   (INTEL_FINISHED | INTEL_ERASESUS)

/* 28F160C3B CFI Data offset - This could vary */
#define INTEL_CFI_MFG	0x00	/* Manufacturer ID */
#define INTEL_CFI_PART	0x01	/* Product ID */
#define INTEL_CFI_LOCK  0x02	/* */
#define INTEL_CFI_TWPRG 0x1F	/* Typical Single Word Program Timeout 2^n us */
#define INTEL_CFI_MBUFW 0x20	/* Typical Max Buffer Write Timeout 2^n us */
#define INTEL_CFI_TERB	0x21	/* Typical Block Erase Timeout 2^n ms */
#define INTEL_CFI_MWPRG 0x23	/* Maximum Word program timeout 2^n us */
#define INTEL_CFI_MERB  0x25	/* Maximum Block Erase Timeout 2^n s */
#define INTEL_CFI_SIZE	0x27	/* Device size 2^n bytes */
#define INTEL_CFI_BANK	0x2C	/* Number of Bank */
#define INTEL_CFI_SZ1A	0x2F	/* Block Region Size */
#define INTEL_CFI_SZ1B	0x30
#define INTEL_CFI_SZ2A	0x33
#define INTEL_CFI_SZ2B	0x34
#define INTEL_CFI_BLK1	0x2D	/* Number of Blocks */
#define INTEL_CFI_BLK2	0x31

#define WR_BLOCK        0x20

#define SYNC			__asm__("nop")

/*-----------------------------------------------------------------------
 * Functions
 */

ulong flash_get_size(FPWV * addr, flash_info_t * info);
int flash_get_offsets(ulong base, flash_info_t * info);
int flash_cmd_rd(FPWV * addr, int index);
int write_data(flash_info_t * info, ulong dest, FPW data);
void flash_sync_real_protect(flash_info_t * info);
uchar intel_sector_protected(flash_info_t * info, ushort sector);

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

ulong flash_init(void)
{
	FPWV *flash_addr[CFG_MAX_FLASH_BANKS];
	ulong size;
	int i;

	flash_addr[0] = (FPW *) CFG_FLASH0_BASE;
#ifdef CFG_FLASH1_BASE
	flash_addr[1] = (FPW *) CFG_FLASH1_BASE;
#endif

	for (i = 0; i < CFG_MAX_FLASH_BANKS; i++) {
		memset(&flash_info[i], 0, sizeof(flash_info_t));

		size = flash_get_size(flash_addr[i], &flash_info[i]);
		flash_protect(FLAG_PROTECT_CLEAR,
			      flash_info[i].start[0],
			      flash_info[i].start[0] + size - 1,
			      &flash_info[0]);
		/* get the h/w and s/w protection status in sync */
		flash_sync_real_protect(&flash_info[i]);
	}

	/* Protect monitor and environment sectors */
	flash_protect(FLAG_PROTECT_SET,
		      CFG_MONITOR_BASE,
		      CFG_MONITOR_BASE + monitor_flash_len - 1, &flash_info[0]);

	return size;
}

void flash_print_info(flash_info_t * info)
{
	int i;

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_INTEL:
		printf("INTEL ");
		break;
	default:
		printf("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F160C3B:
		printf("28F160C3B\n");
		break;
	case FLASH_28F160C3T:
		printf("28F160C3T\n");
		break;
	case FLASH_28F320C3B:
		printf("28F320C3B\n");
		break;
	case FLASH_28F320C3T:
		printf("28F320C3T\n");
		break;
	case FLASH_28F640C3B:
		printf("28F640C3B\n");
		break;
	case FLASH_28F640C3T:
		printf("28F640C3T\n");
		break;
	default:
		printf("Unknown Chip Type\n");
		return;
	}

	if (info->size > 0x100000) {
		int remainder;

		printf("  Size: %ld", info->size >> 20);

		remainder = (info->size % 0x100000);
		if (remainder) {
			remainder >>= 10;
			remainder = (int)((float)
					  (((float)remainder / (float)1024) *
					   10000));
			printf(".%d ", remainder);
		}

		printf("MB in %d Sectors\n", info->sector_count);
	} else
		printf("  Size: %ld KB in %d Sectors\n",
		       info->size >> 10, info->sector_count);

	printf("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf("\n   ");
		printf(" %08lX%s",
		       info->start[i], info->protect[i] ? " (RO)" : "     ");
	}
	printf("\n");
}

/*
 * The following code cannot be run from FLASH!
 */
ulong flash_get_size(FPWV * addr, flash_info_t * info)
{
	int intel = 0;
	u16 value;
	static int bank = 0;

	/* Write auto select command: read Manufacturer ID */
	/* Write auto select command sequence and test FLASH answer */
	*addr = (FPW) INTEL_RESET;	/* restore read mode */
	*addr = (FPW) INTEL_READID;

	switch (addr[INTEL_CFI_MFG] & 0xff) {
	case (ushort) INTEL_MANUFACT:
		info->flash_id = FLASH_MAN_INTEL;
		value = addr[INTEL_CFI_PART];
		intel = 1;
		break;
	default:
		printf("Unknown Flash\n");
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		*addr = (FPW) INTEL_RESET;	/* restore read mode */
		return (0);	/* no or unknown flash  */
	}

	switch (value) {
	case (u16) INTEL_ID_28F160C3B:
		info->flash_id += FLASH_28F160C3B;
		break;
	case (u16) INTEL_ID_28F160C3T:
		info->flash_id += FLASH_28F160C3T;
		break;
	case (u16) INTEL_ID_28F320C3B:
		info->flash_id += FLASH_28F320C3B;
		break;
	case (u16) INTEL_ID_28F320C3T:
		info->flash_id += FLASH_28F320C3T;
		break;
	case (u16) INTEL_ID_28F640C3B:
		info->flash_id += FLASH_28F640C3B;
		break;
	case (u16) INTEL_ID_28F640C3T:
		info->flash_id += FLASH_28F640C3T;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		break;
	}

	if (intel) {
		/* Intel spec. under CFI section */
		u32 sz, size, offset;
		int sec, sectors, bs;
		int part, i, j, cnt;

		part = flash_cmd_rd(addr, INTEL_CFI_BANK);

		/* Geometry y1 = y1 + 1, y2 = y2 + 1, CFI spec.
		 * To be exact, Z = [0x2f 0x30] (LE) * 256 bytes * [0x2D 0x2E] block count
		 * Z = [0x33 0x34] (LE) * 256 bytes * [0x31 0x32] block count
		 */
		offset = (u32) addr;
		sectors = sec = 0;
		size = sz = cnt = 0;
		for (i = 0; i < part; i++) {
			bs = (((addr[INTEL_CFI_SZ1B + i * 4] << 8) |
			       addr[INTEL_CFI_SZ1A + i * 4]) * 0x100);
			sec = addr[INTEL_CFI_BLK1 + i * 4] + 1;
			sz = bs * sec;

			for (j = 0; j < sec; j++) {
				info->start[cnt++] = offset;
				offset += bs;
			}

			sectors += sec;
			size += sz;
		}
		info->sector_count = sectors;
		info->size = size;
	}

	if (info->sector_count > CFG_MAX_FLASH_SECT) {
		printf("** ERROR: sector count %d > max (%d) **\n",
		       info->sector_count, CFG_MAX_FLASH_SECT);
		info->sector_count = CFG_MAX_FLASH_SECT;
	}

	*addr = (FPW) INTEL_RESET;	/* restore read mode */

	return (info->size);
}

int flash_cmd_rd(FPWV * addr, int index)
{
	return (int)addr[index];
}

/*
 * This function gets the u-boot flash sector protection status
 * (flash_info_t.protect[]) in sync with the sector protection
 * status stored in hardware.
 */
void flash_sync_real_protect(flash_info_t * info)
{
	int i;

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F160C3B:
	case FLASH_28F160C3T:
	case FLASH_28F320C3B:
	case FLASH_28F320C3T:
	case FLASH_28F640C3B:
	case FLASH_28F640C3T:
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
uchar intel_sector_protected(flash_info_t * info, ushort sector)
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
	/*udelay(1); */
	addr = (FPWV *) info->start[sector];
	*addr = (FPW) INTEL_STATUS;

	start = get_timer(0);
	while ((*addr & (FPW) INTEL_FINISHED) != (FPW) INTEL_FINISHED) {
		if (get_timer(start) > CFG_FLASH_UNLOCK_TOUT) {
			*addr = (FPW) INTEL_RESET;	/* restore read mode */
			printf("WSM busy too long, can't get prot status\n");
			return 1;
		}
	}

	/* issue the Read Identifier Codes command */
	*addr = (FPW) INTEL_READID;

	/* Intel example code uses offset of 4 for 8-bit flash */
	lock_conf_addr = (FPWV *) info->start[sector];
	ret = (lock_conf_addr[INTEL_CFI_LOCK] & (FPW) INTEL_PROTECT) ? 1 : 0;

	/* put flash back in read mode */
	*addr = (FPW) INTEL_RESET;

	return ret;
}

int flash_erase(flash_info_t * info, int s_first, int s_last)
{
	int flag, prot, sect;
	ulong type, start, last;
	int rcode = 0;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN)
			printf("- missing\n");
		else
			printf("- no sectors to erase\n");
		return 1;
	}

	type = (info->flash_id & FLASH_VENDMASK);
	if ((type != FLASH_MAN_INTEL)) {
		type = (info->flash_id & FLASH_VENDMASK);
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

	if (prot)
		printf("- Warning: %d protected sectors will not be erased!\n",
		       prot);
	else
		printf("\n");

	start = get_timer(0);
	last = start;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */

			FPWV *addr = (FPWV *) (info->start[sect]);
			int min = 0;

			printf("Erasing sector %2d ... ", sect);

			/* arm simple, non interrupt dependent timer */
			start = get_timer(0);

			*addr = (FPW) INTEL_READID;
			min = addr[INTEL_CFI_TERB];
			min = 1 << min;	/* ms */
			min = (min / info->sector_count) * 1000;

			/* start erase block */
			*addr = (FPW) INTEL_CLEAR;	/* clear status register */
			*addr = (FPW) INTEL_ERASE;	/* erase setup */
			*addr = (FPW) INTEL_CONFIRM;	/* erase confirm */

			while ((*addr & (FPW) INTEL_FINISHED) !=
			       (FPW) INTEL_FINISHED) {

				if (get_timer(start) > CFG_FLASH_ERASE_TOUT) {
					printf("Timeout\n");
					*addr = (FPW) INTEL_SUSERASE;	/* suspend erase     */
					*addr = (FPW) INTEL_RESET;	/* reset to read mode */

					rcode = 1;
					break;
				}
			}

			*addr = (FPW) INTEL_RESET;	/* resest to read mode          */

			printf(" done\n");
		}
	}

	return rcode;
}

int write_buff(flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	if (info->flash_id == FLASH_UNKNOWN)
		return 4;

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_INTEL:
		{
			ulong cp, wp;
			FPW data;
			int i, l, rc, port_width;

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

				if ((rc = write_data(info, wp, data)) != 0)
					return (rc);

				wp += port_width;
			}

			/* handle word aligned part */
			while (cnt >= 2) {
				data = *((FPW *) src);

				if ((rc =
				     write_data(info, (ulong) ((FPWV *) wp),
						(FPW) data)) != 0) {
					return (rc);
				}

				src += sizeof(FPW);
				wp += sizeof(FPW);
				cnt -= sizeof(FPW);
			}

			if (cnt == 0)
				return ERR_OK;

			/*
			 * handle unaligned tail bytes
			 */
			data = 0;
			for (i = 0, cp = wp; i < 2 && cnt > 0; ++i, ++cp) {
				data = (data >> 8) | (*src++ << 8);
				--cnt;
			}
			for (; i < 2; ++i, ++cp) {
				data |= (*(uchar *) cp);
			}

			return write_data(info, (ulong) ((FPWV *) wp),
					  (FPW) data);

		}		/* case FLASH_MAN_INTEL */

	}			/* switch */

	return ERR_OK;
}

/*-----------------------------------------------------------------------
 * Write a word or halfword to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_data(flash_info_t * info, ulong dest, FPW data)
{
	FPWV *addr = (FPWV *) dest;
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*addr & data) != data) {
		printf("not erased at %08lx (%lx)\n", (ulong) addr,
		       (ulong) * addr);
		return (2);
	}

	/* Disable interrupts which might cause a timeout here */
	flag = (int)disable_interrupts();

	*addr = (FPW) INTEL_CLEAR;
	*addr = (FPW) INTEL_RESET;

	*addr = (FPW) INTEL_WRSETUP;	/* write setup */
	*addr = data;

	/* arm simple, non interrupt dependent timer */
	start = get_timer(0);

	/* wait while polling the status register */
	while ((*addr & (FPW) INTEL_OK) != (FPW) INTEL_OK) {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			*addr = (FPW) INTEL_SUSERASE;	/* suspend mode */
			*addr = (FPW) INTEL_CLEAR;	/* clear status */
			*addr = (FPW) INTEL_RESET;	/* reset */
			return (1);
		}
	}

	*addr = (FPW) INTEL_CLEAR;	/* clear status */
	*addr = (FPW) INTEL_RESET;	/* restore read mode */

	return (0);
}

#ifdef CFG_FLASH_PROTECTION
/*-----------------------------------------------------------------------
 */
int flash_real_protect(flash_info_t * info, long sector, int prot)
{
	int rcode = 0;		/* assume success */
	FPWV *addr;		/* address of sector */
	FPW value;

	addr = (FPWV *) (info->start[sector]);

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F160C3B:
	case FLASH_28F160C3T:
	case FLASH_28F320C3B:
	case FLASH_28F320C3T:
	case FLASH_28F640C3B:
	case FLASH_28F640C3T:
		*addr = (FPW) INTEL_RESET;	/* make sure in read mode */
		*addr = (FPW) INTEL_LOCKBIT;	/* lock command setup */

		if (prot)
			*addr = (FPW) INTEL_PROTECT;	/* lock sector */
		else
			*addr = (FPW) INTEL_CONFIRM;	/* unlock sector */

		/* now see if it really is locked/unlocked as requested */
		*addr = (FPW) INTEL_READID;

		/* read sector protection at sector address, (A7 .. A0) = 0x02.
		 * D0 = 1 for each device if protected.
		 * If at least one device is protected the sector is marked
		 * protected, but return failure. Mixed protected and
		 * unprotected devices within a sector should never happen.
		 */
		value = addr[2] & (FPW) INTEL_PROTECT;
		if (value == 0)
			info->protect[sector] = 0;
		else if (value == (FPW) INTEL_PROTECT)
			info->protect[sector] = 1;
		else {
			/* error, mixed protected and unprotected */
			rcode = 1;
			info->protect[sector] = 1;
		}
		if (info->protect[sector] != prot)
			rcode = 1;	/* failed to protect/unprotect as requested */

		/* reload all protection bits from hardware for now */
		flash_sync_real_protect(info);
		break;

	default:
		/* no hardware protect that we support */
		info->protect[sector] = prot;
		break;
	}

	return rcode;
}
#endif				/* CFG_FLASH_PROTECTION */
#endif				/* CFG_FLASH_CFI */
