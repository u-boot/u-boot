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

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];	/* info for FLASH chips */

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

#define QUAD_ID(id)	((((ulong)(id) & 0xFF) << 24) | \
			 (((ulong)(id) & 0xFF) << 16) | \
			 (((ulong)(id) & 0xFF) << 8)  | \
			 (((ulong)(id) & 0xFF) << 0)    \
			)

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_long * addr, flash_info_t * info);
static int write_word (flash_info_t * info, ulong dest, ulong data);

/*-----------------------------------------------------------------------
 */
unsigned long flash_init (void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	vu_long *bcsr = (vu_long *)BCSR_ADDR;
	unsigned long pd_size, total_size, bsize, or_am;
	int i;

	/* Init: no FLASHes known */
	for (i = 0; i < CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
		flash_info[i].size = 0;
		flash_info[i].sector_count = 0;
		flash_info[i].start[0] = 0xFFFFFFFF; /* For TFTP */
	}

	switch ((bcsr[2] & BCSR2_FLASH_PD_MASK) >> BCSR2_FLASH_PD_SHIFT) {
	case 2:
	case 4:
	case 6:
		pd_size = 0x800000;
		or_am = 0xFF800000;
		break;

	case 5:
	case 7:
		pd_size = 0x400000;
		or_am = 0xFFC00000;
		break;

	case 8:
		pd_size = 0x200000;
		or_am = 0xFFE00000;
		break;

	default:
		pd_size = 0;
		or_am = 0xFFE00000;
		printf("## Unsupported flash detected by BCSR: 0x%08X\n", bcsr[2]);
	}

	total_size = 0;
	for (i = 0; i < CFG_MAX_FLASH_BANKS && total_size < pd_size; ++i) {
		bsize = flash_get_size((vu_long *)(CFG_FLASH_BASE + total_size),
				       &flash_info[i]);

		if (flash_info[i].flash_id == FLASH_UNKNOWN) {
			printf ("## Unknown FLASH on Bank %d - Size = 0x%08lx = %ld MB\n",
				i, bsize, bsize >> 20);
		}

		total_size += bsize;
	}

	if (total_size != pd_size) {
		printf("## Detected flash size %lu conflicts with PD data %lu\n",
		       total_size, pd_size);
	}

	/* Remap FLASH according to real size */
	memctl->memc_or0 = or_am | CFG_OR_TIMING_FLASH;

	for (i = 0; i < CFG_MAX_FLASH_BANKS && flash_info[i].size != 0; ++i) {
#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
		/* monitor protection ON by default */
		if (CFG_MONITOR_BASE >= flash_info[i].start[0])
			flash_protect (FLAG_PROTECT_SET,
				       CFG_MONITOR_BASE,
				       CFG_MONITOR_BASE + monitor_flash_len - 1,
				       &flash_info[i]);
#endif

#ifdef	CFG_ENV_IS_IN_FLASH
		/* ENV protection ON by default */
		if (CFG_ENV_ADDR >= flash_info[i].start[0])
			flash_protect (FLAG_PROTECT_SET,
				       CFG_ENV_ADDR,
				       CFG_ENV_ADDR + CFG_ENV_SIZE - 1,
				       &flash_info[i]);
#endif
	}

	return total_size;
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
	case FLASH_MAN_AMD:
		printf ("AMD ");
		break;
	case FLASH_MAN_FUJ:
		printf ("FUJITSU ");
		break;
	case FLASH_MAN_BM:
		printf ("BRIGHT MICRO ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM040:
		printf ("29F040 or 29LV040 (4 Mbit, uniform sectors)\n");
		break;
	case FLASH_AM080:
		printf ("29F080 or 29LV080 (8 Mbit, uniform sectors)\n");
		break;
	case FLASH_AM400B:
		printf ("AM29LV400B (4 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM400T:
		printf ("AM29LV400T (4 Mbit, top boot sector)\n");
		break;
	case FLASH_AM800B:
		printf ("AM29LV800B (8 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM800T:
		printf ("AM29LV800T (8 Mbit, top boot sector)\n");
		break;
	case FLASH_AM160B:
		printf ("AM29LV160B (16 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM160T:
		printf ("AM29LV160T (16 Mbit, top boot sector)\n");
		break;
	case FLASH_AM320B:
		printf ("AM29LV320B (32 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM320T:
		printf ("AM29LV320T (32 Mbit, top boot sector)\n");
		break;
	default:
		printf ("Unknown Chip Type\n");
		break;
	}

	printf ("  Size: %ld MB in %d Sectors\n", info->size >> 20,
		info->sector_count);

	printf ("  Sector Start Addresses:");

	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0) {
			printf ("\n   ");
		}

		printf (" %08lX%s",
			info->start[i], info->protect[i] ? " (RO)" : "     ");
	}

	printf ("\n");
}

/*-----------------------------------------------------------------------
 * The following code can not run from flash!
 */
static ulong flash_get_size (vu_long * addr, flash_info_t * info)
{
	short i;

	/* Write auto select command: read Manufacturer ID */
	addr[0x0555] = 0xAAAAAAAA;
	addr[0x02AA] = 0x55555555;
	addr[0x0555] = 0x90909090;

	switch (addr[0]) {
	case QUAD_ID(AMD_MANUFACT):
		info->flash_id = FLASH_MAN_AMD;
		break;

	case QUAD_ID(FUJ_MANUFACT):
		info->flash_id = FLASH_MAN_FUJ;
		break;

	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		break;
	}

	switch (addr[1]) {	/* device ID            */
	case QUAD_ID(AMD_ID_F040B):
	case QUAD_ID(AMD_ID_LV040B):
		info->flash_id += FLASH_AM040;
		info->sector_count = 8;
		info->size = 0x00200000;
		break;		/* => 2 MB              */

	case QUAD_ID(AMD_ID_F080B):
		info->flash_id += FLASH_AM080;
		info->sector_count = 16;
		info->size = 0x00400000;
		break;		/* => 4 MB              */
#if 0
	case AMD_ID_LV400T:
		info->flash_id += FLASH_AM400T;
		info->sector_count = 11;
		info->size = 0x00100000;
		break;		/* => 1 MB              */

	case AMD_ID_LV400B:
		info->flash_id += FLASH_AM400B;
		info->sector_count = 11;
		info->size = 0x00100000;
		break;		/* => 1 MB              */

	case AMD_ID_LV800T:
		info->flash_id += FLASH_AM800T;
		info->sector_count = 19;
		info->size = 0x00200000;
		break;		/* => 2 MB              */

	case AMD_ID_LV800B:
		info->flash_id += FLASH_AM800B;
		info->sector_count = 19;
		info->size = 0x00200000;
		break;		/* => 2 MB              */

	case AMD_ID_LV160T:
		info->flash_id += FLASH_AM160T;
		info->sector_count = 35;
		info->size = 0x00400000;
		break;		/* => 4 MB              */

	case AMD_ID_LV160B:
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00400000;
		break;		/* => 4 MB              */

	case AMD_ID_LV320T:
		info->flash_id += FLASH_AM320T;
		info->sector_count = 67;
		info->size = 0x00800000;
		break;		/* => 8 MB              */

	case AMD_ID_LV320B:
		info->flash_id += FLASH_AM320B;
		info->sector_count = 67;
		info->size = 0x00800000;
		break;		/* => 8 MB              */
#endif /* 0 */
	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);	/* => no or unknown flash */
	}

#if 0
	/* set up sector start address table */
	if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block type        */
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00008000;
		info->start[2] = base + 0x0000C000;
		info->start[3] = base + 0x00010000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00020000) - 0x00060000;
		}
	} else {
		/* set sector offsets for top boot block type           */
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00008000;
		info->start[i--] = base + info->size - 0x0000C000;
		info->start[i--] = base + info->size - 0x00010000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00020000;
		}
	}
#else
	/* set sector offsets for uniform sector type */
	for (i = 0; i < info->sector_count; i++)
		info->start[i] = (ulong)addr + (i * 0x00040000);
#endif

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr = (volatile unsigned long *) (info->start[i]);
		info->protect[i] = addr[2] & 1;
	}

	if (info->flash_id != FLASH_UNKNOWN) {
		addr = (volatile unsigned long *) info->start[0];
		*addr = 0xF0F0F0F0;	/* reset bank */
	}

	return (info->size);
}

/*-----------------------------------------------------------------------
 */
int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	vu_long *addr = (vu_long *) (info->start[0]);
	int flag, prot, sect, l_sect;
	ulong start, now, last;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return ERR_INVAL;
	}

	if ((info->flash_id == FLASH_UNKNOWN) ||
	    (info->flash_id > FLASH_AMD_COMP)) {
		printf ("Can't erase unknown flash type - aborted\n");
		return ERR_UNKNOWN_FLASH_TYPE;
	}

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

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	addr[0x0555] = 0xAAAAAAAA;
	addr[0x02AA] = 0x55555555;
	addr[0x0555] = 0x80808080;
	addr[0x0555] = 0xAAAAAAAA;
	addr[0x02AA] = 0x55555555;

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = (vu_long *) (info->start[sect]);
			addr[0] = 0x30303030;
			l_sect = sect;
		}
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();

	/* wait at least 80us - let's wait 1 ms */
	udelay (1000);

	/*
	 * We wait for the last triggered sector
	 */
	if (l_sect < 0)
		goto DONE;

	start = get_timer (0);
	last = start;
	addr = (vu_long *) (info->start[l_sect]);
	while ((addr[0] & 0xFFFFFFFF) != 0xFFFFFFFF)
	{
		if ((now = get_timer (start)) > CFG_FLASH_ERASE_TOUT) {
			printf ("Timeout\n");
			return ERR_TIMOUT;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) {	/* every second */
			putc ('.');
			last = now;
		}
	}

      DONE:
	/* reset to read mode */
	addr = (volatile unsigned long *) info->start[0];
	addr[0] = 0xF0F0F0F0;	/* reset bank */

	printf (" done\n");

	return 0;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int i, l, rc;

	wp = (addr & ~3);	/* get lower word aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i = 0, cp = wp; i < l; ++i, ++cp) {
			data = (data << 8) | (*(uchar *) cp);
		}
		for (; i < 4 && cnt > 0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt == 0 && i < 4; ++i, ++cp) {
			data = (data << 8) | (*(uchar *) cp);
		}

		if ((rc = write_word (info, wp, data)) != 0) {
			return (rc);
		}
		wp += 4;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= 4) {
		data = 0;
		for (i = 0; i < 4; ++i) {
			data = (data << 8) | *src++;
		}
		if ((rc = write_word (info, wp, data)) != 0) {
			return (rc);
		}
		wp += 4;
		cnt -= 4;
	}

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i = 0, cp = wp; i < 4 && cnt > 0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i < 4; ++i, ++cp) {
		data = (data << 8) | (*(uchar *) cp);
	}

	return (write_word (info, wp, data));
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word (flash_info_t * info, ulong dest, ulong data)
{
	vu_long *addr = (vu_long *) (info->start[0]);
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*((vu_long *) dest) & data) != data) {
		return ERR_NOT_ERASED;
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	addr[0x0555] = 0xAAAAAAAA;
	addr[0x02AA] = 0x55555555;
	addr[0x0555] = 0xA0A0A0A0;

	*((vu_long *) dest) = data;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();

	/* data polling for D7 */
	start = get_timer (0);
	while ((*((vu_long *) dest) & 0x80808080) != (data & 0x80808080))
	{
		if (get_timer (start) > CFG_FLASH_WRITE_TOUT) {
			return ERR_TIMOUT;
		}
	}
	return (0);
}
