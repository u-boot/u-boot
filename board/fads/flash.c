/*
 * (C) Copyright 2000
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
#include <mpc8xx.h>

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

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);
static void flash_get_offsets (ulong base, flash_info_t *info);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	volatile immap_t     *immap  = (immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	unsigned long total_size;
	unsigned long size_b0, size_b1;
	int i;

	/* Init: no FLASHes known */
	for (i=0; i < CFG_MAX_FLASH_BANKS; ++i)
	{
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	total_size = 0;
	size_b0 = 0xffffffff;

	for (i=0; i < CFG_MAX_FLASH_BANKS; ++i)
	{
		size_b1 = flash_get_size((vu_long *)(CFG_FLASH_BASE + total_size), &flash_info[i]);

		if (flash_info[i].flash_id == FLASH_UNKNOWN)
		{
			printf ("## Unknown FLASH on Bank %d - Size = 0x%08lx = %ld MB\n", i, size_b1, size_b1>>20);
		}

		/* Is this really needed ? - LP */
		if (size_b1 > size_b0) {
			printf ("## ERROR: Bank %d (0x%08lx = %ld MB) > Bank %d (0x%08lx = %ld MB)\n",
			        i, size_b1, size_b1>>20, i-1, size_b0, size_b0>>20);
			goto out_error;
		}
		size_b0 = size_b1;
		total_size += size_b1;
	}

	/* Compute the Address Mask */
	for (i=0; (total_size >> i) != 0; ++i) {};
	i--;

	if (total_size != (1 << i)) {
		printf ("## WARNING: Total FLASH size (0x%08lx = %ld MB) is not a power of 2\n",
			total_size, total_size>>20);
	}

	/* Remap FLASH according to real size */
	memctl->memc_or0 = ((((unsigned long)~1) << i) & OR_AM_MSK) | CFG_OR_TIMING_FLASH;
	memctl->memc_br0 = CFG_BR0_PRELIM;

	total_size = 0;

	for (i=0; i < CFG_MAX_FLASH_BANKS && flash_info[i].size != 0; ++i)
	{
		/* Re-do sizing to get full correct info */
		/* Why ? - LP */
		size_b1 = flash_get_size((vu_long *)(CFG_FLASH_BASE + total_size), &flash_info[i]);

		/* This is done by flash_get_size - LP */
		/* flash_get_offsets (CFG_FLASH_BASE + total_size, &flash_info[i]); */

#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
		/* monitor protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CFG_MONITOR_BASE,
			      CFG_MONITOR_BASE+monitor_flash_len-1,
			      &flash_info[i]);
#endif

#ifdef	CFG_ENV_IS_IN_FLASH
		/* ENV protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CFG_ENV_ADDR,
			      CFG_ENV_ADDR+CFG_ENV_SIZE-1,
			      &flash_info[i]);
#endif

		total_size += size_b1;
	}

	return (total_size);

out_error:
	for (i=0; i < CFG_MAX_FLASH_BANKS; ++i)
	{
		flash_info[i].flash_id		= FLASH_UNKNOWN;
		flash_info[i].sector_count	= -1;
		flash_info[i].size		= 0;
	}

	return (0);
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets (ulong base, flash_info_t *info)
{
	int i;

	/* set up sector start address table */
	if ((info->flash_id & FLASH_TYPEMASK) == FLASH_AM040 || (info->flash_id & FLASH_TYPEMASK) == FLASH_AM080 ) {
		/* set sector offsets for uniform sector type	*/
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00040000);
		}
	}
}

/*-----------------------------------------------------------------------
 */
void flash_print_info  (flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN)
	{
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK)
	{
		case FLASH_MAN_AMD:	printf ("AMD ");		break;
		case FLASH_MAN_FUJ:	printf ("FUJITSU ");		break;
		case FLASH_MAN_BM:	printf ("BRIGHT MICRO ");	break;
		default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK)
	{
		case FLASH_AM040:	printf ("29F040 or 29LV040 (4 Mbit, uniform sectors)\n");
			break;
		case FLASH_AM080:	printf ("29F080 or 29LV080 (8 Mbit, uniform sectors)\n");
		                	break;
		case FLASH_AM400B:	printf ("AM29LV400B (4 Mbit, bottom boot sect)\n");
					break;
		case FLASH_AM400T:	printf ("AM29LV400T (4 Mbit, top boot sector)\n");
					break;
		case FLASH_AM800B:	printf ("AM29LV800B (8 Mbit, bottom boot sect)\n");
					break;
		case FLASH_AM800T:	printf ("AM29LV800T (8 Mbit, top boot sector)\n");
					break;
		case FLASH_AM160B:	printf ("AM29LV160B (16 Mbit, bottom boot sect)\n");
					break;
		case FLASH_AM160T:	printf ("AM29LV160T (16 Mbit, top boot sector)\n");
					break;
		case FLASH_AM320B:	printf ("AM29LV320B (32 Mbit, bottom boot sect)\n");
					break;
		case FLASH_AM320T:	printf ("AM29LV320T (32 Mbit, top boot sector)\n");
					break;
		default:		printf ("Unknown Chip Type\n");
					break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");

	for (i=0; i<info->sector_count; ++i)
	{
		if ((i % 5) == 0)
		{
			printf ("\n   ");
		}

		printf (" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     ");
	}

	printf ("\n");
	return;
}

/*-----------------------------------------------------------------------
 */


/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */

static ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
	short i;
#if 0
	ulong base = (ulong)addr;
#endif
	uchar value;

	/* Write auto select command: read Manufacturer ID */
#if 0
	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;
	addr[0x0555] = 0x00900090;
#else
	addr[0x0555] = 0xAAAAAAAA;
	addr[0x02AA] = 0x55555555;
	addr[0x0555] = 0x90909090;
#endif

	value = addr[0];

	switch (value + (value << 16))
	{
		case AMD_MANUFACT:
			info->flash_id = FLASH_MAN_AMD;
			break;

		case FUJ_MANUFACT:
			info->flash_id = FLASH_MAN_FUJ;
			break;

		default:
			info->flash_id = FLASH_UNKNOWN;
			info->sector_count = 0;
			info->size = 0;
			break;
	}

	value = addr[1];			/* device ID		*/

	switch (value)
	{
		case AMD_ID_F040B:
			info->flash_id += FLASH_AM040;
			info->sector_count = 8;
			info->size = 0x00200000;
			break;				/* => 2 MB		*/

		case AMD_ID_F080B:
			info->flash_id += FLASH_AM080;
			info->sector_count =16;
			info->size = 0x00400000;
			break;				/* => 4 MB		*/

		case AMD_ID_LV400T:
			info->flash_id += FLASH_AM400T;
			info->sector_count = 11;
			info->size = 0x00100000;
			break;				/* => 1 MB		*/

		case AMD_ID_LV400B:
			info->flash_id += FLASH_AM400B;
			info->sector_count = 11;
			info->size = 0x00100000;
			break;				/* => 1 MB		*/

		case AMD_ID_LV800T:
			info->flash_id += FLASH_AM800T;
			info->sector_count = 19;
			info->size = 0x00200000;
			break;				/* => 2 MB		*/

		case AMD_ID_LV800B:
			info->flash_id += FLASH_AM800B;
			info->sector_count = 19;
			info->size = 0x00200000;
			break;				/* => 2 MB		*/

		case AMD_ID_LV160T:
			info->flash_id += FLASH_AM160T;
			info->sector_count = 35;
			info->size = 0x00400000;
			break;				/* => 4 MB		*/

		case AMD_ID_LV160B:
			info->flash_id += FLASH_AM160B;
			info->sector_count = 35;
			info->size = 0x00400000;
			break;				/* => 4 MB		*/
#if 0	/* enable when device IDs are available */
		case AMD_ID_LV320T:
			info->flash_id += FLASH_AM320T;
			info->sector_count = 67;
			info->size = 0x00800000;
			break;				/* => 8 MB		*/

		case AMD_ID_LV320B:
			info->flash_id += FLASH_AM320B;
			info->sector_count = 67;
			info->size = 0x00800000;
			break;				/* => 8 MB		*/
#endif
		default:
			info->flash_id = FLASH_UNKNOWN;
			return (0);			/* => no or unknown flash */

	}

#if 0
	/* set up sector start address table */
	if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block type	*/
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00008000;
		info->start[2] = base + 0x0000C000;
		info->start[3] = base + 0x00010000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00020000) - 0x00060000;
		}
	} else {
		/* set sector offsets for top boot block type		*/
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00008000;
		info->start[i--] = base + info->size - 0x0000C000;
		info->start[i--] = base + info->size - 0x00010000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00020000;
		}
	}
#else
	flash_get_offsets ((ulong)addr, info);
#endif

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++)
	{
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr = (volatile unsigned long *)(info->start[i]);
		info->protect[i] = addr[2] & 1;
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN)
	{
		addr = (volatile unsigned long *)info->start[0];
#if 0
		*addr = 0x00F000F0;	/* reset bank */
#else
		*addr = 0xF0F0F0F0;	/* reset bank */
#endif
	}

	return (info->size);
}


/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	vu_long *addr = (vu_long*)(info->start[0]);
	int flag, prot, sect, l_sect;
	ulong start, now, last;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	if ((info->flash_id == FLASH_UNKNOWN) ||
	    (info->flash_id > FLASH_AMD_COMP)) {
		printf ("Can't erase unknown flash type - aborted\n");
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

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

#if 0
	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;
	addr[0x0555] = 0x00800080;
	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;
#else
	addr[0x0555] = 0xAAAAAAAA;
	addr[0x02AA] = 0x55555555;
	addr[0x0555] = 0x80808080;
	addr[0x0555] = 0xAAAAAAAA;
	addr[0x02AA] = 0x55555555;
#endif

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = (vu_long*)(info->start[sect]);
#if 0
			addr[0] = 0x00300030;
#else
			addr[0] = 0x30303030;
#endif
			l_sect = sect;
		}
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* wait at least 80us - let's wait 1 ms */
	udelay (1000);

	/*
	 * We wait for the last triggered sector
	 */
	if (l_sect < 0)
		goto DONE;

	start = get_timer (0);
	last  = start;
	addr = (vu_long*)(info->start[l_sect]);
#if 0
	while ((addr[0] & 0x00800080) != 0x00800080)
#else
	while ((addr[0] & 0xFFFFFFFF) != 0xFFFFFFFF)
#endif
	{
		if ((now = get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
			printf ("Timeout\n");
			return 1;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) {	/* every second */
			putc ('.');
			last = now;
		}
	}

DONE:
	/* reset to read mode */
	addr = (volatile unsigned long *)info->start[0];
#if 0
	addr[0] = 0x00F000F0;	/* reset bank */
#else
	addr[0] = 0xF0F0F0F0;	/* reset bank */
#endif

	printf (" done\n");
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
	vu_long *addr = (vu_long*)(info->start[0]);
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*((vu_long *)dest) & data) != data) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

#if 0
	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;
	addr[0x0555] = 0x00A000A0;
#else
	addr[0x0555] = 0xAAAAAAAA;
	addr[0x02AA] = 0x55555555;
	addr[0x0555] = 0xA0A0A0A0;
#endif

	*((vu_long *)dest) = data;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer (0);
#if 0
	while ((*((vu_long *)dest) & 0x00800080) != (data & 0x00800080))
#else
	while ((*((vu_long *)dest) & 0x80808080) != (data & 0x80808080))
#endif
	{
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			return (1);
		}
	}
	return (0);
}

/*-----------------------------------------------------------------------
 */
