/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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

ulong myflush(void);


#define FLASH_BANK_SIZE 0x400000	/* 4 MB */
#define MAIN_SECT_SIZE  0x20000		/* 128 KB */

flash_info_t    flash_info[CFG_MAX_FLASH_BANKS];


#define CMD_READ_ARRAY		0x00F000F0
#define CMD_UNLOCK1		0x00AA00AA
#define CMD_UNLOCK2		0x00550055
#define CMD_ERASE_SETUP		0x00800080
#define CMD_ERASE_CONFIRM	0x00300030
#define CMD_PROGRAM		0x00A000A0
#define CMD_UNLOCK_BYPASS	0x00200020

#define MEM_FLASH_ADDR1		(*(volatile u32 *)(CFG_FLASH_BASE + (0x00000555 << 2)))
#define MEM_FLASH_ADDR2		(*(volatile u32 *)(CFG_FLASH_BASE + (0x000002AA << 2)))

#define BIT_ERASE_DONE		0x00800080
#define BIT_RDY_MASK		0x00800080
#define BIT_PROGRAM_ERROR	0x00200020
#define BIT_TIMEOUT		0x80000000 /* our flag */

#define READY 1
#define ERR   2
#define TMO   4

/*-----------------------------------------------------------------------
 */

ulong flash_init(void)
{
    int i, j;
    ulong size = 0;

    for (i = 0; i < CFG_MAX_FLASH_BANKS; i++)
    {
	ulong flashbase = 0;
	flash_info[i].flash_id =
	  (AMD_MANUFACT & FLASH_VENDMASK) |
	  (AMD_ID_LV160B & FLASH_TYPEMASK);
	flash_info[i].size = FLASH_BANK_SIZE;
	flash_info[i].sector_count = CFG_MAX_FLASH_SECT;
	memset(flash_info[i].protect, 0, CFG_MAX_FLASH_SECT);
	if (i == 0)
	  flashbase = PHYS_FLASH_1;
	else
	  panic("configured too many flash banks!\n");
	for (j = 0; j < flash_info[i].sector_count; j++)
	{

	    if (j <= 3)
	    {
		/* 1st one is 32 KB */
		if (j == 0)
		{
			flash_info[i].start[j] = flashbase + 0;
		}

		/* 2nd and 3rd are both 16 KB */
		if ((j == 1) || (j == 2))
		{
			flash_info[i].start[j] = flashbase + 0x8000 + (j-1)*0x4000;
		}

		/* 4th 64 KB */
		if (j == 3)
		{
			flash_info[i].start[j] = flashbase + 0x10000;
		}
	    }
	    else
	    {
		flash_info[i].start[j] = flashbase + (j - 3)*MAIN_SECT_SIZE;
	    }
	}
	size += flash_info[i].size;
    }

    /*
     * Protect monitor and environment sectors
     * Inferno is complicated, it's hardware locked
     */
#ifdef CONFIG_INFERNO
    /* first one, 0x00000 to 0x07fff */
    flash_protect(FLAG_PROTECT_SET,
		  CFG_FLASH_BASE + 0x00000,
		  CFG_FLASH_BASE + 0x08000 - 1,
		  &flash_info[0]);

    /* third to 10th, 0x0c000 - 0xdffff */
    flash_protect(FLAG_PROTECT_SET,
		  CFG_FLASH_BASE + 0x0c000,
		  CFG_FLASH_BASE + 0xe0000 - 1,
		  &flash_info[0]);
#else
    flash_protect(FLAG_PROTECT_SET,
		  CFG_FLASH_BASE,
		  CFG_FLASH_BASE + monitor_flash_len - 1,
		  &flash_info[0]);

    flash_protect(FLAG_PROTECT_SET,
		  CFG_ENV_ADDR,
		  CFG_ENV_ADDR + CFG_ENV_SIZE - 1,
		  &flash_info[0]);
#endif
    return size;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info  (flash_info_t *info)
{
    int i;

    switch (info->flash_id & FLASH_VENDMASK)
    {
    case (AMD_MANUFACT & FLASH_VENDMASK):
	printf("AMD: ");
	break;
    default:
	printf("Unknown Vendor ");
	break;
    }

    switch (info->flash_id & FLASH_TYPEMASK)
    {
    case (AMD_ID_LV160B & FLASH_TYPEMASK):
	printf("2x Amd29F160BB (16Mbit)\n");
	break;
    default:
	printf("Unknown Chip Type\n");
	goto Done;
	break;
    }

    printf("  Size: %ld MB in %d Sectors\n",
	   info->size >> 20, info->sector_count);

    printf("  Sector Start Addresses:");
    for (i = 0; i < info->sector_count; i++)
    {
	if ((i % 5) == 0)
	{
	    printf ("\n   ");
	}
	printf (" %08lX%s", info->start[i],
		info->protect[i] ? " (RO)" : "     ");
    }
    printf ("\n");

Done:
    ;
}

/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
    ulong result;
    int iflag, cflag, prot, sect;
    int rc = ERR_OK;
    int chip1, chip2;

    /* first look for protection bits */

    if (info->flash_id == FLASH_UNKNOWN)
	return ERR_UNKNOWN_FLASH_TYPE;

    if ((s_first < 0) || (s_first > s_last)) {
	return ERR_INVAL;
    }

    if ((info->flash_id & FLASH_VENDMASK) !=
	(AMD_MANUFACT & FLASH_VENDMASK)) {
	return ERR_UNKNOWN_FLASH_VENDOR;
    }

    prot = 0;
    for (sect=s_first; sect<=s_last; ++sect) {
	if (info->protect[sect]) {
	    prot++;
	}
    }
    if (prot)
	return ERR_PROTECTED;

    /*
     * Disable interrupts which might cause a timeout
     * here. Remember that our exception vectors are
     * at address 0 in the flash, and we don't want a
     * (ticker) exception to happen while the flash
     * chip is in programming mode.
     */
    cflag = icache_status();
    icache_disable();
    iflag = disable_interrupts();

    /* Start erase on unprotected sectors */
    for (sect = s_first; sect<=s_last && !ctrlc(); sect++)
    {
	printf("Erasing sector %2d ... ", sect);

	/* arm simple, non interrupt dependent timer */
	reset_timer_masked();

	if (info->protect[sect] == 0)
	{	/* not protected */
	    vu_long *addr = (vu_long *)(info->start[sect]);

	    MEM_FLASH_ADDR1 = CMD_UNLOCK1;
	    MEM_FLASH_ADDR2 = CMD_UNLOCK2;
	    MEM_FLASH_ADDR1 = CMD_ERASE_SETUP;

	    MEM_FLASH_ADDR1 = CMD_UNLOCK1;
	    MEM_FLASH_ADDR2 = CMD_UNLOCK2;
	    *addr = CMD_ERASE_CONFIRM;

	    /* wait until flash is ready */
	    chip1 = chip2 = 0;

	    do
	    {
		result = *addr;

		/* check timeout */
		if (get_timer_masked() > CFG_FLASH_ERASE_TOUT)
		{
		    MEM_FLASH_ADDR1 = CMD_READ_ARRAY;
		    chip1 = TMO;
		    break;
		}

		if (!chip1 && (result & 0xFFFF) & BIT_ERASE_DONE)
			chip1 = READY;

		if (!chip1 && (result & 0xFFFF) & BIT_PROGRAM_ERROR)
			chip1 = ERR;

		if (!chip2 && (result >> 16) & BIT_ERASE_DONE)
			chip2 = READY;

		if (!chip2 && (result >> 16) & BIT_PROGRAM_ERROR)
			chip2 = ERR;

	    }  while (!chip1 || !chip2);

	    MEM_FLASH_ADDR1 = CMD_READ_ARRAY;

	    if (chip1 == ERR || chip2 == ERR)
	    {
		rc = ERR_PROG_ERROR;
		goto outahere;
	    }
	    if (chip1 == TMO)
	    {
		rc = ERR_TIMOUT;
		goto outahere;
	    }

	    printf("ok.\n");
	}
	else /* it was protected */
	{
	    printf("protected!\n");
	}
    }

    if (ctrlc())
      printf("User Interrupt!\n");

outahere:
    /* allow flash to settle - wait 10 ms */
    udelay_masked(10000);

    if (iflag)
      enable_interrupts();

    if (cflag)
      icache_enable();

    return rc;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash
 */

volatile static int write_word (flash_info_t *info, ulong dest, ulong data)
{
    vu_long *addr = (vu_long *)dest;
    ulong result;
    int rc = ERR_OK;
    int cflag, iflag;
    int chip1, chip2;

    /*
     * Check if Flash is (sufficiently) erased
     */
    result = *addr;
    if ((result & data) != data)
	return ERR_NOT_ERASED;


    /*
     * Disable interrupts which might cause a timeout
     * here. Remember that our exception vectors are
     * at address 0 in the flash, and we don't want a
     * (ticker) exception to happen while the flash
     * chip is in programming mode.
     */
    cflag = icache_status();
    icache_disable();
    iflag = disable_interrupts();

    MEM_FLASH_ADDR1 = CMD_UNLOCK1;
    MEM_FLASH_ADDR2 = CMD_UNLOCK2;
    MEM_FLASH_ADDR1 = CMD_UNLOCK_BYPASS;
    *addr = CMD_PROGRAM;
    *addr = data;

    /* arm simple, non interrupt dependent timer */
    reset_timer_masked();

    /* wait until flash is ready */
    chip1 = chip2 = 0;
    do
    {
	result = *addr;

	/* check timeout */
	if (get_timer_masked() > CFG_FLASH_ERASE_TOUT)
	{
	    chip1 = ERR | TMO;
	    break;
	}
	if (!chip1 && ((result & 0x80) == (data & 0x80)))
		chip1 = READY;

	if (!chip1 && ((result & 0xFFFF) & BIT_PROGRAM_ERROR))
	{
		result = *addr;

		if ((result & 0x80) == (data & 0x80))
			chip1 = READY;
		else
			chip1 = ERR;
	}

	if (!chip2 && ((result & (0x80 << 16)) == (data & (0x80 << 16))))
		chip2 = READY;

	if (!chip2 && ((result >> 16) & BIT_PROGRAM_ERROR))
	{
		result = *addr;

		if ((result & (0x80 << 16)) == (data & (0x80 << 16)))
			chip2 = READY;
		else
			chip2 = ERR;
	}

    }  while (!chip1 || !chip2);

    *addr = CMD_READ_ARRAY;

    if (chip1 == ERR || chip2 == ERR || *addr != data)
	rc = ERR_PROG_ERROR;

    if (iflag)
      enable_interrupts();

    if (cflag)
      icache_enable();

    return rc;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash.
 */

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
    ulong cp, wp, data;
    int l;
    int i, rc;

    wp = (addr & ~3);	/* get lower word aligned address */

    /*
     * handle unaligned start bytes
     */
    if ((l = addr - wp) != 0) {
	data = 0;
	for (i=0, cp=wp; i<l; ++i, ++cp) {
	    data = (data >> 8) | (*(uchar *)cp << 24);
	}
	for (; i<4 && cnt>0; ++i) {
	    data = (data >> 8) | (*src++ << 24);
	    --cnt;
	    ++cp;
	}
	for (; cnt==0 && i<4; ++i, ++cp) {
	    data = (data >> 8) | (*(uchar *)cp << 24);
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
	data = *((vu_long*)src);
	if ((rc = write_word(info, wp, data)) != 0) {
	    return (rc);
	}
	src += 4;
	wp  += 4;
	cnt -= 4;
    }

    if (cnt == 0) {
	return ERR_OK;
    }

    /*
     * handle unaligned tail bytes
     */
    data = 0;
    for (i=0, cp=wp; i<4 && cnt>0; ++i, ++cp) {
	data = (data >> 8) | (*src++ << 24);
	--cnt;
    }
    for (; i<4; ++i, ++cp) {
	data = (data >> 8) | (*(uchar *)cp << 24);
    }

    return write_word(info, wp, data);
}
