/*
 * (C) Copyright 2001
 * Erik Theisen, Wave 7 Optics, etheisen@mindspring.com.
 *  Based on code by:
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
#include <ppc4xx.h>
#include <asm/processor.h>

#include <watchdog.h>

flash_info_t    flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips    */

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static int write_word8(flash_info_t *info, ulong dest, ulong data);
static int write_word32 (flash_info_t *info, ulong dest, ulong data);
static void flash_get_offsets (ulong base, flash_info_t *info);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
    int i;
    unsigned long size_b0, base_b0;
    unsigned long size_b1, base_b1;

    /* Init: no FLASHes known */
    for (i = 0; i < CFG_MAX_FLASH_BANKS; ++i) {
	flash_info[i].flash_id = FLASH_UNKNOWN;
    }

    /* Get Size of Boot and Main Flashes */
    size_b0 = flash_get_size((vu_long *)FLASH_BASE0_PRELIM, &flash_info[0]);
    if (flash_info[0].flash_id == FLASH_UNKNOWN) {
	printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
	    size_b0, size_b0<<20);
	return 0;
    }
    size_b1 = flash_get_size((vu_long *)FLASH_BASE1_PRELIM, &flash_info[1]);
    if (flash_info[1].flash_id == FLASH_UNKNOWN) {
	printf ("## Unknown FLASH on Bank 1 - Size = 0x%08lx = %ld MB\n",
	    size_b1, size_b1<<20);
	return 0;
    }

    /* Calculate base addresses */
    base_b0 = -size_b0;
    base_b1 = -size_b1;

    /* Setup offsets for Boot Flash */
    flash_get_offsets (base_b0, &flash_info[0]);

    /* Protect board level data */
    (void)flash_protect(FLAG_PROTECT_SET,
			base_b0,
			flash_info[0].start[1] - 1,
			&flash_info[0]);


    /* Monitor protection ON by default */
    (void)flash_protect(FLAG_PROTECT_SET,
			base_b0 + size_b0 - monitor_flash_len,
			base_b0 + size_b0 - 1,
			&flash_info[0]);

    /* Protect the FPGA image */
    (void)flash_protect(FLAG_PROTECT_SET,
			FLASH_BASE1_PRELIM,
			FLASH_BASE1_PRELIM + CFG_FPGA_IMAGE_LEN - 1,
			&flash_info[1]);

    /* Protect the default boot image */
    (void)flash_protect(FLAG_PROTECT_SET,
			FLASH_BASE1_PRELIM + CFG_FPGA_IMAGE_LEN,
			FLASH_BASE1_PRELIM + CFG_FPGA_IMAGE_LEN + 0x600000 - 1,
			&flash_info[1]);

    /* Setup offsets for Main Flash */
    flash_get_offsets (FLASH_BASE1_PRELIM, &flash_info[1]);

    return (size_b0 + size_b1);
} /* end flash_init() */

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets (ulong base, flash_info_t *info)
{
    int i;

    /* set up sector start address table - FOR BOOT ROM ONLY!!! */
    if ((info->flash_id & FLASH_TYPEMASK)  == FLASH_AM040) {
	for (i = 0; i < info->sector_count; i++)
	    info->start[i] = base + (i * 0x00010000);
    }
} /* end flash_get_offsets() */

/*-----------------------------------------------------------------------
 */
void flash_print_info  (flash_info_t *info)
{
    int i;
    int k;
    int size;
    int erased;
    volatile unsigned long *flash;

    if (info->flash_id == FLASH_UNKNOWN) {
	printf ("missing or unknown FLASH type\n");
	return;
    }

    switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:     printf ("1 x AMD ");    break;
	case FLASH_MAN_STM:	printf ("1 x STM ");	break;
	case FLASH_MAN_INTEL:   printf ("2 x Intel ");  break;
	default:                printf ("Unknown Vendor ");
    }

    switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM040:
	    if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_AMD)
		printf ("AM29LV040 (4096 Kbit, uniform sector size)\n");
	    else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_STM)
		printf ("M29W040B (4096 Kbit, uniform block size)\n");
	    else
		printf ("UNKNOWN 29x040x (4096 Kbit, uniform sector size)\n");
	    break;
	case FLASH_28F320J3A:
	    printf ("28F320J3A (32 Mbit = 128K x 32)\n");
	    break;
	case FLASH_28F640J3A:
	    printf ("28F640J3A (64 Mbit = 128K x 64)\n");
	    break;
	case FLASH_28F128J3A:
	    printf ("28F128J3A (128 Mbit = 128K x 128)\n");
	    break;
	default:
	    printf ("Unknown Chip Type\n");
    }

    if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_STM) {
	printf ("  Size: %ld KB in %d Blocks\n",
		info->size >> 10, info->sector_count);
    } else {
	printf ("  Size: %ld KB in %d Sectors\n",
		info->size >> 10, info->sector_count);
    }

    printf ("  Sector Start Addresses:");
    for (i=0; i<info->sector_count; ++i) {
	/*
	 * Check if whole sector is erased
	 */
	if (i != (info->sector_count-1))
	    size = info->start[i+1] - info->start[i];
	else
	    size = info->start[0] + info->size - info->start[i];
	erased = 1;
	flash = (volatile unsigned long *)info->start[i];
	size = size >> 2;        /* divide by 4 for longword access */
	for (k=0; k<size; k++)
	{
	    if (*flash++ != 0xffffffff)
	    {
		erased = 0;
		break;
	    }
	}

	if ((i % 5) == 0)
	    printf ("\n   ");
	printf (" %08lX%s%s",
	    info->start[i],
	    erased ? " E" : "  ",
	    info->protect[i] ? "RO " : "   "
	);
    }
    printf ("\n");
} /* end flash_print_info() */

/*
 * The following code cannot be run from FLASH!
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
    short i;
    ulong base = (ulong)addr;

    /* Setup default type */
    info->flash_id = FLASH_UNKNOWN;
    info->sector_count =0;
    info->size = 0;

    /* Test for Boot Flash */
    if (base == FLASH_BASE0_PRELIM) {
	unsigned char value;
	volatile unsigned char * addr2 = (unsigned char *)addr;

	/* Write auto select command: read Manufacturer ID */
	*(addr2 + 0x555) = 0xaa;
	*(addr2 + 0x2aa) = 0x55;
	*(addr2 + 0x555) = 0x90;

	/* Manufacture ID */
	value = *addr2;
	switch (value) {
	    case (unsigned char)AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
		break;
	    case (unsigned char)STM_MANUFACT:
		info->flash_id = FLASH_MAN_STM;
		break;
	    default:
		*addr2 = 0xf0;              /* no or unknown flash  */
		return 0;
	}

	/* Device ID */
	value = *(addr2 + 1);
	switch (value) {
	    case (unsigned char)AMD_ID_LV040B:
	    case (unsigned char)STM_ID_29W040B:
		info->flash_id += FLASH_AM040;
		info->sector_count = 8;
		info->size = 0x00080000;
		break;                       /* => 512Kb */
	    default:
		*addr2 = 0xf0;               /* => no or unknown flash */
		return 0;
	}
    }
    else { /* MAIN Flash */
	unsigned long value;
	volatile unsigned long * addr2 = (unsigned long *)addr;

	/* Write auto select command: read Manufacturer ID */
	*addr2 = 0x90909090;

	/* Manufacture ID */
	value = *addr2;
	switch (value) {
	    case (unsigned long)INTEL_MANUFACT:
		info->flash_id = FLASH_MAN_INTEL;
		break;
	    default:
		*addr2 = 0xff;              /* no or unknown flash  */
		return 0;
	}

	/* Device ID - This shit is interleaved... */
	value = *(addr2 + 1);
	switch (value) {
	    case (unsigned long)INTEL_ID_28F320J3A:
		info->flash_id += FLASH_28F320J3A;
		info->sector_count = 32;
		info->size = 0x00400000 * 2;
		break;                       /* => 2 X 4 MB */
	    case (unsigned long)INTEL_ID_28F640J3A:
		info->flash_id += FLASH_28F640J3A;
		info->sector_count = 64;
		info->size = 0x00800000 * 2;
		break;                       /* => 2 X 8 MB */
	    case (unsigned long)INTEL_ID_28F128J3A:
		info->flash_id += FLASH_28F128J3A;
		info->sector_count = 128;
		info->size = 0x01000000 * 2;
		break;                       /* => 2 X 16 MB */
	    default:
		*addr2 = 0xff;               /* => no or unknown flash */
	}
    }

    /* Make sure we don't exceed CFG_MAX_FLASH_SECT */
    if (info->sector_count > CFG_MAX_FLASH_SECT) {
	printf ("** ERROR: sector count %d > max (%d) **\n",
		info->sector_count, CFG_MAX_FLASH_SECT);
	info->sector_count = CFG_MAX_FLASH_SECT;
    }

    /* set up sector start address table */
    switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM040:
	    for (i = 0; i < info->sector_count; i++)
		info->start[i] = base + (i * 0x00010000);
	    break;
	case FLASH_28F320J3A:
	case FLASH_28F640J3A:
	case FLASH_28F128J3A:
	    for (i = 0; i < info->sector_count; i++)
		info->start[i] = base + (i * 0x00020000 * 2); /* 2 Banks */
	    break;
    }

    /* Test for Boot Flash */
    if (base == FLASH_BASE0_PRELIM) {
	volatile unsigned char *addr2;
	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
	    /* read sector protection at sector address, (AX .. A0) = 0x02 */
	    /* D0 = 1 if protected */
	    addr2 = (volatile unsigned char *)(info->start[i]);
	    info->protect[i] = *(addr2 + 2) & 1;
	}

	/* Restore read mode */
	*(unsigned char *)base = 0xF0;       /* Reset NORMAL Flash */
    }
    else { /* Main Flash */
	volatile unsigned long *addr2;
	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
	    /* read sector protection at sector address, (AX .. A0) = 0x02 */
	    /* D0 = 1 if protected */
	    addr2 = (volatile unsigned long *)(info->start[i]);
	    info->protect[i] = *(addr2 + 2) & 0x1;
	}

	/* Restore read mode */
	*(unsigned long *)base = 0xFFFFFFFF; /* Reset  Flash */
    }

    return (info->size);
} /* end flash_get_size() */

/*-----------------------------------------------------------------------
 */

static int wait_for_DQ7(ulong addr, uchar cmp_val, ulong tout)
{
    int i;

    volatile uchar *vaddr =  (uchar *)addr;

    /* Loop X times */
    for (i = 1; i <= (100 * tout); i++) {    /* Wait up to tout ms */
	udelay(10);
	/* Pause 10 us */

	/* Check for completion */
	if ((vaddr[0] & 0x80) == (cmp_val & 0x80)) {
	    return 0;
	}

	/* KEEP THE LUSER HAPPY - Print a dot every 1.1 seconds */
	if (!(i % 110000))
	    putc('.');

	/* Kick the dog if needed */
	WATCHDOG_RESET();
    }

    return 1;
} /* wait_for_DQ7() */

/*-----------------------------------------------------------------------
 */

static int flash_erase8(flash_info_t *info, int s_first, int s_last)
{
    int tcode, rcode = 0;
    volatile uchar *addr = (uchar *)(info->start[0]);
    volatile uchar *sector_addr;
    int flag, prot, sect;

    /* Validate arguments */
    if ((s_first < 0) || (s_first > s_last)) {
	if (info->flash_id == FLASH_UNKNOWN)
	    printf ("- missing\n");
	else
	    printf ("- no sectors to erase\n");
	return 1;
    }

    /* Check for KNOWN flash type */
    if (info->flash_id == FLASH_UNKNOWN) {
	printf ("Can't erase unknown flash type - aborted\n");
	return 1;
    }

    /* Check for protected sectors */
    prot = 0;
    for (sect = s_first; sect <= s_last; ++sect) {
	if (info->protect[sect])
	    prot++;
    }
    if (prot)
	printf ("- Warning: %d protected sectors will not be erased!\n", prot);
    else
	printf ("\n");

    /* Start erase on unprotected sectors */
    for (sect = s_first; sect <= s_last; sect++) {
	if (info->protect[sect] == 0) {      /* not protected */
	    sector_addr = (uchar *)(info->start[sect]);

		if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_STM)
		    printf("Erasing block %p\n", sector_addr);
		else
		    printf("Erasing sector %p\n", sector_addr);

	    /* Disable interrupts which might cause Flash to timeout */
	    flag = disable_interrupts();

	    *(addr + 0x555) = (uchar)0xAA;
	    *(addr + 0x2aa) = (uchar)0x55;
	    *(addr + 0x555) = (uchar)0x80;
	    *(addr + 0x555) = (uchar)0xAA;
	    *(addr + 0x2aa) = (uchar)0x55;
	    *sector_addr = (uchar)0x30;      /* sector erase */

	    /*
	     * Wait for each sector to complete, it's more
	     * reliable.  According to AMD Spec, you must
	     * issue all erase commands within a specified
	     * timeout.  This has been seen to fail, especially
	     * if printf()s are included (for debug)!!
	     * Takes up to 6 seconds.
	     */
	    tcode  = wait_for_DQ7((ulong)sector_addr, 0x80, 6000);

	    /* re-enable interrupts if necessary */
	    if (flag)
		enable_interrupts();

	    /* Make sure we didn't timeout */
	    if (tcode) {
		printf ("Timeout\n");
		rcode = 1;
	    }
	}
    }

    /* wait at least 80us - let's wait 1 ms */
    udelay (1000);

    /* reset to read mode */
    addr = (uchar *)info->start[0];
    *addr = (uchar)0xF0;                     /* reset bank */

    printf (" done\n");
    return rcode;
} /* end flash_erase8() */

static int flash_erase32(flash_info_t *info, int s_first, int s_last)
{
    int flag, sect;
    ulong start, now, last;
    int prot = 0;

    /* Validate arguments */
    if ((s_first < 0) || (s_first > s_last)) {
	if (info->flash_id == FLASH_UNKNOWN)
	    printf ("- missing\n");
	else
	    printf ("- no sectors to erase\n");
	return 1;
    }

    /* Check for KNOWN flash type */
    if ((info->flash_id & FLASH_VENDMASK) != FLASH_MAN_INTEL) {
	printf ("Can erase only Intel flash types - aborted\n");
	return 1;
    }

    /* Check for protected sectors */
    for (sect = s_first; sect <= s_last; ++sect) {
	if (info->protect[sect])
	    prot++;
    }
    if (prot)
	printf ("- Warning: %d protected sectors will not be erased!\n", prot);
    else
	printf ("\n");

    start = get_timer (0);
    last  = start;
    /* Start erase on unprotected sectors */
    for (sect = s_first; sect <= s_last; sect++) {
	WATCHDOG_RESET();
	if (info->protect[sect] == 0) {      /* not protected */
	    vu_long *addr = (vu_long *)(info->start[sect]);
	    unsigned long status;

	    /* Disable interrupts which might cause a timeout here */
	    flag = disable_interrupts();

	    *addr = 0x00500050;              /* clear status register */
	    *addr = 0x00200020;              /* erase setup */
	    *addr = 0x00D000D0;              /* erase confirm */

	    /* re-enable interrupts if necessary */
	    if (flag)
		enable_interrupts();

	    /* Wait at least 80us - let's wait 1 ms */
	    udelay (1000);

	    while (((status = *addr) & 0x00800080) != 0x00800080) {
		if ((now = get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
		    printf ("Timeout\n");
		    *addr = 0x00B000B0;      /* suspend erase      */
		    *addr = 0x00FF00FF;      /* reset to read mode */
		    return 1;
		}

		/* show that we're waiting */
		if ((now - last) > 990) {   /* every second */
		    putc ('.');
		    last = now;
		}
	    }
	    *addr = 0x00FF00FF;              /* reset to read mode */
	}
    }
    printf (" done\n");
    return 0;
} /* end flash_erase32() */

int flash_erase(flash_info_t *info, int s_first, int s_last)
{
    if ((info->flash_id & FLASH_TYPEMASK) == FLASH_AM040)
	return flash_erase8(info, s_first, s_last);
    else
	return flash_erase32(info, s_first, s_last);
} /* end flash_erase() */

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_buff8(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
    ulong cp, wp, data;
    ulong start;
    int i, l, rc;

    start = get_timer (0);

    wp = (addr & ~3);                        /* get lower word
						aligned address */

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

	if ((rc = write_word8(info, wp, data)) != 0) {
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
	if ((rc = write_word8(info, wp, data)) != 0) {
	    return (rc);
	}
	wp  += 4;
	cnt -= 4;
	if (get_timer(start) > 1000) {   /* every second */
	   WATCHDOG_RESET();
	   putc ('.');
	   start = get_timer(0);
	}
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

    return (write_word8(info, wp, data));
} /* end write_buff8() */

#define	FLASH_WIDTH	4	/* flash bus width in bytes */
static int write_buff32 (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong cp, wp, data;
	int i, l, rc;
	ulong start;

	start = get_timer (0);

	if (info->flash_id == FLASH_UNKNOWN) {
		return 4;
	}

	wp = (addr & ~(FLASH_WIDTH-1));	/* get lower FLASH_WIDTH aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i=0, cp=wp; i<l; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}
		for (; i<FLASH_WIDTH && cnt>0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt==0 && i<FLASH_WIDTH; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}

		if ((rc = write_word32(info, wp, data)) != 0) {
			return (rc);
		}
		wp += FLASH_WIDTH;
	}

	/*
	 * handle FLASH_WIDTH aligned part
	 */
	while (cnt >= FLASH_WIDTH) {
		data = 0;
		for (i=0; i<FLASH_WIDTH; ++i) {
			data = (data << 8) | *src++;
		}
		if ((rc = write_word32(info, wp, data)) != 0) {
			return (rc);
		}
		wp  += FLASH_WIDTH;
		cnt -= FLASH_WIDTH;
	  if (get_timer(start) > 990) {   /* every second */
			putc ('.');
			start = get_timer(0);
		}
	}

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i=0, cp=wp; i<FLASH_WIDTH && cnt>0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i<FLASH_WIDTH; ++i, ++cp) {
		data = (data << 8) | (*(uchar *)cp);
	}

	return (write_word32(info, wp, data));
} /* write_buff32() */

int write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
    int retval;

    if ((info->flash_id & FLASH_TYPEMASK) == FLASH_AM040)
	retval = write_buff8(info, src, addr, cnt);
    else
	retval = write_buff32(info, src, addr, cnt);

    return retval;
} /* end write_buff() */

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

static int write_word8(flash_info_t *info, ulong dest, ulong data)
{
    volatile uchar *addr2 = (uchar *)(info->start[0]);
    volatile uchar *dest2 = (uchar *)dest;
    volatile uchar *data2 = (uchar *)&data;
    int flag;
    int i, tcode, rcode = 0;

    /* Check if Flash is (sufficently) erased */
    if ((*((volatile uchar *)dest) &
	(uchar)data) != (uchar)data) {
	return (2);
    }

    for (i=0; i < (4 / sizeof(uchar)); i++) {
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	*(addr2 + 0x555) = (uchar)0xAA;
	*(addr2 + 0x2aa) = (uchar)0x55;
	*(addr2 + 0x555) = (uchar)0xA0;

	dest2[i] = data2[i];

	/* Wait for write to complete, up to 1ms */
	tcode = wait_for_DQ7((ulong)&dest2[i], data2[i], 1);

	/* re-enable interrupts if necessary */
	if (flag)
	    enable_interrupts();

	/* Make sure we didn't timeout */
	if (tcode) {
	    rcode = 1;
	}
    }

    return rcode;
} /* end write_word8() */

static int write_word32(flash_info_t *info, ulong dest, ulong data)
{
    vu_long *addr = (vu_long *)dest;
    ulong status;
    ulong start;
    int flag;

    /* Check if Flash is (sufficiently) erased */
    if ((*addr & data) != data) {
	return (2);
    }
    /* Disable interrupts which might cause a timeout here */
    flag = disable_interrupts();

    *addr = 0x00400040;                      /* write setup */
    *addr = data;

    /* re-enable interrupts if necessary */
    if (flag)
	enable_interrupts();

    start = get_timer (0);

    while (((status = *addr) & 0x00800080) != 0x00800080) {
	WATCHDOG_RESET();
	if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
	    *addr = 0x00FF00FF;              /* restore read mode */
	    return (1);
	}
    }

    *addr = 0x00FF00FF;                      /* restore read mode */

    return (0);
} /* end write_word32() */


static int _flash_protect(flash_info_t *info, long sector)
{
    int i;
    int flag;
    ulong status;
    int rcode = 0;
    volatile long *addr = (long *)sector;

    switch(info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F320J3A:
	case FLASH_28F640J3A:
	case FLASH_28F128J3A:
	    /* Disable interrupts which might cause Flash to timeout */
	    flag = disable_interrupts();

	    /* Issue command */
	    *addr = 0x00500050L;             /* Clear the status register */
	    *addr = 0x00600060L;             /* Set lock bit setup */
	    *addr = 0x00010001L;             /* Set lock bit confirm */

	    /* Wait for command completion */
	    for (i = 0; i < 10; i++) {       /* 75us timeout, wait 100us */
		udelay(10);
		if ((*addr & 0x00800080L) == 0x00800080L)
		    break;
	    }

	    /* Not successful? */
	    status = *addr;
	    if (status != 0x00800080L) {
		printf("Protect %x sector failed: %x\n",
		       (uint)sector, (uint)status);
		rcode = 1;
	    }

	    /* Restore read mode */
	    *addr = 0x00ff00ffL;

	    /* re-enable interrupts if necessary */
	    if (flag)
		enable_interrupts();

	    break;
	case FLASH_AM040:                    /* No soft sector protection */
	    break;
    }

    /* Turn protection on for this sector */
    for (i = 0; i < info->sector_count; i++) {
	if (info->start[i] == sector) {
	    info->protect[i] = 1;
	    break;
	}
    }

    return rcode;
} /* end _flash_protect() */

static int _flash_unprotect(flash_info_t *info, long sector)
{
    int i;
    int flag;
    ulong status;
    int rcode = 0;
    volatile long *addr = (long *)sector;

    switch(info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F320J3A:
	case FLASH_28F640J3A:
	case FLASH_28F128J3A:
	    /* Disable interrupts which might cause Flash to timeout */
	    flag = disable_interrupts();

	    *addr = 0x00500050L;             /* Clear the status register */
	    *addr = 0x00600060L;             /* Clear lock bit setup */
	    *addr = 0x00D000D0L;             /* Clear lock bit confirm */

	    /* Wait for command completion */
	    for (i = 0; i < 80 ; i++) {      /* 700ms timeout, wait 800 */
		udelay(10000);               /* Delay 10ms */
		if ((*addr & 0x00800080L) == 0x00800080L)
		    break;
	    }

	    /* Not successful? */
	    status = *addr;
	    if (status != 0x00800080L) {
		printf("Un-protect %x sector failed: %x\n",
		       (uint)sector, (uint)status);
		*addr = 0x00ff00ffL;
		rcode = 1;
	    }

	    /* restore read mode */
	    *addr = 0x00ff00ffL;

	    /* re-enable interrupts if necessary */
	    if (flag)
		enable_interrupts();

	    break;
	case FLASH_AM040:                    /* No soft sector protection */
	    break;
    }

    /*
     * Fix Intel's little red wagon.  Reprotect
     * sectors that were protected before we undid
     * protection on a specific sector.
     */
    for (i = 0; i < info->sector_count; i++) {
	if (info->start[i] != sector) {
	    if (info->protect[i]) {
		if (_flash_protect(info, info->start[i]))
		    rcode = 1;
	    }
	}
	else /* Turn protection off for this sector */
	    info->protect[i] = 0;
    }

    return rcode;
} /* end _flash_unprotect() */


int flash_real_protect(flash_info_t *info, long sector, int prot)
{
    int rcode;

    if (prot)
	rcode = _flash_protect(info, info->start[sector]);
    else
	rcode = _flash_unprotect(info, info->start[sector]);

    return rcode;
} /* end flash_real_protect() */

/*-----------------------------------------------------------------------
 */
