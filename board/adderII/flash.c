/*
 * (C) Copyright 2000-2003
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
/******************************************************************************
** Notes: AM29LV320DB - 90EI ( 32 Mbit device )
** Sectors - Eight 8 Kb sector
** 	   - Sixty three 64 Kb sector
** Bottom boot sector
******************************************************************************/
#include <common.h>
#include <mpc8xx.h>


/******************************************************************************
**	Defines
******************************************************************************/
#ifdef CONFIG_ADDERII

#define ADDR0	0x0555
#define ADDR1	0x02AA
#define FLASH_WORD_SIZE unsigned short

#endif

#if defined( CFG_ENV_IS_IN_FLASH )
# ifndef  CFG_ENV_ADDR
#  define CFG_ENV_ADDR  ( CFG_FLASH_BASE + CFG_ENV_OFFSET )
# endif
# ifndef  CFG_ENV_SIZE
#  define CFG_ENV_SIZE  CFG_ENV_SECT_SIZE
# endif
# ifndef  CFG_ENV_SECT_SIZE
#  define CFG_ENV_SECT_SIZE  CFG_ENV_SIZE
# endif
#endif

/******************************************************************************
**	Global Parameters
******************************************************************************/
flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

/******************************************************************************
**	Function Prototypes
******************************************************************************/
static ulong flash_get_size( vu_long *addr, flash_info_t *info );

static int write_word( flash_info_t *info, ulong dest, ulong data );

static void flash_get_offsets( ulong base, flash_info_t *info );

int wait_for_DQ7( flash_info_t *info, int sect );

/******************************************************************************
** Function :	flash_init
** Param    :   void
** Notes    :   Initializes the Flash Chip
******************************************************************************/
ulong flash_init (void)
{
	ulong size_b0 = -1;
	int i;
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	/* Set Flash to unknown */
	for (i = 0; i < CFG_MAX_FLASH_BANKS; i++) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Get the Flash Bank Size */

	size_b0 = flash_get_size ((vu_long *) (CFG_FLASH_BASE),
				  &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## UNKNOWN Flash on Bank 0 - Size = 0x%08lx = %ldMB\n",
			size_b0, size_b0 >> 20);
	}

	/* Remap Flash according to size detected  */
	memctl->memc_or0 = 0xFF800774;
	memctl->memc_br0 = CFG_BR0_PRELIM;

	/* Setup Flash Sector Offsets */

	flash_get_offsets (CFG_FLASH_BASE, &flash_info[0]);

	/* Monitor Protection ON - default */

#if ( CFG_MONITOR_BASE >= CFG_FLASH_BASE  )
	flash_protect (FLAG_PROTECT_SET, CFG_MONITOR_BASE,
		       (CFG_MONITOR_BASE + monitor_flash_len - 1),
		       &flash_info[0]);
#endif

	/* Protect Environment Variables */
#ifdef CFG_ENV_IS_IN_FLASH
	flash_protect (FLAG_PROTECT_SET, CFG_ENV_ADDR,
		       (CFG_ENV_ADDR + CFG_ENV_SIZE - 1), &flash_info[0]);
#endif

	return size_b0;
}

/******************************************************************************
** Function : flash_get_offsets
** Param    : ulong base, flash_into_t *info
** Notes    :
******************************************************************************/
static void flash_get_offsets (ulong base, flash_info_t * info)
{
	return;
}


/******************************************************************************
** Function : flash_print_info
** Param    : flash_info_t
** Notes    :
******************************************************************************/
void flash_print_info (flash_info_t * info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("Missing or unknown flash type\n");
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
			info->start[i],
			info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");
	return;
}

/******************************************************************************
** Function : flash_get_size
** Param    : vu_long *addr, flash_info_t *info
** Notes    :
******************************************************************************/
static ulong flash_get_size (vu_long * addr, flash_info_t * info)
{
	short i;
	FLASH_WORD_SIZE manu_id, dev_id;
	ulong base = (ulong) addr;
	volatile FLASH_WORD_SIZE *addr2 = (FLASH_WORD_SIZE *) addr;

	/* Write Auto Select Command and read Manufacturer's ID and Dev ID */

	addr2[ADDR0] = (FLASH_WORD_SIZE) 0xAAAAAAAA;
	addr2[ADDR1] = (FLASH_WORD_SIZE) 0x55555555;
	addr2[ADDR0] = (FLASH_WORD_SIZE) 0x90909090;

	manu_id = addr2[0];

	switch (manu_id) {
	case (FLASH_WORD_SIZE) AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
		break;

	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		break;
	}

	/* Read Device Id */
	dev_id = addr2[1];

	switch (dev_id) {
	case (FLASH_WORD_SIZE) AMD_ID_LV320B:
		info->flash_id += FLASH_AM320B;
		info->sector_count = 71;	/* 8 - boot sec + 63 normal */
		info->size = 0x400000;	/* 4MByte */
		break;

	default:
		info->flash_id = FLASH_UNKNOWN;
		break;
	}

	/* Set up sector start Addresses */

	if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block
		 ** Eight 8 Kb Boot sectors
		 ** Sixty Three 64Kb sectors
		 */
		for (i = 0; i < 8; i++) {
			info->start[i] = base + (i * 0x00002000);
		}
		for (i = 8; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00010000) - 0x00070000;
		}
	}

	/* Reset To read mode */

	if (info->flash_id != FLASH_UNKNOWN) {
		addr = (ulong *) info->start[0];
		*addr = 0xF0F0F0F0;
	}
	return (info->size);
}

/*******************************************************************************
** Function : flash_erase
** Param    : flash_info_t *info, int s_first, int s_last
** Notes    :
******************************************************************************/
int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	volatile FLASH_WORD_SIZE *addr = (FLASH_WORD_SIZE *) (info->start[0]);
	volatile FLASH_WORD_SIZE *addr2;
	int flag, prot, sect, l_sect;
	int i;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("Can't erase unknown flash type - aborted\n");
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf ("Warning: %d protected sectors will not be erased!\n",
			prot);
	} else {
		printf ("\n");
	}

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {

		if (info->protect[sect] == 0) {	/* not protected */
			addr2 = (FLASH_WORD_SIZE *) (info->start[sect]);

			if ((info->flash_id & FLASH_VENDMASK) ==
			    FLASH_MAN_SST) {
				addr[ADDR0] = (FLASH_WORD_SIZE) 0x00AA00AA;
				addr[ADDR1] = (FLASH_WORD_SIZE) 0x00550055;
				addr[ADDR0] = (FLASH_WORD_SIZE) 0x00800080;
				addr[ADDR0] = (FLASH_WORD_SIZE) 0x00AA00AA;
				addr[ADDR1] = (FLASH_WORD_SIZE) 0x00550055;
				addr2[0] = (FLASH_WORD_SIZE) 0x00500050;	/* block erase */
				for (i = 0; i < 50; i++)
					udelay (1000);	/* wait 1 ms */
			} else {
				addr[ADDR0] = (FLASH_WORD_SIZE) 0x00AA00AA;
				addr[ADDR1] = (FLASH_WORD_SIZE) 0x00550055;
				addr[ADDR0] = (FLASH_WORD_SIZE) 0x00800080;
				addr[ADDR0] = (FLASH_WORD_SIZE) 0x00AA00AA;
				addr[ADDR1] = (FLASH_WORD_SIZE) 0x00550055;
				addr2[0] = (FLASH_WORD_SIZE) 0x00300030;	/* sector erase */
			}
			l_sect = sect;
			/*
			 * Wait for each sector to complete, it's more
			 * reliable.  According to AMD Spec, you must
			 * issue all erase commands within a specified
			 * timeout.  This has been seen to fail, especially
			 * if printf()s are included (for debug)!!
			 */
			wait_for_DQ7 (info, sect);
		}
	}
	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();

	/* wait at least 80us - let's wait 1 ms */
	udelay (1000);

	/* reset to read mode */
	addr = (FLASH_WORD_SIZE *) info->start[0];
	addr[0] = (FLASH_WORD_SIZE) 0x00F000F0;	/* reset bank */

	printf (" done\n");
	return 0;
}

int wait_for_DQ7 (flash_info_t * info, int sect)
{
	ulong start, now, last;
	volatile FLASH_WORD_SIZE *addr =
		(FLASH_WORD_SIZE *) (info->start[sect]);

	start = get_timer (0);
	last = start;
	while ((addr[0] & (FLASH_WORD_SIZE) 0x00800080) !=
	       (FLASH_WORD_SIZE) 0x00800080) {
		if ((now = get_timer (start)) > CFG_FLASH_ERASE_TOUT) {
			printf ("Timeout\n");
			return -1;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) {
			putc ('.');
			last = now;
		}
	}
	return 0;
}


/******************************************************************************
** Function : write_buff
** Param    : flash_info_t *info, uchar *src, ulong addr, ulong cnt
** Notes    :
******************************************************************************/
int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int i, l, rc;

	/* get lower word aligned address */
	wp = (addr & ~3);

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


/******************************************************************************
** Function : write_word
** Param    : flash_info_t *info, ulong dest, ulong data
** Notes    :
******************************************************************************/
static int write_word (flash_info_t * info, ulong dest, ulong data)
{
	volatile FLASH_WORD_SIZE *addr2 =
		(FLASH_WORD_SIZE *) (info->start[0]);
	volatile FLASH_WORD_SIZE *dest2 = (FLASH_WORD_SIZE *) dest;
	volatile FLASH_WORD_SIZE *data2 = (FLASH_WORD_SIZE *) & data;
	ulong start;
	int i;

	/* Check if Flash is (sufficiently) erased */
	if ((*((volatile FLASH_WORD_SIZE *) dest) &
	     (FLASH_WORD_SIZE) data) != (FLASH_WORD_SIZE) data) {
		return (2);
	}

	for (i = 0; i < 4 / sizeof (FLASH_WORD_SIZE); i++) {
		int flag;

		/* Disable interrupts which might cause a timeout here */
		flag = disable_interrupts ();

		addr2[ADDR0] = (FLASH_WORD_SIZE) 0x00AA00AA;
		addr2[ADDR1] = (FLASH_WORD_SIZE) 0x00550055;
		addr2[ADDR0] = (FLASH_WORD_SIZE) 0x00A000A0;

		dest2[i] = data2[i];

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts ();

		/* data polling for D7 */
		start = get_timer (0);
		while ((dest2[i] & (FLASH_WORD_SIZE) 0x00800080) !=
		       (data2[i] & (FLASH_WORD_SIZE) 0x00800080)) {
			if (get_timer (start) > CFG_FLASH_WRITE_TOUT) {
				return (1);
			}
		}
	}

	return (0);
}
