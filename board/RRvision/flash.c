/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#define DEBUG

#include <common.h>
#include <mpc8xx.h>

#ifndef	CONFIG_ENV_ADDR
#define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#endif

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	volatile immap_t     *immap  = (immap_t *)CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	unsigned long size;
	int i;

	/* Init: no FLASHes known */
	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Static FLASH Bank configuration here - FIXME XXX */

	size = flash_get_size((vu_long *)FLASH_BASE0_PRELIM, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
			size, size<<20);
	}

	/* Remap FLASH according to real size */
	memctl->memc_or0 = CONFIG_SYS_OR_TIMING_FLASH | (-size & OR_AM_MSK);
	memctl->memc_br0 = (CONFIG_SYS_FLASH_BASE & BR_BA_MSK) | BR_MS_GPCM | BR_V;

	/* Re-do sizing to get full correct info */
	size = flash_get_size((vu_long *)CONFIG_SYS_FLASH_BASE, &flash_info[0]);

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
		      CONFIG_ENV_ADDR+CONFIG_ENV_SIZE-1,
		      &flash_info[0]);
#endif

	flash_info[0].size = size;

	return (size);
}

/*-----------------------------------------------------------------------
 */
void flash_print_info  (flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		puts ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:	puts ("AMD ");			break;
	case FLASH_MAN_FUJ:	puts ("FUJITSU ");		break;
	default:		puts ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM400B:	puts ("AM29LV400B (4 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM400T:	puts ("AM29LV400T (4 Mbit, top boot sector)\n");
				break;
	case FLASH_AM800B:	puts ("AM29LV800B (8 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM800T:	puts ("AM29LV800T (8 Mbit, top boot sector)\n");
				break;
	case FLASH_AM160B:	puts ("AM29LV160B (16 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM160T:	puts ("AM29LV160T (16 Mbit, top boot sector)\n");
				break;
	case FLASH_AM320B:	puts ("AM29LV320B (32 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM320T:	puts ("AM29LV320T (32 Mbit, top boot sector)\n");
				break;
	default:		puts ("Unknown Chip Type\n");
				break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);

	puts ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
		if ((i % 5) == 0)
			puts ("\n   ");
		printf (" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     "
		);
	}
	puts ("\n");
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
	ulong value;
	ulong base = (ulong)addr;

	/* Write auto select command: read Manufacturer ID */
	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;
	addr[0x0555] = 0x00900090;

	value = addr[0];

	switch (value) {
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
		return (0);			/* no or unknown flash	*/
	}

	value = addr[1];			/* device ID		*/

	switch (value) {
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
	case AMD_ID_LV320T:
		info->flash_id += FLASH_AM320T;
		info->sector_count = 71;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/

	case AMD_ID_LV320B:
		info->flash_id += FLASH_AM320B;
		info->sector_count = 71;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/
	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */
	}

	/* set up sector start address table */
	switch (value) {
	case AMD_ID_LV400B:
	case AMD_ID_LV800B:
	case AMD_ID_LV160B:
		/* set sector offsets for bottom boot block type	*/
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00008000;
		info->start[2] = base + 0x0000C000;
		info->start[3] = base + 0x00010000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00020000) - 0x00060000;
		}
		break;
	case AMD_ID_LV400T:
	case AMD_ID_LV800T:
	case AMD_ID_LV160T:
		/* set sector offsets for top boot block type		*/
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00008000;
		info->start[i--] = base + info->size - 0x0000C000;
		info->start[i--] = base + info->size - 0x00010000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00020000;
		}
		break;
	case AMD_ID_LV320B:
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base;
			/*
			 * The first 8 sectors are 8 kB,
			 * all the other ones  are 64 kB
			 */
			base += (i < 8)
				?  2 * ( 8 << 10)
				:  2 * (64 << 10);
		}
		break;
	case AMD_ID_LV320T:
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base;
			/*
			 * The last 8 sectors are 8 kB,
			 * all the other ones  are 64 kB
			 */
			base += (i < (info->sector_count - 8))
				?  2 * (64 << 10)
				:  2 * ( 8 << 10);
		}
		break;
	default:
		return (0);
		break;
	}

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr = (volatile unsigned long *)(info->start[i]);
		info->protect[i] = addr[2] & 1;
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr = (volatile unsigned long *)info->start[0];

		*addr = 0x00F000F0;	/* reset bank */
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
			puts ("- missing\n");
		} else {
			puts ("- no sectors to erase\n");
		}
		return 1;
	}

	if ((info->flash_id == FLASH_UNKNOWN) ||
	    (info->flash_id > FLASH_AMD_COMP)) {
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
		puts ("\n");
	}

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;
	addr[0x0555] = 0x00800080;
	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = (vu_long*)(info->start[sect]);
			addr[0] = 0x00300030;
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
	while ((addr[0] & 0x00800080) != 0x00800080) {
		if ((now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
			puts ("Timeout\n");
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
	addr[0] = 0x00F000F0;	/* reset bank */

	puts (" done\n");
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

	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;
	addr[0x0555] = 0x00A000A0;

	*((vu_long *)dest) = data;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer (0);
	while ((*((vu_long *)dest) & 0x00800080) != (data & 0x00800080)) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			return (1);
		}
	}
	return (0);
}

/*-----------------------------------------------------------------------
 */
