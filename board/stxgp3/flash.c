/*
 * (C) Copyright 2003, Dan Malek, Embedded Edge, LLC.  <dan@embeddededge.com>
 * Copied from ADS85xx.
 * Updated to support the Silicon Tx GP3 8560.  We should only find
 * two Intel 28F640 parts in 16-bit mode (i.e. 32-bit wide flash),
 * but I left other code here in case people order custom boards.
 *
 * (C) Copyright 2003 Motorola Inc.
 *  Xianghua Xiao,(X.Xiao@motorola.com)
 *
 * (C) Copyright 2000, 2001
 *  Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001, Stuart Hughes, Lineo Inc, stuarth@lineo.com
 * Add support the Sharp chips on the mpc8260ads.
 * I started with board/ip860/flash.c and made changes I found in
 * the MTD project by David Schleef.
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

#if !defined(CFG_NO_FLASH)

flash_info_t	flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

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

#undef DEBUG

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);
static int clear_block_lock_bit(vu_long * addr);
/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	unsigned long size;
	int i;

	/* Init: enable write,
	 * or we cannot even write flash commands
	 */
	for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;

		/* set the default sector offset */
	}

	/* Static FLASH Bank configuration here - FIXME XXX */

	size = flash_get_size((vu_long *)CFG_FLASH_BASE, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
			size, size<<20);
	}

	/* Re-do sizing to get full correct info */
	size = flash_get_size((vu_long *)CFG_FLASH_BASE, &flash_info[0]);

	flash_info[0].size = size;

#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CFG_MONITOR_BASE,
		      CFG_MONITOR_BASE+monitor_flash_len-1,
		      &flash_info[0]);

#ifdef	CFG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CFG_ENV_ADDR,
		      CFG_ENV_ADDR+CFG_ENV_SECT_SIZE-1,
		      &flash_info[0]);
#endif
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
	case FLASH_28F640C3T:	printf ("28F640C3T (64 Mbit x 2, 128 x 128k)\n");
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

/*
 * The following code cannot be run from FLASH!
 */

static ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
	short i;
	ulong value;
	ulong base = (ulong)addr;
	ulong sector_offset;

#ifdef DEBUG
	printf("Check flash at 0x%08x\n",(uint)addr);
#endif
	/* Write "Intelligent Identifier" command: read Manufacturer ID */
	*addr = 0x90909090;
	udelay(20);
	asm("sync");

	value = addr[0] & 0x00FF00FF;

#ifdef DEBUG
	printf("manufacturer=0x%x\n",(uint)value);
#endif
	switch (value) {
	case MT_MANUFACT:	/* SHARP, MT or => Intel */
	case INTEL_ALT_MANU:
		info->flash_id = FLASH_MAN_INTEL;
		break;
	default:
		printf("unknown manufacturer: %x\n", (unsigned int)value);
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);			/* no or unknown flash	*/
	}

	value = addr[1];             /* device ID            */

#ifdef DEBUG
	printf("deviceID=0x%x\n",(uint)value);
#endif
	switch (value) {

	case (INTEL_ID_28F640C3T):
		info->flash_id += FLASH_28F640C3T;
		info->sector_count = 135;
		info->size = 0x01000000;
		sector_offset = 0x20000;
		break;				/* => 2x8 MB		*/

	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */

	}

	/* set up sector start address table
	 * The first 127 blocks are large, the last 8 are small.
	 */
	for (i = 0; i < 127; i++) {
		info->start[i] = base;
		base += sector_offset;
		/* Sectors are locked upon reset */
		info->protect[i] = 0;
	}
	for (i = 127; i < 135; i++) {
		info->start[i] = base;
		base += 0x4000;
		/* Sectors are locked upon reset */
		info->protect[i] = 0;
	}


	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr = (vu_long *)info->start[0];
		*addr = 0xFFFFFF;	/* reset bank to read array mode */
		asm("sync");
	}

	return (info->size);
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

#ifdef DEBUG
	printf("\nFlash Erase:\n");
#endif
	/* Make Sure Block Lock Bit is not set. */
	if(clear_block_lock_bit((vu_long *)(info->start[s_first]))){
		return 1;
	}

	/* Start erase on unprotected sectors */
#if defined(DEBUG)
	printf("Begin to erase now,s_first=0x%x s_last=0x%x...\n",s_first,s_last);
#endif
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			vu_long *addr = (vu_long *)(info->start[sect]);
			asm("sync");

			last = start = get_timer (0);

			/* Disable interrupts which might cause a timeout here */
			flag = disable_interrupts();

			/* Reset Array */
			*addr = 0xffffffff;
			asm("sync");
			/* Clear Status Register */
			*addr = 0x50505050;
			asm("sync");
			/* Single Block Erase Command */
			*addr = 0x20202020;
			asm("sync");
			/* Confirm */
			*addr = 0xD0D0D0D0;
			asm("sync");

			if((info->flash_id & FLASH_TYPEMASK) != FLASH_LH28F016SCT) {
			    /* Resume Command, as per errata update */
			    *addr = 0xD0D0D0D0;
			    asm("sync");
			}

			/* re-enable interrupts if necessary */
			if (flag)
				enable_interrupts();

			/* wait at least 80us - let's wait 1 ms */
			udelay (1000);
			while ((*addr & 0x00800080) != 0x00800080) {
				if(*addr & 0x00200020){
					printf("Error in Block Erase - Lock Bit may be set!\n");
					printf("Status Register = 0x%X\n", (uint)*addr);
					*addr = 0xFFFFFFFF;	/* reset bank */
					asm("sync");
					return 1;
				}
				if ((now=get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
					printf ("Timeout\n");
					*addr = 0xFFFFFFFF;	/* reset bank */
					asm("sync");
					return 1;
				}
				/* show that we're waiting */
				if ((now - last) > 1000) {	/* every second */
					putc ('.');
					last = now;
				}
			}

			/* reset to read mode */
			*addr = 0xFFFFFFFF;
			asm("sync");
		}
	}

	printf ("flash erase done\n");
	return 0;
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

	return (write_word(info, wp, data));
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word (flash_info_t *info, ulong dest, ulong data)
{
	vu_long *addr = (vu_long *)dest;
	ulong start, csr;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*addr & data) != data) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	/* Write Command */
	*addr = 0x10101010;
	asm("sync");

	/* Write Data */
	*addr = data;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer (0);
	flag  = 0;

	while (((csr = *addr) & 0x00800080) != 0x00800080) {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			flag = 1;
			break;
		}
	}
	if (csr & 0x40404040) {
		printf ("CSR indicates write error (%08lx) at %08lx\n", csr, (ulong)addr);
		flag = 1;
	}

	/* Clear Status Registers Command */
	*addr = 0x50505050;
	asm("sync");
	/* Reset to read array mode */
	*addr = 0xFFFFFFFF;
	asm("sync");

	return (flag);
}

/*-----------------------------------------------------------------------
 * Clear Block Lock Bit, returns:
 * 0 - OK
 * 1 - Timeout
 */

static int clear_block_lock_bit(vu_long  * addr)
{
	ulong start, now;

	/* Reset Array */
	*addr = 0xffffffff;
	asm("sync");
	/* Clear Status Register */
	*addr = 0x50505050;
	asm("sync");

	*addr = 0x60606060;
	asm("sync");
	*addr = 0xd0d0d0d0;
	asm("sync");

	start = get_timer (0);
	while((*addr & 0x00800080) != 0x00800080){
		if ((now=get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
			printf ("Timeout on clearing Block Lock Bit\n");
			*addr = 0xFFFFFFFF;	/* reset bank */
			asm("sync");
			return 1;
		}
	}
	return 0;
}

#endif /* !CFG_NO_FLASH */
