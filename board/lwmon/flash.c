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

/* #define DEBUG */

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

flash_info_t	flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static int write_data (flash_info_t *info, ulong dest, ulong data);
#ifdef CFG_FLASH_USE_BUFFER_WRITE
static int write_data_buf (flash_info_t * info, ulong dest, uchar * cp, int len);
#endif
static void flash_get_offsets (ulong base, flash_info_t *info);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	volatile immap_t     *immap  = (immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	unsigned long size_b0, size_b1;
	int i;

	/* Init: no FLASHes known */
	for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Static FLASH Bank configuration here - FIXME XXX */

	debug ("\n## Get flash bank 1 size @ 0x%08x\n",FLASH_BASE0_PRELIM);

	size_b0 = flash_get_size((vu_long *)FLASH_BASE0_PRELIM, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0: "
			"ID 0x%lx, Size = 0x%08lx = %ld MB\n",
			flash_info[0].flash_id,
			size_b0, size_b0<<20);
	}

	debug ("## Get flash bank 2 size @ 0x%08x\n",FLASH_BASE1_PRELIM);

	size_b1 = flash_get_size((vu_long *)FLASH_BASE1_PRELIM, &flash_info[1]);

	debug ("## Prelim. Flash bank sizes: %08lx + 0x%08lx\n",size_b0,size_b1);

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

	debug  ("## Before remap: "
		"BR0: 0x%08x    OR0: 0x%08x    "
		"BR1: 0x%08x    OR1: 0x%08x\n",
		memctl->memc_br0, memctl->memc_or0,
		memctl->memc_br1, memctl->memc_or1);

	/* Remap FLASH according to real size */
	memctl->memc_or0 = (-size_b0 & 0xFFFF8000) | CFG_OR_TIMING_FLASH |
				OR_CSNT_SAM | OR_ACS_DIV1;
	memctl->memc_br0 = (CFG_FLASH_BASE & BR_BA_MSK) | BR_PS_32 | BR_V;

	debug ("## BR0: 0x%08x    OR0: 0x%08x\n",
		memctl->memc_br0, memctl->memc_or0);

	/* Re-do sizing to get full correct info */
	size_b0 = flash_get_size((vu_long *)CFG_FLASH_BASE, &flash_info[0]);

	flash_get_offsets (CFG_FLASH_BASE, &flash_info[0]);

	flash_info[0].size = size_b0;

#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CFG_MONITOR_BASE,
		      CFG_MONITOR_BASE+monitor_flash_len-1,
		      &flash_info[0]);
#endif

#ifdef	CFG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CFG_ENV_ADDR,
		      CFG_ENV_ADDR+CFG_ENV_SECT_SIZE-1,
		      &flash_info[0]);
#endif

	if (size_b1) {
		memctl->memc_or1 = (-size_b1 & 0xFFFF8000) | CFG_OR_TIMING_FLASH |
					OR_CSNT_SAM | OR_ACS_DIV1;
		memctl->memc_br1 = ((CFG_FLASH_BASE + size_b0) & BR_BA_MSK) |
					BR_PS_32 | BR_V;

		debug ("## BR1: 0x%08x    OR1: 0x%08x\n",
			memctl->memc_br1, memctl->memc_or1);

		/* Re-do sizing to get full correct info */
		size_b1 = flash_get_size((vu_long *)(CFG_FLASH_BASE + size_b0),
					  &flash_info[1]);

		flash_info[1].size = size_b1;

		flash_get_offsets (CFG_FLASH_BASE + size_b0, &flash_info[1]);

#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
		/* monitor protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CFG_MONITOR_BASE,
			      CFG_MONITOR_BASE+monitor_flash_len-1,
			      &flash_info[1]);
#endif

#ifdef	CFG_ENV_IS_IN_FLASH
		/* ENV protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CFG_ENV_ADDR,
			      CFG_ENV_ADDR+CFG_ENV_SECT_SIZE-1,
			      &flash_info[1]);
#endif
	} else {
		memctl->memc_br1 = 0;		/* invalidate bank */
		memctl->memc_or1 = 0;		/* invalidate bank */

		debug ("## DISABLE BR1: 0x%08x    OR1: 0x%08x\n",
			memctl->memc_br1, memctl->memc_or1);

		flash_info[1].flash_id = FLASH_UNKNOWN;
		flash_info[1].sector_count = -1;
		flash_info[1].size = 0;
	}

	debug ("## Final Flash bank sizes: %08lx + 0x%08lx\n",size_b0,size_b1);

	return (size_b0 + size_b1);
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets (ulong base, flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_INTEL:
	    for (i = 0; i < info->sector_count; i++) {
		info->start[i] = base;
		base += 0x00020000 * 2;		/* 128k * 2 chips per bank */
	    }
	    return;

	default:
	    printf ("Don't know sector ofsets for flash type 0x%lx\n",
		info->flash_id);
	    return;
	}
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
	case FLASH_MAN_FUJ:	printf ("Fujitsu ");		break;
	case FLASH_MAN_SST:	printf ("SST ");		break;
	case FLASH_MAN_STM:	printf ("STM ");		break;
	case FLASH_MAN_INTEL:	printf ("Intel ");		break;
	case FLASH_MAN_MT:	printf ("MT ");			break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F320J3A:	printf ("28F320J3A (32Mbit = 128K x 32)\n");
				break;
	case FLASH_28F640J3A:	printf ("28F640J3A (64Mbit = 128K x 64)\n");
				break;
	case FLASH_28F128J3A:	printf ("28F128J3A (128Mbit = 128K x 128)\n");
				break;
	default:		printf ("Unknown Chip Type\n");
				break;
	}

	if (info->size >= (1 << 20)) {
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
 */


/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */

static ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
	ulong value;

	/* Read Manufacturer ID */
	addr[0] = 0x00900090;
	value = addr[0];

	debug ("Manuf. ID @ 0x%08lx: 0x%08lx\n", (ulong)addr, value);

	switch (value) {
	case AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
		break;
	case FUJ_MANUFACT:
		info->flash_id = FLASH_MAN_FUJ;
		break;
	case SST_MANUFACT:
		info->flash_id = FLASH_MAN_SST;
		break;
	case STM_MANUFACT:
		info->flash_id = FLASH_MAN_STM;
		break;
	case INTEL_MANUFACT:
		info->flash_id = FLASH_MAN_INTEL;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		addr[0] = 0x00FF00FF;		/* restore read mode */
		return (0);			/* no or unknown flash	*/
	}

	value = addr[1];			/* device ID		*/

	debug ("Device ID @ 0x%08lx: 0x%08lx\n", (ulong)(&addr[1]), value);

	switch (value) {
	case INTEL_ID_28F320J3A:
		info->flash_id += FLASH_28F320J3A;
		info->sector_count = 32;
		info->size = 0x00400000 * 2;
		break;				/* =>  8 MB		*/

	case INTEL_ID_28F640J3A:
		info->flash_id += FLASH_28F640J3A;
		info->sector_count = 64;
		info->size = 0x00800000 * 2;
		break;				/* => 16 MB		*/

	case INTEL_ID_28F128J3A:
		info->flash_id += FLASH_28F128J3A;
		info->sector_count = 128;
		info->size = 0x01000000 * 2;
		break;				/* => 32 MB		*/

	default:
		info->flash_id = FLASH_UNKNOWN;
		addr[0] = 0x00FF00FF;		/* restore read mode */
		return (0);			/* => no or unknown flash */

	}

	if (info->sector_count > CFG_MAX_FLASH_SECT) {
		printf ("** ERROR: sector count %d > max (%d) **\n",
			info->sector_count, CFG_MAX_FLASH_SECT);
		info->sector_count = CFG_MAX_FLASH_SECT;
	}

	addr[0] = 0x00FF00FF;		/* restore read mode */

	return (info->size);
}


/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	int flag, prot, sect;
	ulong start, now, last;

	debug ("flash_erase: first: %d last: %d\n", s_first, s_last);

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
	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			vu_long *addr = (vu_long *)(info->start[sect]);
			unsigned long status;

			/* Disable interrupts which might cause a timeout here */
			flag = disable_interrupts();

			*addr = 0x00600060;	/* clear lock bit setup */
			*addr = 0x00D000D0;	/* clear lock bit confirm */

			udelay (1000);
			/* This takes awfully long - up to 50 ms and more */
			while (((status = *addr) & 0x00800080) != 0x00800080) {
				if ((now=get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
					printf ("Timeout\n");
					*addr = 0x00FF00FF; /* reset to read mode */
					return 1;
				}

				/* show that we're waiting */
				if ((now - last) > 1000) {	/* every second */
					putc ('.');
					last = now;
				}
				udelay (1000);	/* to trigger the watchdog */
			}

			*addr = 0x00500050;	/* clear status register */
			*addr = 0x00200020;	/* erase setup */
			*addr = 0x00D000D0;	/* erase confirm */

			/* re-enable interrupts if necessary */
			if (flag)
				enable_interrupts();

			/* wait at least 80us - let's wait 1 ms */
			udelay (1000);

			while (((status = *addr) & 0x00800080) != 0x00800080) {
				if ((now=get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
					printf ("Timeout\n");
					*addr = 0x00B000B0; /* suspend erase	  */
					*addr = 0x00FF00FF; /* reset to read mode */
					return 1;
				}

				/* show that we're waiting */
				if ((now - last) > 1000) {	/* every second */
					putc ('.');
					last = now;
				}
				udelay (1000);	/* to trigger the watchdog */
			}

			*addr = 0x00FF00FF;	/* reset to read mode */
		}
	}
	printf (" done\n");
	return 0;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 4 - Flash not identified
 */

#define	FLASH_WIDTH	4	/* flash bus width in bytes */

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int i, l, rc;

	if (info->flash_id == FLASH_UNKNOWN) {
		return 4;
	}

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
#ifdef CFG_FLASH_USE_BUFFER_WRITE
	while(cnt >= FLASH_WIDTH) {
		i = CFG_FLASH_BUFFER_SIZE > cnt ?
		    (cnt & ~(FLASH_WIDTH - 1)) : CFG_FLASH_BUFFER_SIZE;
		if((rc = write_data_buf(info, wp, src,i)) != 0)
			return rc;
		wp += i;
		src += i;
		cnt -=i;
	}
#else
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
#endif /* CFG_FLASH_USE_BUFFER_WRITE */

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
 * Check flash status, returns:
 * 0 - OK
 * 1 - timeout
 */
static int flash_status_check(vu_long *addr, ulong tout, char * prompt)
{
	ulong status;
	ulong start;

	/* Wait for command completion */
	start = get_timer (0);
	while(((status = *addr) & 0x00800080) != 0x00800080) {
		if (get_timer(start) > tout) {
			printf("Flash %s timeout at address %p\n", prompt, addr);
			*addr = 0x00FF00FF;	/* restore read mode */
			return (1);
		}
	}
	return 0;
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_data (flash_info_t *info, ulong dest, ulong data)
{
	vu_long *addr = (vu_long *)dest;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*addr & data) != data) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	*addr = 0x00400040;		/* write setup */
	*addr = data;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	if (flash_status_check(addr, CFG_FLASH_WRITE_TOUT, "write") != 0) {
		return (1);
	}

	*addr = 0x00FF00FF;	/* restore read mode */

	return (0);
}

#ifdef CFG_FLASH_USE_BUFFER_WRITE
/*-----------------------------------------------------------------------
 * Write a buffer to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 */
static int write_data_buf(flash_info_t * info, ulong dest, uchar * cp, int len)
{
	vu_long *addr = (vu_long *)dest;
	int sector;
	int cnt;
	int retcode;
	vu_long * src = (vu_long *)cp;
	vu_long * dst = (vu_long *)dest;

	/* find sector */
	for(sector = info->sector_count - 1; sector >= 0; sector--) {
		if(dest >= info->start[sector])
			break;
	}

	*addr = 0x00500050;		/* clear status */
	*addr = 0x00e800e8;		/* write buffer */

	if((retcode = flash_status_check(addr, CFG_FLASH_BUFFER_WRITE_TOUT,
					 "write to buffer")) == 0) {
		cnt = len / FLASH_WIDTH;
		*addr = (cnt-1) | ((cnt-1) << 16);
		while(cnt-- > 0) {
			*dst++ = *src++;
		}
		*addr = 0x00d000d0;		/* write buffer confirm */
		retcode = flash_status_check(addr, CFG_FLASH_BUFFER_WRITE_TOUT,
						 "buffer write");
	}
	*addr = 0x00FF00FF;	/* restore read mode */
	*addr = 0x00500050;	/* clear status */
	return retcode;
}
#endif /* CFG_USE_FLASH_BUFFER_WRITE */

/*-----------------------------------------------------------------------
 */
