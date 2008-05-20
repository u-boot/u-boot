/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Keith Outwater, keith_outwater@mvsi.com
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
#include <mpc8xx.h>

#if defined(CFG_ENV_IS_IN_FLASH)
# ifndef  CFG_ENV_ADDR
#  define CFG_ENV_ADDR	(CFG_FLASH_BASE + CFG_ENV_OFFSET)
# endif
# ifndef  CFG_ENV_SIZE
#  define CFG_ENV_SIZE	CFG_ENV_SECT_SIZE
# endif
# ifndef  CFG_ENV_SECT_SIZE
#  define CFG_ENV_SECT_SIZE  CFG_ENV_SIZE
# endif
#endif

/*
 * Use buffered writes to flash by default - they are about 32x faster than
 * single byte writes.
 */
#ifndef  CFG_GEN860T_FLASH_USE_WRITE_BUFFER
#define CFG_GEN860T_FLASH_USE_WRITE_BUFFER
#endif

/*
 * Max time to wait (in mS) for flash device to allocate a write buffer.
 */
#ifndef CFG_FLASH_ALLOC_BUFFER_TOUT
#define CFG_FLASH_ALLOC_BUFFER_TOUT		100
#endif

/*
 * These functions support a single Intel StrataFlash device (28F128J3A)
 * in byte mode only!.  The flash routines are very basic and simple
 * since there isn't really any remapping necessary.
 */

/*
 * Intel SCS (Scalable Command Set) command definitions
 * (taken from 28F128J3A datasheet)
 */
#define SCS_READ_CMD				0xff
#define SCS_READ_ID_CMD				0x90
#define SCS_QUERY_CMD				0x98
#define SCS_READ_STATUS_CMD			0x70
#define SCS_CLEAR_STATUS_CMD		0x50
#define SCS_WRITE_BUF_CMD			0xe8
#define SCS_PROGRAM_CMD				0x40
#define SCS_BLOCK_ERASE_CMD			0x20
#define SCS_BLOCK_ERASE_RESUME_CMD	0xd0
#define SCS_PROGRAM_RESUME_CMD		0xd0
#define SCS_BLOCK_ERASE_SUSPEND_CMD	0xb0
#define SCS_SET_BLOCK_LOCK_CMD		0x60
#define SCS_CLR_BLOCK_LOCK_CMD		0x60

/*
 * SCS status/extended status register bit definitions
 */
#define  SCS_SR7					0x80
#define  SCS_XSR7					0x80

/*---------------------------------------------------------------------*/
#if 0
#define DEBUG_FLASH
#endif

#ifdef DEBUG_FLASH
#define PRINTF(fmt,args...) printf(fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif
/*---------------------------------------------------------------------*/

flash_info_t	flash_info[CFG_MAX_FLASH_BANKS];

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_char *addr, flash_info_t *info);
static int write_data8 (flash_info_t *info, ulong dest, uchar data);
static void flash_get_offsets (ulong base, flash_info_t *info);

/*-----------------------------------------------------------------------
 * Initialize the flash memory.
 */
unsigned long
flash_init (void)
{
	volatile immap_t     *immap  = (immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	unsigned long size_b0;
	int i;

	for (i= 0; i < CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/*
	 * The gen860t board only has one FLASH memory device, so the
	 * FLASH Bank configuration is done statically.
	 */
	PRINTF("\n## Get flash bank 1 size @ 0x%08x\n", FLASH_BASE0_PRELIM);
	size_b0 = flash_get_size((vu_char *)FLASH_BASE0_PRELIM, &flash_info[0]);
	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0: "
				"ID 0x%lx, Size = 0x%08lx = %ld MB\n",
				flash_info[0].flash_id,size_b0, size_b0 << 20);
	}

	PRINTF("## Before remap:\n"
		   "  BR0: 0x%08x    OR0: 0x%08x\n  BR1: 0x%08x    OR1: 0x%08x\n",
		   memctl->memc_br0, memctl->memc_or0,
		   memctl->memc_br1, memctl->memc_or1);

	/*
	 * Remap FLASH according to real size
	 */
	memctl->memc_or0 |= (-size_b0 & 0xFFFF8000);
	memctl->memc_br0 |= (CFG_FLASH_BASE & BR_BA_MSK);

	PRINTF("## After remap:\n"
		   "  BR0: 0x%08x    OR0: 0x%08x\n", memctl->memc_br0, memctl->memc_or0);

	/*
	 * Re-do sizing to get full correct info
	 */
	size_b0 = flash_get_size ((vu_char *)CFG_FLASH_BASE, &flash_info[0]);
	flash_get_offsets (CFG_FLASH_BASE, &flash_info[0]);
	flash_info[0].size = size_b0;

#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
	/*
	 * Monitor protection is ON by default
	 */
	flash_protect(FLAG_PROTECT_SET,
			  CFG_MONITOR_BASE,
			  CFG_MONITOR_BASE + monitor_flash_len - 1,
			  &flash_info[0]);
#endif

#ifdef	CFG_ENV_IS_IN_FLASH
	/*
	 * Environment protection ON by default
	 */
	flash_protect(FLAG_PROTECT_SET,
			  CFG_ENV_ADDR,
			  CFG_ENV_ADDR + CFG_ENV_SECT_SIZE - 1,
			  &flash_info[0]);
#endif

	PRINTF("## Final Flash bank size: 0x%08lx\n",size_b0);
	return (size_b0);
}


/*-----------------------------------------------------------------------
 * Fill in the FLASH offset table
 */
static void
flash_get_offsets (ulong base, flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
		case FLASH_MAN_INTEL:
		for (i = 0; i < info->sector_count; i++) {
				info->start[i] = base;
				base += 1024 * 128;
		}
		return;

		default:
			printf ("Don't know sector offsets for FLASH"
			        " type 0x%lx\n", info->flash_id);
	    return;
	}
}


/*-----------------------------------------------------------------------
 * Display FLASH device info
 */
void
flash_print_info (flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("Missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_INTEL:
			printf ("Intel ");
			break;
	default:
			printf ("Unknown Vendor ");
			break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F128J3A:
			printf ("28F128J3A (128Mbit = 128K x 128)\n");
			break;
	default:
			printf ("Unknown Chip Type\n");
			break;
	}

	if (info->size >= (1024 * 1024)) {
		i = 20;
	} else {
		i = 10;
	}
	printf ("  Size: %ld %cB in %d Sectors\n",
			info->size >> i,
			(i == 20) ? 'M' : 'k',
			info->sector_count);

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
	return;
}


/*-----------------------------------------------------------------------
 * Get size and other information for a FLASH device.
 * NOTE: The following code cannot be run from FLASH!
 */
static
ulong flash_get_size (vu_char *addr, flash_info_t *info)
{
#define NO_FLASH	0

	vu_char value[2];

	/*
	 * Try to read the manufacturer ID
	 */
	addr[0] = SCS_READ_CMD;
	addr[0] = SCS_READ_ID_CMD;
	value[0] = addr[0];
	value[1] = addr[2];
	addr[0] = SCS_READ_CMD;

	PRINTF("Manuf. ID @ 0x%08lx: 0x%02x\n", (ulong)addr, value[0]);
	switch (value[0]) {
		case (INTEL_MANUFACT & 0xff):
			info->flash_id = FLASH_MAN_INTEL;
			break;
		default:
			info->flash_id = FLASH_UNKNOWN;
			info->sector_count = 0;
			info->size = 0;
			return (NO_FLASH);
	}

	/*
	 * Read the device ID
	 */
	PRINTF("Device ID @ 0x%08lx: 0x%02x\n", (ulong)(&addr[2]), value[1]);
	switch (value[1]) {
		case (INTEL_ID_28F128J3A & 0xff):
			info->flash_id += FLASH_28F128J3A;
			info->sector_count = 128;
			info->size = 16 * 1024 * 1024;
			break;

		default:
			info->flash_id = FLASH_UNKNOWN;
			return (NO_FLASH);
	}

	if (info->sector_count > CFG_MAX_FLASH_SECT) {
		printf ("** ERROR: sector count %d > max (%d) **\n",
				info->sector_count, CFG_MAX_FLASH_SECT);
				info->sector_count = CFG_MAX_FLASH_SECT;
	}
	return (info->size);
}


/*-----------------------------------------------------------------------
 * Erase the specified sectors in the specified FLASH device
 */
int
flash_erase(flash_info_t *info, int s_first, int s_last)
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

	if ((info->flash_id & FLASH_VENDMASK) != FLASH_MAN_INTEL) {
		printf ("Can erase only Intel flash types - aborted\n");
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

	start = get_timer (0);
	last  = start;

	/*
	 * Start erase on unprotected sectors
	 */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			vu_char *addr = (uchar *)(info->start[sect]);
			vu_char status;

			/*
			 * Disable interrupts which might cause a timeout
			 */
			flag = disable_interrupts();

			*addr = SCS_CLEAR_STATUS_CMD;
			*addr = SCS_BLOCK_ERASE_CMD;
			*addr = SCS_BLOCK_ERASE_RESUME_CMD;

			/*
			 * Re-enable interrupts if necessary
			 */
			if (flag)
				enable_interrupts();

			/*
			 * Wait at least 80us - let's wait 1 ms
			 */
			udelay (1000);

			while (((status = *addr) & SCS_SR7) != SCS_SR7) {
				if ((now=get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
					printf ("Timeout\n");
					*addr = SCS_BLOCK_ERASE_SUSPEND_CMD;
					*addr = SCS_READ_CMD;
					return 1;
				}

				/*
				 * Show that we're waiting
				 */
				if ((now - last) > 1000) {	/* 1 second */
					putc ('.');
					last = now;
				}
			}
			*addr = SCS_READ_CMD;
		}
	}
	printf (" done\n");
	return 0;
}


#ifdef CFG_GEN860T_FLASH_USE_WRITE_BUFFER
/*
 * Allocate a flash buffer, fill it with data and write it to the flash.
 * 0 - OK
 * 1 - Timeout on buffer request
 *
 * NOTE: After the last call to this function, WSM status needs to be checked!
 */
static int
write_flash_buffer8(flash_info_t *info_p, vu_char *src_p, vu_char *dest_p,
				    uint count)
{
	vu_char *block_addr_p = NULL;
	vu_char *start_addr_p = NULL;
	ulong blocksize = info_p->size / (ulong)info_p->sector_count;

	int i;
	uint time = get_timer(0);

	PRINTF("%s:%d: src: 0x%p dest: 0x%p  count: %d\n",
		   __FUNCTION__, __LINE__, src_p, dest_p, count);

	/*
	 * What block are we in? We already know that the source address is
	 * in the flash address range, but we also can't cross a block boundary.
	 * We assume that the block does not cross a boundary (we'll check before
	 * calling this function).
	 */
	for (i = 0; i < info_p->sector_count; ++i) {
		if ( ((ulong)dest_p >= info_p->start[i]) &&
		    ((ulong)dest_p < (info_p->start[i] + blocksize)) ) {
			PRINTF("%s:%d: Dest addr 0x%p is in block %d @ 0x%.8lx\n",
				   __FUNCTION__, __LINE__, dest_p, i, info_p->start[i]);
			block_addr_p = (vu_char *)info_p->start[i];
			break;
		}
	}

	/*
	 * Request a buffer
	 */
	*block_addr_p = SCS_WRITE_BUF_CMD;
	while ((*block_addr_p & SCS_XSR7) != SCS_XSR7) {
		if (get_timer(time) >  CFG_FLASH_ALLOC_BUFFER_TOUT) {
			PRINTF("%s:%d: Buffer allocation timeout @ 0x%p (waited %d mS)\n",
				   __FUNCTION__, __LINE__, block_addr_p,
				   CFG_FLASH_ALLOC_BUFFER_TOUT);
			return 1;
		}
		*block_addr_p = SCS_WRITE_BUF_CMD;
	}

	/*
	 * Fill the buffer with data
	 */
	start_addr_p = dest_p;
	*block_addr_p = count - 1; /* flash device wants count - 1 */
	PRINTF("%s:%d: Fill buffer at block addr 0x%p\n",
		   __FUNCTION__, __LINE__, block_addr_p);
	for (i = 0; i < count; i++) {
		*start_addr_p++ = *src_p++;
	}

	/*
	 * Flush buffer to flash
	 */
	*block_addr_p = SCS_PROGRAM_RESUME_CMD;
#if 1
	time = get_timer(0);
	while ((*block_addr_p & SCS_SR7) != SCS_SR7) {
		if (get_timer(time) >  CFG_FLASH_WRITE_TOUT) {
			PRINTF("%s:%d: Write timeout @ 0x%p (waited %d mS)\n",
				   __FUNCTION__, __LINE__, block_addr_p, CFG_FLASH_WRITE_TOUT);
			return 1;
		}
	}

#endif
	return 0;
}
#endif


/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 4 - Flash not identified
 */
int
write_buff(flash_info_t *info_p, uchar *src_p, ulong addr, ulong count)
{
	int rc = 0;
#ifdef CFG_GEN860T_FLASH_USE_WRITE_BUFFER
#define FLASH_WRITE_BUF_SIZE	0x00000020	/* 32 bytes */
	int i;
	uint bufs;
	ulong buf_count;
	vu_char *sp;
	vu_char *dp;
#else
	ulong wp;
#endif

	PRINTF("\n%s:%d: src: 0x%.8lx dest: 0x%.8lx size: %d (0x%.8lx)\n",
		   __FUNCTION__, __LINE__, (ulong)src_p, addr, (uint)count, count);

	if (info_p->flash_id == FLASH_UNKNOWN) {
		return 4;
	}

#ifdef CFG_GEN860T_FLASH_USE_WRITE_BUFFER
	sp = src_p;
	dp = (uchar *)addr;

	/*
	 * For maximum performance, we want to align the start address to
	 * the beginning of a write buffer boundary (i.e. A4-A0 of the
	 * start address = 0). See how many bytes are required to get to a
	 * write-buffer-aligned address.  If that number is non-zero, do
	 * non buffered writes of the non-aligned data.  By doing non-buffered
	 * writes, we avoid the problem of crossing a block (sector) boundary
	 * with buffered writes.
	 */
	buf_count = FLASH_WRITE_BUF_SIZE - (addr & (FLASH_WRITE_BUF_SIZE - 1));
	if (buf_count == FLASH_WRITE_BUF_SIZE) { /* already on a boundary */
		buf_count = 0;
	}
	if (buf_count > count) { /* not a full buffers worth of data to write */
		buf_count = count;
	}
	count -= buf_count;

	PRINTF("%s:%d: Write buffer alignment count = %ld\n",
		   __FUNCTION__, __LINE__, buf_count);
	while (buf_count-- >= 1) {
		if ((rc = write_data8(info_p, (ulong)dp++, *sp++)) != 0)  {
			return (rc);
		}
	}

	PRINTF("%s:%d: count = %ld\n", __FUNCTION__, __LINE__, count);
	if (count == 0) { /* all done */
		PRINTF("%s:%d: Less than 1 buffer (%d) worth of bytes\n",
			   __FUNCTION__, __LINE__, FLASH_WRITE_BUF_SIZE);
		return (rc);
	}

	/*
	 * Now that we are write buffer aligned, write full or partial buffers.
	 * The fact that we are write buffer aligned automatically avoids
	 * crossing a block address during a write buffer operation.
	 */
	bufs = count / FLASH_WRITE_BUF_SIZE;
	PRINTF("%s:%d: %d (0x%x) buffers to write\n", __FUNCTION__, __LINE__,
		   bufs, bufs);
	while (bufs >= 1) {
		rc = write_flash_buffer8(info_p, sp, dp, FLASH_WRITE_BUF_SIZE);
		if (rc != 0) {
			PRINTF("%s:%d: ** Error writing buf %d\n",
				   __FUNCTION__, __LINE__, bufs);
			return (rc);
		}
		bufs--;
		sp += FLASH_WRITE_BUF_SIZE;
		dp += FLASH_WRITE_BUF_SIZE;
	}

	/*
	 * Do the leftovers
	 */
	i = count % FLASH_WRITE_BUF_SIZE;
	PRINTF("%s:%d: %d (0x%x) leftover bytes\n", __FUNCTION__, __LINE__, i, i);
	if (i > 0) {
		rc = write_flash_buffer8(info_p, sp, dp, i);
	}

	sp = (vu_char*)info_p->start[0];
	*sp = SCS_READ_CMD;
	return (rc);

#else
	wp = addr;
	while (count-- >= 1) {
		if((rc = write_data8(info_p, wp++, *src_p++)) != 0)
			return (rc);
	}
	return 0;
#endif
}


/*-----------------------------------------------------------------------
 * Write a byte to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int
write_data8 (flash_info_t *info, ulong dest, uchar data)
{
	vu_char *addr = (vu_char *)dest;
	vu_char status;
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*addr & data) != data) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	*addr = SCS_PROGRAM_CMD;
	*addr = data;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	start = get_timer (0);

	while (((status = *addr) & SCS_SR7) != SCS_SR7) {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			*addr = SCS_READ_CMD;
			return (1);
		}
	}
	*addr = SCS_READ_CMD;
	return (0);
}

/* vim: set ts=4 sw=4 tw=78: */
