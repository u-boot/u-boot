/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * 07-10-2002 Frank Gottschling: added 29F032 flash (ELPPC).
 *        fixed monitor protection part
 *
 * 09-18-2001 Andreas Heppel: Reduced the code in here to the usage
 *        of AMD's 29F040 and 29F016 flashes, since the BAB7xx does use
 *        any other.
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/pci_io.h>

flash_info_t    flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips */

ulong flash_get_size (vu_long *addr, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);

/*flash command address offsets*/

#define ADDR0           (0x555)
#define ADDR1           (0x2AA)
#define ADDR3           (0x001)

#define FLASH_WORD_SIZE unsigned char

/*----------------------------------------------------------------------------*/

unsigned long flash_init (void)
{
    unsigned long size1, size2;
    int i;

    /* Init: no FLASHes known */
    for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i)
    {
	flash_info[i].flash_id = FLASH_UNKNOWN;
    }

    /* initialise 1st flash */
    size1 = flash_get_size((vu_long *)FLASH_BASE0_PRELIM, &flash_info[0]);

    if (flash_info[0].flash_id == FLASH_UNKNOWN)
    {
	printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
	    size1, size1<<20);
    }

    /* initialise 2nd flash */
    size2 = flash_get_size((vu_long *)FLASH_BASE1_PRELIM, &flash_info[1]);

    if (flash_info[1].flash_id == FLASH_UNKNOWN)
    {
	printf ("## Unknown FLASH on Bank 1 - Size = 0x%08lx = %ld MB\n",
	    size2, size2<<20);
    }

    /* monitor protection ON by default */
    if (size1 == 512*1024)
    {
	(void)flash_protect(FLAG_PROTECT_SET,
		FLASH_BASE0_PRELIM,
		FLASH_BASE0_PRELIM+monitor_flash_len-1,
		&flash_info[0]);
    }
    if (size2 == 512*1024)
    {
	(void)flash_protect(FLAG_PROTECT_SET,
		FLASH_BASE1_PRELIM,
		FLASH_BASE1_PRELIM+monitor_flash_len-1,
		&flash_info[1]);
    }
    if (size2 == 4*1024*1024)
    {
	(void)flash_protect(FLAG_PROTECT_SET,
		CONFIG_SYS_FLASH_BASE,
		CONFIG_SYS_FLASH_BASE+monitor_flash_len-1,
		&flash_info[1]);
    }

    return (size1 + size2);
}

/*----------------------------------------------------------------------------*/

void flash_print_info  (flash_info_t *info)
{
    int i;
    int k;
    int size;
    int erased;
    volatile unsigned long *flash;

    if (info->flash_id == FLASH_UNKNOWN) {
	printf ("missing or unknown FLASH type\n");
	flash_init();
    }

    if (info->flash_id == FLASH_UNKNOWN) {
	printf ("missing or unknown FLASH type\n");
	return;
    }

    switch (info->flash_id & FLASH_VENDMASK) {
    case FLASH_MAN_AMD:
	printf ("AMD ");
	break;
    default:
	printf ("Unknown Vendor ");
	break;
    }

    switch (info->flash_id & FLASH_TYPEMASK) {
    case AMD_ID_F040B:
	printf ("AM29F040B (4 Mbit)\n");
	break;
    case AMD_ID_F016D:
	printf ("AM29F016D (16 Mbit)\n");
	break;
    case AMD_ID_F032B:
	printf ("AM29F032B (32 Mbit)\n");
	break;
   default:
	printf ("Unknown Chip Type\n");
	break;
    }

    if (info->size >= (1 << 20)) {
	printf ("  Size: %ld MB in %d Sectors\n", info->size >> 20, info->sector_count);
    } else {
	printf ("  Size: %ld kB in %d Sectors\n", info->size >> 10, info->sector_count);
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
	for (k=0; k<size; k++) {
	    if (*flash++ != 0xffffffff) {
		erased = 0;
		break;
	    }
	}

	if ((i % 5) == 0)
	    printf ("\n   ");

	printf (" %08lX%s%s",
	    info->start[i],
	    erased ? " E" : "  ",
	    info->protect[i] ? "RO " : "   ");
    }
    printf ("\n");
}

/*----------------------------------------------------------------------------*/
/*
 * The following code cannot be run from FLASH!
 */
ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
    short i;
    ulong vendor, devid;
    ulong base = (ulong)addr;
    volatile unsigned char *caddr = (unsigned char *)addr;

#ifdef DEBUG
    printf("flash_get_size for address 0x%lx: \n", (unsigned long)caddr);
#endif

    /* Write auto select command: read Manufacturer ID */
    caddr[0] = 0xF0;   /* reset bank */
    udelay(10);

    eieio();
    caddr[0x555] = 0xAA;
    udelay(10);
    caddr[0x2AA] = 0x55;
    udelay(10);
    caddr[0x555] = 0x90;

    udelay(10);

    vendor = caddr[0];
    devid = caddr[1];

#ifdef DEBUG
    printf("Manufacturer: 0x%lx\n", vendor);
#endif

    vendor &= 0xff;
    devid &= 0xff;

    /* We accept only two AMD types */
    switch (vendor) {
    case (FLASH_WORD_SIZE)AMD_MANUFACT:
	info->flash_id = FLASH_MAN_AMD;
	break;
    default:
	info->flash_id = FLASH_UNKNOWN;
	info->sector_count = 0;
	info->size = 0;
	return (0);         /* no or unknown flash  */
    }

    switch (devid) {
    case (FLASH_WORD_SIZE)AMD_ID_F040B:
	info->flash_id |= AMD_ID_F040B;
	info->sector_count = 8;
	info->size = 0x00080000;
	break;              /* => 0.5 MB      */

    case (FLASH_WORD_SIZE)AMD_ID_F016D:
	info->flash_id |= AMD_ID_F016D;
	info->sector_count = 32;
	info->size         = 0x00200000;
	break;              /* => 2 MB      */

    case (FLASH_WORD_SIZE)AMD_ID_F032B:
	info->flash_id |= AMD_ID_F032B;
	info->sector_count = 64;
	info->size         = 0x00400000;
	break;              /* => 4 MB      */

    default:
	info->flash_id = FLASH_UNKNOWN;
	return (0);         /* => no or unknown flash */

    }

#ifdef DEBUG
    printf("flash id 0x%lx; sector count 0x%x, size 0x%lx\n", info->flash_id, info->sector_count, info->size);
#endif

    /* check for protected sectors */
    for (i = 0; i < info->sector_count; i++) {
	/* sector base address */
	info->start[i] = base + i * (info->size / info->sector_count);
	/* read sector protection at sector address, (A7 .. A0) = 0x02 */
	/* D0 = 1 if protected */
	caddr = (volatile unsigned char *)(info->start[i]);
	info->protect[i] = caddr[2] & 1;
    }

    /*
     * Prevent writes to uninitialized FLASH.
     */
    if (info->flash_id != FLASH_UNKNOWN) {
	caddr = (volatile unsigned char *)info->start[0];
	caddr[0] = 0xF0;   /* reset bank */
    }

    return (info->size);
}

/*----------------------------------------------------------------------------*/

int flash_erase (flash_info_t *info, int s_first, int s_last)
{
    volatile FLASH_WORD_SIZE *addr = (FLASH_WORD_SIZE *)(info->start[0]);
    int flag, prot, sect, l_sect;
    ulong start, now, last;
    int rc = 0;

    if ((s_first < 0) || (s_first > s_last)) {
	if (info->flash_id == FLASH_UNKNOWN) {
	    printf ("- missing\n");
	} else {
	    printf ("- no sectors to erase\n");
	}
	return 1;
    }

    if ((info->flash_id == FLASH_UNKNOWN) ||
	(info->flash_id > FLASH_AMD_COMP)) {
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

    addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
    addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
    addr[ADDR0] = (FLASH_WORD_SIZE)0x00800080;
    addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
    addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;

    /* Start erase on unprotected sectors */
    for (sect = s_first; sect<=s_last; sect++) {
	if (info->protect[sect] == 0) { /* not protected */
	    addr = (FLASH_WORD_SIZE *)(info->start[sect]);
	    if (info->flash_id & FLASH_MAN_SST) {
		addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
		addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
		addr[ADDR0] = (FLASH_WORD_SIZE)0x00800080;
		addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
		addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
		addr[0] = (FLASH_WORD_SIZE)0x00500050;  /* block erase */
		udelay(30000);  /* wait 30 ms */
	    }
	    else
		addr[0] = (FLASH_WORD_SIZE)0x00300030;  /* sector erase */
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
    addr = (FLASH_WORD_SIZE *)(info->start[l_sect]);
    while ((addr[0] & (FLASH_WORD_SIZE)0x00800080) != (FLASH_WORD_SIZE)0x00800080) {
	if ((now = get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
	    printf ("Timeout\n");
	    return 1;
	}
	/* show that we're waiting */
	if ((now - last) > 1000) {  /* every second */
	    serial_putc ('.');
	    last = now;
	}
    }

DONE:
    /* reset to read mode */
    addr = (FLASH_WORD_SIZE *)info->start[0];
    addr[0] = (FLASH_WORD_SIZE)0x00F000F0;  /* reset bank */

    printf (" done\n");
    return rc;
}

/*----------------------------------------------------------------------------*/
/*
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
    ulong cp, wp, data;
    int i, l, rc;

    wp = (addr & ~3);   /* get lower word aligned address */

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

/*----------------------------------------------------------------------------*/
/* Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word (flash_info_t *info, ulong dest, ulong data)
{
	volatile FLASH_WORD_SIZE *addr2 = (FLASH_WORD_SIZE *)(info->start[0]);
	volatile FLASH_WORD_SIZE *dest2 = (FLASH_WORD_SIZE *)dest;
	volatile FLASH_WORD_SIZE *data2 = (FLASH_WORD_SIZE *)&data;
    ulong start;
    int flag;
	int i;

    /* Check if Flash is (sufficiently) erased */
    if ((*((volatile FLASH_WORD_SIZE *)dest) &
	     (FLASH_WORD_SIZE)data) != (FLASH_WORD_SIZE)data) {
	return (2);
    }
    /* Disable interrupts which might cause a timeout here */
    flag = disable_interrupts();

	for (i=0; i<4/sizeof(FLASH_WORD_SIZE); i++)
	  {
	    addr2[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
	    addr2[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
	    addr2[ADDR0] = (FLASH_WORD_SIZE)0x00A000A0;

	    dest2[i] = data2[i];

	    /* re-enable interrupts if necessary */
	    if (flag)
	      enable_interrupts();

	    /* data polling for D7 */
	    start = get_timer (0);
	    while ((dest2[i] & (FLASH_WORD_SIZE)0x00800080) !=
		   (data2[i] & (FLASH_WORD_SIZE)0x00800080)) {
	      if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
		return (1);
	      }
	    }
	  }

    return (0);
}

/*----------------------------------------------------------------------------*/
