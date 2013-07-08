/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <configs/sacsng.h>


#undef  DEBUG

#ifndef	CONFIG_ENV_ADDR
#define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
#endif
#ifndef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE	CONFIG_ENV_SECT_SIZE
#endif


flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_short *addr, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	unsigned long size_b0, size_b1;
	int i;

	/* Init: no FLASHes known */
	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	size_b0 = flash_get_size((vu_short *)CONFIG_SYS_FLASH0_BASE, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
			size_b0, size_b0<<20);
	}

	size_b1 = flash_get_size((vu_short *)CONFIG_SYS_FLASH1_BASE, &flash_info[1]);

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

	if (size_b1) {
#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
		/* monitor protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CONFIG_SYS_MONITOR_BASE,
			      CONFIG_SYS_MONITOR_BASE+monitor_flash_len-1,
			      &flash_info[1]);
#endif

#ifdef	CONFIG_ENV_IS_IN_FLASH
		/* ENV protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CONFIG_ENV_ADDR,
			      CONFIG_ENV_ADDR+CONFIG_ENV_SIZE-1,
			      &flash_info[1]);
#endif
	} else {
		flash_info[1].flash_id = FLASH_UNKNOWN;
		flash_info[1].sector_count = -1;
	}

	flash_info[0].size = size_b0;
	flash_info[1].size = size_b1;

	/*
	 * We only report the primary flash for U-Boot's use.
	 */
	return (size_b0);
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

static ulong flash_get_size (vu_short *addr, flash_info_t *info)
{
	short i;
	ushort value;
	ulong  base = (ulong)addr;

	/* Write auto select command: read Manufacturer ID */
	addr[0x0555] = 0xAAAA;
	addr[0x02AA] = 0x5555;
	addr[0x0555] = 0x9090;
	__asm__ __volatile__(" sync\n ");

	value = addr[0];
#ifdef DEBUG
	printf("Flash manufacturer 0x%04X\n", value);
#endif

	if(value == (ushort)AMD_MANUFACT) {
		info->flash_id = FLASH_MAN_AMD;
	} else if (value == (ushort)FUJ_MANUFACT) {
		info->flash_id = FLASH_MAN_FUJ;
	} else {
#ifdef DEBUG
		printf("Unknown flash manufacturer 0x%04X\n", value);
#endif
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);			/* no or unknown flash	*/
	}

	value = addr[1];			/* device ID		*/
#ifdef DEBUG
	printf("Flash type 0x%04X\n", value);
#endif

	if(value == (ushort)AMD_ID_LV400T) {
		info->flash_id += FLASH_AM400T;
		info->sector_count = 11;
		info->size = 0x00080000;	/* => 0.5 MB		*/
	} else if(value == (ushort)AMD_ID_LV400B) {
		info->flash_id += FLASH_AM400B;
		info->sector_count = 11;
		info->size = 0x00080000;	/* => 0.5 MB		*/
	} else if(value == (ushort)AMD_ID_LV800T) {
		info->flash_id += FLASH_AM800T;
		info->sector_count = 19;
		info->size = 0x00100000;	/* => 1 MB		*/
	} else if(value == (ushort)AMD_ID_LV800B) {
		info->flash_id += FLASH_AM800B;
		info->sector_count = 19;
		info->size = 0x00100000;	/* => 1 MB		*/
	} else if(value == (ushort)AMD_ID_LV160T) {
		info->flash_id += FLASH_AM160T;
		info->sector_count = 35;
		info->size = 0x00200000;	/* => 2 MB		*/
	} else if(value == (ushort)AMD_ID_LV160B) {
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00200000;	/* => 2 MB		*/
	} else if(value == (ushort)AMD_ID_LV320T) {
		info->flash_id += FLASH_AM320T;
		info->sector_count = 67;
		info->size = 0x00400000;	/* => 4 MB		*/
	} else if(value == (ushort)AMD_ID_LV320B) {
		info->flash_id += FLASH_AM320B;
		info->sector_count = 67;
		info->size = 0x00400000;	/* => 4 MB		*/
	} else {
#ifdef DEBUG
		printf("Unknown flash type 0x%04X\n", value);
		info->size = CONFIG_SYS_FLASH_SIZE;
#else
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */
#endif
	}

	/* set up sector start address table */
	if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block type	*/
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00004000;
		info->start[2] = base + 0x00006000;
		info->start[3] = base + 0x00008000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + ((i - 3) * 0x00010000);
		}
	} else {
		/* set sector offsets for top boot block type		*/
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00004000;
		info->start[i--] = base + info->size - 0x00006000;
		info->start[i--] = base + info->size - 0x00008000;
		for (; i >= 0; i--) {
			info->start[i] = base + (i * 0x00010000);
		}
	}

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr = (volatile unsigned short *)(info->start[i]);
		info->protect[i] = addr[2] & 1;
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr = (volatile unsigned short *)info->start[0];

	}

	addr[0] = 0xF0F0;	/* reset bank */
	__asm__ __volatile__(" sync\n ");
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

	addr[0x0555] = 0xAAAA;
	addr[0x02AA] = 0x5555;
	addr[0x0555] = 0x8080;
	addr[0x0555] = 0xAAAA;
	addr[0x02AA] = 0x5555;
	__asm__ __volatile__(" sync\n ");

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = (vu_short*)(info->start[sect]);
			addr[0] = 0x3030;
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
		if ((now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
			printf ("Timeout\n");
			addr[0] = 0xF0F0;	/* reset bank */
			__asm__ __volatile__(" sync\n ");
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
	addr = (vu_short*)info->start[0];
	addr[0] = 0xF0F0;	/* reset bank */
	__asm__ __volatile__(" sync\n ");

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
	vu_short *addr = (vu_short*)(info->start[0]);
	ulong start;
	int flag;
	int j;

	/* Check if Flash is (sufficiently) erased */
	if (((*(vu_long *)dest) & data) != data) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	/* The original routine was designed to write 32 bit words to
	 * 32 bit wide memory.	We have 16 bit wide memory so we do
	 * two writes.	We write the LSB first at dest+2 and then the
	 * MSB at dest (lousy big endian).
	 */
	dest += 2;
	for(j = 0; j < 2; j++) {
		addr[0x0555] = 0xAAAA;
		addr[0x02AA] = 0x5555;
		addr[0x0555] = 0xA0A0;
		__asm__ __volatile__(" sync\n ");

		*((vu_short *)dest) = (ushort)data;

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts();

		/* data polling for D7 */
		start = get_timer (0);
		while (*(vu_short *)dest != (ushort)data) {
			if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
				return (1);
			}
		}
		dest -= 2;
		data >>= 16;
	}
	return (0);
}

/*-----------------------------------------------------------------------
 */
