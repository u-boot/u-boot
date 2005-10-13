/*
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
/* environment.h defines the various CFG_ENV_... values in terms
 * of whichever ones are given in the configuration file.
 */
#include <environment.h>

flash_info_t	flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/* NOTE - CONFIG_FLASH_16BIT means the CPU interface is 16-bit, it
 *        has nothing to do with the flash chip being 8-bit or 16-bit.
 */
#ifdef CONFIG_FLASH_16BIT
typedef unsigned short FLASH_PORT_WIDTH;
typedef volatile unsigned short FLASH_PORT_WIDTHV;
#define	FLASH_ID_MASK	0xFFFF
#else
typedef unsigned long FLASH_PORT_WIDTH;
typedef volatile unsigned long FLASH_PORT_WIDTHV;
#define	FLASH_ID_MASK	0xFFFFFFFF
#endif

#define FPW	FLASH_PORT_WIDTH
#define FPWV	FLASH_PORT_WIDTHV

#define ORMASK(size) ((-size) & OR_AM_MSK)

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size(FPWV *addr, flash_info_t *info);
static void flash_reset(flash_info_t *info);
static int write_word_intel(flash_info_t *info, FPWV *dest, FPW data);
static int write_word_amd(flash_info_t *info, FPWV *dest, FPW data);
static void flash_get_offsets(ulong base, flash_info_t *info);
#ifdef CFG_FLASH_PROTECTION
static void flash_sync_real_protect(flash_info_t *info);
#endif

/*-----------------------------------------------------------------------
 * flash_init()
 *
 * sets up flash_info and returns size of FLASH (bytes)
 */
unsigned long flash_init (void)
{
	volatile immap_t     *immap  = (immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	unsigned long size_b;
	int i;

	/* Init: no FLASHes known */
	for (i=0; i < CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	size_b = flash_get_size((FPW *)CFG_FLASH_BASE, &flash_info[0]);

	flash_info[0].size = size_b;

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx\n",size_b);
	}

	/* Remap FLASH according to real size, so only at proper address */
	memctl->memc_or0 = (memctl->memc_or0 & ~OR_AM_MSK) | ORMASK(size_b);

	/* Do this again (was done already in flast_get_size), just
	 * in case we move it when remap the FLASH.
	 */
	flash_get_offsets (CFG_FLASH_BASE, &flash_info[0]);

#ifdef CFG_FLASH_PROTECTION
	/* read the hardware protection status (if any) into the
	 * protection array in flash_info.
	 */
	flash_sync_real_protect(&flash_info[0]);
#endif

#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CFG_MONITOR_BASE,
		      CFG_MONITOR_BASE+monitor_flash_len-1,
		      &flash_info[0]);
#endif

#ifdef CFG_ENV_ADDR
	flash_protect ( FLAG_PROTECT_SET,
			CFG_ENV_ADDR,
			CFG_ENV_ADDR + CFG_ENV_SIZE - 1, &flash_info[0]);
#endif

#ifdef CFG_ENV_ADDR_REDUND
	flash_protect ( FLAG_PROTECT_SET,
			CFG_ENV_ADDR_REDUND,
			CFG_ENV_ADDR_REDUND + CFG_ENV_SIZE_REDUND - 1,
			&flash_info[0]);
#endif

	return (size_b);
}

/*-----------------------------------------------------------------------
 */
static void flash_reset(flash_info_t *info)
{
	FPWV *base = (FPWV *)(info->start[0]);

	/* Put FLASH back in read mode */
	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL)
		*base = (FPW)0x00FF00FF;	/* Intel Read Mode */
	else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_AMD)
		*base = (FPW)0x00F000F0;	/* AMD Read Mode */
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets (ulong base, flash_info_t *info)
{
	int i;

	/* set up sector start address table */
	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL
	    && (info->flash_id & FLASH_BTYPE)) {
		int bootsect_size;	/* number of bytes/boot sector	*/
		int sect_size;		/* number of bytes/regular sector */

		bootsect_size = 0x00002000 * (sizeof(FPW)/2);
		sect_size =     0x00010000 * (sizeof(FPW)/2);

		/* set sector offsets for bottom boot block type	*/
		for (i = 0; i < 8; ++i) {
			info->start[i] = base + (i * bootsect_size);
		}
		for (i = 8; i < info->sector_count; i++) {
			info->start[i] = base + ((i - 7) * sect_size);
		}
	}
	else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_AMD
		 && (info->flash_id & FLASH_TYPEMASK) == FLASH_AM640U) {

		int sect_size;		/* number of bytes/sector */

		sect_size = 0x00010000 * (sizeof(FPW)/2);

		/* set up sector start address table (uniform sector type) */
		for( i = 0; i < info->sector_count; i++ )
			info->start[i] = base + (i * sect_size);
	}
	else if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_AMD
		 && (info->flash_id & FLASH_TYPEMASK) == FLASH_AM800T) {

		int sect_size;		/* number of bytes/sector */

		sect_size = 0x00010000 * (sizeof(FPW)/2);

		/* set up sector start address table (top boot sector type) */
		for (i = 0; i < info->sector_count - 3; i++)
			info->start[i] = base + (i * sect_size);
		i = info->sector_count - 1;
		info->start[i--] = base + (info->size - 0x00004000) * (sizeof(FPW)/2);
		info->start[i--] = base + (info->size - 0x00006000) * (sizeof(FPW)/2);
		info->start[i--] = base + (info->size - 0x00008000) * (sizeof(FPW)/2);
	}
}

/*-----------------------------------------------------------------------
 */

void flash_print_info (flash_info_t *info)
{
	int i;
	uchar *boottype;
	uchar *bootletter;
	char *fmt;
	uchar botbootletter[] = "B";
	uchar topbootletter[] = "T";
	uchar botboottype[] = "bottom boot sector";
	uchar topboottype[] = "top boot sector";

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:	printf ("AMD ");		break;
	case FLASH_MAN_BM:	printf ("BRIGHT MICRO ");	break;
	case FLASH_MAN_FUJ:	printf ("FUJITSU ");		break;
	case FLASH_MAN_SST:	printf ("SST ");		break;
	case FLASH_MAN_STM:	printf ("STM ");		break;
	case FLASH_MAN_INTEL:	printf ("INTEL ");		break;
	default:		printf ("Unknown Vendor ");	break;
	}

	/* check for top or bottom boot, if it applies */
	if (info->flash_id & FLASH_BTYPE) {
		boottype = botboottype;
		bootletter = botbootletter;
	}
	else {
		boottype = topboottype;
		bootletter = topbootletter;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM800T:
		fmt = "29LV800B%s (8 Mbit, %s)\n";
		break;
	case FLASH_AM640U:
		fmt = "29LV641D (64 Mbit, uniform sectors)\n";
		break;
	case FLASH_28F800C3B:
	case FLASH_28F800C3T:
		fmt = "28F800C3%s (8 Mbit, %s)\n";
		break;
	case FLASH_INTEL800B:
	case FLASH_INTEL800T:
		fmt = "28F800B3%s (8 Mbit, %s)\n";
		break;
	case FLASH_28F160C3B:
	case FLASH_28F160C3T:
		fmt = "28F160C3%s (16 Mbit, %s)\n";
		break;
	case FLASH_INTEL160B:
	case FLASH_INTEL160T:
		fmt = "28F160B3%s (16 Mbit, %s)\n";
		break;
	case FLASH_28F320C3B:
	case FLASH_28F320C3T:
		fmt = "28F320C3%s (32 Mbit, %s)\n";
		break;
	case FLASH_INTEL320B:
	case FLASH_INTEL320T:
		fmt = "28F320B3%s (32 Mbit, %s)\n";
		break;
	case FLASH_28F640C3B:
	case FLASH_28F640C3T:
		fmt = "28F640C3%s (64 Mbit, %s)\n";
		break;
	case FLASH_INTEL640B:
	case FLASH_INTEL640T:
		fmt = "28F640B3%s (64 Mbit, %s)\n";
		break;
	default:
		fmt = "Unknown Chip Type\n";
		break;
	}

	printf (fmt, bootletter, boottype);

	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20,
		info->sector_count);

	printf ("  Sector Start Addresses:");

	for (i=0; i<info->sector_count; ++i) {
		if ((i % 5) == 0) {
			printf ("\n   ");
		}

		printf (" %08lX%s", info->start[i],
			info->protect[i] ? " (RO)" : "     ");
	}

	printf ("\n");
}

/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */

ulong flash_get_size (FPWV *addr, flash_info_t *info)
{
	/* Write auto select command: read Manufacturer ID */

	/* Write auto select command sequence and test FLASH answer */
	addr[0x0555] = (FPW)0x00AA00AA;	/* for AMD, Intel ignores this */
	addr[0x02AA] = (FPW)0x00550055;	/* for AMD, Intel ignores this */
	addr[0x0555] = (FPW)0x00900090;	/* selects Intel or AMD */

	/* The manufacturer codes are only 1 byte, so just use 1 byte.
	 * This works for any bus width and any FLASH device width.
	 */
	switch (addr[0] & 0xff) {

	case (uchar)AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
		break;

	case (uchar)INTEL_MANUFACT:
		info->flash_id = FLASH_MAN_INTEL;
		break;

	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		break;
	}

	/* Check 16 bits or 32 bits of ID so work on 32 or 16 bit bus. */
	if (info->flash_id != FLASH_UNKNOWN) switch (addr[1]) {

	case (FPW)AMD_ID_LV800T:
		info->flash_id += FLASH_AM800T;
		info->sector_count = 19;
		info->size = 0x00100000 * (sizeof(FPW)/2);
		break;				/* => 1 or 2 MiB	*/

	case (FPW)AMD_ID_LV640U:	/* 29LV640 and 29LV641 have same ID */
		info->flash_id += FLASH_AM640U;
		info->sector_count = 128;
		info->size = 0x00800000 * (sizeof(FPW)/2);
		break;				/* => 8 or 16 MB	*/

	case (FPW)INTEL_ID_28F800C3B:
		info->flash_id += FLASH_28F800C3B;
		info->sector_count = 23;
		info->size = 0x00100000 * (sizeof(FPW)/2);
		break;				/* => 1 or 2 MB		*/

	case (FPW)INTEL_ID_28F800B3B:
		info->flash_id += FLASH_INTEL800B;
		info->sector_count = 23;
		info->size = 0x00100000 * (sizeof(FPW)/2);
		break;				/* => 1 or 2 MB		*/

	case (FPW)INTEL_ID_28F160C3B:
		info->flash_id += FLASH_28F160C3B;
		info->sector_count = 39;
		info->size = 0x00200000 * (sizeof(FPW)/2);
		break;				/* => 2 or 4 MB		*/

	case (FPW)INTEL_ID_28F160B3B:
		info->flash_id += FLASH_INTEL160B;
		info->sector_count = 39;
		info->size = 0x00200000 * (sizeof(FPW)/2);
		break;				/* => 2 or 4 MB		*/

	case (FPW)INTEL_ID_28F320C3B:
		info->flash_id += FLASH_28F320C3B;
		info->sector_count = 71;
		info->size = 0x00400000 * (sizeof(FPW)/2);
		break;				/* => 4 or 8 MB		*/

	case (FPW)INTEL_ID_28F320B3B:
		info->flash_id += FLASH_INTEL320B;
		info->sector_count = 71;
		info->size = 0x00400000 * (sizeof(FPW)/2);
		break;				/* => 4 or 8 MB		*/

	case (FPW)INTEL_ID_28F640C3B:
		info->flash_id += FLASH_28F640C3B;
		info->sector_count = 135;
		info->size = 0x00800000 * (sizeof(FPW)/2);
		break;				/* => 8 or 16 MB	*/

	case (FPW)INTEL_ID_28F640B3B:
		info->flash_id += FLASH_INTEL640B;
		info->sector_count = 135;
		info->size = 0x00800000 * (sizeof(FPW)/2);
		break;				/* => 8 or 16 MB	*/

	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);			/* => no or unknown flash */
	}

	flash_get_offsets((ulong)addr, info);

	/* Put FLASH back in read mode */
	flash_reset(info);

	return (info->size);
}

#ifdef CFG_FLASH_PROTECTION
/*-----------------------------------------------------------------------
 */

static void flash_sync_real_protect(flash_info_t *info)
{
    FPWV *addr = (FPWV *)(info->start[0]);
    FPWV *sect;
    int i;

    switch (info->flash_id & FLASH_TYPEMASK) {
    case FLASH_28F800C3B:
    case FLASH_28F800C3T:
    case FLASH_28F160C3B:
    case FLASH_28F160C3T:
    case FLASH_28F320C3B:
    case FLASH_28F320C3T:
    case FLASH_28F640C3B:
    case FLASH_28F640C3T:
	/* check for protected sectors */
	*addr = (FPW)0x00900090;
	for (i = 0; i < info->sector_count; i++) {
	    /* read sector protection at sector address, (A7 .. A0) = 0x02.
	     * D0 = 1 for each device if protected.
	     * If at least one device is protected the sector is marked
	     * protected, but mixed protected and  unprotected devices
	     * within a sector should never happen.
	     */
	    sect = (FPWV *)(info->start[i]);
	    info->protect[i] = (sect[2] & (FPW)(0x00010001)) ? 1 : 0;
	}

	/* Put FLASH back in read mode */
	flash_reset(info);
	break;

    case FLASH_AM640U:
    case FLASH_AM800T:
    default:
	/* no hardware protect that we support */
	break;
    }
}
#endif

/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	FPWV *addr;
	int flag, prot, sect;
	int intel = (info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL;
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

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_INTEL800B:
	case FLASH_INTEL160B:
	case FLASH_INTEL320B:
	case FLASH_INTEL640B:
	case FLASH_28F800C3B:
	case FLASH_28F160C3B:
	case FLASH_28F320C3B:
	case FLASH_28F640C3B:
	case FLASH_AM640U:
	case FLASH_AM800T:
		break;
	case FLASH_UNKNOWN:
	default:
		printf ("Can't erase unknown flash type %08lx - aborted\n",
			info->flash_id);
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

	start = get_timer(0);
	last  = start;

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last && rcode == 0; sect++) {

		if (info->protect[sect] != 0)	/* protected, skip it */
			continue;

		/* Disable interrupts which might cause a timeout here */
		flag = disable_interrupts();

		addr = (FPWV *)(info->start[sect]);
		if (intel) {
			*addr = (FPW)0x00500050; /* clear status register */
			*addr = (FPW)0x00200020; /* erase setup */
			*addr = (FPW)0x00D000D0; /* erase confirm */
		}
		else {
			/* must be AMD style if not Intel */
			FPWV *base;		/* first address in bank */

			base = (FPWV *)(info->start[0]);
			base[0x0555] = (FPW)0x00AA00AA;	/* unlock */
			base[0x02AA] = (FPW)0x00550055;	/* unlock */
			base[0x0555] = (FPW)0x00800080;	/* erase mode */
			base[0x0555] = (FPW)0x00AA00AA;	/* unlock */
			base[0x02AA] = (FPW)0x00550055;	/* unlock */
			*addr = (FPW)0x00300030;	/* erase sector */
		}

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts();

		/* wait at least 50us for AMD, 80us for Intel.
		 * Let's wait 1 ms.
		 */
		udelay (1000);

		while ((*addr & (FPW)0x00800080) != (FPW)0x00800080) {
			if ((now = get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
				printf ("Timeout\n");

				if (intel) {
					/* suspend erase	*/
					*addr = (FPW)0x00B000B0;
				}

				flash_reset(info);	/* reset to read mode */
				rcode = 1;		/* failed */
				break;
			}

			/* show that we're waiting */
			if ((now - last) > 1000) {	/* every second */
				putc ('.');
				last = now;
			}
		}

		flash_reset(info);	/* reset to read mode	*/
	}

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
    FPW data = 0; /* 16 or 32 bit word, matches flash bus width on MPC8XX */
    int bytes;	  /* number of bytes to program in current word		*/
    int left;	  /* number of bytes left to program			*/
    int i, res;

    for (left = cnt, res = 0;
	 left > 0 && res == 0;
	 addr += sizeof(data), left -= sizeof(data) - bytes) {

	bytes = addr & (sizeof(data) - 1);
	addr &= ~(sizeof(data) - 1);

	/* combine source and destination data so can program
	 * an entire word of 16 or 32 bits
	 */
	for (i = 0; i < sizeof(data); i++) {
	    data <<= 8;
	    if (i < bytes || i - bytes >= left )
		data += *((uchar *)addr + i);
	    else
		data += *src++;
	}

	/* write one word to the flash */
	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:
		res = write_word_amd(info, (FPWV *)addr, data);
		break;
	case FLASH_MAN_INTEL:
		res = write_word_intel(info, (FPWV *)addr, data);
		break;
	default:
		/* unknown flash type, error! */
		printf ("missing or unknown FLASH type\n");
		res = 1;	/* not really a timeout, but gives error */
		break;
	}
    }

    return (res);
}

/*-----------------------------------------------------------------------
 * Write a word to Flash for AMD FLASH
 * A word is 16 or 32 bits, whichever the bus width of the flash bank
 * (not an individual chip) is.
 *
 * returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word_amd (flash_info_t *info, FPWV *dest, FPW data)
{
    ulong start;
    int flag;
    int res = 0;	/* result, assume success	*/
    FPWV *base;		/* first address in flash bank	*/

    /* Check if Flash is (sufficiently) erased */
    if ((*dest & data) != data) {
	return (2);
    }


    base = (FPWV *)(info->start[0]);

    /* Disable interrupts which might cause a timeout here */
    flag = disable_interrupts();

    base[0x0555] = (FPW)0x00AA00AA;	/* unlock */
    base[0x02AA] = (FPW)0x00550055;	/* unlock */
    base[0x0555] = (FPW)0x00A000A0;	/* selects program mode */

    *dest = data;		/* start programming the data	*/

    /* re-enable interrupts if necessary */
    if (flag)
	enable_interrupts();

    start = get_timer (0);

    /* data polling for D7 */
    while (res == 0 && (*dest & (FPW)0x00800080) != (data & (FPW)0x00800080)) {
	if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
	    *dest = (FPW)0x00F000F0;	/* reset bank */
	    res = 1;
	}
    }

    return (res);
}

/*-----------------------------------------------------------------------
 * Write a word to Flash for Intel FLASH
 * A word is 16 or 32 bits, whichever the bus width of the flash bank
 * (not an individual chip) is.
 *
 * returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word_intel (flash_info_t *info, FPWV *dest, FPW data)
{
    ulong start;
    int flag;
    int res = 0;	/* result, assume success	*/

    /* Check if Flash is (sufficiently) erased */
    if ((*dest & data) != data) {
	return (2);
    }

    /* Disable interrupts which might cause a timeout here */
    flag = disable_interrupts();

    *dest = (FPW)0x00500050;	/* clear status register	*/
    *dest = (FPW)0x00FF00FF;	/* make sure in read mode	*/
    *dest = (FPW)0x00400040;	/* program setup		*/

    *dest = data;		/* start programming the data	*/

    /* re-enable interrupts if necessary */
    if (flag)
	enable_interrupts();

    start = get_timer (0);

    while (res == 0 && (*dest & (FPW)0x00800080) != (FPW)0x00800080) {
	if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
	    *dest = (FPW)0x00B000B0;	/* Suspend program	*/
	    res = 1;
	}
    }

    if (res == 0 && (*dest & (FPW)0x00100010))
	res = 1;	/* write failed, time out error is close enough	*/

    *dest = (FPW)0x00500050;	/* clear status register	*/
    *dest = (FPW)0x00FF00FF;	/* make sure in read mode	*/

    return (res);
}

#ifdef CFG_FLASH_PROTECTION
/*-----------------------------------------------------------------------
 */
int flash_real_protect (flash_info_t * info, long sector, int prot)
{
	int rcode = 0;		/* assume success */
	FPWV *addr;		/* address of sector */
	FPW value;

	addr = (FPWV *) (info->start[sector]);

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F800C3B:
	case FLASH_28F800C3T:
	case FLASH_28F160C3B:
	case FLASH_28F160C3T:
	case FLASH_28F320C3B:
	case FLASH_28F320C3T:
	case FLASH_28F640C3B:
	case FLASH_28F640C3T:
		flash_reset (info);		/* make sure in read mode */
		*addr = (FPW) 0x00600060L;	/* lock command setup */
		if (prot)
			*addr = (FPW) 0x00010001L;	/* lock sector */
		else
			*addr = (FPW) 0x00D000D0L;	/* unlock sector */
		flash_reset (info);		/* reset to read mode */

		/* now see if it really is locked/unlocked as requested */
		*addr = (FPW) 0x00900090;
		/* read sector protection at sector address, (A7 .. A0) = 0x02.
		 * D0 = 1 for each device if protected.
		 * If at least one device is protected the sector is marked
		 * protected, but return failure. Mixed protected and
		 * unprotected devices within a sector should never happen.
		 */
		value = addr[2] & (FPW) 0x00010001;
		if (value == 0)
			info->protect[sector] = 0;
		else if (value == (FPW) 0x00010001)
			info->protect[sector] = 1;
		else {
			/* error, mixed protected and unprotected */
			rcode = 1;
			info->protect[sector] = 1;
		}
		if (info->protect[sector] != prot)
			rcode = 1;	/* failed to protect/unprotect as requested */

		/* reload all protection bits from hardware for now */
		flash_sync_real_protect (info);
		break;

	case FLASH_AM640U:
	case FLASH_AM800T:
	default:
		/* no hardware protect that we support */
		info->protect[sector] = prot;
		break;
	}

	return rcode;
}
#endif
