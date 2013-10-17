/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc8xx.h>

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

#if defined(CONFIG_ENV_IS_IN_FLASH)
# ifndef  CONFIG_ENV_ADDR
#  define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
# endif
# ifndef  CONFIG_ENV_SIZE
#  define CONFIG_ENV_SIZE	CONFIG_ENV_SECT_SIZE
# endif
# ifndef  CONFIG_ENV_SECT_SIZE
#  define CONFIG_ENV_SECT_SIZE  CONFIG_ENV_SIZE
# endif
#endif

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static int write_data (flash_info_t *info, ulong dest, ulong data);
static void flash_get_offsets (ulong base, flash_info_t *info);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	volatile immap_t     *immap  = (immap_t *)CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	unsigned long size_b0;
	int i;

	/* Init: no FLASHes known */
	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Static FLASH Bank configuration here - FIXME XXX */

	size_b0 = flash_get_size((vu_long *)FLASH_BASE0_PRELIM, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0: "
			"ID 0x%lx, Size = 0x%08lx = %ld MB\n",
			flash_info[0].flash_id,
			size_b0, size_b0<<20);
	}

	/* Remap FLASH according to real size */
	memctl->memc_or0 = CONFIG_SYS_OR_TIMING_FLASH | (-size_b0 & 0xFFFF8000);
	memctl->memc_br0 = (CONFIG_SYS_FLASH_BASE & BR_BA_MSK) | \
				BR_MS_GPCM | BR_PS_16 | BR_V;

	/* Re-do sizing to get full correct info */
	size_b0 = flash_get_size((vu_long *)CONFIG_SYS_FLASH_BASE, &flash_info[0]);

	flash_get_offsets (CONFIG_SYS_FLASH_BASE, &flash_info[0]);

	flash_info[0].size = size_b0;

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
		      CONFIG_ENV_ADDR+CONFIG_ENV_SECT_SIZE-1,
		      &flash_info[0]);
#endif

	return (size_b0);
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
	case FLASH_MAN_MT:
	    if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block type	*/
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00004000;
		info->start[2] = base + 0x00006000;
		info->start[3] = base + 0x00008000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + ((i-3) * 0x00020000);
		}
	    } else {
		/* set sector offsets for top boot block type		*/
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00004000;
		info->start[i--] = base + info->size - 0x00006000;
		info->start[i--] = base + info->size - 0x00008000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00020000;
		}
	    }
	    return;

	case FLASH_MAN_SST:
	    for (i = 0; i < info->sector_count; i++) {
		info->start[i] = base + (i * 0x00002000);
	    }
	    return;

	case FLASH_MAN_AMD:
	case FLASH_MAN_FUJ:

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
	case FLASH_MAN_MT:	printf ("MT ");			break;
	case FLASH_MAN_INTEL:	printf ("Intel ");		break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
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
	case FLASH_SST200A:	printf ("39xF200A (2M = 128K x 16)\n");
				break;
	case FLASH_SST400A:	printf ("39xF400A (4M = 256K x 16)\n");
				break;
	case FLASH_SST800A:	printf ("39xF800A (8M = 512K x 16)\n");
				break;
	case FLASH_STM800AB:	printf ("M29W800AB (8M = 512K x 16)\n");
				break;
	case FLASH_28F008S5:	printf ("28F008S5 (1M = 64K x 16)\n");
				break;
	case FLASH_28F400_T:	printf ("28F400B3 (4Mbit, top boot sector)\n");
				break;
	case FLASH_28F400_B:	printf ("28F400B3 (4Mbit, bottom boot sector)\n");
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
	ushort value;
	vu_short *saddr = (vu_short *)addr;

	/* Read Manufacturer ID */
	saddr[0] = 0x0090;
	value = saddr[0];

	switch (value) {
	case (AMD_MANUFACT & 0xFFFF):
		info->flash_id = FLASH_MAN_AMD;
		break;
	case (FUJ_MANUFACT & 0xFFFF):
		info->flash_id = FLASH_MAN_FUJ;
		break;
	case (SST_MANUFACT & 0xFFFF):
		info->flash_id = FLASH_MAN_SST;
		break;
	case (STM_MANUFACT & 0xFFFF):
		info->flash_id = FLASH_MAN_STM;
		break;
	case (MT_MANUFACT & 0xFFFF):
		info->flash_id = FLASH_MAN_MT;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		saddr[0] = 0x00FF;		/* restore read mode */
		return (0);			/* no or unknown flash	*/
	}

	value = saddr[1];			/* device ID		*/

	switch (value) {
	case (AMD_ID_LV400T & 0xFFFF):
		info->flash_id += FLASH_AM400T;
		info->sector_count = 11;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/

	case (AMD_ID_LV400B & 0xFFFF):
		info->flash_id += FLASH_AM400B;
		info->sector_count = 11;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/

	case (AMD_ID_LV800T & 0xFFFF):
		info->flash_id += FLASH_AM800T;
		info->sector_count = 19;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/

	case (AMD_ID_LV800B & 0xFFFF):
		info->flash_id += FLASH_AM800B;
		info->sector_count = 19;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/

	case (AMD_ID_LV160T & 0xFFFF):
		info->flash_id += FLASH_AM160T;
		info->sector_count = 35;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/

	case (AMD_ID_LV160B & 0xFFFF):
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/
#if 0	/* enable when device IDs are available */
	case (AMD_ID_LV320T & 0xFFFF):
		info->flash_id += FLASH_AM320T;
		info->sector_count = 67;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/

	case (AMD_ID_LV320B & 0xFFFF):
		info->flash_id += FLASH_AM320B;
		info->sector_count = 67;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/
#endif
	case (SST_ID_xF200A & 0xFFFF):
		info->flash_id += FLASH_SST200A;
		info->sector_count = 64;	/* 39xF200A ID ( 2M = 128K x 16	) */
		info->size = 0x00080000;
		break;
	case (SST_ID_xF400A & 0xFFFF):
		info->flash_id += FLASH_SST400A;
		info->sector_count = 128;	/* 39xF400A ID ( 4M = 256K x 16	) */
		info->size = 0x00100000;
		break;
	case (SST_ID_xF800A & 0xFFFF):
		info->flash_id += FLASH_SST800A;
		info->sector_count = 256;	/* 39xF800A ID ( 8M = 512K x 16	) */
		info->size = 0x00200000;
		break;				/* => 2 MB		*/
	case (STM_ID_x800AB & 0xFFFF):
		info->flash_id += FLASH_STM800AB;
		info->sector_count = 19;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/
	case (MT_ID_28F400_T & 0xFFFF):
		info->flash_id += FLASH_28F400_T;
		info->sector_count = 7;
		info->size = 0x00080000;
		break;				/* => 512 kB		*/
	case (MT_ID_28F400_B & 0xFFFF):
		info->flash_id += FLASH_28F400_B;
		info->sector_count = 7;
		info->size = 0x00080000;
		break;				/* => 512 kB		*/
	default:
		info->flash_id = FLASH_UNKNOWN;
		saddr[0] = 0x00FF;		/* restore read mode */
		return (0);			/* => no or unknown flash */

	}

	if (info->sector_count > CONFIG_SYS_MAX_FLASH_SECT) {
		printf ("** ERROR: sector count %d > max (%d) **\n",
			info->sector_count, CONFIG_SYS_MAX_FLASH_SECT);
		info->sector_count = CONFIG_SYS_MAX_FLASH_SECT;
	}

	saddr[0] = 0x00FF;		/* restore read mode */

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

	if ((info->flash_id & FLASH_VENDMASK) != FLASH_MAN_MT) {
		printf ("Can erase only MT flash types - aborted\n");
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
			vu_short *addr = (vu_short *)(info->start[sect]);
			unsigned short status;

			/* Disable interrupts which might cause a timeout here */
			flag = disable_interrupts();

			*addr = 0x0050;	/* clear status register */
			*addr = 0x0020;	/* erase setup */
			*addr = 0x00D0;	/* erase confirm */

			/* re-enable interrupts if necessary */
			if (flag)
				enable_interrupts();

			/* wait at least 80us - let's wait 1 ms */
			udelay (1000);

			while (((status = *addr) & 0x0080) != 0x0080) {
				if ((now=get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
					printf ("Timeout\n");
					*addr = 0x00FF;	/* reset to read mode */
					return 1;
				}

				/* show that we're waiting */
				if ((now - last) > 1000) {	/* every second */
					putc ('.');
					last = now;
				}
			}

			*addr = 0x00FF;	/* reset to read mode */
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

#define	FLASH_WIDTH	2	/* flash bus width in bytes */

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
	vu_short *addr = (vu_short *)dest;
	ushort sdata = (ushort)data;
	ushort status;
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*addr & sdata) != sdata) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	*addr = 0x0040;		/* write setup */
	*addr = sdata;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	start = get_timer (0);

	while (((status = *addr) & 0x0080) != 0x0080) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			*addr = 0x00FF;	/* restore read mode */
			return (1);
		}
	}

	*addr = 0x00FF;		/* restore read mode */

	return (0);
}

/*-----------------------------------------------------------------------
 */
