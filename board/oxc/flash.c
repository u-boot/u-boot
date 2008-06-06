/*
 * (C) Copyright 2000
 * Marius Groeger <mgroeger@sysgo.de>
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Flash Routines for STM29W320DB/STM29W800D flash chips
 *
 *--------------------------------------------------------------------
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

flash_info_t	flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/*-----------------------------------------------------------------------
 * Functions
 */

static ulong flash_get_size (vu_char *addr, flash_info_t *info);
static int write_byte (flash_info_t *info, ulong dest, uchar data);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
    unsigned long size;
    int i;

    /* Init: no FLASHes known */
    for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
	flash_info[i].flash_id = FLASH_UNKNOWN;
    }

    /*
     * We use the following trick here: since flash is cyclically
     * mapped in the 0xFF800000-0xFFFFFFFF area, we detect the type
     * and the size of flash using 0xFF800000 as the base address,
     * and then call flash_get_size() again to fill flash_info.
     */
    size = flash_get_size((vu_char *)CFG_FLASH_PRELIMBASE, &flash_info[0]);
    if (size)
    {
	flash_get_size((vu_char *)(-size), &flash_info[0]);
    }

#if (CFG_MONITOR_BASE >= CFG_FLASH_PRELIMBASE)
    /* monitor protection ON by default */
    flash_protect(FLAG_PROTECT_SET,
		  CFG_MONITOR_BASE,
		  CFG_MONITOR_BASE+monitor_flash_len-1,
		  &flash_info[0]);
#endif

#if (CFG_ENV_IS_IN_FLASH == 1) && defined(CFG_ENV_ADDR)
# ifndef  CFG_ENV_SIZE
#  define CFG_ENV_SIZE	CFG_ENV_SECT_SIZE
# endif
    flash_protect(FLAG_PROTECT_SET,
		  CFG_ENV_ADDR,
		  CFG_ENV_ADDR + CFG_ENV_SIZE - 1,
		  &flash_info[0]);
#endif

    return (size);
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
    case FLASH_MAN_STM:
	printf ("ST ");
	break;
    default:
	printf ("Unknown Vendor ");
	break;
    }

    switch (info->flash_id & FLASH_TYPEMASK) {
    case FLASH_STM320DB:
	printf ("M29W320DB (32 Mbit)\n");
	break;
    case FLASH_STM800DB:
	printf ("M29W800DB (8 Mbit, bottom boot block)\n");
	break;
    case FLASH_STM800DT:
	printf ("M29W800DT (8 Mbit, top boot block)\n");
	break;
    default:
	printf ("Unknown Chip Type\n");
	break;
    }

    printf ("  Size: %ld KB in %d Sectors\n",
	    info->size >> 10, info->sector_count);

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

/*
 * The following code cannot be run from FLASH!
 */

static ulong flash_get_size (vu_char *addr, flash_info_t *info)
{
    short i;
    uchar vendor, devid;
    ulong base = (ulong)addr;

    /* Write auto select command: read Manufacturer ID */
    addr[0x0AAA] = 0xAA;
    addr[0x0555] = 0x55;
    addr[0x0AAA] = 0x90;

    udelay(1000);

    vendor = addr[0];
    devid = addr[2];

    /* only support STM */
    if ((vendor << 16) != FLASH_MAN_STM) {
	return 0;
    }

    if (devid == FLASH_STM320DB) {
	/* MPC8240 can address maximum 2Mb of flash, that is why the MSB
	 * lead is grounded and we can access only 2 first Mb */
	info->flash_id     = vendor << 16 | devid;
	info->sector_count = 32;
	info->size         = info->sector_count * 0x10000;
	for (i = 0; i < info->sector_count; i++) {
	    info->start[i] = base + i * 0x10000;
	}
    }
    else if (devid == FLASH_STM800DB) {
	info->flash_id     = vendor << 16 | devid;
	info->sector_count = 19;
	info->size         = 0x100000;
	info->start[0]     = 0x0000;
	info->start[1]     = 0x4000;
	info->start[2]     = 0x6000;
	info->start[3]     = 0x8000;
	for (i = 4; i < info->sector_count; i++) {
	    info->start[i] = base + (i-3) * 0x10000;
	}
    }
    else if (devid == FLASH_STM800DT) {
	info->flash_id     = vendor << 16 | devid;
	info->sector_count = 19;
	info->size         = 0x100000;
	for (i = 0; i < info->sector_count-4; i++) {
	    info->start[i] = base + i * 0x10000;
	}
	info->start[i]     = base + i * 0x10000;
	info->start[i+1]   = base + i * 0x10000 + 0x8000;
	info->start[i+2]   = base + i * 0x10000 + 0xa000;
	info->start[i+3]   = base + i * 0x10000 + 0xc000;
    }
    else {
	return 0;
    }

    /* mark all sectors as unprotected */
    for (i = 0; i < info->sector_count; i++) {
	info->protect[i] = 0;
    }

    /* Issue the reset command */
    if (info->flash_id != FLASH_UNKNOWN) {
	addr[0] = 0xF0;	/* reset bank */
    }

    return (info->size);
}


/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
    vu_char *addr = (vu_char *)(info->start[0]);
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

    prot = 0;
    for (sect = s_first; sect <= s_last; sect++) {
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

    addr[0x0AAA] = 0xAA;
    addr[0x0555] = 0x55;
    addr[0x0AAA] = 0x80;
    addr[0x0AAA] = 0xAA;
    addr[0x0555] = 0x55;

    /* wait at least 80us - let's wait 1 ms */
    udelay (1000);

    /* Start erase on unprotected sectors */
    for (sect = s_first; sect<=s_last; sect++) {
	if (info->protect[sect] == 0) {	/* not protected */
	    addr = (vu_char *)(info->start[sect]);
	    addr[0] = 0x30;
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
    addr = (vu_char *)(info->start[l_sect]);
    while ((addr[0] & 0x80) != 0x80) {
	if ((now = get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
	    printf ("Timeout\n");
	    return 1;
	}
	/* show that we're waiting */
	if ((now - last) > 1000) {	/* every second */
	    serial_putc ('.');
	    last = now;
	}
    }

    DONE:
    /* reset to read mode */
    addr = (volatile unsigned char *)info->start[0];
    addr[0] = 0xF0;	/* reset bank */

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
    int rc;

    while (cnt > 0) {
	if ((rc = write_byte(info, addr, *src)) != 0) {
	    return (rc);
	}
	addr++;
	src++;
	cnt--;
    }

    return (0);
}

/*-----------------------------------------------------------------------
 * Write a byte to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_byte (flash_info_t *info, ulong dest, uchar data)
{
    vu_char *addr = (vu_char *)(info->start[0]);
    ulong start;
    int flag;

    /* Check if Flash is (sufficiently) erased */
    if ((*((vu_char *)dest) & data) != data) {
	return (2);
    }
    /* Disable interrupts which might cause a timeout here */
    flag = disable_interrupts();

    addr[0x0AAA] = 0xAA;
    addr[0x0555] = 0x55;
    addr[0x0AAA] = 0xA0;

    *((vu_char *)dest) = data;

    /* re-enable interrupts if necessary */
    if (flag)
      enable_interrupts();

    /* data polling for D7 */
    start = get_timer (0);
    while ((*((vu_char *)dest) & 0x80) != (data & 0x80)) {
	if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
	    return (1);
	}
    }
    return (0);
}

/*-----------------------------------------------------------------------
 */
