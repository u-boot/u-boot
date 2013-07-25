/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * (C) Copyright 2001-2003
 *
 * Changes for MATRIX Vision mvBLUE devices
 * MATRIX Vision GmbH / hg,as info@matrix-vision.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc824x.h>

#if 0
	#define mvdebug(p) printf ##p
#else
	#define mvdebug(p)
#endif

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

#define FLASH_BUS_WIDTH		8

#if (FLASH_BUS_WIDTH==32)
	#define FLASH_DATA_MASK 0xffffffff
	#define FLASH_SHIFT 1
	#define FDT	vu_long
#elif (FLASH_BUS_WIDTH==16)
	#define FLASH_DATA_MASK 0xff
	#define FLASH_SHIFT 0
	#define FDT	vu_short
#elif (FLASH_BUS_WIDTH==8)
	#define FLASH_DATA_MASK 0xff
	#define FLASH_SHIFT 0
	#define FDT	vu_char
#else
	#error FLASH_BUS_WIDTH undefined
#endif

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_long *address, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);
static void flash_get_offsets (ulong base, flash_info_t *info);

/*-----------------------------------------------------------------------
 */
unsigned long flash_init (void)
{
	unsigned long size_b0;
	int i;

	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	size_b0 = flash_get_size((vu_long *)0xffc00000, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH : Size = 0x%08lx = %ld MB\n",
			size_b0, size_b0<<20);
	}

	flash_get_offsets (0xffc00000, &flash_info[0]);
	flash_info[0].size = size_b0;

	/* monitor protection OFF by default */
	flash_protect ( FLAG_PROTECT_CLEAR, 0xffc00000, 0x2000, flash_info );

	return size_b0;
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets (ulong base, flash_info_t *info)
{
	int i;

	/* set up sector start address table */
	if (info->flash_id & FLASH_BTYPE)
	{	/* bottom boot sector types - these are the useful ones! */
		/* set sector offsets for bottom boot block type */
		if ((info->flash_id & FLASH_TYPEMASK) == FLASH_AM320B)
		{	/* AMDLV320B has 8 x 8k bottom boot sectors */
			for (i = 0; i < 8; i++)												/* +8k		*/
				info->start[i] = base + (i * (0x00002000 << FLASH_SHIFT));
			for (; i < info->sector_count; i++)									/* +64k		*/
				info->start[i] = base + (i * (0x00010000 << FLASH_SHIFT)) - (0x00070000 << FLASH_SHIFT);
		}
		else
		{	/* other types have 4 bottom boot sectors (16,8,8,32) */
			i = 0;
			info->start[i++] = base +  0x00000000;								/* -		*/
			info->start[i++] = base + (0x00004000 << FLASH_SHIFT);				/* +16k		*/
			info->start[i++] = base + (0x00006000 << FLASH_SHIFT);				/* +8k		*/
			info->start[i++] = base + (0x00008000 << FLASH_SHIFT);				/* +8k		*/
			info->start[i++] = base + (0x00010000 << FLASH_SHIFT);				/* +32k		*/
			for (; i < info->sector_count; i++)									/* +64k		*/
				info->start[i] = base + (i * (0x00010000 << FLASH_SHIFT)) - (0x00030000 << FLASH_SHIFT);
		}
	}
	else
	{	/* top boot sector types - not so useful */
		/* set sector offsets for top boot block type */
		if ((info->flash_id & FLASH_TYPEMASK) == FLASH_AM320T)
		{	/* AMDLV320T has 8 x 8k top boot sectors */
			for (i = 0; i < info->sector_count - 8; i++)						/* +64k		*/
				info->start[i] = base + (i * (0x00010000 << FLASH_SHIFT));
			for (; i < info->sector_count; i++)									/* +8k		*/
				info->start[i] = base + (i * (0x00002000 << FLASH_SHIFT));
		}
		else
		{	/* other types have 4 top boot sectors (32,8,8,16) */
			for (i = 0; i < info->sector_count - 4; i++)						/* +64k		*/
				info->start[i] = base + (i * (0x00010000 << FLASH_SHIFT));

			info->start[i++] = base + info->size - (0x00010000 << FLASH_SHIFT);	/* -32k		*/
			info->start[i++] = base + info->size - (0x00008000 << FLASH_SHIFT);	/* -8k		*/
			info->start[i++] = base + info->size - (0x00006000 << FLASH_SHIFT);	/* -8k		*/
			info->start[i]   = base + info->size - (0x00004000 << FLASH_SHIFT);	/* -16k		*/
		}
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
	case FLASH_MAN_FUJ:	printf ("FUJITSU ");	break;
	case FLASH_MAN_STM:	printf ("ST ");			break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM160B:	printf ("AM29LV160B (16 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM160T:	printf ("AM29LV160T (16 Mbit, top boot sector)\n");
				break;
	case FLASH_AM320B:	printf ("AM29LV320B (32 Mbit, bottom boot sect)\n");
				break;
	case FLASH_AM320T:	printf ("AM29LV320T (32 Mbit, top boot sector)\n");
				break;
	case FLASH_STMW320DB:	printf ("M29W320B (32 Mbit, bottom boot sect)\n");
				break;
	case FLASH_STMW320DT:	printf ("M29W320T (32 Mbit, top boot sector)\n");
				break;
	default:		printf ("Unknown Chip Type\n");
				break;
	}

	printf ("  Size: %ld MB in %d Sectors\n", info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s", info->start[i], info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");
}

/*
 * The following code cannot be run from FLASH!
 */

#define	AMD_ID_LV160T_MVS	(AMD_ID_LV160T & FLASH_DATA_MASK)
#define AMD_ID_LV160B_MVS	(AMD_ID_LV160B & FLASH_DATA_MASK)
#define AMD_ID_LV320T_MVS	(AMD_ID_LV320T & FLASH_DATA_MASK)
#define AMD_ID_LV320B_MVS	(AMD_ID_LV320B & FLASH_DATA_MASK)
#define STM_ID_W320DT_MVS	(STM_ID_29W320DT & FLASH_DATA_MASK)
#define STM_ID_W320DB_MVS	(STM_ID_29W320DB & FLASH_DATA_MASK)
#define AMD_MANUFACT_MVS	(AMD_MANUFACT  & FLASH_DATA_MASK)
#define FUJ_MANUFACT_MVS	(FUJ_MANUFACT  & FLASH_DATA_MASK)
#define STM_MANUFACT_MVS	(STM_MANUFACT  & FLASH_DATA_MASK)

#if (FLASH_BUS_WIDTH >= 16)
	#define AUTOSELECT_ADDR1	0x0555
	#define AUTOSELECT_ADDR2	0x02AA
	#define AUTOSELECT_ADDR3	AUTOSELECT_ADDR1
#else
	#define AUTOSELECT_ADDR1	0x0AAA
	#define AUTOSELECT_ADDR2	0x0555
	#define AUTOSELECT_ADDR3	AUTOSELECT_ADDR1
#endif

#define AUTOSELECT_DATA1	(0x00AA00AA & FLASH_DATA_MASK)
#define AUTOSELECT_DATA2	(0x00550055 & FLASH_DATA_MASK)
#define AUTOSELECT_DATA3	(0x00900090 & FLASH_DATA_MASK)

#define RESET_BANK_DATA		(0x00F000F0 & FLASH_DATA_MASK)


static ulong flash_get_size (vu_long *address, flash_info_t *info)
{
	short i;
	FDT value;
	FDT *addr = (FDT *)address;

	ulong base = (ulong)address;
	addr[AUTOSELECT_ADDR1] = AUTOSELECT_DATA1;
	addr[AUTOSELECT_ADDR2] = AUTOSELECT_DATA2;
	addr[AUTOSELECT_ADDR3] = AUTOSELECT_DATA3;
	__asm__ __volatile__("sync");

	udelay(180);

	value = addr[0];			/* manufacturer ID	*/
	switch (value) {
	case AMD_MANUFACT_MVS:
		info->flash_id = FLASH_MAN_AMD;
		break;
	case FUJ_MANUFACT_MVS:
		info->flash_id = FLASH_MAN_FUJ;
		break;
	case STM_MANUFACT_MVS:
		info->flash_id = FLASH_MAN_STM;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);			/* no or unknown flash	*/
	}
#if (FLASH_BUS_WIDTH >= 16)
	value = addr[1];			/* device ID		*/
#else
	value = addr[2];			/* device ID		*/
#endif

	switch (value) {
	case AMD_ID_LV160T_MVS:
		info->flash_id += FLASH_AM160T;
		info->sector_count = 37;
		info->size = (0x00200000 << FLASH_SHIFT);
		break;				/* => 2 or 4 MB		*/

	case AMD_ID_LV160B_MVS:
		info->flash_id += FLASH_AM160B;
		info->sector_count = 37;
		info->size = (0x00200000 << FLASH_SHIFT);
		break;				/* => 2 or 4 MB		*/

	case AMD_ID_LV320T_MVS:
		info->flash_id += FLASH_AM320T;
		info->sector_count = 71;
		info->size = (0x00400000 << FLASH_SHIFT);
		break;				/* => 4 or 8 MB		*/

	case AMD_ID_LV320B_MVS:
		info->flash_id += FLASH_AM320B;
		info->sector_count = 71;
		info->size = (0x00400000 << FLASH_SHIFT);
		break;				/* => 4 or 8MB		*/

	case STM_ID_W320DT_MVS:
		info->flash_id += FLASH_STMW320DT;
		info->sector_count = 67;
		info->size = (0x00400000 << FLASH_SHIFT);
		break;				/* => 4 or 8 MB		*/

	case STM_ID_W320DB_MVS:
		info->flash_id += FLASH_STMW320DB;
		info->sector_count = 67;
		info->size = (0x00400000 << FLASH_SHIFT);
		break;				/* => 4 or 8MB		*/

	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */

	}

	/* set up sector start address table */
	flash_get_offsets (base, info);

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr = (FDT *)(info->start[i]);
		info->protect[i] = addr[2] & 1;
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr = (FDT *)info->start[0];
		*addr = RESET_BANK_DATA;	/* reset bank */
	}
	return (info->size);
}


/*-----------------------------------------------------------------------
 */

#if (FLASH_BUS_WIDTH >= 16)
	#define ERASE_ADDR1 0x0555
	#define ERASE_ADDR2 0x02AA
#else
	#define ERASE_ADDR1 0x0AAA
	#define ERASE_ADDR2 0x0555
#endif

#define ERASE_ADDR3 ERASE_ADDR1
#define ERASE_ADDR4 ERASE_ADDR1
#define ERASE_ADDR5 ERASE_ADDR2

#define ERASE_DATA1 (0x00AA00AA & FLASH_DATA_MASK)
#define ERASE_DATA2 (0x00550055 & FLASH_DATA_MASK)
#define ERASE_DATA3 (0x00800080 & FLASH_DATA_MASK)
#define ERASE_DATA4 ERASE_DATA1
#define ERASE_DATA5 ERASE_DATA2

#define ERASE_SECTOR_DATA	(0x00300030 & FLASH_DATA_MASK)
#define ERASE_CHIP_DATA		(0x00100010 & FLASH_DATA_MASK)
#define ERASE_CONFIRM_DATA	(0x00800080 & FLASH_DATA_MASK)

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	FDT *addr = (FDT *)(info->start[0]);

	int prot, sect, l_sect, flag;
	ulong start, now, last;

	__asm__ __volatile__ ("sync");
	addr[0] = 0xf0;
	udelay(1000);

	printf("\nflash_erase: first = %d @ 0x%08lx\n", s_first, info->start[s_first] );
	printf("             last  = %d @ 0x%08lx\n", s_last , info->start[s_last ] );

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	if ((info->flash_id == FLASH_UNKNOWN) || (info->flash_id > FLASH_AMD_COMP)) {
		printf ("Can't erase unknown flash type %08lx - aborted\n", info->flash_id);
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

	addr[ERASE_ADDR1] = ERASE_DATA1;
	addr[ERASE_ADDR2] = ERASE_DATA2;
	addr[ERASE_ADDR3] = ERASE_DATA3;
	addr[ERASE_ADDR4] = ERASE_DATA4;
	addr[ERASE_ADDR5] = ERASE_DATA5;

	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {
			addr = (FDT *)(info->start[sect]);
			addr[0] = ERASE_SECTOR_DATA;
			l_sect = sect;
		}
	}

	if (flag)
		enable_interrupts();

	/*
	 * We wait for the last triggered sector
	 */
	if (l_sect < 0)
		goto DONE;

	start = get_timer (0);
	last  = start;
	addr = (FDT *)(info->start[l_sect]);

	while ((addr[0] & ERASE_CONFIRM_DATA) != ERASE_CONFIRM_DATA) {
		if ((now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
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
#define BUFF_INC 4
	ulong cp, wp, data;
	int i, l, rc;

	mvdebug (("+write_buff %p ==> 0x%08lx, count = 0x%08lx\n", src, addr, cnt));

	wp = (addr & ~3);	/* get lower word aligned address */
	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		mvdebug ((" handle unaligned start bytes (cnt = 0x%08lx)\n", cnt));
		data = 0;
		for (i=0, cp=wp; i<l; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}
		for (; i<BUFF_INC && cnt>0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt==0 && i<BUFF_INC; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}

		if ((rc = write_word(info, wp, data)) != 0) {
			return (rc);
		}
		wp += BUFF_INC;
	}

	/*
	 * handle (half)word aligned part
	 */
	mvdebug ((" handle word aligned part (cnt = 0x%08lx)\n", cnt));
	while (cnt >= BUFF_INC) {
		data = 0;
		for (i=0; i<BUFF_INC; ++i) {
			data = (data << 8) | *src++;
		}
		if ((rc = write_word(info, wp, data)) != 0) {
			return (rc);
		}
		wp  += BUFF_INC;
		cnt -= BUFF_INC;
	}

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	mvdebug ((" handle unaligned tail bytes (cnt = 0x%08lx)\n", cnt));
	data = 0;
	for (i=0, cp=wp; i<BUFF_INC && cnt>0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i<BUFF_INC; ++i, ++cp) {
		data = (data << 8) | (*(uchar *)cp);
	}

	return (write_word(info, wp, data));
}

#if (FLASH_BUS_WIDTH >= 16)
	#define WRITE_ADDR1 0x0555
	#define WRITE_ADDR2 0x02AA
#else
	#define WRITE_ADDR1 0x0AAA
	#define WRITE_ADDR2 0x0555
	#define WRITE_ADDR3 WRITE_ADDR1
#endif

#define WRITE_DATA1 (0x00AA00AA & FLASH_DATA_MASK)
#define WRITE_DATA2 (0x00550055 & FLASH_DATA_MASK)
#define WRITE_DATA3 (0x00A000A0 & FLASH_DATA_MASK)

#define WRITE_CONFIRM_DATA ERASE_CONFIRM_DATA

/*-----------------------------------------------------------------------
 * Write a byte to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_char (flash_info_t *info, ulong dest, uchar data)
{
	vu_char *addr = (vu_char *)(info->start[0]);
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*((vu_char *)dest) & data) != data) {
		printf(" *** ERROR: Flash not erased !\n");
		return (2);
	}
	flag = disable_interrupts();

	addr[WRITE_ADDR1] = WRITE_DATA1;
	addr[WRITE_ADDR2] = WRITE_DATA2;
	addr[WRITE_ADDR3] = WRITE_DATA3;
	*((vu_char *)dest) = data;

	if (flag)
		enable_interrupts();

	/* data polling for D7 */
	start = get_timer (0);
	addr = (vu_char *)dest;
	while (( (*addr) & WRITE_CONFIRM_DATA) != (data & WRITE_CONFIRM_DATA)) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			printf(" *** ERROR: Flash write timeout !");
			return (1);
		}
	}
	mvdebug (("-write_byte\n"));
	return (0);
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word (flash_info_t *info, ulong dest, ulong data)
{
	int i,
		result = 0;

	mvdebug (("+write_word : 0x%08lx @ 0x%08lx\n", data, dest));
	for ( i=0; (i < 4) && (result == 0); i++, dest+=1 )
		result = write_char (info, dest, (data >> (8*(3-i))) & 0xff );
	mvdebug (("-write_word\n"));
	return result;
}
/*---------------------------------------------------------------- */
