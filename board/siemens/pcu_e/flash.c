/*
 * (C) Copyright 2001
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

/*---------------------------------------------------------------------*/
#undef DEBUG_FLASH

#ifdef DEBUG_FLASH
#define DEBUGF(fmt,args...) printf(fmt ,##args)
#else
#define DEBUGF(fmt,args...)
#endif
/*---------------------------------------------------------------------*/


flash_info_t	flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static int write_data (flash_info_t *info, ulong dest, ulong data);
static void flash_get_offsets (ulong base, flash_info_t *info);

/*-----------------------------------------------------------------------
 *
 * The PCU E uses an address map where flash banks are aligned top
 * down, so that the "first" flash bank ends at top of memory, and
 * the monitor entry point is at address (0xFFF00100). The second
 * flash bank is mapped immediately below bank 0.
 *
 * This is NOT in conformance to the "official" memory map!
 *
 */

#define PCU_MONITOR_BASE   ( (flash_info[0].start[0] + flash_info[0].size - 1) \
			   - (0xFFFFFFFF - CFG_MONITOR_BASE) )

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	volatile immap_t     *immap  = (immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	unsigned long base, size_b0, size_b1;
	int i;

	/* Init: no FLASHes known */
	for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Static FLASH Bank configuration here - FIXME XXX */

	/*
	 * Warning:
	 *
	 * Since the PCU E memory map assigns flash banks top down,
	 * we swap the numbering later if both banks are equipped,
	 * so they look like a contiguous area of memory.
	 */
	DEBUGF("\n## Get flash bank 1 size @ 0x%08x\n",FLASH_BASE0_PRELIM);

	size_b0 = flash_get_size((vu_long *)FLASH_BASE0_PRELIM, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
			size_b0, size_b0<<20);
	}

	DEBUGF("## Get flash bank 2 size @ 0x%08x\n",FLASH_BASE6_PRELIM);
	size_b1 = flash_get_size((vu_long *)FLASH_BASE6_PRELIM, &flash_info[1]);

	DEBUGF("## Prelim. Flash bank sizes: %08lx + 0x%08lx\n", size_b0, size_b1);

	if (size_b1 > size_b0) {
		printf ("## ERROR: "
			"Bank 1 (0x%08lx = %ld MB) > Bank 0 (0x%08lx = %ld MB)\n",
			size_b1, size_b1<<20,
			size_b0, size_b0<<20
		);
		flash_info[0].flash_id	= FLASH_UNKNOWN;
		flash_info[1].flash_id	= FLASH_UNKNOWN;
		flash_info[0].sector_count	= -1;
		flash_info[1].sector_count	= -1;
		flash_info[0].size		= 0;
		flash_info[1].size		= 0;
		return (0);
	}

	DEBUGF ("## Before remap: "
		"BR0: 0x%08x    OR0: 0x%08x    "
		"BR6: 0x%08x    OR6: 0x%08x\n",
		memctl->memc_br0, memctl->memc_or0,
		memctl->memc_br6, memctl->memc_or6);

	/* Remap FLASH according to real size */
	base = 0 - size_b0;
	memctl->memc_or0 = CFG_OR_TIMING_FLASH | (-size_b0 & 0xFFFF8000);
	memctl->memc_br0 = (base & BR_BA_MSK) | BR_PS_16 | BR_MS_GPCM | BR_V;

	DEBUGF("## BR0: 0x%08x    OR0: 0x%08x\n",
		memctl->memc_br0, memctl->memc_or0);

	/* Re-do sizing to get full correct info */
	size_b0 = flash_get_size((vu_long *)base, &flash_info[0]);
	base = 0 - size_b0;

	flash_info[0].size = size_b0;

	flash_get_offsets (base, &flash_info[0]);

	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      PCU_MONITOR_BASE,
		      PCU_MONITOR_BASE+monitor_flash_len-1,
		      &flash_info[0]);

#ifdef	CFG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CFG_ENV_ADDR,
		      CFG_ENV_ADDR+CFG_ENV_SECT_SIZE-1,
		      &flash_info[0]);
#endif

	if (size_b1) {
		flash_info_t tmp_info;

		memctl->memc_or6 = CFG_OR_TIMING_FLASH | (-size_b1 & 0xFFFF8000);
		memctl->memc_br6 = ((base - size_b1) & BR_BA_MSK) |
				    BR_PS_16 | BR_MS_GPCM | BR_V;

		DEBUGF("## New BR6: 0x%08x    OR6: 0x%08x\n",
			memctl->memc_br6, memctl->memc_or6);

		/* Re-do sizing to get full correct info */
		size_b1 = flash_get_size((vu_long *)(base - size_b1),
					  &flash_info[1]);
		base -= size_b1;

		flash_get_offsets (base, &flash_info[1]);

		flash_info[1].size = size_b1;

#ifdef	CFG_ENV_IS_IN_FLASH
		/* ENV protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CFG_ENV_ADDR,
			      CFG_ENV_ADDR+CFG_ENV_SECT_SIZE-1,
			      &flash_info[1]);
#endif
		/*
		 * Swap bank numbers so that addresses are in ascending order
		 */
		tmp_info = flash_info[0];
		flash_info[0] = flash_info[1];
		flash_info[1] = tmp_info;
	} else {
		memctl->memc_br1 = 0;		/* invalidate bank */

		flash_info[1].flash_id = FLASH_UNKNOWN;
		flash_info[1].sector_count = -1;
	}


	DEBUGF("## Final Flash bank sizes: %08lx + 0x%08lx\n",size_b0,size_b1);

	return (size_b0 + size_b1);
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets (ulong base, flash_info_t *info)
{
	int i;
	short n;

	if (info->flash_id == FLASH_UNKNOWN) {
		return;
	}

	if ((info->flash_id & FLASH_VENDMASK) != FLASH_MAN_AMD) {
		return;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AMDL322T:
	case FLASH_AMDL323T:
	case FLASH_AMDL324T:
		/* set sector offsets for top boot block type		*/

		base += info->size;
		i = info->sector_count;
		for (n=0; n<8; ++n) {		/*  8 x 8k boot sectors	*/
			base -= 8 << 10;
			--i;
			info->start[i] = base;
		}
		while (i > 0) {			/* 64k regular sectors	*/
			base -= 64 << 10;
			--i;
			info->start[i] = base;
		}
		return;
	case FLASH_AMDL322B:
	case FLASH_AMDL323B:
	case FLASH_AMDL324B:
		/* set sector offsets for bottom boot block type	*/
		for (i=0; i<8; ++i) {		/*  8 x 8k boot sectors	*/
			info->start[i] = base;
			base += 8 << 10;
		}
		while (base < info->size) {	/* 64k regular sectors	*/
			info->start[i] = base;
			base += 64 << 10;
			++i;
		}
		return;
	case FLASH_AMDL640:
		/* set sector offsets for dual boot block type		*/
		for (i=0; i<8; ++i) {		/*  8 x 8k boot sectors	*/
			info->start[i] = base;
			base += 8 << 10;
		}
		n = info->sector_count - 8;
		while (i < n) {			/* 64k regular sectors	*/
			info->start[i] = base;
			base += 64 << 10;
			++i;
		}
		while (i < info->sector_count) { /* 8 x 8k boot sectors	*/
			info->start[i] = base;
			base += 8 << 10;
			++i;
		}
		return;
	default:
		return;
	}
	/* NOTREACHED */
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
	case FLASH_MAN_AMD:	printf ("AMD ");		break;
	case FLASH_MAN_FUJ:	printf ("FUJITSU ");		break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AMDL322B:	printf ("AM29DL322B (32 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AMDL322T:	printf ("AM29DL322T (32 Mbit, top boot sector)\n");
				break;
	case FLASH_AMDL323B:	printf ("AM29DL323B (32 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AMDL323T:	printf ("AM29DL323T (32 Mbit, top boot sector)\n");
				break;
	case FLASH_AMDL324B:	printf ("AM29DL324B (32 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AMDL324T:	printf ("AM29DL324T (32 Mbit, top boot sector)\n");
				break;
	case FLASH_AMDL640:	printf ("AM29DL640D (64 Mbit, dual boot sector)\n");
				break;
	default:		printf ("Unknown Chip Type 0x%lX\n",
					info->flash_id);
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
	ushort value;
	vu_short *saddr = (vu_short *)addr;

	/* Write auto select command: read Manufacturer ID */
	saddr[0x0555] = 0x00AA;
	saddr[0x02AA] = 0x0055;
	saddr[0x0555] = 0x0090;

	value = saddr[0];

	DEBUGF("Manuf. ID @ 0x%08lx: 0x%04x\n", (ulong)addr, value);

	switch (value) {
	case (AMD_MANUFACT & 0xFFFF):
		info->flash_id = FLASH_MAN_AMD;
		break;
	case (FUJ_MANUFACT & 0xFFFF):
		info->flash_id = FLASH_MAN_FUJ;
		break;
	default:
		DEBUGF("Unknown Manufacturer ID\n");
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);			/* no or unknown flash	*/
	}

	value = saddr[1];			/* device ID		*/

	DEBUGF("Device ID @ 0x%08lx: 0x%04x\n", (ulong)(&addr[1]), value);

	switch (value) {

	case (AMD_ID_DL322T & 0xFFFF):
		info->flash_id += FLASH_AMDL322T;
		info->sector_count = 71;
		info->size = 0x00400000;
		break;				/* => 8 MB		*/

	case (AMD_ID_DL322B & 0xFFFF):
		info->flash_id += FLASH_AMDL322B;
		info->sector_count = 71;
		info->size = 0x00400000;
		break;				/* => 8 MB		*/

	case (AMD_ID_DL323T & 0xFFFF):
		info->flash_id += FLASH_AMDL323T;
		info->sector_count = 71;
		info->size = 0x00400000;
		break;				/* => 8 MB		*/

	case (AMD_ID_DL323B & 0xFFFF):
		info->flash_id += FLASH_AMDL323B;
		info->sector_count = 71;
		info->size = 0x00400000;
		break;				/* => 8 MB		*/

	case (AMD_ID_DL324T & 0xFFFF):
		info->flash_id += FLASH_AMDL324T;
		info->sector_count = 71;
		info->size = 0x00400000;
		break;				/* => 8 MB		*/

	case (AMD_ID_DL324B & 0xFFFF):
		info->flash_id += FLASH_AMDL324B;
		info->sector_count = 71;
		info->size = 0x00400000;
		break;				/* => 8 MB		*/
	case (AMD_ID_DL640  & 0xFFFF):
		info->flash_id += FLASH_AMDL640;
		info->sector_count = 142;
		info->size = 0x00800000;
		break;
	default:
		DEBUGF("Unknown Device ID\n");
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */

	}

	flash_get_offsets ((ulong)addr, info);

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
#if 0
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		saddr = (vu_short *)(info->start[i]);
		info->protect[i] = saddr[2] & 1;
#else
		info->protect[i] =0;
#endif
	}

	if (info->sector_count > CFG_MAX_FLASH_SECT) {
		printf ("** ERROR: sector count %d > max (%d) **\n",
			info->sector_count, CFG_MAX_FLASH_SECT);
		info->sector_count = CFG_MAX_FLASH_SECT;
	}

	saddr = (vu_short *)info->start[0];
	*saddr = 0x00F0;	/* restore read mode */

	return (info->size);
}


/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	vu_short *addr = (vu_short*)(info->start[0]);
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

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	addr[0x0555] = 0x00AA;
	addr[0x02AA] = 0x0055;
	addr[0x0555] = 0x0080;
	addr[0x0555] = 0x00AA;
	addr[0x02AA] = 0x0055;

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = (vu_short*)(info->start[sect]);
			addr[0] = 0x0030;
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
	addr = (vu_short*)(info->start[l_sect]);
	while ((addr[0] & 0x0080) != 0x0080) {
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
	addr = (vu_short *)info->start[0];
	addr[0] = 0x00F0;	/* reset bank */

	printf (" done\n");
	return 0;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

#define FLASH_WIDTH	2	/* flash bus width in bytes */

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int i, l, rc;

	wp = (addr & ~(FLASH_WIDTH-1));	/* get lower FLASH_WIDTH aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i=0, cp=wp; i<l; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}
		for (; i<FLASH_WIDTH && cnt>0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt==0 && i<FLASH_WIDTH; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}

		if ((rc = write_data(info, wp, data)) != 0) {
			return (rc);
		}
		wp += FLASH_WIDTH;
	}

	/*
	 * handle FLASH_WIDTH aligned part
	 */
	while (cnt >= FLASH_WIDTH) {
		data = 0;
		for (i=0; i<FLASH_WIDTH; ++i) {
			data = (data << 8) | *src++;
		}
		if ((rc = write_data(info, wp, data)) != 0) {
			return (rc);
		}
		wp  += FLASH_WIDTH;
		cnt -= FLASH_WIDTH;
	}

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i=0, cp=wp; i<FLASH_WIDTH && cnt>0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i<FLASH_WIDTH; ++i, ++cp) {
		data = (data << 8) | (*(uchar *)cp);
	}

	return (write_data(info, wp, data));
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_data (flash_info_t *info, ulong dest, ulong data)
{
	vu_short *addr  = (vu_short*)(info->start[0]);
	vu_short *sdest = (vu_short *)dest;
	ushort sdata = (ushort)data;
	ushort sval;
	ulong start, passed;
	int flag, rc;

	/* Check if Flash is (sufficiently) erased */
	if ((*sdest & sdata) != sdata) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	addr[0x0555] = 0x00AA;
	addr[0x02AA] = 0x0055;
	addr[0x0555] = 0x00A0;

#ifdef WORKAROUND_FOR_BROKEN_HARDWARE
	/* work around the timeout bugs */
	udelay(20);
#endif

	*sdest = sdata;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	rc = 0;
	/* data polling for D7 */
	start = get_timer (0);

	for (passed=0; passed < CFG_FLASH_WRITE_TOUT; passed=get_timer(start)) {

		sval = *sdest;

		if ((sval & 0x0080) == (sdata & 0x0080))
			break;

		if ((sval & 0x0020) == 0)	/* DQ5: Timeout? */
			continue;

		sval = *sdest;

		if ((sval & 0x0080) != (sdata & 0x0080))
			rc = 1;

		break;
	}

	if (rc) {
	    DEBUGF ("Program cycle failed @ addr 0x%08lX: val %04X data %04X\n",
		 dest, sval, sdata);
	}

	if (passed >= CFG_FLASH_WRITE_TOUT) {
		DEBUGF ("Timeout @ addr 0x%08lX: val %04X data %04X\n",
			dest, sval, sdata);
		rc = 1;
	}

	/* reset to read mode */
	addr = (vu_short *)info->start[0];
	addr[0] = 0x00F0;	/* reset bank */

	return (rc);
}

/*-----------------------------------------------------------------------
 */
