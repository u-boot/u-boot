/*
 * (C) Copyright 2000
 * Marius Groeger <mgroeger@sysgo.de>
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Flash Routines for AMD 29F080B devices
 * Added support for 64bit and AMD 29DL323B
 *
 *--------------------------------------------------------------------
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc8xx.h>
#include <asm/io.h>

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

#define RD_SWP32(x) in_le32((volatile u32*)x)

/*-----------------------------------------------------------------------
 * Functions
 */

static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init(void)
{
	int i;

	/* Init: no FLASHes known */
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i)
		flash_info[i].flash_id = FLASH_UNKNOWN;

	/* for now, only support the 4 MB Flash SIMM */
	(void)flash_get_size((vu_long *) CONFIG_SYS_FLASH0_BASE,
			      &flash_info[0]);

	/*
	 * protect monitor and environment sectors
	 */

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH0_BASE
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE + monitor_flash_len - 1,
		      &flash_info[0]);
#endif

#if defined(CONFIG_ENV_IS_IN_FLASH) && defined(CONFIG_ENV_ADDR)
#ifndef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE	CONFIG_ENV_SECT_SIZE
#endif
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_ENV_ADDR,
		      CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1, &flash_info[0]);
#endif

	return CONFIG_SYS_FLASH0_SIZE * 1024 * 1024;
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
    case (AMD_MANUFACT & FLASH_VENDMASK):
	printf ("AMD ");
	break;
    case (FUJ_MANUFACT & FLASH_VENDMASK):
	printf ("FUJITSU ");
	break;
    case (SST_MANUFACT & FLASH_VENDMASK):
	printf ("SST ");
	break;
    default:
	printf ("Unknown Vendor ");
	break;
    }

    switch (info->flash_id & FLASH_TYPEMASK) {
    case (AMD_ID_DL323B & FLASH_TYPEMASK):
	printf("AM29DL323B (32 MBit)\n");
	break;
    default:
	printf ("Unknown Chip Type\n");
	break;
    }

    printf ("  Size: %ld MB in %d Sectors\n",
	    info->size >> 20, info->sector_count);

    printf ("  Sector Start Addresses:");
    for (i = 0; i < info->sector_count; ++i) {
	if ((i % 5) == 0) printf ("\n   ");
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

static ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
    short i;
    vu_long vendor[2], devid[2];
    ulong base = (ulong)addr;

    /* Reset and Write auto select command: read Manufacturer ID */
    addr[0] = 0xf0f0f0f0;
    addr[2 * 0x0555] = 0xAAAAAAAA;
    addr[2 * 0x02AA] = 0x55555555;
    addr[2 * 0x0555] = 0x90909090;
    addr[1] = 0xf0f0f0f0;
    addr[2 * 0x0555 + 1] = 0xAAAAAAAA;
    addr[2 * 0x02AA + 1] = 0x55555555;
    addr[2 * 0x0555 + 1] = 0x90909090;
    udelay (1000);

    vendor[0] = RD_SWP32(&addr[0]);
    vendor[1] = RD_SWP32(&addr[1]);
    if (vendor[0] != vendor[1] || vendor[0] != AMD_MANUFACT) {
	info->size = 0;
	goto out;
    }

    devid[0] = RD_SWP32(&addr[2]);
    devid[1] = RD_SWP32(&addr[3]);

    if (devid[0] == AMD_ID_DL323B) {
	/*
	* we have 2 Banks
	* Bank 1 (23 Sectors): 0-7=8kbyte, 8-22=64kbyte
	* Bank 2 (48 Sectors): 23-70=64kbyte
	*/
	info->flash_id     = (AMD_MANUFACT & FLASH_VENDMASK) |
			     (AMD_ID_DL323B & FLASH_TYPEMASK);
	info->sector_count = 71;
	info->size         = 4 * (8 * 8 + 63 * 64) * 1024;
    }
    else {
	info->size = 0;
	goto out;
    }

    /* set up sector start address table */
    for (i = 0; i < 8; i++) {
	info->start[i] = base + (i * 0x8000);
    }
    for (i = 8; i < info->sector_count; i++) {
	info->start[i] = base + (i * 0x40000) + 8 * 0x8000 - 8 * 0x40000;
    }

    /* check for protected sectors */
    for (i = 0; i < info->sector_count; i++) {
	/* read sector protection at sector address */
	addr = (volatile unsigned long *)(info->start[i]);
	addr[2 * 0x0555] = 0xAAAAAAAA;
	addr[2 * 0x02AA] = 0x55555555;
	addr[2 * 0x0555] = 0x90909090;
	addr[2 * 0x0555 + 1] = 0xAAAAAAAA;
	addr[2 * 0x02AA + 1] = 0x55555555;
	addr[2 * 0x0555 + 1] = 0x90909090;
	udelay (1000);
	base = RD_SWP32(&addr[4]);
	base |= RD_SWP32(&addr[5]);
	info->protect[i] = base & 0x00010001 ? 1 : 0;
    }
    addr = (vu_long*)info->start[0];

out:
    /* reset command */
    addr[0] = 0xf0f0f0f0;
    addr[1] = 0xf0f0f0f0;

    return info->size;
}


/*-----------------------------------------------------------------------
 */

int flash_erase (flash_info_t *info, int s_first, int s_last)
{
    vu_long *addr = (vu_long*)(info->start[0]);
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

    addr[2 * 0x0555] = 0xAAAAAAAA;
    addr[2 * 0x02AA] = 0x55555555;
    addr[2 * 0x0555] = 0x80808080;
    addr[2 * 0x0555] = 0xAAAAAAAA;
    addr[2 * 0x02AA] = 0x55555555;
    addr[2 * 0x0555 + 1] = 0xAAAAAAAA;
    addr[2 * 0x02AA + 1] = 0x55555555;
    addr[2 * 0x0555 + 1] = 0x80808080;
    addr[2 * 0x0555 + 1] = 0xAAAAAAAA;
    addr[2 * 0x02AA + 1] = 0x55555555;
    udelay (100);

    /* Start erase on unprotected sectors */
    for (sect = s_first; sect<=s_last; sect++) {
	if (info->protect[sect] == 0) {	/* not protected */
	    addr = (vu_long*)(info->start[sect]);
	    addr[0] = 0x30303030;
	    addr[1] = 0x30303030;
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
    addr = (vu_long*)(info->start[l_sect]);
    while (	(addr[0] & 0x80808080) != 0x80808080 ||
		(addr[1] & 0x80808080) != 0x80808080) {
	if ((now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
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
    addr = (volatile unsigned long *)info->start[0];
    addr[0] = 0xF0F0F0F0;	/* reset bank */
    addr[1] = 0xF0F0F0F0;	/* reset bank */

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
    vu_long *addr = (vu_long*)(info->start[0]);
    ulong start;
    int flag;

    /* Check if Flash is (sufficiently) erased */
    if ((*((vu_long *)dest) & data) != data) {
	return (2);
    }
    /* Disable interrupts which might cause a timeout here */
    flag = disable_interrupts();

    if ((dest & 0x00000004) == 0) {
	addr[2 * 0x0555] = 0xAAAAAAAA;
	addr[2 * 0x02AA] = 0x55555555;
	addr[2 * 0x0555] = 0xA0A0A0A0;
    }
    else {
	addr[2 * 0x0555 + 1] = 0xAAAAAAAA;
	addr[2 * 0x02AA + 1] = 0x55555555;
	addr[2 * 0x0555 + 1] = 0xA0A0A0A0;
    }

    *((vu_long *)dest) = data;

    /* re-enable interrupts if necessary */
    if (flag)
      enable_interrupts();

    /* data polling for D7 */
    start = get_timer (0);
    while ((*((vu_long *)dest) & 0x80808080) != (data & 0x80808080)) {
	if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
	    return (1);
	}
    }
    return (0);
}

/*-----------------------------------------------------------------------
 */
