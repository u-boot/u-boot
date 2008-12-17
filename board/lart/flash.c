/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
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


#define FLASH_BANK_SIZE 0x800000
#define MAIN_SECT_SIZE  0x20000
#define PARAM_SECT_SIZE 0x4000

/* puzzle magic for lart
 * data_*_flash are def'd in flashasm.S
 */

extern u32 data_from_flash(u32);
extern u32 data_to_flash(u32);

#define PUZZLE_FROM_FLASH(x)	data_from_flash((x))
#define PUZZLE_TO_FLASH(x)	data_to_flash((x))

flash_info_t    flash_info[CONFIG_SYS_MAX_FLASH_BANKS];


#define CMD_READ_ARRAY		0x00FF00FF
#define CMD_IDENTIFY		0x00900090
#define CMD_ERASE_SETUP		0x00200020
#define CMD_ERASE_CONFIRM	0x00D000D0
#define CMD_PROGRAM		0x00400040
#define CMD_RESUME		0x00D000D0
#define CMD_SUSPEND		0x00B000B0
#define CMD_STATUS_READ		0x00700070
#define CMD_STATUS_RESET	0x00500050

#define BIT_BUSY		0x00800080
#define BIT_ERASE_SUSPEND	0x00400040
#define BIT_ERASE_ERROR		0x00200020
#define BIT_PROGRAM_ERROR	0x00100010
#define BIT_VPP_RANGE_ERROR	0x00080008
#define BIT_PROGRAM_SUSPEND	0x00040004
#define BIT_PROTECT_ERROR	0x00020002
#define BIT_UNDEFINED		0x00010001

#define BIT_SEQUENCE_ERROR	0x00300030
#define BIT_TIMEOUT		0x80000000

/*-----------------------------------------------------------------------
 */

ulong flash_init(void)
{
    int i, j;
    ulong size = 0;

    for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++)
    {
	ulong flashbase = 0;
	flash_info[i].flash_id =
	  (INTEL_MANUFACT & FLASH_VENDMASK) |
	  (INTEL_ID_28F160F3B & FLASH_TYPEMASK);
	flash_info[i].size = FLASH_BANK_SIZE;
	flash_info[i].sector_count = CONFIG_SYS_MAX_FLASH_SECT;
	memset(flash_info[i].protect, 0, CONFIG_SYS_MAX_FLASH_SECT);
	if (i == 0)
	  flashbase = PHYS_FLASH_1;
	else
	  panic("configured too many flash banks!\n");
	for (j = 0; j < flash_info[i].sector_count; j++)
	{
	    if (j <= 7)
	    {
		flash_info[i].start[j] = flashbase + j * PARAM_SECT_SIZE;
	    }
	    else
	    {
		flash_info[i].start[j] = flashbase + (j - 7)*MAIN_SECT_SIZE;
	    }
	}
	size += flash_info[i].size;
    }

    /* Protect monitor and environment sectors
     */
    flash_protect(FLAG_PROTECT_SET,
		  CONFIG_SYS_FLASH_BASE,
		  CONFIG_SYS_FLASH_BASE + monitor_flash_len - 1,
		  &flash_info[0]);

    flash_protect(FLAG_PROTECT_SET,
		  CONFIG_ENV_ADDR,
		  CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1,
		  &flash_info[0]);

    return size;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info  (flash_info_t *info)
{
    int i;

    switch (info->flash_id & FLASH_VENDMASK)
    {
    case (INTEL_MANUFACT & FLASH_VENDMASK):
	printf("Intel: ");
	break;
    default:
	printf("Unknown Vendor ");
	break;
    }

    switch (info->flash_id & FLASH_TYPEMASK)
    {
    case (INTEL_ID_28F160F3B & FLASH_TYPEMASK):
	printf("2x 28F160F3B (16Mbit)\n");
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

int flash_error (ulong code)
{
	/* Check bit patterns */
	/* SR.7=0 is busy, SR.7=1 is ready */
	/* all other flags indicate error on 1 */
	/* SR.0 is undefined */
	/* Timeout is our faked flag */

	/* sequence is described in Intel 290644-005 document */

	/* check Timeout */
	if (code & BIT_TIMEOUT)
	{
		printf ("Timeout\n");
		return ERR_TIMOUT;
	}

	/* check Busy, SR.7 */
	if (~code & BIT_BUSY)
	{
		printf ("Busy\n");
		return ERR_PROG_ERROR;
	}

	/* check Vpp low, SR.3 */
	if (code & BIT_VPP_RANGE_ERROR)
	{
		printf ("Vpp range error\n");
		return ERR_PROG_ERROR;
	}

	/* check Device Protect Error, SR.1 */
	if (code & BIT_PROTECT_ERROR)
	{
		printf ("Device protect error\n");
		return ERR_PROG_ERROR;
	}

	/* check Command Seq Error, SR.4 & SR.5 */
	if (code & BIT_SEQUENCE_ERROR)
	{
		printf ("Command seqence error\n");
		return ERR_PROG_ERROR;
	}

	/* check Block Erase Error, SR.5 */
	if (code & BIT_ERASE_ERROR)
	{
		printf ("Block erase error\n");
		return ERR_PROG_ERROR;
	}

	/* check Program Error, SR.4 */
	if (code & BIT_PROGRAM_ERROR)
	{
		printf ("Program error\n");
		return ERR_PROG_ERROR;
	}

	/* check Block Erase Suspended, SR.6 */
	if (code & BIT_ERASE_SUSPEND)
	{
		printf ("Block erase suspended\n");
		return ERR_PROG_ERROR;
	}

	/* check Program Suspended, SR.2 */
	if (code & BIT_PROGRAM_SUSPEND)
	{
		printf ("Program suspended\n");
		return ERR_PROG_ERROR;
	}

	/* OK, no error */
	return ERR_OK;
}

/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
    ulong result;
    int iflag, cflag, prot, sect;
    int rc = ERR_OK;

    /* first look for protection bits */

    if (info->flash_id == FLASH_UNKNOWN)
	return ERR_UNKNOWN_FLASH_TYPE;

    if ((s_first < 0) || (s_first > s_last)) {
	return ERR_INVAL;
    }

    if ((info->flash_id & FLASH_VENDMASK) !=
	(INTEL_MANUFACT & FLASH_VENDMASK)) {
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

	    *addr = PUZZLE_TO_FLASH(CMD_STATUS_RESET);
	    *addr = PUZZLE_TO_FLASH(CMD_ERASE_SETUP);
	    *addr = PUZZLE_TO_FLASH(CMD_ERASE_CONFIRM);

	    /* wait until flash is ready */
	    do
	    {
		/* check timeout */
		if (get_timer_masked() > CONFIG_SYS_FLASH_ERASE_TOUT)
		{
		    *addr = PUZZLE_TO_FLASH(CMD_SUSPEND);
		    result = BIT_TIMEOUT;
		    break;
		}

		result = PUZZLE_FROM_FLASH(*addr);
	    }  while (~result & BIT_BUSY);

	    *addr = PUZZLE_TO_FLASH(CMD_READ_ARRAY);

	    if ((rc = flash_error(result)) != ERR_OK)
		goto outahere;

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

static int write_word (flash_info_t *info, ulong dest, ulong data)
{
    vu_long *addr = (vu_long *)dest;
    ulong result;
    int rc = ERR_OK;
    int cflag, iflag;

    /* Check if Flash is (sufficiently) erased
     */
    result = PUZZLE_FROM_FLASH(*addr);
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

    *addr = PUZZLE_TO_FLASH(CMD_STATUS_RESET);
    *addr = PUZZLE_TO_FLASH(CMD_PROGRAM);
    *addr = data;

    /* arm simple, non interrupt dependent timer */
    reset_timer_masked();

    /* wait until flash is ready */
    do
    {
	/* check timeout */
	if (get_timer_masked() > CONFIG_SYS_FLASH_ERASE_TOUT)
	{
	    *addr = PUZZLE_TO_FLASH(CMD_SUSPEND);
	    result = BIT_TIMEOUT;
	    break;
	}

	result = PUZZLE_FROM_FLASH(*addr);
    }  while (~result & BIT_BUSY);

    *addr = PUZZLE_TO_FLASH(CMD_READ_ARRAY);

    rc = flash_error(result);

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
