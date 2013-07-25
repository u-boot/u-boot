/*
 * (C) Copyright 2003
 * MuLogic B.V.
 *
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/ppc4xx.h>
#include <asm/u-boot.h>
#include <asm/processor.h>

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips */


#define FLASH_WORD_SIZE unsigned long
#define FLASH_ID_MASK 0xFFFFFFFF

/*-----------------------------------------------------------------------
 * Functions
 */
/* stolen from esteem192e/flash.c */
ulong flash_get_size (volatile FLASH_WORD_SIZE *addr, flash_info_t *info);

static int write_word (flash_info_t *info, ulong dest, ulong data);
static void flash_get_offsets (ulong base, flash_info_t *info);


/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	unsigned long size_b0, size_b1;
	int i;
	uint pbcr;
	unsigned long base_b0, base_b1;
	volatile FLASH_WORD_SIZE* flash_base;

	/* Init: no FLASHes known */
	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Static FLASH Bank configuration here */
	/* Test for 8M Flash first */
	debug ("\n## Get flash bank 1 size @ 0x%08x\n",FLASH_BASE0_8M_PRELIM);
	flash_base = (volatile FLASH_WORD_SIZE*)(FLASH_BASE0_8M_PRELIM);
	size_b0 = flash_get_size(flash_base, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
		size_b0, size_b0<<20);
		return 0;
	}

	if (size_b0 < 8*1024*1024) {
		/* Not quite 8M, try 4M Flash base address */
		debug ("\n## Get flash bank 1 size @ 0x%08x\n",FLASH_BASE0_4M_PRELIM);
		flash_base = (volatile FLASH_WORD_SIZE*)(FLASH_BASE0_4M_PRELIM);
		size_b0 = flash_get_size(flash_base, &flash_info[0]);
	}

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
		size_b0, size_b0<<20);
		return 0;
	}

	/* Only one bank */
	if (CONFIG_SYS_MAX_FLASH_BANKS == 1) {
		/* Setup offsets */
		flash_get_offsets ((ulong)flash_base, &flash_info[0]);

		/* Monitor protection ON by default */
		(void)flash_protect(FLAG_PROTECT_SET, CONFIG_SYS_MONITOR_BASE,
			CONFIG_SYS_MONITOR_BASE+CONFIG_SYS_MONITOR_LEN-1, &flash_info[0]);
		size_b1 = 0 ;
		flash_info[0].size = size_b0;
		return(size_b0);
	}

	/* We have 2 banks */
	size_b1 = flash_get_size(flash_base, &flash_info[1]);

	/* Re-do sizing to get full correct info */
	if (size_b1) {
		mtdcr(EBC0_CFGADDR, PB0CR);
		pbcr = mfdcr(EBC0_CFGDATA);
		mtdcr(EBC0_CFGADDR, PB0CR);
		base_b1 = -size_b1;
		pbcr = (pbcr & 0x0001ffff) | base_b1 | (((size_b1/1024/1024)-1)<<17);
		mtdcr(EBC0_CFGDATA, pbcr);
	}

	if (size_b0) {
		mtdcr(EBC0_CFGADDR, PB1CR);
		pbcr = mfdcr(EBC0_CFGDATA);
		mtdcr(EBC0_CFGADDR, PB1CR);
		base_b0 = base_b1 - size_b0;
		pbcr = (pbcr & 0x0001ffff) | base_b0 | (((size_b0/1024/1024)-1)<<17);
		mtdcr(EBC0_CFGDATA, pbcr);
	}

	size_b0 = flash_get_size((volatile FLASH_WORD_SIZE *)base_b0, &flash_info[0]);
	flash_get_offsets (base_b0, &flash_info[0]);

	/* monitor protection ON by default */
	(void)flash_protect(FLAG_PROTECT_SET, CONFIG_SYS_MONITOR_BASE,
		CONFIG_SYS_MONITOR_BASE+CONFIG_SYS_MONITOR_LEN-1, &flash_info[0]);

	if (size_b1) {
		/* Re-do sizing to get full correct info */
		size_b1 = flash_get_size((volatile FLASH_WORD_SIZE *)base_b1, &flash_info[1]);
		flash_get_offsets (base_b1, &flash_info[1]);

		/* monitor protection ON by default */
		(void)flash_protect(FLAG_PROTECT_SET, base_b1+size_b1-CONFIG_SYS_MONITOR_LEN,
			base_b1+size_b1-1, &flash_info[1]);

		/* monitor protection OFF by default (one is enough) */
		(void)flash_protect(FLAG_PROTECT_CLEAR, base_b0+size_b0-CONFIG_SYS_MONITOR_LEN,
			base_b0+size_b0-1, &flash_info[0]);
	} else {
		flash_info[1].flash_id = FLASH_UNKNOWN;
		flash_info[1].sector_count = -1;
	}

	flash_info[0].size = size_b0;
	flash_info[1].size = size_b1;
	return (size_b0 + size_b1);
}


/*-----------------------------------------------------------------------
 This code is specific to the AM29DL163/AM29DL232 for the QS850/QS823.
 */

static void flash_get_offsets (ulong base, flash_info_t *info)
{
	int i;
	long large_sect_size;
	long small_sect_size;

	/* set up sector start adress table */
	large_sect_size = info->size / (info->sector_count - 8 + 1);
	small_sect_size = large_sect_size / 8;

	if (info->flash_id & FLASH_BTYPE) {

		/* set sector offsets for bottom boot block type */
		for (i = 0; i < 7; i++) {
			info->start[i] = base;
			base += small_sect_size;
		}

		for (; i < info->sector_count; i++) {
			info->start[i] = base;
			base += large_sect_size;
		}
	}
	else
	{
		/* set sector offsets for top boot block type */
		for (i = 0; i < (info->sector_count - 8); i++) {
			info->start[i] = base;
			base += large_sect_size;
		}

		for (; i < info->sector_count; i++) {
			info->start[i] = base;
			base += small_sect_size;
		}
	}
}

/*-----------------------------------------------------------------------
 */

void flash_print_info  (flash_info_t *info)
{
	int i;
	uchar *boottype;
	uchar botboot[]=", bottom boot sect)\n";
	uchar topboot[]=", top boot sector)\n";

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
		case FLASH_MAN_SST:
			printf ("SST ");
			break;
		case FLASH_MAN_STM:
			printf ("STM ");
			break;
		case FLASH_MAN_INTEL:
			printf ("INTEL ");
			break;
		default:
			printf ("Unknown Vendor ");
			break;
	}

	if (info->flash_id & 0x0001 ) {
		boottype = botboot;
	} else {
		boottype = topboot;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
		case FLASH_AM160B:
			printf ("AM29LV160B (16 Mbit%s",boottype);
			break;
		case FLASH_AM160T:
			printf ("AM29LV160T (16 Mbit%s",boottype);
			break;
		case FLASH_AMDL163T:
			printf ("AM29DL163T (16 Mbit%s",boottype);
			break;
		case FLASH_AMDL163B:
			printf ("AM29DL163B (16 Mbit%s",boottype);
			break;
		case FLASH_AM320B:
			printf ("AM29LV320B (32 Mbit%s",boottype);
			break;
		case FLASH_AM320T:
			printf ("AM29LV320T (32 Mbit%s",boottype);
			break;
		case FLASH_AMDL323T:
			printf ("AM29DL323T (32 Mbit%s",boottype);
			break;
		case FLASH_AMDL323B:
			printf ("AM29DL323B (32 Mbit%s",boottype);
			break;
		case FLASH_AMDL322T:
			printf ("AM29DL322T (32 Mbit%s",boottype);
			break;
		default:
			printf ("Unknown Chip Type\n");
			break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
	info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s", info->start[i],
			info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");
	return;
}


/*-----------------------------------------------------------------------
 * The following code cannot be run from FLASH!
 */
ulong flash_get_size (volatile FLASH_WORD_SIZE *addr, flash_info_t *info)
{
	short i;
	ulong base = (ulong)addr;
	FLASH_WORD_SIZE value;

	/* Write auto select command: read Manufacturer ID */

	/*
	 * Note: if it is an AMD flash and the word at addr[0000]
	 * is 0x00890089 this routine will think it is an Intel
	 * flash device and may(most likely) cause trouble.
	 */

	addr[0x0000] = 0x00900090;
	if(addr[0x0000] != 0x00890089){
		addr[0x0555] = 0x00AA00AA;
		addr[0x02AA] = 0x00550055;
		addr[0x0555] = 0x00900090;
	}
	value = addr[0];

	switch (value) {
		case (AMD_MANUFACT & FLASH_ID_MASK):
			info->flash_id = FLASH_MAN_AMD;
			break;
		case (FUJ_MANUFACT & FLASH_ID_MASK):
			info->flash_id = FLASH_MAN_FUJ;
			break;
		case (STM_MANUFACT & FLASH_ID_MASK):
			info->flash_id = FLASH_MAN_STM;
			break;
		case (SST_MANUFACT & FLASH_ID_MASK):
			info->flash_id = FLASH_MAN_SST;
			break;
		case (INTEL_MANUFACT & FLASH_ID_MASK):
			info->flash_id = FLASH_MAN_INTEL;
			break;
		default:
			info->flash_id = FLASH_UNKNOWN;
			info->sector_count = 0;
			info->size = 0;
			return (0); /* no or unknown flash */
	}

	value = addr[1]; /* device ID */

	switch (value) {
		case (AMD_ID_LV160T & FLASH_ID_MASK):
			info->flash_id += FLASH_AM160T;
			info->sector_count = 35;
			info->size = 0x00400000;
			break; /* => 4 MB */

		case (AMD_ID_LV160B & FLASH_ID_MASK):
			info->flash_id += FLASH_AM160B;
			info->sector_count = 35;
			info->size = 0x00400000;
			break; /* => 4 MB */

		case (AMD_ID_DL163T & FLASH_ID_MASK):
			info->flash_id += FLASH_AMDL163T;
			info->sector_count = 39;
			info->size = 0x00400000;
			break; /* => 4 MB */

		case (AMD_ID_DL163B & FLASH_ID_MASK):
			info->flash_id += FLASH_AMDL163B;
			info->sector_count = 39;
			info->size = 0x00400000;
			break; /* => 4 MB */

		case (AMD_ID_DL323T & FLASH_ID_MASK):
			info->flash_id += FLASH_AMDL323T;
			info->sector_count = 71;
			info->size = 0x00800000;
			break; /* => 8 MB */

		case (AMD_ID_DL323B & FLASH_ID_MASK):
			info->flash_id += FLASH_AMDL323B;
			info->sector_count = 71;
			info->size = 0x00800000;
			break; /* => 8 MB */

		case (AMD_ID_DL322T & FLASH_ID_MASK):
			info->flash_id += FLASH_AMDL322T;
			info->sector_count = 71;
			info->size = 0x00800000;
			break; /* => 8 MB */

		default:
			/* FIXME*/
			info->flash_id = FLASH_UNKNOWN;
			return (0); /* => no or unknown flash */
	}

	flash_get_offsets(base, info);

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr = (volatile FLASH_WORD_SIZE *)(info->start[i]);
		info->protect[i] = addr[2] & 1;
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr = (volatile FLASH_WORD_SIZE *)info->start[0];
		*addr = (0x00FF00FF & FLASH_ID_MASK);	/* reset bank */
	}

	return (info->size);
}


/*-----------------------------------------------------------------------
 */

int flash_erase (flash_info_t *info, int s_first, int s_last)
{
	volatile FLASH_WORD_SIZE *addr=(volatile FLASH_WORD_SIZE*)(info->start[0]);
	int flag, prot, sect, l_sect;
	ulong start, now, last;
	int rcode = 0;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	if ((info->flash_id == FLASH_UNKNOWN) ||
		(info->flash_id > FLASH_AMD_COMP) ) {
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
	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;
	addr[0x0555] = 0x00800080;
	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;
	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) { /* not protected */
			addr = (volatile FLASH_WORD_SIZE *)(info->start[sect]);
			addr[0] = (0x00300030 & FLASH_ID_MASK);
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
	addr = (volatile FLASH_WORD_SIZE*)(info->start[l_sect]);
	while ((addr[0] & (0x00800080&FLASH_ID_MASK)) !=
			(0x00800080&FLASH_ID_MASK)  )
	{
		if ((now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
			printf ("Timeout\n");
			return 1;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) { /* every second */
			serial_putc ('.');
			last = now;
		}
	}

DONE:
	/* reset to read mode */
	addr = (volatile FLASH_WORD_SIZE *)info->start[0];
	addr[0] = (0x00F000F0 & FLASH_ID_MASK); /* reset bank */

	printf (" done\n");
	return rcode;
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
	int l;
	int i, rc;

	wp = (addr & ~3); /* get lower word aligned address */

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

	/* AMD stuff */
	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;
	addr[0x0555] = 0x00A000A0;

	*((vu_long *)dest) = data;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer(0);

	while ((*((vu_long *)dest) & 0x00800080) != (data & 0x00800080)) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			return (1);
		}
	}

	return (0);
}
