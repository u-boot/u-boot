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

#include <asm/immap.h>

#ifndef CFG_FLASH_CFI
typedef unsigned char FLASH_PORT_WIDTH;
typedef volatile unsigned char FLASH_PORT_WIDTHV;

#define FPW             FLASH_PORT_WIDTH
#define FPWV            FLASH_PORT_WIDTHV

#define CFG_FLASH_CFI_WIDTH	FLASH_CFI_8BIT
#define CFG_FLASH_NONCFI_WIDTH	FLASH_CFI_8BIT

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
#define INTEL_WRBLK	0x00e800e8
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
#define INTEL_CFI_CAP	0x28
#define INTEL_CFI_WRBUF	0x2A
#define INTEL_CFI_BANK	0x2C	/* Number of Bank */
#define INTEL_CFI_BLK1A	0x2D	/* Number of Blocks */
#define INTEL_CFI_BLK1B	0x2E	/* Number of Blocks */
#define INTEL_CFI_SZ1A	0x2F	/* Block Region Size */
#define INTEL_CFI_SZ1B	0x30
#define INTEL_CFI_BLK2A	0x31
#define INTEL_CFI_BLK2B	0x32
#define INTEL_CFI_SZ2A	0x33
#define INTEL_CFI_SZ2B	0x34

#define FLASH_CYCLE1    0x0555
#define FLASH_CYCLE2    0x0aaa

#define WR_BLOCK        0x20

/* not in the flash.h yet */
#define FLASH_28F64P30T		0x00B9	/* Intel 28F64P30T   (  64M)            */
#define FLASH_28F64P30B		0x00BA	/* Intel 28F64P30B   (  64M)            */
#define FLASH_28F128P30T	0x00BB	/* Intel 28F128P30T  ( 128M = 8M x 16 ) */
#define FLASH_28F128P30B	0x00BC	/* Intel 28F128P30B  ( 128M = 8M x 16 ) */
#define FLASH_28F256P30T	0x00BD	/* Intel 28F256P30T  ( 256M = 16M x 16 )        */
#define FLASH_28F256P30B	0x00BE	/* Intel 28F256P30B  ( 256M = 16M x 16 )        */

#if defined(CONFIG_SERIAL_FLASH) && defined(CONFIG_CF_DSPI)
#define STM_ID_M25P16		0x20152015
#define FLASH_M25P16		0x0055
#endif

#define SYNC			__asm__("nop")

/*-----------------------------------------------------------------------
 * Functions
 */

ulong flash_get_size(FPWV * addr, flash_info_t * info);
int flash_get_offsets(ulong base, flash_info_t * info);
int flash_cmd_rd(volatile u16 * addr, int index);
int write_data(flash_info_t * info, ulong dest, FPW data);
int write_data_block(flash_info_t * info, ulong src, ulong dest);
int write_word_atm(flash_info_t * info, volatile u8 * dest, u16 data);
void inline spin_wheel(void);
void flash_sync_real_protect(flash_info_t * info);
uchar intel_sector_protected(flash_info_t * info, ushort sector);

#if defined(CONFIG_SERIAL_FLASH) && defined(CONFIG_CF_DSPI)
int write_ser_data(flash_info_t * info, ulong dest, uchar * data, ulong cnt);
int serial_flash_read_status(int chipsel);
static int ser_flash_cs = 0;
#endif

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

ulong flash_init(void)
{
	int i;
	ulong size = 0;
	ulong fbase = 0;

#if defined(CONFIG_SERIAL_FLASH) && defined(CONFIG_CF_DSPI)
	dspi_init();
#endif

	for (i = 0; i < CFG_MAX_FLASH_BANKS; i++) {
		memset(&flash_info[i], 0, sizeof(flash_info_t));

		switch (i) {
		case 0:
			fbase = (ulong) CFG_FLASH0_BASE;
			break;
		case 1:
			fbase = (ulong) CFG_FLASH1_BASE;
			break;
#if defined(CONFIG_SERIAL_FLASH) && defined(CONFIG_CF_DSPI)
		case 2:
			fbase = (ulong) CFG_FLASH2_BASE;
			break;
#endif
		}

		flash_get_size((FPWV *) fbase, &flash_info[i]);
		flash_get_offsets((ulong) fbase, &flash_info[i]);
		fbase += flash_info[i].size;
		size += flash_info[i].size;

		/* get the h/w and s/w protection status in sync */
		flash_sync_real_protect(&flash_info[i]);
	}

	/* Protect monitor and environment sectors */
	flash_protect(FLAG_PROTECT_SET,
		      CFG_MONITOR_BASE,
		      CFG_MONITOR_BASE + monitor_flash_len - 1, &flash_info[0]);

	return size;
}

int flash_get_offsets(ulong base, flash_info_t * info)
{
	int i, j, k;
	int sectors, bs, banks;

	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_ATM) {
		int sect[] = CFG_ATMEL_SECT;
		int sectsz[] = CFG_ATMEL_SECTSZ;

		info->start[0] = base;
		for (k = 0, i = 0; i < CFG_ATMEL_REGION; i++) {
			for (j = 0; j < sect[i]; j++, k++) {
				info->start[k + 1] = info->start[k] + sectsz[i];
				info->protect[k] = 0;
			}
		}
	}

	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL) {
		volatile u16 *addr16 = (volatile u16 *)base;

		*addr16 = (FPW) INTEL_RESET;	/* restore read mode */
		*addr16 = (FPW) INTEL_READID;

		banks = addr16[INTEL_CFI_BANK] & 0xff;

		sectors = 0;
		info->start[0] = base;

		for (k = 0, i = 0; i < banks; i++) {
			/* Geometry y1 = y1 + 1, y2 = y2 + 1, CFI spec.
			 * To be exact, Z = [0x2f 0x30] (LE) * 256 bytes * [0x2D 0x2E] block count
			 * Z = [0x33 0x34] (LE) * 256 bytes * [0x31 0x32] block count
			 */
			bs = ((((addr16[INTEL_CFI_SZ1B + (i * 4)] & 0xff) << 8)
			       | (addr16[INTEL_CFI_SZ1A + (i * 4)] & 0xff)) *
			      0x100);
			sectors =
			    (addr16[INTEL_CFI_BLK1A + (i * 4)] & 0xff) + 1;

			for (j = 0; j < sectors; j++, k++) {
				info->start[k + 1] = info->start[k] + bs;
			}
		}

		*addr16 = (FPW) INTEL_RESET;	/* restore read mode */
	}
#if defined(CONFIG_SERIAL_FLASH) && defined(CONFIG_CF_DSPI)
	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_STM) {
		info->start[0] = CFG_FLASH2_BASE;
		for (k = 0, i = 0; i < CFG_STM_SECT; i++, k++) {
			info->start[k + 1] = info->start[k] + CFG_STM_SECTSZ;
			info->protect[k] = 0;
		}
	}
#endif

	return ERR_OK;
}

void flash_print_info(flash_info_t * info)
{
	int i;

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_INTEL:
		printf("INTEL ");
		break;
	case FLASH_MAN_ATM:
		printf("ATMEL ");
		break;
#if defined(CONFIG_SERIAL_FLASH) && defined(CONFIG_CF_DSPI)
	case FLASH_MAN_STM:
		printf("ST ");
		break;
#endif
	default:
		printf("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AT040:
		printf("AT49BV040A\n");
		break;
	case FLASH_28F128J3A:
		printf("28F128J3A\n");
		break;
#if defined(CONFIG_SERIAL_FLASH) && defined(CONFIG_CF_DSPI)
	case FLASH_M25P16:
		printf("M25P16\n");
		break;
#endif
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
	volatile u16 *addr16 = (volatile u16 *)addr;
	int intel = 0, banks = 0;
	u16 value;
	int i;

#if defined(CONFIG_SERIAL_FLASH) && defined(CONFIG_CF_DSPI)
	if ((ulong) addr == CFG_FLASH2_BASE) {
		int manufactId = 0;
		int deviceId = 0;

		ser_flash_cs = 1;

		dspi_tx(ser_flash_cs, 0x80, SER_RDID);
		dspi_tx(ser_flash_cs, 0x80, 0);
		dspi_tx(ser_flash_cs, 0x80, 0);
		dspi_tx(ser_flash_cs, 0x80, 0);

		dspi_rx();
		manufactId = dspi_rx();
		deviceId = dspi_rx() << 8;
		deviceId |= dspi_rx();

		dspi_tx(ser_flash_cs, 0x00, 0);
		dspi_rx();

		switch (manufactId) {
		case (u8) STM_MANUFACT:
			info->flash_id = FLASH_MAN_STM;
			break;
		}

		switch (deviceId) {
		case (u16) STM_ID_M25P16:
			info->flash_id += FLASH_M25P16;
			break;
		}

		info->sector_count = CFG_STM_SECT;
		info->size = CFG_STM_SECT * CFG_STM_SECTSZ;

		return (info->size);
	}
#endif

	addr[FLASH_CYCLE1] = (FPWV) 0x00AA00AA;	/* for Atmel, Intel ignores this */
	addr[FLASH_CYCLE2] = (FPWV) 0x00550055;	/* for Atmel, Intel ignores this */
	addr[FLASH_CYCLE1] = (FPWV) 0x00900090;	/* selects Intel or Atmel */

	switch (addr[0] & 0xff) {
	case (u8) ATM_MANUFACT:
		info->flash_id = FLASH_MAN_ATM;
		value = addr[1];
		break;
	case (u8) INTEL_MANUFACT:
		/* Terminate Atmel ID read */
		addr[0] = (FPWV) 0x00F000F0;
		/* Write auto select command: read Manufacturer ID */
		/* Write auto select command sequence and test FLASH answer */
		*addr16 = (FPW) INTEL_RESET;	/* restore read mode */
		*addr16 = (FPW) INTEL_READID;

		info->flash_id = FLASH_MAN_INTEL;
		value = (addr16[INTEL_CFI_MFG] << 8);
		value |= addr16[INTEL_CFI_PART] & 0xff;
		intel = 1;
		break;
	default:
		printf("Unknown Flash\n");
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;

		*addr = (FPW) 0x00F000F0;
		*addr = (FPW) INTEL_RESET;	/* restore read mode */
		return (0);	/* no or unknown flash  */
	}

	switch (value) {
	case (u8) ATM_ID_LV040:
		info->flash_id += FLASH_AT040;
		break;
	case (u16) INTEL_ID_28F128J3:
		info->flash_id += FLASH_28F128J3A;
		break;
	case (u16) INTEL_ID_28F64P30T:
		info->flash_id += FLASH_28F64P30T;
		break;
	case (u16) INTEL_ID_28F64P30B:
		info->flash_id += FLASH_28F64P30B;
		break;
	case (u16) INTEL_ID_28F128P30T:
		info->flash_id += FLASH_28F128P30T;
		break;
	case (u16) INTEL_ID_28F128P30B:
		info->flash_id += FLASH_28F128P30B;
		break;
	case (u16) INTEL_ID_28F256P30T:
		info->flash_id += FLASH_28F256P30T;
		break;
	case (u16) INTEL_ID_28F256P30B:
		info->flash_id += FLASH_28F256P30B;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		break;
	}

	if (intel) {
		/* Intel spec. under CFI section */
		u32 sz;
		int sectors, bs;

		banks = addr16[INTEL_CFI_BANK] & 0xff;

		sectors = sz = 0;
		for (i = 0; i < banks; i++) {
			/* Geometry y1 = y1 + 1, y2 = y2 + 1, CFI spec.
			 * To be exact, Z = [0x2f 0x30] (LE) * 256 bytes * [0x2D 0x2E] block count
			 * Z = [0x33 0x34] (LE) * 256 bytes * [0x31 0x32] block count
			 */
			bs = ((((addr16[INTEL_CFI_SZ1B + (i * 4)] & 0xff) << 8)
			       | (addr16[INTEL_CFI_SZ1A + (i * 4)] & 0xff)) *
			      0x100);
			sectors +=
			    (addr16[INTEL_CFI_BLK1A + (i * 4)] & 0xff) + 1;
			sz += (bs * sectors);
		}

		info->sector_count = sectors;
		info->size = sz;
		*addr = (FPW) INTEL_RESET;	/* restore read mode */
	} else {
		int sect[] = CFG_ATMEL_SECT;
		int sectsz[] = CFG_ATMEL_SECTSZ;

		info->sector_count = 0;
		info->size = 0;
		for (i = 0; i < CFG_ATMEL_REGION; i++) {
			info->sector_count += sect[i];
			info->size += sect[i] * sectsz[i];
		}

		/* reset ID mode */
		addr[0] = (FPWV) 0x00F000F0;
	}

	if (info->sector_count > CFG_MAX_FLASH_SECT) {
		printf("** ERROR: sector count %d > max (%d) **\n",
		       info->sector_count, CFG_MAX_FLASH_SECT);
		info->sector_count = CFG_MAX_FLASH_SECT;
	}

	return (info->size);
}

int flash_cmd_rd(volatile u16 * addr, int index)
{
	return (int)addr[index];
}

#if defined(CONFIG_SERIAL_FLASH) && defined(CONFIG_CF_DSPI)
int serial_flash_read_status(int chipsel)
{
	u16 status;

	dspi_tx(chipsel, 0x80, SER_RDSR);
	dspi_rx();

	dspi_tx(chipsel, 0x00, 0);
	status = dspi_rx();

	return status;
}
#endif

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
	int rcode = 0, flashtype = 0;
#if defined(CONFIG_SERIAL_FLASH) && defined(CONFIG_CF_DSPI)
	int count;
	u16 status;
#endif
	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN)
			printf("- missing\n");
		else
			printf("- no sectors to erase\n");
		return 1;
	}

	type = (info->flash_id & FLASH_VENDMASK);

	switch (type) {
	case FLASH_MAN_ATM:
		flashtype = 1;
		break;
	case FLASH_MAN_INTEL:
		flashtype = 2;
		break;
#if defined(CONFIG_SERIAL_FLASH) && defined(CONFIG_CF_DSPI)
	case FLASH_MAN_STM:
		flashtype = 3;
		break;
#endif
	default:
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

#if defined(CONFIG_SERIAL_FLASH) && defined(CONFIG_CF_DSPI)
	/* Perform bulk erase */
	if (flashtype == 3) {
		if ((s_last - s_first) == (CFG_STM_SECT - 1)) {
			if (prot == 0) {
				dspi_tx(ser_flash_cs, 0x00, SER_WREN);
				dspi_rx();

				status = serial_flash_read_status(ser_flash_cs);
				if (((status & 0x9C) != 0)
				    && ((status & 0x02) != 0x02)) {
					printf("Can't erase flash\n");
					return 1;
				}

				dspi_tx(ser_flash_cs, 0x00, SER_BULK_ERASE);
				dspi_rx();

				count = 0;
				start = get_timer(0);
				do {
					status =
					    serial_flash_read_status
					    (ser_flash_cs);

					if (count++ > 0x10000) {
						spin_wheel();
						count = 0;
					}

					if (get_timer(start) >
					    CFG_FLASH_ERASE_TOUT) {
						printf("Timeout\n");
						return 1;
					}
				} while (status & 0x01);

				printf("\b. done\n");
				return 0;
			} else if (prot == CFG_STM_SECT) {
				return 1;
			}
		}
	}
#endif
	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */

			FPWV *addr = (FPWV *) (info->start[sect]);
			int min = 0;

			printf(".");

			/* arm simple, non interrupt dependent timer */
			start = get_timer(0);

			switch (flashtype) {
			case 1:
				{
					FPWV *base;	/* first address in bank */
					FPWV *atmeladdr;

					flag = disable_interrupts();

					atmeladdr = (FPWV *) addr;	/* concatenate to 8 bit */
					base = (FPWV *) (CFG_ATMEL_BASE);	/* First sector */

					base[FLASH_CYCLE1] = (u8) 0x00AA00AA;	/* unlock */
					base[FLASH_CYCLE2] = (u8) 0x00550055;	/* unlock */
					base[FLASH_CYCLE1] = (u8) 0x00800080;	/* erase mode */
					base[FLASH_CYCLE1] = (u8) 0x00AA00AA;	/* unlock */
					base[FLASH_CYCLE2] = (u8) 0x00550055;	/* unlock */
					*atmeladdr = (u8) 0x00300030;	/* erase sector */

					if (flag)
						enable_interrupts();

					while ((*atmeladdr & (u8) 0x00800080) !=
					       (u8) 0x00800080) {
						if (get_timer(start) >
						    CFG_FLASH_ERASE_TOUT) {
							printf("Timeout\n");
							*atmeladdr = (u8) 0x00F000F0;	/* reset to read mode */

							rcode = 1;
							break;
						}
					}

					*atmeladdr = (u8) 0x00F000F0;	/* reset to read mode */
					break;
				}

			case 2:
				{
					*addr = (FPW) INTEL_READID;
					min = addr[INTEL_CFI_TERB] & 0xff;
					min = 1 << min;	/* ms */
					min = (min / info->sector_count) * 1000;

					/* start erase block */
					*addr = (FPW) INTEL_CLEAR;	/* clear status register */
					*addr = (FPW) INTEL_ERASE;	/* erase setup */
					*addr = (FPW) INTEL_CONFIRM;	/* erase confirm */

					while ((*addr & (FPW) INTEL_FINISHED) !=
					       (FPW) INTEL_FINISHED) {

						if (get_timer(start) >
						    CFG_FLASH_ERASE_TOUT) {
							printf("Timeout\n");
							*addr = (FPW) INTEL_SUSERASE;	/* suspend erase     */
							*addr = (FPW) INTEL_RESET;	/* reset to read mode */

							rcode = 1;
							break;
						}
					}

					*addr = (FPW) INTEL_RESET;	/* resest to read mode          */
					break;
				}

#if defined(CONFIG_SERIAL_FLASH) && defined(CONFIG_CF_DSPI)
			case 3:
				{
					u8 sec = ((ulong) addr >> 16) & 0xFF;

					dspi_tx(ser_flash_cs, 0x00, SER_WREN);
					dspi_rx();
					status =
					    serial_flash_read_status
					    (ser_flash_cs);
					if (((status & 0x9C) != 0)
					    && ((status & 0x02) != 0x02)) {
						printf("Error Programming\n");
						return 1;
					}

					dspi_tx(ser_flash_cs, 0x80,
						SER_SECT_ERASE);
					dspi_tx(ser_flash_cs, 0x80, sec);
					dspi_tx(ser_flash_cs, 0x80, 0);
					dspi_tx(ser_flash_cs, 0x00, 0);

					dspi_rx();
					dspi_rx();
					dspi_rx();
					dspi_rx();

					do {
						status =
						    serial_flash_read_status
						    (ser_flash_cs);

						if (get_timer(start) >
						    CFG_FLASH_ERASE_TOUT) {
							printf("Timeout\n");
							return 1;
						}
					} while (status & 0x01);

					break;
				}
#endif
			}	/* switch (flashtype) */
		}
	}
	printf(" done\n");

	return rcode;
}

int write_buff(flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	int count;

	if (info->flash_id == FLASH_UNKNOWN)
		return 4;

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_ATM:
		{
			u16 data = 0;
			int bytes;	/* number of bytes to program in current word */
			int left;	/* number of bytes left to program */
			int i, res;

			for (left = cnt, res = 0;
			     left > 0 && res == 0;
			     addr += sizeof(data), left -=
			     sizeof(data) - bytes) {

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

				data = (data >> 8) | (data << 8);
				res = write_word_atm(info, (FPWV *) addr, data);
			}
			return res;
		}		/* case FLASH_MAN_ATM */

	case FLASH_MAN_INTEL:
		{
			ulong cp, wp;
			u16 data;
			int i, l, rc, port_width;

			/* get lower word aligned address */
			wp = addr;
			port_width = sizeof(FPW);

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

			if (cnt > WR_BLOCK) {
				/*
				 * handle word aligned part
				 */
				count = 0;
				while (cnt >= WR_BLOCK) {

					if ((rc =
					     write_data_block(info,
							      (ulong) src,
							      wp)) != 0)
						return (rc);

					wp += WR_BLOCK;
					src += WR_BLOCK;
					cnt -= WR_BLOCK;

					if (count++ > 0x800) {
						spin_wheel();
						count = 0;
					}
				}
			}

			/* handle word aligned part */
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
					     write_data(info,
							(ulong) ((FPWV *) wp),
							(FPW) (data))) != 0)
						return (rc);

					wp += port_width;
					cnt -= port_width;
					if (count++ > 0x800) {
						spin_wheel();
						count = 0;
					}
				}
			}

			if (cnt == 0)
				return ERR_OK;

			/*
			 * handle unaligned tail bytes
			 */
			data = 0;
			for (i = 0, cp = wp; i < port_width && cnt > 0;
			     ++i, ++cp) {
				data = (data << 8) | (*src++);
				--cnt;
			}
			for (; i < port_width; ++i, ++cp) {
				data = (data << 8) | (*(uchar *) cp);
			}

			return write_data(info, (ulong) ((FPWV *) wp),
					  (FPW) data);

		}		/* case FLASH_MAN_INTEL */

#if defined(CONFIG_SERIAL_FLASH) && defined(CONFIG_CF_DSPI)
	case FLASH_MAN_STM:
		{
			ulong wp;
			u8 *data = (u8 *) src;
			int left;	/* number of bytes left to program */

			wp = addr;

			/* page align, each page is 256 bytes */
			if ((wp % 0x100) != 0) {
				left = (0x100 - (wp & 0xFF));
				write_ser_data(info, wp, data, left);
				cnt -= left;
				wp += left;
				data += left;
			}

			/* page program - 256 bytes at a time */
			if (cnt > 255) {
				count = 0;
				while (cnt >= 0x100) {
					write_ser_data(info, wp, data, 0x100);
					cnt -= 0x100;
					wp += 0x100;
					data += 0x100;

					if (count++ > 0x400) {
						spin_wheel();
						count = 0;
					}
				}
			}

			/* remainint bytes */
			if (cnt && (cnt < 256)) {
				write_ser_data(info, wp, data, cnt);
				wp += cnt;
				data += cnt;
				cnt -= cnt;
			}

			printf("\b.");
		}
#endif
	}			/* switch */

	return ERR_OK;
}

/*-----------------------------------------------------------------------
 * Write a word or halfword to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_data_block(flash_info_t * info, ulong src, ulong dest)
{
	FPWV *srcaddr = (FPWV *) src;
	FPWV *dstaddr = (FPWV *) dest;
	ulong start;
	int flag, i;

	/* Check if Flash is (sufficiently) erased */
	for (i = 0; i < WR_BLOCK; i++)
		if ((*dstaddr++ & 0xff) != 0xff) {
			printf("not erased at %08lx (%lx)\n",
			       (ulong) dstaddr, *dstaddr);
			return (2);
		}

	dstaddr = (FPWV *) dest;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	*dstaddr = (FPW) INTEL_WRBLK;	/* write block setup */

	if (flag)
		enable_interrupts();

	/* arm simple, non interrupt dependent timer */
	start = get_timer(0);

	/* wait while polling the status register */
	while ((*dstaddr & (FPW) INTEL_FINISHED) != (FPW) INTEL_OK) {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			*dstaddr = (FPW) INTEL_RESET;	/* restore read mode */
			return (1);
		}
	}

	*dstaddr = (FPW) WR_BLOCK - 1;	/* write 32 to buffer */
	for (i = 0; i < WR_BLOCK; i++)
		*dstaddr++ = *srcaddr++;

	dstaddr -= 1;
	*dstaddr = (FPW) INTEL_CONFIRM;	/* write 32 to buffer */

	/* arm simple, non interrupt dependent timer */
	start = get_timer(0);

	/* wait while polling the status register */
	while ((*dstaddr & (FPW) INTEL_FINISHED) != (FPW) INTEL_OK) {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			*dstaddr = (FPW) INTEL_RESET;	/* restore read mode */
			return (1);
		}
	}

	*dstaddr = (FPW) INTEL_RESET;	/* restore read mode */

	return (0);
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

	if (flag)
		enable_interrupts();

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

#if defined(CONFIG_SERIAL_FLASH) && defined(CONFIG_CF_DSPI)
int write_ser_data(flash_info_t * info, ulong dest, uchar * data, ulong cnt)
{
	ulong start;
	int status, i;
	u8 flashdata;

	/* Check if Flash is (sufficiently) erased */
	dspi_tx(ser_flash_cs, 0x80, SER_READ);
	dspi_tx(ser_flash_cs, 0x80, (dest >> 16) & 0xFF);
	dspi_tx(ser_flash_cs, 0x80, (dest >> 8) & 0xFF);
	dspi_tx(ser_flash_cs, 0x80, dest & 0xFF);
	dspi_rx();
	dspi_rx();
	dspi_rx();
	dspi_rx();
	dspi_tx(ser_flash_cs, 0x80, 0);
	flashdata = dspi_rx();
	dspi_tx(ser_flash_cs, 0x00, 0);
	dspi_rx();

	if ((flashdata & *data) != *data) {
		printf("not erased at %08lx (%lx)\n", (ulong) dest,
		       (ulong) flashdata);
		return (2);
	}

	dspi_tx(ser_flash_cs, 0x00, SER_WREN);
	dspi_rx();

	status = serial_flash_read_status(ser_flash_cs);
	if (((status & 0x9C) != 0) && ((status & 0x02) != 0x02)) {
		printf("Error Programming\n");
		return 1;
	}

	start = get_timer(0);

	dspi_tx(ser_flash_cs, 0x80, SER_PAGE_PROG);
	dspi_tx(ser_flash_cs, 0x80, ((dest & 0xFF0000) >> 16));
	dspi_tx(ser_flash_cs, 0x80, ((dest & 0xFF00) >> 8));
	dspi_tx(ser_flash_cs, 0x80, (dest & 0xFF));
	dspi_rx();
	dspi_rx();
	dspi_rx();
	dspi_rx();

	for (i = 0; i < (cnt - 1); i++) {
		dspi_tx(ser_flash_cs, 0x80, *data);
		dspi_rx();
		data++;
	}

	dspi_tx(ser_flash_cs, 0x00, *data);
	dspi_rx();

	do {
		status = serial_flash_read_status(ser_flash_cs);

		if (get_timer(start) > CFG_FLASH_ERASE_TOUT) {
			printf("Timeout\n");
			return 1;
		}
	} while (status & 0x01);

	return (0);
}
#endif

/*-----------------------------------------------------------------------
 * Write a word to Flash for ATMEL FLASH
 * A word is 16 bits, whichever the bus width of the flash bank
 * (not an individual chip) is.
 *
 * returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_word_atm(flash_info_t * info, volatile u8 * dest, u16 data)
{
	ulong start;
	int flag, i;
	int res = 0;		/* result, assume success */
	FPWV *base;		/* first address in flash bank */

	/* Check if Flash is (sufficiently) erased */
	if ((*((volatile u16 *)dest) & data) != data) {
		return (2);
	}

	base = (FPWV *) (CFG_ATMEL_BASE);

	for (i = 0; i < sizeof(u16); i++) {
		/* Disable interrupts which might cause a timeout here */
		flag = disable_interrupts();

		base[FLASH_CYCLE1] = (u8) 0x00AA00AA;	/* unlock */
		base[FLASH_CYCLE2] = (u8) 0x00550055;	/* unlock */
		base[FLASH_CYCLE1] = (u8) 0x00A000A0;	/* selects program mode */

		*dest = data;	/* start programming the data */

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts();

		start = get_timer(0);

		/* data polling for D7 */
		while (res == 0
		       && (*dest & (u8) 0x00800080) !=
		       (data & (u8) 0x00800080)) {
			if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
				*dest = (u8) 0x00F000F0;	/* reset bank */
				res = 1;
			}
		}

		*dest++ = (u8) 0x00F000F0;	/* reset bank */
		data >>= 8;
	}

	return (res);
}

void inline spin_wheel(void)
{
	static int p = 0;
	static char w[] = "\\/-";

	printf("\010%c", w[p]);
	(++p == 3) ? (p = 0) : 0;
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
#endif
#endif
