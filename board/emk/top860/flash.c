/*
 * (C) Copyright 2003
 * EMK Elektronik GmbH <www.emk-elektronik.de>
 * Reinhard Meyer <r.meyer@emk-elektronik.de>
 *
 * copied from the BMW Port - seems that its similiar enough
 * to be easily adaped ;) --- Well, it turned out to become a
 * merger between parts of the EMKstax Flash routines and the
 * BMW funtion frames...
 *
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

#define	FLASH_WORD_SIZE		unsigned short
#define FLASH_WORD_WIDTH	(sizeof (FLASH_WORD_SIZE))

flash_info_t flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips    */

/*-----------------------------------------------------------------------
 * Functions
 */
static int write_word (flash_info_t *info, ulong dest, ulong data);


/*****************************************************************************
 * software product ID entry/exit
 *****************************************************************************/
static void FlashProductIdMode (
	volatile FLASH_WORD_SIZE *b,
	int on_off)
{
	b[0x5555] = 0xaa;
	b[0x2aaa] = 0x55;
	b[0x5555] = on_off ? 0x90 : 0xf0;
}

/*****************************************************************************
 * sector erase start
 *****************************************************************************/
static void FlashSectorErase (
	volatile FLASH_WORD_SIZE *b,
	volatile FLASH_WORD_SIZE *a)
{
	b[0x5555] = 0xaa;
	b[0x2aaa] = 0x55;
	b[0x5555] = 0x80;
	b[0x5555] = 0xaa;
	b[0x2aaa] = 0x55;
	a[0] = 0x30;
}

/*****************************************************************************
 * program a word
 *****************************************************************************/
static void FlashProgWord (
	volatile FLASH_WORD_SIZE *b,
	volatile FLASH_WORD_SIZE *a,
	FLASH_WORD_SIZE v)
{
	b[0x5555] = 0xaa;
	b[0x2aaa] = 0x55;
	b[0x5555] = 0xa0;
	a[0] = v;
}

/*****************************************************************************
 * reset bank, back to read mode
 *****************************************************************************/
static void FlashReset (volatile FLASH_WORD_SIZE *b)
{
	b[0] = 0xf0;
}

/*****************************************************************************
 * identify FLASH chip
 * this code is a stripped version of the FlashGetType() function in EMKstax
 *****************************************************************************/
unsigned long flash_init (void)
{
	volatile FLASH_WORD_SIZE * const flash = (volatile FLASH_WORD_SIZE *) CFG_FLASH_BASE;
	FLASH_WORD_SIZE	manu, dev;
	flash_info_t * const pflinfo = &flash_info[0];
	int j;

	/* get Id Bytes */
	FlashProductIdMode (flash, 1);
	manu = flash[0];
	dev  = flash[1];
	FlashProductIdMode (flash, 0);

	pflinfo->size = 0;
	pflinfo->sector_count = 0;
	pflinfo->flash_id = 0xffffffff;
	pflinfo->portwidth = FLASH_CFI_16BIT;
	pflinfo->chipwidth = FLASH_CFI_BY16;

	switch (manu&0xff)
	{
	case 0x01:	/* AMD */
		pflinfo->flash_id = FLASH_MAN_AMD;
		switch (dev&0xff)
		{
		case 0x49:
			pflinfo->size = 0x00200000;
			pflinfo->sector_count = 35;
			pflinfo->flash_id |= FLASH_AM160B;
			pflinfo->start[0] = CFG_FLASH_BASE;
			pflinfo->start[1] = CFG_FLASH_BASE + 0x4000;
			pflinfo->start[2] = CFG_FLASH_BASE + 0x6000;
			pflinfo->start[3] = CFG_FLASH_BASE + 0x8000;
			for (j = 4; j < 35; j++)
			{
				pflinfo->start[j] = CFG_FLASH_BASE + 0x00010000 * (j-3);
			}
			break;

		case 0xf9:
			pflinfo->size = 0x00400000;
			pflinfo->sector_count = 71;
			pflinfo->flash_id |= FLASH_AM320B;
			pflinfo->start[0] = CFG_FLASH_BASE;
			pflinfo->start[1] = CFG_FLASH_BASE + 0x4000;
			pflinfo->start[2] = CFG_FLASH_BASE + 0x6000;
			pflinfo->start[3] = CFG_FLASH_BASE + 0x8000;
			for (j = 0; j < 8; j++)
			{
				pflinfo->start[j] = CFG_FLASH_BASE + 0x00002000 * (j);
			}
			for (j = 8; j < 71; j++)
			{
				pflinfo->start[j] = CFG_FLASH_BASE + 0x00010000 * (j-7);
			}
			break;

		default:
			printf ("unknown AMD dev=%x ", dev);
			pflinfo->flash_id |= FLASH_UNKNOWN;
		}
		break;

	default:
		printf ("unknown manu=%x ", manu);
	}
	return pflinfo->size;
}

/*****************************************************************************
 * print info about a FLASH
 *****************************************************************************/
void flash_print_info (flash_info_t *info)
{
	static const char	unk[] = "Unknown";
	unsigned int		i;
	const char			*mfct=unk,
						*type=unk;

	if(info->flash_id != FLASH_UNKNOWN)
	{
		switch (info->flash_id & FLASH_VENDMASK)
		{
		case FLASH_MAN_AMD:
			mfct = "AMD";
			break;
		}

		switch (info->flash_id & FLASH_TYPEMASK)
		{
		case FLASH_AM160B:
			type = "AM29LV160B (16 Mbit, bottom boot sect)";
			break;
		case FLASH_AM320B:
			type = "AM29LV320B (32 Mbit, bottom boot sect)";
			break;
		}
	}

	printf (
		"\n  Brand: %s Type: %s\n"
		"  Size: %lu KB in %d Sectors\n",
		mfct,
		type,
		info->size >> 10,
		info->sector_count
		);

	printf ("  Sector Start Addresses:");

	for (i = 0; i < info->sector_count; i++)
	{
		unsigned long	size;
		unsigned int	erased;
		unsigned long	*flash = (unsigned long *) info->start[i];

		/*
		 * Check if whole sector is erased
		 */
		size =
			(i != (info->sector_count - 1)) ?
			(info->start[i + 1] - info->start[i]) >> 2 :
		(info->start[0] + info->size - info->start[i]) >> 2;

		for (
			flash = (unsigned long *) info->start[i], erased = 1;
				(flash != (unsigned long *) info->start[i] + size) && erased;
					flash++
			)
			erased = *flash == ~0x0UL;

		printf (
			"%s %08lX %s %s",
			(i % 5) ? "" : "\n   ",
			info->start[i],
			erased ? "E" : " ",
			info->protect[i] ? "RO" : "  "
			);
	}

	puts ("\n");
	return;
}

/*****************************************************************************
 * erase one or more sectors
 *****************************************************************************/
int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	volatile FLASH_WORD_SIZE	*addr = (FLASH_WORD_SIZE *)(info->start[0]);
	int							flag,
								prot,
								sect,
								l_sect;
	ulong						start,
								now,
								last;

	if ((s_first < 0) || (s_first > s_last))
	{
		if (info->flash_id == FLASH_UNKNOWN)
		{
			printf ("- missing\n");
		}
		else
		{
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	if ((info->flash_id == FLASH_UNKNOWN) ||
		(info->flash_id > (FLASH_MAN_STM | FLASH_AMD_COMP)))
	{
		printf ("Can't erase unknown flash type - aborted\n");
		return 1;
	}

	prot = 0;
	for (sect=s_first; sect<=s_last; ++sect)
	{
		if (info->protect[sect])
		{
			prot++;
		}
	}

	if (prot)
	{
		printf ("- Warning: %d protected sectors will not be erased!\n",
			prot);
	}
	else
	{
		printf ("\n");
	}

	l_sect = -1;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++)
	{
		if (info->protect[sect] == 0)
		{ /* not protected */
			FlashSectorErase ((FLASH_WORD_SIZE *)info->start[0], (FLASH_WORD_SIZE *)info->start[sect]);
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
	addr = (FLASH_WORD_SIZE *)info->start[l_sect];
	while ((addr[0] & 0x0080) != 0x0080)
	{
		if ((now = get_timer (start)) > CFG_FLASH_ERASE_TOUT)
		{
			printf ("Timeout\n");
			return 1;
		}
		/* show that we're waiting */
		if ((now - last) > 1000)
		{  /* every second */
			serial_putc ('.');
			last = now;
		}
	}

	DONE:
	/* reset to read mode */
	FlashReset ((FLASH_WORD_SIZE *)info->start[0]);

	printf (" done\n");
	return 0;
}

/*****************************************************************************
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 *****************************************************************************/
int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong		cp,
				wp,
				data;
	int			i,
				l,
				rc;

	wp = (addr & ~(FLASH_WORD_WIDTH-1));   /* get lower word aligned address */

	/*
	 * handle unaligned start bytes, if there are...
	 */
	if ((l = addr - wp) != 0)
	{
		data = 0;
		
		/* get the current before the new data into our data word */
		for (i=0, cp=wp; i<l; ++i, ++cp)
		{
			data = (data << 8) | (*(uchar *)cp);
		}
		
		/* now merge the to be programmed values */
		for (; i<4 && cnt>0; ++i, ++cp, --cnt)
		{
			data = (data << 8) | *src++;
		}

		/* get the current after the new data into our data word */
		for (; cnt==0 && i<FLASH_WORD_WIDTH; ++i, ++cp)
		{
			data = (data << 8) | (*(uchar *)cp);
		}

		/* now write the combined word */
		if ((rc = write_word (info, wp, data)) != 0)
		{
			return (rc);
		}
		wp += FLASH_WORD_WIDTH;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= FLASH_WORD_WIDTH)
	{
		data = 0;
		for (i=0; i<FLASH_WORD_WIDTH; ++i)
		{
			data = (data << 8) | *src++;
		}
		if ((rc = write_word (info, wp, data)) != 0)
		{
			return (rc);
		}
		wp  += FLASH_WORD_WIDTH;
		cnt -= FLASH_WORD_WIDTH;
	}

	if (cnt == 0)
	{
		return (0);
	}

	/*
	 * handle unaligned tail bytes, if there are...
	 */
	data = 0;

	/* now merge the to be programmed values */
	for (i=0, cp=wp; i<FLASH_WORD_WIDTH && cnt>0; ++i, ++cp)
	{
		data = (data << 8) | *src++;
		--cnt;
	}

	/* get the current after the new data into our data word */
	for (; i<FLASH_WORD_WIDTH; ++i, ++cp)
	{
		data = (data << 8) | (*(uchar *)cp);
	}

	/* now write the combined word */
	return (write_word (info, wp, data));
}

/*****************************************************************************
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 *****************************************************************************/
static int write_word (flash_info_t *info, ulong dest, ulong data)
{
	volatile FLASH_WORD_SIZE	*addr2 = (FLASH_WORD_SIZE *)info->start[0];
	volatile FLASH_WORD_SIZE	*dest2 = (FLASH_WORD_SIZE *)dest;
	FLASH_WORD_SIZE				data2 = data;
	ulong						start;
	int							flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*dest2 & data2) != data2)
	{
		return (2);
	}
	
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	FlashProgWord (addr2, dest2, data2);

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();

	/* data polling for D7 */
	start = get_timer (0);
	while ((*dest2 & 0x0080) != (data2 & 0x0080))
	{
		if (get_timer (start) > CFG_FLASH_WRITE_TOUT)
		{
			return (1);
		}
	}

	return (0);
}

/*-----------------------------------------------------------------------
 */

