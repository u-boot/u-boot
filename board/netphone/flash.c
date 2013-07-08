/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc8xx.h>

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];	/* info for FLASH chips    */

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size(vu_long * addr, flash_info_t * info);
static int write_byte(flash_info_t * info, ulong dest, uchar data);
static void flash_get_offsets(ulong base, flash_info_t * info);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init(void)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	unsigned long size;
#if CONFIG_NETPHONE_VERSION == 2
	unsigned long size1;
#endif
	int i;

	/* Init: no FLASHes known */
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i)
		flash_info[i].flash_id = FLASH_UNKNOWN;

	size = flash_get_size((vu_long *) FLASH_BASE0_PRELIM, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
			size, size << 20);
	}

	/* Remap FLASH according to real size */
	memctl->memc_or0 = CONFIG_SYS_OR_TIMING_FLASH | (-size & 0xFFFF8000);
	memctl->memc_br0 = (CONFIG_SYS_FLASH_BASE & BR_BA_MSK) | (memctl->memc_br0 & ~(BR_BA_MSK));

	/* Re-do sizing to get full correct info */
	size = flash_get_size((vu_long *) CONFIG_SYS_FLASH_BASE, &flash_info[0]);

	flash_get_offsets(CONFIG_SYS_FLASH_BASE, &flash_info[0]);

	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
			CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_BASE + monitor_flash_len - 1,
			&flash_info[0]);

	flash_protect ( FLAG_PROTECT_SET,
			CONFIG_ENV_ADDR,
			CONFIG_ENV_ADDR + CONFIG_ENV_SECT_SIZE - 1,
			&flash_info[0]);

#ifdef CONFIG_ENV_ADDR_REDUND
	flash_protect ( FLAG_PROTECT_SET,
			CONFIG_ENV_ADDR_REDUND,
			CONFIG_ENV_ADDR_REDUND + CONFIG_ENV_SECT_SIZE - 1,
			&flash_info[0]);
#endif

	flash_info[0].size = size;

#if CONFIG_NETPHONE_VERSION == 2
	size1 = flash_get_size((vu_long *) FLASH_BASE4_PRELIM, &flash_info[1]);
	if (size1 > 0) {
		if (flash_info[1].flash_id == FLASH_UNKNOWN)
			printf("## Unknown FLASH on Bank 1 - Size = 0x%08lx = %ld MB\n", size1, size1 << 20);

		/* Remap FLASH according to real size */
		memctl->memc_or4 = CONFIG_SYS_OR_TIMING_FLASH | (-size1 & 0xFFFF8000);
		memctl->memc_br4 = (CONFIG_SYS_FLASH_BASE4 & BR_BA_MSK) | (memctl->memc_br4 & ~(BR_BA_MSK));

		/* Re-do sizing to get full correct info */
		size1 = flash_get_size((vu_long *) CONFIG_SYS_FLASH_BASE4, &flash_info[1]);

		flash_get_offsets(CONFIG_SYS_FLASH_BASE4, &flash_info[1]);

		size += size1;
	} else
		memctl->memc_br4 &= ~BR_V;
#endif

	return (size);
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets(ulong base, flash_info_t * info)
{
	int i;

	/* set up sector start address table */
	if ((info->flash_id & FLASH_TYPEMASK) == FLASH_AM040) {
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00010000);
		}
	} else if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block type    */
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00004000;
		info->start[2] = base + 0x00006000;
		info->start[3] = base + 0x00008000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00010000) - 0x00030000;
		}
	} else {
		/* set sector offsets for top boot block type       */
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00004000;
		info->start[i--] = base + info->size - 0x00006000;
		info->start[i--] = base + info->size - 0x00008000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00010000;
		}
	}
}

/*-----------------------------------------------------------------------
 */
void flash_print_info(flash_info_t * info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:
		printf("AMD ");
		break;
	case FLASH_MAN_FUJ:
		printf("FUJITSU ");
		break;
	case FLASH_MAN_MX:
		printf("MXIC ");
		break;
	default:
		printf("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM040:
		printf("AM29LV040B (4 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM400B:
		printf("AM29LV400B (4 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM400T:
		printf("AM29LV400T (4 Mbit, top boot sector)\n");
		break;
	case FLASH_AM800B:
		printf("AM29LV800B (8 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM800T:
		printf("AM29LV800T (8 Mbit, top boot sector)\n");
		break;
	case FLASH_AM160B:
		printf("AM29LV160B (16 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM160T:
		printf("AM29LV160T (16 Mbit, top boot sector)\n");
		break;
	case FLASH_AM320B:
		printf("AM29LV320B (32 Mbit, bottom boot sect)\n");
		break;
	case FLASH_AM320T:
		printf("AM29LV320T (32 Mbit, top boot sector)\n");
		break;
	default:
		printf("Unknown Chip Type\n");
		break;
	}

	printf("  Size: %ld MB in %d Sectors\n", info->size >> 20, info->sector_count);

	printf("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf("\n   ");
		printf(" %08lX%s", info->start[i], info->protect[i] ? " (RO)" : "     ");
	}
	printf("\n");
}

/*-----------------------------------------------------------------------
 */


/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */

static ulong flash_get_size(vu_long * addr, flash_info_t * info)
{
	short i;
	uchar mid;
	uchar pid;
	vu_char *caddr = (vu_char *) addr;
	ulong base = (ulong) addr;

	/* Write auto select command: read Manufacturer ID */
	caddr[0x0555] = 0xAA;
	caddr[0x02AA] = 0x55;
	caddr[0x0555] = 0x90;

	mid = caddr[0];
	switch (mid) {
	case (AMD_MANUFACT & 0xFF):
		info->flash_id = FLASH_MAN_AMD;
		break;
	case (FUJ_MANUFACT & 0xFF):
		info->flash_id = FLASH_MAN_FUJ;
		break;
	case (MX_MANUFACT & 0xFF):
		info->flash_id = FLASH_MAN_MX;
		break;
	case (STM_MANUFACT & 0xFF):
		info->flash_id = FLASH_MAN_STM;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);				/* no or unknown flash  */
	}

	pid = caddr[1];				/* device ID        */
	switch (pid) {
	case (AMD_ID_LV400T & 0xFF):
		info->flash_id += FLASH_AM400T;
		info->sector_count = 11;
		info->size = 0x00080000;
		break;					/* => 512 kB        */

	case (AMD_ID_LV400B & 0xFF):
		info->flash_id += FLASH_AM400B;
		info->sector_count = 11;
		info->size = 0x00080000;
		break;					/* => 512 kB        */

	case (AMD_ID_LV800T & 0xFF):
		info->flash_id += FLASH_AM800T;
		info->sector_count = 19;
		info->size = 0x00100000;
		break;					/* => 1 MB      */

	case (AMD_ID_LV800B & 0xFF):
		info->flash_id += FLASH_AM800B;
		info->sector_count = 19;
		info->size = 0x00100000;
		break;					/* => 1 MB      */

	case (AMD_ID_LV160T & 0xFF):
		info->flash_id += FLASH_AM160T;
		info->sector_count = 35;
		info->size = 0x00200000;
		break;					/* => 2 MB      */

	case (AMD_ID_LV160B & 0xFF):
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00200000;
		break;					/* => 2 MB      */

	case (AMD_ID_LV040B & 0xFF):
		info->flash_id += FLASH_AM040;
		info->sector_count = 8;
		info->size = 0x00080000;
		break;

	case (STM_ID_M29W040B & 0xFF):
		info->flash_id += FLASH_AM040;
		info->sector_count = 8;
		info->size = 0x00080000;
		break;

#if 0							/* enable when device IDs are available */
	case (AMD_ID_LV320T & 0xFF):
		info->flash_id += FLASH_AM320T;
		info->sector_count = 67;
		info->size = 0x00400000;
		break;					/* => 4 MB      */

	case (AMD_ID_LV320B & 0xFF):
		info->flash_id += FLASH_AM320B;
		info->sector_count = 67;
		info->size = 0x00400000;
		break;					/* => 4 MB      */
#endif
	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);				/* => no or unknown flash */

	}

	printf(" ");
	/* set up sector start address table */
	if ((info->flash_id & FLASH_TYPEMASK) == FLASH_AM040) {
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00010000);
		}
	} else if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block type    */
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00004000;
		info->start[2] = base + 0x00006000;
		info->start[3] = base + 0x00008000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00010000) - 0x00030000;
		}
	} else {
		/* set sector offsets for top boot block type       */
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00004000;
		info->start[i--] = base + info->size - 0x00006000;
		info->start[i--] = base + info->size - 0x00008000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00010000;
		}
	}

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection: D0 = 1 if protected */
		caddr = (volatile unsigned char *)(info->start[i]);
		info->protect[i] = caddr[2] & 1;
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		caddr = (vu_char *) info->start[0];

		caddr[0x0555] = 0xAA;
		caddr[0x02AA] = 0x55;
		caddr[0x0555] = 0xF0;

		udelay(20000);
	}

	return (info->size);
}


/*-----------------------------------------------------------------------
 */

int flash_erase(flash_info_t * info, int s_first, int s_last)
{
	vu_char *addr = (vu_char *) (info->start[0]);
	int flag, prot, sect, l_sect;
	ulong start, now, last;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf("- missing\n");
		} else {
			printf("- no sectors to erase\n");
		}
		return 1;
	}

	if ((info->flash_id == FLASH_UNKNOWN) ||
	    (info->flash_id > FLASH_AMD_COMP)) {
		printf("Can't erase unknown flash type %08lx - aborted\n", info->flash_id);
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf("- Warning: %d protected sectors will not be erased!\n", prot);
	} else {
		printf("\n");
	}

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	addr[0x0555] = 0xAA;
	addr[0x02AA] = 0x55;
	addr[0x0555] = 0x80;
	addr[0x0555] = 0xAA;
	addr[0x02AA] = 0x55;

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			addr = (vu_char *) (info->start[sect]);
			addr[0] = 0x30;
			l_sect = sect;
		}
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* wait at least 80us - let's wait 1 ms */
	udelay(1000);

	/*
	 * We wait for the last triggered sector
	 */
	if (l_sect < 0)
		goto DONE;

	start = get_timer(0);
	last = start;
	addr = (vu_char *) (info->start[l_sect]);
	while ((addr[0] & 0x80) != 0x80) {
		if ((now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
			printf("Timeout\n");
			return 1;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) {	/* every second */
			putc('.');
			last = now;
		}
	}

DONE:
	/* reset to read mode */
	addr = (vu_char *) info->start[0];
	addr[0] = 0xF0;				/* reset bank */

	printf(" done\n");
	return 0;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

int write_buff(flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	int rc;

	while (cnt > 0) {
		if ((rc = write_byte(info, addr++, *src++)) != 0) {
			return (rc);
		}
		--cnt;
	}

	return (0);
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_byte(flash_info_t * info, ulong dest, uchar data)
{
	vu_char *addr = (vu_char *) (info->start[0]);
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*((vu_char *) dest) & data) != data) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	addr[0x0555] = 0xAA;
	addr[0x02AA] = 0x55;
	addr[0x0555] = 0xA0;

	*((vu_char *) dest) = data;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer(0);
	while ((*((vu_char *) dest) & 0x80) != (data & 0x80)) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			return (1);
		}
	}
	return (0);
}
