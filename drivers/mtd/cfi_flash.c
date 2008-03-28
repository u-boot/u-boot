/*
 * (C) Copyright 2002-2004
 * Brad Kemp, Seranoa Networks, Brad.Kemp@seranoa.com
 *
 * Copyright (C) 2003 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 *
 * Copyright (C) 2004
 * Ed Okerson
 *
 * Copyright (C) 2006
 * Tolunay Orkun <listmember@orkun.us>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

/* The DEBUG define must be before common to enable debugging */
/* #define DEBUG	*/

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/byteorder.h>
#include <environment.h>
#ifdef	CFG_FLASH_CFI_DRIVER

/*
 * This file implements a Common Flash Interface (CFI) driver for
 * U-Boot.
 *
 * The width of the port and the width of the chips are determined at
 * initialization.  These widths are used to calculate the address for
 * access CFI data structures.
 *
 * References
 * JEDEC Standard JESD68 - Common Flash Interface (CFI)
 * JEDEC Standard JEP137-A Common Flash Interface (CFI) ID Codes
 * Intel Application Note 646 Common Flash Interface (CFI) and Command Sets
 * Intel 290667-008 3 Volt Intel StrataFlash Memory datasheet
 * AMD CFI Specification, Release 2.0 December 1, 2001
 * AMD/Spansion Application Note: Migration from Single-byte to Three-byte
 *   Device IDs, Publication Number 25538 Revision A, November 8, 2001
 *
 * Define CFG_WRITE_SWAPPED_DATA, if you have to swap the Bytes between
 * reading and writing ... (yes there is such a Hardware).
 */

#ifndef CFG_FLASH_BANKS_LIST
#define CFG_FLASH_BANKS_LIST { CFG_FLASH_BASE }
#endif

#define FLASH_CMD_CFI			0x98
#define FLASH_CMD_READ_ID		0x90
#define FLASH_CMD_RESET			0xff
#define FLASH_CMD_BLOCK_ERASE		0x20
#define FLASH_CMD_ERASE_CONFIRM		0xD0
#define FLASH_CMD_WRITE			0x40
#define FLASH_CMD_PROTECT		0x60
#define FLASH_CMD_PROTECT_SET		0x01
#define FLASH_CMD_PROTECT_CLEAR		0xD0
#define FLASH_CMD_CLEAR_STATUS		0x50
#define FLASH_CMD_WRITE_TO_BUFFER	0xE8
#define FLASH_CMD_WRITE_BUFFER_CONFIRM	0xD0

#define FLASH_STATUS_DONE		0x80
#define FLASH_STATUS_ESS		0x40
#define FLASH_STATUS_ECLBS		0x20
#define FLASH_STATUS_PSLBS		0x10
#define FLASH_STATUS_VPENS		0x08
#define FLASH_STATUS_PSS		0x04
#define FLASH_STATUS_DPS		0x02
#define FLASH_STATUS_R			0x01
#define FLASH_STATUS_PROTECT		0x01

#define AMD_CMD_RESET			0xF0
#define AMD_CMD_WRITE			0xA0
#define AMD_CMD_ERASE_START		0x80
#define AMD_CMD_ERASE_SECTOR		0x30
#define AMD_CMD_UNLOCK_START		0xAA
#define AMD_CMD_UNLOCK_ACK		0x55
#define AMD_CMD_WRITE_TO_BUFFER		0x25
#define AMD_CMD_WRITE_BUFFER_CONFIRM	0x29

#define AMD_STATUS_TOGGLE		0x40
#define AMD_STATUS_ERROR		0x20

#define FLASH_OFFSET_MANUFACTURER_ID	0x00
#define FLASH_OFFSET_DEVICE_ID		0x01
#define FLASH_OFFSET_DEVICE_ID2		0x0E
#define FLASH_OFFSET_DEVICE_ID3		0x0F
#define FLASH_OFFSET_CFI		0x55
#define FLASH_OFFSET_CFI_ALT		0x555
#define FLASH_OFFSET_CFI_RESP		0x10
#define FLASH_OFFSET_PRIMARY_VENDOR	0x13
/* extended query table primary address */
#define FLASH_OFFSET_EXT_QUERY_T_P_ADDR	0x15
#define FLASH_OFFSET_WTOUT		0x1F
#define FLASH_OFFSET_WBTOUT		0x20
#define FLASH_OFFSET_ETOUT		0x21
#define FLASH_OFFSET_CETOUT		0x22
#define FLASH_OFFSET_WMAX_TOUT		0x23
#define FLASH_OFFSET_WBMAX_TOUT		0x24
#define FLASH_OFFSET_EMAX_TOUT		0x25
#define FLASH_OFFSET_CEMAX_TOUT		0x26
#define FLASH_OFFSET_SIZE		0x27
#define FLASH_OFFSET_INTERFACE		0x28
#define FLASH_OFFSET_BUFFER_SIZE	0x2A
#define FLASH_OFFSET_NUM_ERASE_REGIONS	0x2C
#define FLASH_OFFSET_ERASE_REGIONS	0x2D
#define FLASH_OFFSET_PROTECT		0x02
#define FLASH_OFFSET_USER_PROTECTION	0x85
#define FLASH_OFFSET_INTEL_PROTECTION	0x81

#define CFI_CMDSET_NONE			0
#define CFI_CMDSET_INTEL_EXTENDED	1
#define CFI_CMDSET_AMD_STANDARD		2
#define CFI_CMDSET_INTEL_STANDARD	3
#define CFI_CMDSET_AMD_EXTENDED		4
#define CFI_CMDSET_MITSU_STANDARD	256
#define CFI_CMDSET_MITSU_EXTENDED	257
#define CFI_CMDSET_SST			258

#ifdef CFG_FLASH_CFI_AMD_RESET /* needed for STM_ID_29W320DB on UC100 */
# undef  FLASH_CMD_RESET
# define FLASH_CMD_RESET	AMD_CMD_RESET /* use AMD-Reset instead */
#endif

typedef union {
	unsigned char c;
	unsigned short w;
	unsigned long l;
	unsigned long long ll;
} cfiword_t;

#define NUM_ERASE_REGIONS	4 /* max. number of erase regions */

static uint flash_offset_cfi[2] = { FLASH_OFFSET_CFI, FLASH_OFFSET_CFI_ALT };

/* use CFG_MAX_FLASH_BANKS_DETECT if defined */
#ifdef CFG_MAX_FLASH_BANKS_DETECT
static ulong bank_base[CFG_MAX_FLASH_BANKS_DETECT] = CFG_FLASH_BANKS_LIST;
flash_info_t flash_info[CFG_MAX_FLASH_BANKS_DETECT];	/* FLASH chips info */
#else
static ulong bank_base[CFG_MAX_FLASH_BANKS] = CFG_FLASH_BANKS_LIST;
flash_info_t flash_info[CFG_MAX_FLASH_BANKS];		/* FLASH chips info */
#endif

/*
 * Check if chip width is defined. If not, start detecting with 8bit.
 */
#ifndef CFG_FLASH_CFI_WIDTH
#define CFG_FLASH_CFI_WIDTH	FLASH_CFI_8BIT
#endif

typedef unsigned long flash_sect_t;

/* CFI standard query structure */
struct cfi_qry {
	u8	qry[3];
	u16	p_id;
	u16	p_adr;
	u16	a_id;
	u16	a_adr;
	u8	vcc_min;
	u8	vcc_max;
	u8	vpp_min;
	u8	vpp_max;
	u8	word_write_timeout_typ;
	u8	buf_write_timeout_typ;
	u8	block_erase_timeout_typ;
	u8	chip_erase_timeout_typ;
	u8	word_write_timeout_max;
	u8	buf_write_timeout_max;
	u8	block_erase_timeout_max;
	u8	chip_erase_timeout_max;
	u8	dev_size;
	u16	interface_desc;
	u16	max_buf_write_size;
	u8	num_erase_regions;
	u32	erase_region_info[NUM_ERASE_REGIONS];
} __attribute__((packed));

struct cfi_pri_hdr {
	u8	pri[3];
	u8	major_version;
	u8	minor_version;
} __attribute__((packed));

static void flash_write8(u8 value, void *addr)
{
	__raw_writeb(value, addr);
}

static void flash_write16(u16 value, void *addr)
{
	__raw_writew(value, addr);
}

static void flash_write32(u32 value, void *addr)
{
	__raw_writel(value, addr);
}

static void flash_write64(u64 value, void *addr)
{
	/* No architectures currently implement __raw_writeq() */
	*(volatile u64 *)addr = value;
}

static u8 flash_read8(void *addr)
{
	return __raw_readb(addr);
}

static u16 flash_read16(void *addr)
{
	return __raw_readw(addr);
}

static u32 flash_read32(void *addr)
{
	return __raw_readl(addr);
}

static u64 __flash_read64(void *addr)
{
	/* No architectures currently implement __raw_readq() */
	return *(volatile u64 *)addr;
}

u64 flash_read64(void *addr)__attribute__((weak, alias("__flash_read64")));

/*-----------------------------------------------------------------------
 */
#if defined(CFG_ENV_IS_IN_FLASH) || defined(CFG_ENV_ADDR_REDUND) || (CFG_MONITOR_BASE >= CFG_FLASH_BASE)
static flash_info_t *flash_get_info(ulong base)
{
	int i;
	flash_info_t * info = 0;

	for (i = 0; i < CFG_MAX_FLASH_BANKS; i++) {
		info = & flash_info[i];
		if (info->size && info->start[0] <= base &&
		    base <= info->start[0] + info->size - 1)
			break;
	}

	return i == CFG_MAX_FLASH_BANKS ? 0 : info;
}
#endif

unsigned long flash_sector_size(flash_info_t *info, flash_sect_t sect)
{
	if (sect != (info->sector_count - 1))
		return info->start[sect + 1] - info->start[sect];
	else
		return info->start[0] + info->size - info->start[sect];
}

/*-----------------------------------------------------------------------
 * create an address based on the offset and the port width
 */
static inline void *
flash_map (flash_info_t * info, flash_sect_t sect, uint offset)
{
	unsigned int byte_offset = offset * info->portwidth;

	return map_physmem(info->start[sect] + byte_offset,
			flash_sector_size(info, sect) - byte_offset,
			MAP_NOCACHE);
}

static inline void flash_unmap(flash_info_t *info, flash_sect_t sect,
		unsigned int offset, void *addr)
{
	unsigned int byte_offset = offset * info->portwidth;

	unmap_physmem(addr, flash_sector_size(info, sect) - byte_offset);
}

/*-----------------------------------------------------------------------
 * make a proper sized command based on the port and chip widths
 */
static void flash_make_cmd (flash_info_t * info, uchar cmd, void *cmdbuf)
{
	int i;
	uchar *cp = (uchar *) cmdbuf;

#if defined(__LITTLE_ENDIAN) || defined(CFG_WRITE_SWAPPED_DATA)
	for (i = info->portwidth; i > 0; i--)
#else
	for (i = 1; i <= info->portwidth; i++)
#endif
		*cp++ = (i & (info->chipwidth - 1)) ? '\0' : cmd;
}

#ifdef DEBUG
/*-----------------------------------------------------------------------
 * Debug support
 */
static void print_longlong (char *str, unsigned long long data)
{
	int i;
	char *cp;

	cp = (unsigned char *) &data;
	for (i = 0; i < 8; i++)
		sprintf (&str[i * 2], "%2.2x", *cp++);
}

static void flash_printqry (struct cfi_qry *qry)
{
	u8 *p = (u8 *)qry;
	int x, y;

	for (x = 0; x < sizeof(struct cfi_qry); x += 16) {
		debug("%02x : ", x);
		for (y = 0; y < 16; y++)
			debug("%2.2x ", p[x + y]);
		debug(" ");
		for (y = 0; y < 16; y++) {
			unsigned char c = p[x + y];
			if (c >= 0x20 && c <= 0x7e)
				debug("%c", c);
			else
				debug(".");
		}
		debug("\n");
	}
}
#endif


/*-----------------------------------------------------------------------
 * read a character at a port width address
 */
static inline uchar flash_read_uchar (flash_info_t * info, uint offset)
{
	uchar *cp;
	uchar retval;

	cp = flash_map (info, 0, offset);
#if defined(__LITTLE_ENDIAN) || defined(CFG_WRITE_SWAPPED_DATA)
	retval = flash_read8(cp);
#else
	retval = flash_read8(cp + info->portwidth - 1);
#endif
	flash_unmap (info, 0, offset, cp);
	return retval;
}

/*-----------------------------------------------------------------------
 * read a word at a port width address, assume 16bit bus
 */
static inline ushort flash_read_word (flash_info_t * info, uint offset)
{
	ushort *addr, retval;

	addr = flash_map (info, 0, offset);
	retval = flash_read16 (addr);
	flash_unmap (info, 0, offset, addr);
	return retval;
}


/*-----------------------------------------------------------------------
 * read a long word by picking the least significant byte of each maximum
 * port size word. Swap for ppc format.
 */
static ulong flash_read_long (flash_info_t * info, flash_sect_t sect,
			      uint offset)
{
	uchar *addr;
	ulong retval;

#ifdef DEBUG
	int x;
#endif
	addr = flash_map (info, sect, offset);

#ifdef DEBUG
	debug ("long addr is at %p info->portwidth = %d\n", addr,
	       info->portwidth);
	for (x = 0; x < 4 * info->portwidth; x++) {
		debug ("addr[%x] = 0x%x\n", x, flash_read8(addr + x));
	}
#endif
#if defined(__LITTLE_ENDIAN) || defined(CFG_WRITE_SWAPPED_DATA)
	retval = ((flash_read8(addr) << 16) |
		  (flash_read8(addr + info->portwidth) << 24) |
		  (flash_read8(addr + 2 * info->portwidth)) |
		  (flash_read8(addr + 3 * info->portwidth) << 8));
#else
	retval = ((flash_read8(addr + 2 * info->portwidth - 1) << 24) |
		  (flash_read8(addr + info->portwidth - 1) << 16) |
		  (flash_read8(addr + 4 * info->portwidth - 1) << 8) |
		  (flash_read8(addr + 3 * info->portwidth - 1)));
#endif
	flash_unmap(info, sect, offset, addr);

	return retval;
}

/*
 * Write a proper sized command to the correct address
 */
static void flash_write_cmd (flash_info_t * info, flash_sect_t sect,
			     uint offset, uchar cmd)
{

	void *addr;
	cfiword_t cword;

	addr = flash_map (info, sect, offset);
	flash_make_cmd (info, cmd, &cword);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		debug ("fwc addr %p cmd %x %x 8bit x %d bit\n", addr, cmd,
		       cword.c, info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		flash_write8(cword.c, addr);
		break;
	case FLASH_CFI_16BIT:
		debug ("fwc addr %p cmd %x %4.4x 16bit x %d bit\n", addr,
		       cmd, cword.w,
		       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		flash_write16(cword.w, addr);
		break;
	case FLASH_CFI_32BIT:
		debug ("fwc addr %p cmd %x %8.8lx 32bit x %d bit\n", addr,
		       cmd, cword.l,
		       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		flash_write32(cword.l, addr);
		break;
	case FLASH_CFI_64BIT:
#ifdef DEBUG
		{
			char str[20];

			print_longlong (str, cword.ll);

			debug ("fwrite addr %p cmd %x %s 64 bit x %d bit\n",
			       addr, cmd, str,
			       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		}
#endif
		flash_write64(cword.ll, addr);
		break;
	}

	/* Ensure all the instructions are fully finished */
	sync();

	flash_unmap(info, sect, offset, addr);
}

static void flash_unlock_seq (flash_info_t * info, flash_sect_t sect)
{
	flash_write_cmd (info, sect, info->addr_unlock1, AMD_CMD_UNLOCK_START);
	flash_write_cmd (info, sect, info->addr_unlock2, AMD_CMD_UNLOCK_ACK);
}

/*-----------------------------------------------------------------------
 */
static int flash_isequal (flash_info_t * info, flash_sect_t sect,
			  uint offset, uchar cmd)
{
	void *addr;
	cfiword_t cword;
	int retval;

	addr = flash_map (info, sect, offset);
	flash_make_cmd (info, cmd, &cword);

	debug ("is= cmd %x(%c) addr %p ", cmd, cmd, addr);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		debug ("is= %x %x\n", flash_read8(addr), cword.c);
		retval = (flash_read8(addr) == cword.c);
		break;
	case FLASH_CFI_16BIT:
		debug ("is= %4.4x %4.4x\n", flash_read16(addr), cword.w);
		retval = (flash_read16(addr) == cword.w);
		break;
	case FLASH_CFI_32BIT:
		debug ("is= %8.8lx %8.8lx\n", flash_read32(addr), cword.l);
		retval = (flash_read32(addr) == cword.l);
		break;
	case FLASH_CFI_64BIT:
#ifdef DEBUG
		{
			char str1[20];
			char str2[20];

			print_longlong (str1, flash_read64(addr));
			print_longlong (str2, cword.ll);
			debug ("is= %s %s\n", str1, str2);
		}
#endif
		retval = (flash_read64(addr) == cword.ll);
		break;
	default:
		retval = 0;
		break;
	}
	flash_unmap(info, sect, offset, addr);

	return retval;
}

/*-----------------------------------------------------------------------
 */
static int flash_isset (flash_info_t * info, flash_sect_t sect,
			uint offset, uchar cmd)
{
	void *addr;
	cfiword_t cword;
	int retval;

	addr = flash_map (info, sect, offset);
	flash_make_cmd (info, cmd, &cword);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		retval = ((flash_read8(addr) & cword.c) == cword.c);
		break;
	case FLASH_CFI_16BIT:
		retval = ((flash_read16(addr) & cword.w) == cword.w);
		break;
	case FLASH_CFI_32BIT:
		retval = ((flash_read32(addr) & cword.l) == cword.l);
		break;
	case FLASH_CFI_64BIT:
		retval = ((flash_read64(addr) & cword.ll) == cword.ll);
		break;
	default:
		retval = 0;
		break;
	}
	flash_unmap(info, sect, offset, addr);

	return retval;
}

/*-----------------------------------------------------------------------
 */
static int flash_toggle (flash_info_t * info, flash_sect_t sect,
			 uint offset, uchar cmd)
{
	void *addr;
	cfiword_t cword;
	int retval;

	addr = flash_map (info, sect, offset);
	flash_make_cmd (info, cmd, &cword);
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		retval = ((flash_read8(addr) & cword.c) !=
			  (flash_read8(addr) & cword.c));
		break;
	case FLASH_CFI_16BIT:
		retval = ((flash_read16(addr) & cword.w) !=
			  (flash_read16(addr) & cword.w));
		break;
	case FLASH_CFI_32BIT:
		retval = ((flash_read32(addr) & cword.l) !=
			  (flash_read32(addr) & cword.l));
		break;
	case FLASH_CFI_64BIT:
		retval = ((flash_read64(addr) & cword.ll) !=
			  (flash_read64(addr) & cword.ll));
		break;
	default:
		retval = 0;
		break;
	}
	flash_unmap(info, sect, offset, addr);

	return retval;
}

/*
 * flash_is_busy - check to see if the flash is busy
 *
 * This routine checks the status of the chip and returns true if the
 * chip is busy.
 */
static int flash_is_busy (flash_info_t * info, flash_sect_t sect)
{
	int retval;

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_STANDARD:
	case CFI_CMDSET_INTEL_EXTENDED:
		retval = !flash_isset (info, sect, 0, FLASH_STATUS_DONE);
		break;
	case CFI_CMDSET_AMD_STANDARD:
	case CFI_CMDSET_AMD_EXTENDED:
#ifdef CONFIG_FLASH_CFI_LEGACY
	case CFI_CMDSET_AMD_LEGACY:
#endif
		retval = flash_toggle (info, sect, 0, AMD_STATUS_TOGGLE);
		break;
	default:
		retval = 0;
	}
	debug ("flash_is_busy: %d\n", retval);
	return retval;
}

/*-----------------------------------------------------------------------
 *  wait for XSR.7 to be set. Time out with an error if it does not.
 *  This routine does not set the flash to read-array mode.
 */
static int flash_status_check (flash_info_t * info, flash_sect_t sector,
			       ulong tout, char *prompt)
{
	ulong start;

#if CFG_HZ != 1000
	tout *= CFG_HZ/1000;
#endif

	/* Wait for command completion */
	start = get_timer (0);
	while (flash_is_busy (info, sector)) {
		if (get_timer (start) > tout) {
			printf ("Flash %s timeout at address %lx data %lx\n",
				prompt, info->start[sector],
				flash_read_long (info, sector, 0));
			flash_write_cmd (info, sector, 0, info->cmd_reset);
			return ERR_TIMOUT;
		}
		udelay (1);		/* also triggers watchdog */
	}
	return ERR_OK;
}

/*-----------------------------------------------------------------------
 * Wait for XSR.7 to be set, if it times out print an error, otherwise
 * do a full status check.
 *
 * This routine sets the flash to read-array mode.
 */
static int flash_full_status_check (flash_info_t * info, flash_sect_t sector,
				    ulong tout, char *prompt)
{
	int retcode;

	retcode = flash_status_check (info, sector, tout, prompt);
	switch (info->vendor) {
	case CFI_CMDSET_INTEL_EXTENDED:
	case CFI_CMDSET_INTEL_STANDARD:
		if ((retcode == ERR_OK)
		    && !flash_isequal (info, sector, 0, FLASH_STATUS_DONE)) {
			retcode = ERR_INVAL;
			printf ("Flash %s error at address %lx\n", prompt,
				info->start[sector]);
			if (flash_isset (info, sector, 0, FLASH_STATUS_ECLBS |
					 FLASH_STATUS_PSLBS)) {
				puts ("Command Sequence Error.\n");
			} else if (flash_isset (info, sector, 0,
						FLASH_STATUS_ECLBS)) {
				puts ("Block Erase Error.\n");
				retcode = ERR_NOT_ERASED;
			} else if (flash_isset (info, sector, 0,
						FLASH_STATUS_PSLBS)) {
				puts ("Locking Error\n");
			}
			if (flash_isset (info, sector, 0, FLASH_STATUS_DPS)) {
				puts ("Block locked.\n");
				retcode = ERR_PROTECTED;
			}
			if (flash_isset (info, sector, 0, FLASH_STATUS_VPENS))
				puts ("Vpp Low Error.\n");
		}
		flash_write_cmd (info, sector, 0, info->cmd_reset);
		break;
	default:
		break;
	}
	return retcode;
}

/*-----------------------------------------------------------------------
 */
static void flash_add_byte (flash_info_t * info, cfiword_t * cword, uchar c)
{
#if defined(__LITTLE_ENDIAN) && !defined(CFG_WRITE_SWAPPED_DATA)
	unsigned short	w;
	unsigned int	l;
	unsigned long long ll;
#endif

	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		cword->c = c;
		break;
	case FLASH_CFI_16BIT:
#if defined(__LITTLE_ENDIAN) && !defined(CFG_WRITE_SWAPPED_DATA)
		w = c;
		w <<= 8;
		cword->w = (cword->w >> 8) | w;
#else
		cword->w = (cword->w << 8) | c;
#endif
		break;
	case FLASH_CFI_32BIT:
#if defined(__LITTLE_ENDIAN) && !defined(CFG_WRITE_SWAPPED_DATA)
		l = c;
		l <<= 24;
		cword->l = (cword->l >> 8) | l;
#else
		cword->l = (cword->l << 8) | c;
#endif
		break;
	case FLASH_CFI_64BIT:
#if defined(__LITTLE_ENDIAN) && !defined(CFG_WRITE_SWAPPED_DATA)
		ll = c;
		ll <<= 56;
		cword->ll = (cword->ll >> 8) | ll;
#else
		cword->ll = (cword->ll << 8) | c;
#endif
		break;
	}
}

/* loop through the sectors from the highest address when the passed
 * address is greater or equal to the sector address we have a match
 */
static flash_sect_t find_sector (flash_info_t * info, ulong addr)
{
	flash_sect_t sector;

	for (sector = info->sector_count - 1; sector >= 0; sector--) {
		if (addr >= info->start[sector])
			break;
	}
	return sector;
}

/*-----------------------------------------------------------------------
 */
static int flash_write_cfiword (flash_info_t * info, ulong dest,
				cfiword_t cword)
{
	void *dstaddr;
	int flag;

	dstaddr = map_physmem(dest, info->portwidth, MAP_NOCACHE);

	/* Check if Flash is (sufficiently) erased */
	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		flag = ((flash_read8(dstaddr) & cword.c) == cword.c);
		break;
	case FLASH_CFI_16BIT:
		flag = ((flash_read16(dstaddr) & cword.w) == cword.w);
		break;
	case FLASH_CFI_32BIT:
		flag = ((flash_read32(dstaddr) & cword.l) == cword.l);
		break;
	case FLASH_CFI_64BIT:
		flag = ((flash_read64(dstaddr) & cword.ll) == cword.ll);
		break;
	default:
		flag = 0;
		break;
	}
	if (!flag) {
		unmap_physmem(dstaddr, info->portwidth);
		return ERR_NOT_ERASED;
	}

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_EXTENDED:
	case CFI_CMDSET_INTEL_STANDARD:
		flash_write_cmd (info, 0, 0, FLASH_CMD_CLEAR_STATUS);
		flash_write_cmd (info, 0, 0, FLASH_CMD_WRITE);
		break;
	case CFI_CMDSET_AMD_EXTENDED:
	case CFI_CMDSET_AMD_STANDARD:
#ifdef CONFIG_FLASH_CFI_LEGACY
	case CFI_CMDSET_AMD_LEGACY:
#endif
		flash_unlock_seq (info, 0);
		flash_write_cmd (info, 0, info->addr_unlock1, AMD_CMD_WRITE);
		break;
	}

	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		flash_write8(cword.c, dstaddr);
		break;
	case FLASH_CFI_16BIT:
		flash_write16(cword.w, dstaddr);
		break;
	case FLASH_CFI_32BIT:
		flash_write32(cword.l, dstaddr);
		break;
	case FLASH_CFI_64BIT:
		flash_write64(cword.ll, dstaddr);
		break;
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();

	unmap_physmem(dstaddr, info->portwidth);

	return flash_full_status_check (info, find_sector (info, dest),
					info->write_tout, "write");
}

#ifdef CFG_FLASH_USE_BUFFER_WRITE

static int flash_write_cfibuffer (flash_info_t * info, ulong dest, uchar * cp,
				  int len)
{
	flash_sect_t sector;
	int cnt;
	int retcode;
	void *src = cp;
	void *dst = map_physmem(dest, len, MAP_NOCACHE);
	void *dst2 = dst;
	int flag = 0;

	switch (info->portwidth) {
	case FLASH_CFI_8BIT:
		cnt = len;
		break;
	case FLASH_CFI_16BIT:
		cnt = len >> 1;
		break;
	case FLASH_CFI_32BIT:
		cnt = len >> 2;
		break;
	case FLASH_CFI_64BIT:
		cnt = len >> 3;
		break;
	default:
		retcode = ERR_INVAL;
		goto out_unmap;
	}

	while ((cnt-- > 0) && (flag == 0)) {
		switch (info->portwidth) {
		case FLASH_CFI_8BIT:
			flag = ((flash_read8(dst2) & flash_read8(src)) ==
				flash_read8(src));
			src += 1, dst2 += 1;
			break;
		case FLASH_CFI_16BIT:
			flag = ((flash_read16(dst2) & flash_read16(src)) ==
				flash_read16(src));
			src += 2, dst2 += 2;
			break;
		case FLASH_CFI_32BIT:
			flag = ((flash_read32(dst2) & flash_read32(src)) ==
				flash_read32(src));
			src += 4, dst2 += 4;
			break;
		case FLASH_CFI_64BIT:
			flag = ((flash_read64(dst2) & flash_read64(src)) ==
				flash_read64(src));
			src += 8, dst2 += 8;
			break;
		}
	}
	if (!flag) {
		retcode = ERR_NOT_ERASED;
		goto out_unmap;
	}

	src = cp;
	sector = find_sector (info, dest);

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_STANDARD:
	case CFI_CMDSET_INTEL_EXTENDED:
		flash_write_cmd (info, sector, 0, FLASH_CMD_CLEAR_STATUS);
		flash_write_cmd (info, sector, 0, FLASH_CMD_WRITE_TO_BUFFER);
		retcode = flash_status_check (info, sector,
					      info->buffer_write_tout,
					      "write to buffer");
		if (retcode == ERR_OK) {
			/* reduce the number of loops by the width of
			 * the port */
			switch (info->portwidth) {
			case FLASH_CFI_8BIT:
				cnt = len;
				break;
			case FLASH_CFI_16BIT:
				cnt = len >> 1;
				break;
			case FLASH_CFI_32BIT:
				cnt = len >> 2;
				break;
			case FLASH_CFI_64BIT:
				cnt = len >> 3;
				break;
			default:
				retcode = ERR_INVAL;
				goto out_unmap;
			}
			flash_write_cmd (info, sector, 0, (uchar) cnt - 1);
			while (cnt-- > 0) {
				switch (info->portwidth) {
				case FLASH_CFI_8BIT:
					flash_write8(flash_read8(src), dst);
					src += 1, dst += 1;
					break;
				case FLASH_CFI_16BIT:
					flash_write16(flash_read16(src), dst);
					src += 2, dst += 2;
					break;
				case FLASH_CFI_32BIT:
					flash_write32(flash_read32(src), dst);
					src += 4, dst += 4;
					break;
				case FLASH_CFI_64BIT:
					flash_write64(flash_read64(src), dst);
					src += 8, dst += 8;
					break;
				default:
					retcode = ERR_INVAL;
					goto out_unmap;
				}
			}
			flash_write_cmd (info, sector, 0,
					 FLASH_CMD_WRITE_BUFFER_CONFIRM);
			retcode = flash_full_status_check (
				info, sector, info->buffer_write_tout,
				"buffer write");
		}

		break;

	case CFI_CMDSET_AMD_STANDARD:
	case CFI_CMDSET_AMD_EXTENDED:
		flash_unlock_seq(info,0);
		flash_write_cmd (info, sector, 0, AMD_CMD_WRITE_TO_BUFFER);

		switch (info->portwidth) {
		case FLASH_CFI_8BIT:
			cnt = len;
			flash_write_cmd (info, sector, 0,  (uchar) cnt - 1);
			while (cnt-- > 0) {
				flash_write8(flash_read8(src), dst);
				src += 1, dst += 1;
			}
			break;
		case FLASH_CFI_16BIT:
			cnt = len >> 1;
			flash_write_cmd (info, sector, 0,  (uchar) cnt - 1);
			while (cnt-- > 0) {
				flash_write16(flash_read16(src), dst);
				src += 2, dst += 2;
			}
			break;
		case FLASH_CFI_32BIT:
			cnt = len >> 2;
			flash_write_cmd (info, sector, 0,  (uchar) cnt - 1);
			while (cnt-- > 0) {
				flash_write32(flash_read32(src), dst);
				src += 4, dst += 4;
			}
			break;
		case FLASH_CFI_64BIT:
			cnt = len >> 3;
			flash_write_cmd (info, sector, 0,  (uchar) cnt - 1);
			while (cnt-- > 0) {
				flash_write64(flash_read64(src), dst);
				src += 8, dst += 8;
			}
			break;
		default:
			retcode = ERR_INVAL;
			goto out_unmap;
		}

		flash_write_cmd (info, sector, 0, AMD_CMD_WRITE_BUFFER_CONFIRM);
		retcode = flash_full_status_check (info, sector,
						   info->buffer_write_tout,
						   "buffer write");
		break;

	default:
		debug ("Unknown Command Set\n");
		retcode = ERR_INVAL;
		break;
	}

out_unmap:
	unmap_physmem(dst, len);
	return retcode;
}
#endif /* CFG_FLASH_USE_BUFFER_WRITE */


/*-----------------------------------------------------------------------
 */
int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int rcode = 0;
	int prot;
	flash_sect_t sect;

	if (info->flash_id != FLASH_MAN_CFI) {
		puts ("Can't erase unknown flash type - aborted\n");
		return 1;
	}
	if ((s_first < 0) || (s_first > s_last)) {
		puts ("- no sectors to erase\n");
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
		putc ('\n');
	}


	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) { /* not protected */
			switch (info->vendor) {
			case CFI_CMDSET_INTEL_STANDARD:
			case CFI_CMDSET_INTEL_EXTENDED:
				flash_write_cmd (info, sect, 0,
						 FLASH_CMD_CLEAR_STATUS);
				flash_write_cmd (info, sect, 0,
						 FLASH_CMD_BLOCK_ERASE);
				flash_write_cmd (info, sect, 0,
						 FLASH_CMD_ERASE_CONFIRM);
				break;
			case CFI_CMDSET_AMD_STANDARD:
			case CFI_CMDSET_AMD_EXTENDED:
				flash_unlock_seq (info, sect);
				flash_write_cmd (info, sect,
						info->addr_unlock1,
						AMD_CMD_ERASE_START);
				flash_unlock_seq (info, sect);
				flash_write_cmd (info, sect, 0,
						 AMD_CMD_ERASE_SECTOR);
				break;
#ifdef CONFIG_FLASH_CFI_LEGACY
			case CFI_CMDSET_AMD_LEGACY:
				flash_unlock_seq (info, 0);
				flash_write_cmd (info, 0, info->addr_unlock1,
						AMD_CMD_ERASE_START);
				flash_unlock_seq (info, 0);
				flash_write_cmd (info, sect, 0,
						AMD_CMD_ERASE_SECTOR);
				break;
#endif
			default:
				debug ("Unkown flash vendor %d\n",
				       info->vendor);
				break;
			}

			if (flash_full_status_check
			    (info, sect, info->erase_blk_tout, "erase")) {
				rcode = 1;
			} else
				putc ('.');
		}
	}
	puts (" done\n");
	return rcode;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info (flash_info_t * info)
{
	int i;

	if (info->flash_id != FLASH_MAN_CFI) {
		puts ("missing or unknown FLASH type\n");
		return;
	}

	printf ("%s FLASH (%d x %d)",
		info->name,
		(info->portwidth << 3), (info->chipwidth << 3));
	if (info->size < 1024*1024)
		printf ("  Size: %ld kB in %d Sectors\n",
			info->size >> 10, info->sector_count);
	else
		printf ("  Size: %ld MB in %d Sectors\n",
			info->size >> 20, info->sector_count);
	printf ("  ");
	switch (info->vendor) {
		case CFI_CMDSET_INTEL_STANDARD:
			printf ("Intel Standard");
			break;
		case CFI_CMDSET_INTEL_EXTENDED:
			printf ("Intel Extended");
			break;
		case CFI_CMDSET_AMD_STANDARD:
			printf ("AMD Standard");
			break;
		case CFI_CMDSET_AMD_EXTENDED:
			printf ("AMD Extended");
			break;
#ifdef CONFIG_FLASH_CFI_LEGACY
		case CFI_CMDSET_AMD_LEGACY:
			printf ("AMD Legacy");
			break;
#endif
		default:
			printf ("Unknown (%d)", info->vendor);
			break;
	}
	printf (" command set, Manufacturer ID: 0x%02X, Device ID: 0x%02X",
		info->manufacturer_id, info->device_id);
	if (info->device_id == 0x7E) {
		printf("%04X", info->device_id2);
	}
	printf ("\n  Erase timeout: %ld ms, write timeout: %ld ms\n",
		info->erase_blk_tout,
		info->write_tout);
	if (info->buffer_size > 1) {
		printf ("  Buffer write timeout: %ld ms, "
			"buffer size: %d bytes\n",
		info->buffer_write_tout,
		info->buffer_size);
	}

	puts ("\n  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf ("\n");
#ifdef CFG_FLASH_EMPTY_INFO
		int k;
		int size;
		int erased;
		volatile unsigned long *flash;

		/*
		 * Check if whole sector is erased
		 */
		size = flash_sector_size(info, i);
		erased = 1;
		flash = (volatile unsigned long *) info->start[i];
		size = size >> 2;	/* divide by 4 for longword access */
		for (k = 0; k < size; k++) {
			if (*flash++ != 0xffffffff) {
				erased = 0;
				break;
			}
		}

		/* print empty and read-only info */
		printf ("  %08lX %c %s ",
			info->start[i],
			erased ? 'E' : ' ',
			info->protect[i] ? "RO" : "  ");
#else	/* ! CFG_FLASH_EMPTY_INFO */
		printf ("  %08lX   %s ",
			info->start[i],
			info->protect[i] ? "RO" : "  ");
#endif
	}
	putc ('\n');
	return;
}

/*-----------------------------------------------------------------------
 * This is used in a few places in write_buf() to show programming
 * progress.  Making it a function is nasty because it needs to do side
 * effect updates to digit and dots.  Repeated code is nasty too, so
 * we define it once here.
 */
#ifdef CONFIG_FLASH_SHOW_PROGRESS
#define FLASH_SHOW_PROGRESS(scale, dots, digit, dots_sub) \
	dots -= dots_sub; \
	if ((scale > 0) && (dots <= 0)) { \
		if ((digit % 5) == 0) \
			printf ("%d", digit / 5); \
		else \
			putc ('.'); \
		digit--; \
		dots += scale; \
	}
#else
#define FLASH_SHOW_PROGRESS(scale, dots, digit, dots_sub)
#endif

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong wp;
	uchar *p;
	int aln;
	cfiword_t cword;
	int i, rc;
#ifdef CFG_FLASH_USE_BUFFER_WRITE
	int buffered_size;
#endif
#ifdef CONFIG_FLASH_SHOW_PROGRESS
	int digit = CONFIG_FLASH_SHOW_PROGRESS;
	int scale = 0;
	int dots  = 0;

	/*
	 * Suppress if there are fewer than CONFIG_FLASH_SHOW_PROGRESS writes.
	 */
	if (cnt >= CONFIG_FLASH_SHOW_PROGRESS) {
		scale = (int)((cnt + CONFIG_FLASH_SHOW_PROGRESS - 1) /
			CONFIG_FLASH_SHOW_PROGRESS);
	}
#endif

	/* get lower aligned address */
	wp = (addr & ~(info->portwidth - 1));

	/* handle unaligned start */
	if ((aln = addr - wp) != 0) {
		cword.l = 0;
		p = map_physmem(wp, info->portwidth, MAP_NOCACHE);
		for (i = 0; i < aln; ++i)
			flash_add_byte (info, &cword, flash_read8(p + i));

		for (; (i < info->portwidth) && (cnt > 0); i++) {
			flash_add_byte (info, &cword, *src++);
			cnt--;
		}
		for (; (cnt == 0) && (i < info->portwidth); ++i)
			flash_add_byte (info, &cword, flash_read8(p + i));

		rc = flash_write_cfiword (info, wp, cword);
		unmap_physmem(p, info->portwidth);
		if (rc != 0)
			return rc;

		wp += i;
		FLASH_SHOW_PROGRESS(scale, dots, digit, i);
	}

	/* handle the aligned part */
#ifdef CFG_FLASH_USE_BUFFER_WRITE
	buffered_size = (info->portwidth / info->chipwidth);
	buffered_size *= info->buffer_size;
	while (cnt >= info->portwidth) {
		/* prohibit buffer write when buffer_size is 1 */
		if (info->buffer_size == 1) {
			cword.l = 0;
			for (i = 0; i < info->portwidth; i++)
				flash_add_byte (info, &cword, *src++);
			if ((rc = flash_write_cfiword (info, wp, cword)) != 0)
				return rc;
			wp += info->portwidth;
			cnt -= info->portwidth;
			continue;
		}

		/* write buffer until next buffered_size aligned boundary */
		i = buffered_size - (wp % buffered_size);
		if (i > cnt)
			i = cnt;
		if ((rc = flash_write_cfibuffer (info, wp, src, i)) != ERR_OK)
			return rc;
		i -= i & (info->portwidth - 1);
		wp += i;
		src += i;
		cnt -= i;
		FLASH_SHOW_PROGRESS(scale, dots, digit, i);
	}
#else
	while (cnt >= info->portwidth) {
		cword.l = 0;
		for (i = 0; i < info->portwidth; i++) {
			flash_add_byte (info, &cword, *src++);
		}
		if ((rc = flash_write_cfiword (info, wp, cword)) != 0)
			return rc;
		wp += info->portwidth;
		cnt -= info->portwidth;
		FLASH_SHOW_PROGRESS(scale, dots, digit, info->portwidth);
	}
#endif /* CFG_FLASH_USE_BUFFER_WRITE */

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	cword.l = 0;
	p = map_physmem(wp, info->portwidth, MAP_NOCACHE);
	for (i = 0; (i < info->portwidth) && (cnt > 0); ++i) {
		flash_add_byte (info, &cword, *src++);
		--cnt;
	}
	for (; i < info->portwidth; ++i)
		flash_add_byte (info, &cword, flash_read8(p + i));
	unmap_physmem(p, info->portwidth);

	return flash_write_cfiword (info, wp, cword);
}

/*-----------------------------------------------------------------------
 */
#ifdef CFG_FLASH_PROTECTION

int flash_real_protect (flash_info_t * info, long sector, int prot)
{
	int retcode = 0;

	flash_write_cmd (info, sector, 0, FLASH_CMD_CLEAR_STATUS);
	flash_write_cmd (info, sector, 0, FLASH_CMD_PROTECT);
	if (prot)
		flash_write_cmd (info, sector, 0, FLASH_CMD_PROTECT_SET);
	else
		flash_write_cmd (info, sector, 0, FLASH_CMD_PROTECT_CLEAR);

	if ((retcode =
	     flash_full_status_check (info, sector, info->erase_blk_tout,
				      prot ? "protect" : "unprotect")) == 0) {

		info->protect[sector] = prot;

		/*
		 * On some of Intel's flash chips (marked via legacy_unlock)
		 * unprotect unprotects all locking.
		 */
		if ((prot == 0) && (info->legacy_unlock)) {
			flash_sect_t i;

			for (i = 0; i < info->sector_count; i++) {
				if (info->protect[i])
					flash_real_protect (info, i, 1);
			}
		}
	}
	return retcode;
}

/*-----------------------------------------------------------------------
 * flash_read_user_serial - read the OneTimeProgramming cells
 */
void flash_read_user_serial (flash_info_t * info, void *buffer, int offset,
			     int len)
{
	uchar *src;
	uchar *dst;

	dst = buffer;
	src = flash_map (info, 0, FLASH_OFFSET_USER_PROTECTION);
	flash_write_cmd (info, 0, 0, FLASH_CMD_READ_ID);
	memcpy (dst, src + offset, len);
	flash_write_cmd (info, 0, 0, info->cmd_reset);
	flash_unmap(info, 0, FLASH_OFFSET_USER_PROTECTION, src);
}

/*
 * flash_read_factory_serial - read the device Id from the protection area
 */
void flash_read_factory_serial (flash_info_t * info, void *buffer, int offset,
				int len)
{
	uchar *src;

	src = flash_map (info, 0, FLASH_OFFSET_INTEL_PROTECTION);
	flash_write_cmd (info, 0, 0, FLASH_CMD_READ_ID);
	memcpy (buffer, src + offset, len);
	flash_write_cmd (info, 0, 0, info->cmd_reset);
	flash_unmap(info, 0, FLASH_OFFSET_INTEL_PROTECTION, src);
}

#endif /* CFG_FLASH_PROTECTION */

/*-----------------------------------------------------------------------
 * Reverse the order of the erase regions in the CFI QRY structure.
 * This is needed for chips that are either a) correctly detected as
 * top-boot, or b) buggy.
 */
static void cfi_reverse_geometry(struct cfi_qry *qry)
{
	unsigned int i, j;
	u32 tmp;

	for (i = 0, j = qry->num_erase_regions - 1; i < j; i++, j--) {
		tmp = qry->erase_region_info[i];
		qry->erase_region_info[i] = qry->erase_region_info[j];
		qry->erase_region_info[j] = tmp;
	}
}

/*-----------------------------------------------------------------------
 * read jedec ids from device and set corresponding fields in info struct
 *
 * Note: assume cfi->vendor, cfi->portwidth and cfi->chipwidth are correct
 *
 */
static void cmdset_intel_read_jedec_ids(flash_info_t *info)
{
	flash_write_cmd(info, 0, 0, FLASH_CMD_RESET);
	flash_write_cmd(info, 0, 0, FLASH_CMD_READ_ID);
	udelay(1000); /* some flash are slow to respond */
	info->manufacturer_id = flash_read_uchar (info,
					FLASH_OFFSET_MANUFACTURER_ID);
	info->device_id = flash_read_uchar (info,
					FLASH_OFFSET_DEVICE_ID);
	flash_write_cmd(info, 0, 0, FLASH_CMD_RESET);
}

static int cmdset_intel_init(flash_info_t *info, struct cfi_qry *qry)
{
	info->cmd_reset = FLASH_CMD_RESET;

	cmdset_intel_read_jedec_ids(info);
	flash_write_cmd(info, 0, info->cfi_offset, FLASH_CMD_CFI);

#ifdef CFG_FLASH_PROTECTION
	/* read legacy lock/unlock bit from intel flash */
	if (info->ext_addr) {
		info->legacy_unlock = flash_read_uchar (info,
				info->ext_addr + 5) & 0x08;
	}
#endif

	return 0;
}

static void cmdset_amd_read_jedec_ids(flash_info_t *info)
{
	flash_write_cmd(info, 0, 0, AMD_CMD_RESET);
	flash_unlock_seq(info, 0);
	flash_write_cmd(info, 0, info->addr_unlock1, FLASH_CMD_READ_ID);
	udelay(1000); /* some flash are slow to respond */

	info->manufacturer_id = flash_read_uchar (info,
					FLASH_OFFSET_MANUFACTURER_ID);

	switch (info->chipwidth){
	case FLASH_CFI_8BIT:
		info->device_id = flash_read_uchar (info,
						FLASH_OFFSET_DEVICE_ID);
		if (info->device_id == 0x7E) {
			/* AMD 3-byte (expanded) device ids */
			info->device_id2 = flash_read_uchar (info,
						FLASH_OFFSET_DEVICE_ID2);
			info->device_id2 <<= 8;
			info->device_id2 |= flash_read_uchar (info,
						FLASH_OFFSET_DEVICE_ID3);
		}
		break;
	case FLASH_CFI_16BIT:
		info->device_id = flash_read_word (info,
						FLASH_OFFSET_DEVICE_ID);
		break;
	default:
		break;
	}
	flash_write_cmd(info, 0, 0, AMD_CMD_RESET);
}

static int cmdset_amd_init(flash_info_t *info, struct cfi_qry *qry)
{
	info->cmd_reset = AMD_CMD_RESET;

	cmdset_amd_read_jedec_ids(info);
	flash_write_cmd(info, 0, info->cfi_offset, FLASH_CMD_CFI);

	return 0;
}

#ifdef CONFIG_FLASH_CFI_LEGACY
static void flash_read_jedec_ids (flash_info_t * info)
{
	info->manufacturer_id = 0;
	info->device_id       = 0;
	info->device_id2      = 0;

	switch (info->vendor) {
	case CFI_CMDSET_INTEL_STANDARD:
	case CFI_CMDSET_INTEL_EXTENDED:
		cmdset_intel_read_jedec_ids(info);
		break;
	case CFI_CMDSET_AMD_STANDARD:
	case CFI_CMDSET_AMD_EXTENDED:
		cmdset_amd_read_jedec_ids(info);
		break;
	default:
		break;
	}
}

/*-----------------------------------------------------------------------
 * Call board code to request info about non-CFI flash.
 * board_flash_get_legacy needs to fill in at least:
 * info->portwidth, info->chipwidth and info->interface for Jedec probing.
 */
static int flash_detect_legacy(ulong base, int banknum)
{
	flash_info_t *info = &flash_info[banknum];

	if (board_flash_get_legacy(base, banknum, info)) {
		/* board code may have filled info completely. If not, we
		   use JEDEC ID probing. */
		if (!info->vendor) {
			int modes[] = {
				CFI_CMDSET_AMD_STANDARD,
				CFI_CMDSET_INTEL_STANDARD
			};
			int i;

			for (i = 0; i < sizeof(modes) / sizeof(modes[0]); i++) {
				info->vendor = modes[i];
				info->start[0] = base;
				if (info->portwidth == FLASH_CFI_8BIT
					&& info->interface == FLASH_CFI_X8X16) {
					info->addr_unlock1 = 0x2AAA;
					info->addr_unlock2 = 0x5555;
				} else {
					info->addr_unlock1 = 0x5555;
					info->addr_unlock2 = 0x2AAA;
				}
				flash_read_jedec_ids(info);
				debug("JEDEC PROBE: ID %x %x %x\n",
						info->manufacturer_id,
						info->device_id,
						info->device_id2);
				if (jedec_flash_match(info, base))
					break;
			}
		}

		switch(info->vendor) {
		case CFI_CMDSET_INTEL_STANDARD:
		case CFI_CMDSET_INTEL_EXTENDED:
			info->cmd_reset = FLASH_CMD_RESET;
			break;
		case CFI_CMDSET_AMD_STANDARD:
		case CFI_CMDSET_AMD_EXTENDED:
		case CFI_CMDSET_AMD_LEGACY:
			info->cmd_reset = AMD_CMD_RESET;
			break;
		}
		info->flash_id = FLASH_MAN_CFI;
		return 1;
	}
	return 0; /* use CFI */
}
#else
static inline int flash_detect_legacy(ulong base, int banknum)
{
	return 0; /* use CFI */
}
#endif

/*-----------------------------------------------------------------------
 * detect if flash is compatible with the Common Flash Interface (CFI)
 * http://www.jedec.org/download/search/jesd68.pdf
 */
static void flash_read_cfi (flash_info_t *info, void *buf,
		unsigned int start, size_t len)
{
	u8 *p = buf;
	unsigned int i;

	for (i = 0; i < len; i++)
		p[i] = flash_read_uchar(info, start + i);
}

static int __flash_detect_cfi (flash_info_t * info, struct cfi_qry *qry)
{
	int cfi_offset;

	/* We do not yet know what kind of commandset to use, so we issue
	   the reset command in both Intel and AMD variants, in the hope
	   that AMD flash roms ignore the Intel command. */
	flash_write_cmd (info, 0, 0, AMD_CMD_RESET);
	flash_write_cmd (info, 0, 0, FLASH_CMD_RESET);

	for (cfi_offset=0;
	     cfi_offset < sizeof(flash_offset_cfi) / sizeof(uint);
	     cfi_offset++) {
		flash_write_cmd (info, 0, flash_offset_cfi[cfi_offset],
				 FLASH_CMD_CFI);
		if (flash_isequal (info, 0, FLASH_OFFSET_CFI_RESP, 'Q')
		    && flash_isequal (info, 0, FLASH_OFFSET_CFI_RESP + 1, 'R')
		    && flash_isequal (info, 0, FLASH_OFFSET_CFI_RESP + 2, 'Y')) {
			flash_read_cfi(info, qry, FLASH_OFFSET_CFI_RESP,
					sizeof(struct cfi_qry));
			info->interface	= le16_to_cpu(qry->interface_desc);

			info->cfi_offset = flash_offset_cfi[cfi_offset];
			debug ("device interface is %d\n",
			       info->interface);
			debug ("found port %d chip %d ",
			       info->portwidth, info->chipwidth);
			debug ("port %d bits chip %d bits\n",
			       info->portwidth << CFI_FLASH_SHIFT_WIDTH,
			       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);

			/* calculate command offsets as in the Linux driver */
			info->addr_unlock1 = 0x555;
			info->addr_unlock2 = 0x2aa;

			/*
			 * modify the unlock address if we are
			 * in compatibility mode
			 */
			if (	/* x8/x16 in x8 mode */
				((info->chipwidth == FLASH_CFI_BY8) &&
					(info->interface == FLASH_CFI_X8X16)) ||
				/* x16/x32 in x16 mode */
				((info->chipwidth == FLASH_CFI_BY16) &&
					(info->interface == FLASH_CFI_X16X32)))
			{
				info->addr_unlock1 = 0xaaa;
				info->addr_unlock2 = 0x555;
			}

			info->name = "CFI conformant";
			return 1;
		}
	}

	return 0;
}

static int flash_detect_cfi (flash_info_t * info, struct cfi_qry *qry)
{
	debug ("flash detect cfi\n");

	for (info->portwidth = CFG_FLASH_CFI_WIDTH;
	     info->portwidth <= FLASH_CFI_64BIT; info->portwidth <<= 1) {
		for (info->chipwidth = FLASH_CFI_BY8;
		     info->chipwidth <= info->portwidth;
		     info->chipwidth <<= 1)
			if (__flash_detect_cfi(info, qry))
				return 1;
	}
	debug ("not found\n");
	return 0;
}

/*
 * Manufacturer-specific quirks. Add workarounds for geometry
 * reversal, etc. here.
 */
static void flash_fixup_amd(flash_info_t *info, struct cfi_qry *qry)
{
	/* check if flash geometry needs reversal */
	if (qry->num_erase_regions > 1) {
		/* reverse geometry if top boot part */
		if (info->cfi_version < 0x3131) {
			/* CFI < 1.1, try to guess from device id */
			if ((info->device_id & 0x80) != 0)
				cfi_reverse_geometry(qry);
		} else if (flash_read_uchar(info, info->ext_addr + 0xf) == 3) {
			/* CFI >= 1.1, deduct from top/bottom flag */
			/* note: ext_addr is valid since cfi_version > 0 */
			cfi_reverse_geometry(qry);
		}
	}
}

static void flash_fixup_atmel(flash_info_t *info, struct cfi_qry *qry)
{
	int reverse_geometry = 0;

	/* Check the "top boot" bit in the PRI */
	if (info->ext_addr && !(flash_read_uchar(info, info->ext_addr + 6) & 1))
		reverse_geometry = 1;

	/* AT49BV6416(T) list the erase regions in the wrong order.
	 * However, the device ID is identical with the non-broken
	 * AT49BV642D since u-boot only reads the low byte (they
	 * differ in the high byte.) So leave out this fixup for now.
	 */
#if 0
	if (info->device_id == 0xd6 || info->device_id == 0xd2)
		reverse_geometry = !reverse_geometry;
#endif

	if (reverse_geometry)
		cfi_reverse_geometry(qry);
}

/*
 * The following code cannot be run from FLASH!
 *
 */
ulong flash_get_size (ulong base, int banknum)
{
	flash_info_t *info = &flash_info[banknum];
	int i, j;
	flash_sect_t sect_cnt;
	unsigned long sector;
	unsigned long tmp;
	int size_ratio;
	uchar num_erase_regions;
	int erase_region_size;
	int erase_region_count;
	struct cfi_qry qry;

	info->ext_addr = 0;
	info->cfi_version = 0;
#ifdef CFG_FLASH_PROTECTION
	info->legacy_unlock = 0;
#endif

	info->start[0] = base;

	if (flash_detect_cfi (info, &qry)) {
		info->vendor = le16_to_cpu(qry.p_id);
		info->ext_addr = le16_to_cpu(qry.p_adr);
		num_erase_regions = qry.num_erase_regions;

		if (info->ext_addr) {
			info->cfi_version = (ushort) flash_read_uchar (info,
						info->ext_addr + 3) << 8;
			info->cfi_version |= (ushort) flash_read_uchar (info,
						info->ext_addr + 4);
		}

#ifdef DEBUG
		flash_printqry (&qry);
#endif

		switch (info->vendor) {
		case CFI_CMDSET_INTEL_STANDARD:
		case CFI_CMDSET_INTEL_EXTENDED:
			cmdset_intel_init(info, &qry);
			break;
		case CFI_CMDSET_AMD_STANDARD:
		case CFI_CMDSET_AMD_EXTENDED:
			cmdset_amd_init(info, &qry);
			break;
		default:
			printf("CFI: Unknown command set 0x%x\n",
					info->vendor);
			/*
			 * Unfortunately, this means we don't know how
			 * to get the chip back to Read mode. Might
			 * as well try an Intel-style reset...
			 */
			flash_write_cmd(info, 0, 0, FLASH_CMD_RESET);
			return 0;
		}

		/* Do manufacturer-specific fixups */
		switch (info->manufacturer_id) {
		case 0x0001:
			flash_fixup_amd(info, &qry);
			break;
		case 0x001f:
			flash_fixup_atmel(info, &qry);
			break;
		}

		debug ("manufacturer is %d\n", info->vendor);
		debug ("manufacturer id is 0x%x\n", info->manufacturer_id);
		debug ("device id is 0x%x\n", info->device_id);
		debug ("device id2 is 0x%x\n", info->device_id2);
		debug ("cfi version is 0x%04x\n", info->cfi_version);

		size_ratio = info->portwidth / info->chipwidth;
		/* if the chip is x8/x16 reduce the ratio by half */
		if ((info->interface == FLASH_CFI_X8X16)
		    && (info->chipwidth == FLASH_CFI_BY8)) {
			size_ratio >>= 1;
		}
		debug ("size_ratio %d port %d bits chip %d bits\n",
		       size_ratio, info->portwidth << CFI_FLASH_SHIFT_WIDTH,
		       info->chipwidth << CFI_FLASH_SHIFT_WIDTH);
		debug ("found %d erase regions\n", num_erase_regions);
		sect_cnt = 0;
		sector = base;
		for (i = 0; i < num_erase_regions; i++) {
			if (i > NUM_ERASE_REGIONS) {
				printf ("%d erase regions found, only %d used\n",
					num_erase_regions, NUM_ERASE_REGIONS);
				break;
			}

			tmp = le32_to_cpu(qry.erase_region_info[i]);
			debug("erase region %u: 0x%08lx\n", i, tmp);

			erase_region_count = (tmp & 0xffff) + 1;
			tmp >>= 16;
			erase_region_size =
				(tmp & 0xffff) ? ((tmp & 0xffff) * 256) : 128;
			debug ("erase_region_count = %d erase_region_size = %d\n",
				erase_region_count, erase_region_size);
			for (j = 0; j < erase_region_count; j++) {
				if (sect_cnt >= CFG_MAX_FLASH_SECT) {
					printf("ERROR: too many flash sectors\n");
					break;
				}
				info->start[sect_cnt] = sector;
				sector += (erase_region_size * size_ratio);

				/*
				 * Only read protection status from
				 * supported devices (intel...)
				 */
				switch (info->vendor) {
				case CFI_CMDSET_INTEL_EXTENDED:
				case CFI_CMDSET_INTEL_STANDARD:
					info->protect[sect_cnt] =
						flash_isset (info, sect_cnt,
							     FLASH_OFFSET_PROTECT,
							     FLASH_STATUS_PROTECT);
					break;
				default:
					/* default: not protected */
					info->protect[sect_cnt] = 0;
				}

				sect_cnt++;
			}
		}

		info->sector_count = sect_cnt;
		info->size = 1 << qry.dev_size;
		/* multiply the size by the number of chips */
		info->size *= size_ratio;
		info->buffer_size = 1 << le16_to_cpu(qry.max_buf_write_size);
		tmp = 1 << qry.block_erase_timeout_typ;
		info->erase_blk_tout = tmp *
			(1 << qry.block_erase_timeout_max);
		tmp = (1 << qry.buf_write_timeout_typ) *
			(1 << qry.buf_write_timeout_max);

		/* round up when converting to ms */
		info->buffer_write_tout = (tmp + 999) / 1000;
		tmp = (1 << qry.word_write_timeout_typ) *
			(1 << qry.word_write_timeout_max);
		/* round up when converting to ms */
		info->write_tout = (tmp + 999) / 1000;
		info->flash_id = FLASH_MAN_CFI;
		if ((info->interface == FLASH_CFI_X8X16) &&
		    (info->chipwidth == FLASH_CFI_BY8)) {
			/* XXX - Need to test on x8/x16 in parallel. */
			info->portwidth >>= 1;
		}
	}

	flash_write_cmd (info, 0, 0, info->cmd_reset);
	return (info->size);
}

/*-----------------------------------------------------------------------
 */
unsigned long flash_init (void)
{
	unsigned long size = 0;
	int i;

#ifdef CFG_FLASH_PROTECTION
	char *s = getenv("unlock");
#endif

	/* Init: no FLASHes known */
	for (i = 0; i < CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;

		if (!flash_detect_legacy (bank_base[i], i))
			flash_get_size (bank_base[i], i);
		size += flash_info[i].size;
		if (flash_info[i].flash_id == FLASH_UNKNOWN) {
#ifndef CFG_FLASH_QUIET_TEST
			printf ("## Unknown FLASH on Bank %d "
				"- Size = 0x%08lx = %ld MB\n",
				i+1, flash_info[i].size,
				flash_info[i].size << 20);
#endif /* CFG_FLASH_QUIET_TEST */
		}
#ifdef CFG_FLASH_PROTECTION
		else if ((s != NULL) && (strcmp(s, "yes") == 0)) {
			/*
			 * Only the U-Boot image and it's environment
			 * is protected, all other sectors are
			 * unprotected (unlocked) if flash hardware
			 * protection is used (CFG_FLASH_PROTECTION)
			 * and the environment variable "unlock" is
			 * set to "yes".
			 */
			if (flash_info[i].legacy_unlock) {
				int k;

				/*
				 * Disable legacy_unlock temporarily,
				 * since flash_real_protect would
				 * relock all other sectors again
				 * otherwise.
				 */
				flash_info[i].legacy_unlock = 0;

				/*
				 * Legacy unlocking (e.g. Intel J3) ->
				 * unlock only one sector. This will
				 * unlock all sectors.
				 */
				flash_real_protect (&flash_info[i], 0, 0);

				flash_info[i].legacy_unlock = 1;

				/*
				 * Manually mark other sectors as
				 * unlocked (unprotected)
				 */
				for (k = 1; k < flash_info[i].sector_count; k++)
					flash_info[i].protect[k] = 0;
			} else {
				/*
				 * No legancy unlocking -> unlock all sectors
				 */
				flash_protect (FLAG_PROTECT_CLEAR,
					       flash_info[i].start[0],
					       flash_info[i].start[0]
					       + flash_info[i].size - 1,
					       &flash_info[i]);
			}
		}
#endif /* CFG_FLASH_PROTECTION */
	}

	/* Monitor protection ON by default */
#if (CFG_MONITOR_BASE >= CFG_FLASH_BASE)
	flash_protect (FLAG_PROTECT_SET,
		       CFG_MONITOR_BASE,
		       CFG_MONITOR_BASE + monitor_flash_len  - 1,
		       flash_get_info(CFG_MONITOR_BASE));
#endif

	/* Environment protection ON by default */
#ifdef CFG_ENV_IS_IN_FLASH
	flash_protect (FLAG_PROTECT_SET,
		       CFG_ENV_ADDR,
		       CFG_ENV_ADDR + CFG_ENV_SECT_SIZE - 1,
		       flash_get_info(CFG_ENV_ADDR));
#endif

	/* Redundant environment protection ON by default */
#ifdef CFG_ENV_ADDR_REDUND
	flash_protect (FLAG_PROTECT_SET,
		       CFG_ENV_ADDR_REDUND,
		       CFG_ENV_ADDR_REDUND + CFG_ENV_SIZE_REDUND - 1,
		       flash_get_info(CFG_ENV_ADDR_REDUND));
#endif
	return (size);
}

#endif /* CFG_FLASH_CFI */
