/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc824x.h>

#if defined(CONFIG_ENV_IS_IN_FLASH)
# ifndef  CONFIG_ENV_ADDR
#  define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
# endif
# ifndef  CONFIG_ENV_SIZE
#  define CONFIG_ENV_SIZE	CONFIG_ENV_SECT_SIZE
# endif
# ifndef  CONFIG_ENV_SECT_SIZE
#  define CONFIG_ENV_SECT_SIZE  CONFIG_ENV_SIZE
# endif
#endif

/*---------------------------------------------------------------------*/
#undef DEBUG_FLASH

#ifdef DEBUG_FLASH
#define DEBUGF(fmt,args...) printf(fmt ,##args)
#else
#define DEBUGF(fmt,args...)
#endif
/*---------------------------------------------------------------------*/

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_char *addr, flash_info_t *info);
static int write_data (flash_info_t *info, uchar *dest, uchar data);
static void flash_get_offsets (ulong base, flash_info_t *info);


/*
 * don't ask.  its stupid, but more than one soul has had to live with this mistake
 * "swaptab[i]" is the value of "i" with the bits reversed.
 */

#define  MUSENKI_BROKEN_FLASH 1

#ifdef MUSENKI_BROKEN_FLASH
unsigned char swaptab[256] = {
  0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
  0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
  0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
  0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
  0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
  0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
  0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
  0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
  0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
  0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
  0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
  0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
  0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
  0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
  0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
  0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
  0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
  0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
  0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
  0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
  0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
  0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
  0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
  0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
  0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
  0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
  0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
  0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
  0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
  0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
  0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
  0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};

#define BS(b)     (swaptab[b])

#else

#define BS(b)     (b)

#endif

#define BYTEME(x) ((x) & 0xFF)

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	unsigned long size_b0, size_b1;
	int i;

	/* Init: no FLASHes known */
	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Static FLASH Bank configuration here - FIXME XXX */

	DEBUGF("\n## Get flash bank 1 size @ 0x%08x\n",CONFIG_SYS_FLASH_BASE0_PRELIM);

	size_b0 = flash_get_size((vu_char *)CONFIG_SYS_FLASH_BASE0_PRELIM, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0: "
			"ID 0x%lx, Size = 0x%08lx = %ld MB\n",
			flash_info[0].flash_id,
			size_b0, size_b0<<20);
	}

	DEBUGF("## Get flash bank 2 size @ 0x%08x\n",CONFIG_SYS_FLASH_BASE1_PRELIM);
	size_b1 = flash_get_size((vu_char *)CONFIG_SYS_FLASH_BASE1_PRELIM, &flash_info[1]);

	DEBUGF("## Prelim. Flash bank sizes: %08lx + 0x%08lx\n",size_b0,size_b1);

	flash_get_offsets (CONFIG_SYS_FLASH_BASE, &flash_info[0]);

	flash_info[0].size = size_b0;

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
	DEBUGF("protect monitor %x @ %x\n", CONFIG_SYS_MONITOR_BASE, monitor_flash_len);
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE+monitor_flash_len-1,
		      &flash_info[0]);
#endif

#ifdef	CONFIG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	DEBUGF("protect environtment %x @ %x\n", CONFIG_ENV_ADDR, CONFIG_ENV_SECT_SIZE);
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_ENV_ADDR,
		      CONFIG_ENV_ADDR+CONFIG_ENV_SECT_SIZE-1,
		      &flash_info[0]);
#endif

	if (size_b1) {
		flash_info[1].size = size_b1;
		flash_get_offsets (CONFIG_SYS_FLASH_BASE + size_b0, &flash_info[1]);

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
		/* monitor protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CONFIG_SYS_MONITOR_BASE,
			      CONFIG_SYS_MONITOR_BASE+monitor_flash_len-1,
			      &flash_info[1]);
#endif

#ifdef	CONFIG_ENV_IS_IN_FLASH
		/* ENV protection ON by default */
		flash_protect(FLAG_PROTECT_SET,
			      CONFIG_ENV_ADDR,
			      CONFIG_ENV_ADDR+CONFIG_ENV_SECT_SIZE-1,
			      &flash_info[1]);
#endif
	} else {
		flash_info[1].flash_id = FLASH_UNKNOWN;
		flash_info[1].sector_count = -1;
		flash_info[1].size = 0;
	}

	DEBUGF("## Final Flash bank sizes: %08lx + 0x%08lx\n",size_b0,size_b1);

	return (size_b0 + size_b1);
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets (ulong base, flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_INTEL:
	    for (i = 0; i < info->sector_count; i++) {
		info->start[i] = base;
		base += 0x00020000;		/* 128k per bank */
	    }
	    return;

	default:
	    printf ("Don't know sector ofsets for flash type 0x%lx\n", info->flash_id);
	    return;
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
	case FLASH_MAN_FUJ:	printf ("Fujitsu ");		break;
	case FLASH_MAN_SST:	printf ("SST ");		break;
	case FLASH_MAN_STM:	printf ("STM ");		break;
	case FLASH_MAN_INTEL:	printf ("Intel ");		break;
	case FLASH_MAN_MT:	printf ("MT ");			break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F320J3A:	printf ("28F320J3A (32Mbit = 128K x 32)\n");
				break;
	case FLASH_28F640J3A:	printf ("28F640J3A (64Mbit = 128K x 64)\n");
				break;
	case FLASH_28F128J3A:	printf ("28F128J3A (128Mbit = 128K x 128)\n");
				break;
	default:		printf ("Unknown Chip Type\n");
				break;
	}

	if (info->size >= (1 << 20)) {
		i = 20;
	} else {
		i = 10;
	}
	printf ("  Size: %ld %cB in %d Sectors\n",
		info->size >> i,
		(i == 20) ? 'M' : 'k',
		info->sector_count);

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

/*-----------------------------------------------------------------------
 */


/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */
static ulong flash_get_size (vu_char *addr, flash_info_t *info)
{
	vu_char manuf, device;

	addr[0] = BS(0x90);
	manuf = BS(addr[0]);
	DEBUGF("Manuf. ID @ 0x%08lx: 0x%08x\n", (vu_char *)addr, manuf);

	switch (manuf) {
	case BYTEME(AMD_MANUFACT):
		info->flash_id = FLASH_MAN_AMD;
		break;
	case BYTEME(FUJ_MANUFACT):
		info->flash_id = FLASH_MAN_FUJ;
		break;
	case BYTEME(SST_MANUFACT):
		info->flash_id = FLASH_MAN_SST;
		break;
	case BYTEME(STM_MANUFACT):
		info->flash_id = FLASH_MAN_STM;
		break;
	case BYTEME(INTEL_MANUFACT):
		info->flash_id = FLASH_MAN_INTEL;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		addr[0] = BS(0xFF);		/* restore read mode, (yes, BS is a NOP) */
		return 0;			/* no or unknown flash	*/
	}

	device = BS(addr[2]);			/* device ID		*/

	DEBUGF("Device ID @ 0x%08x: 0x%08x\n", (&addr[1]), device);

	switch (device) {
	case BYTEME(INTEL_ID_28F320J3A):
		info->flash_id += FLASH_28F320J3A;
		info->sector_count = 32;
		info->size = 0x00400000;
		break;				/* =>  4 MB		*/

	case BYTEME(INTEL_ID_28F640J3A):
		info->flash_id += FLASH_28F640J3A;
		info->sector_count = 64;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/

	case BYTEME(INTEL_ID_28F128J3A):
		info->flash_id += FLASH_28F128J3A;
		info->sector_count = 128;
		info->size = 0x01000000;
		break;				/* => 16 MB		*/

	default:
		info->flash_id = FLASH_UNKNOWN;
		addr[0] = BS(0xFF);		/* restore read mode (yes, a NOP) */
		return 0;			/* => no or unknown flash */

	}

	if (info->sector_count > CONFIG_SYS_MAX_FLASH_SECT) {
		printf ("** ERROR: sector count %d > max (%d) **\n",
			info->sector_count, CONFIG_SYS_MAX_FLASH_SECT);
		info->sector_count = CONFIG_SYS_MAX_FLASH_SECT;
	}

	addr[0] = BS(0xFF);		/* restore read mode */

	return (info->size);
}


/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	int flag, prot, sect;
	ulong start, now, last;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	if ((info->flash_id & FLASH_VENDMASK) != FLASH_MAN_INTEL) {
		printf ("Can erase only Intel flash types - aborted\n");
		return 1;
	}

	prot = 0;
	for (sect=s_first; sect<=s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf ("- Warning: %d protected sectors will not be erased!\n", prot);
	} else {
		printf ("\n");
	}

	start = get_timer (0);
	last  = start;
	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			vu_char *addr = (vu_char *)(info->start[sect]);
			unsigned long status;

			/* Disable interrupts which might cause a timeout here */
			flag = disable_interrupts();

			*addr = BS(0x50);	/* clear status register */
			*addr = BS(0x20);	/* erase setup */
			*addr = BS(0xD0);	/* erase confirm */

			/* re-enable interrupts if necessary */
			if (flag) {
				enable_interrupts();
			}

			/* wait at least 80us - let's wait 1 ms */
			udelay (1000);

			while (((status = BS(*addr)) & BYTEME(0x00800080)) != BYTEME(0x00800080)) {
				if ((now=get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
					printf ("Timeout\n");
					*addr = BS(0xB0); /* suspend erase	  */
					*addr = BS(0xFF); /* reset to read mode */
					return 1;
				}

				/* show that we're waiting */
				if ((now - last) > 1000) {	/* every second */
					putc ('.');
					last = now;
				}
			}

			*addr = BS(0xFF);	/* reset to read mode */
		}
	}
	printf (" done\n");
	return 0;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 4 - Flash not identified
 */

#define	FLASH_WIDTH	1	/* flash bus width in bytes */

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	uchar *wp = (uchar *)addr;
	int rc;

	if (info->flash_id == FLASH_UNKNOWN) {
		return 4;
	}

	while (cnt > 0) {
		if ((rc = write_data(info, wp, *src)) != 0) {
			return rc;
		}
		wp++;
		src++;
		cnt--;
	}

	return cnt;
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_data (flash_info_t *info, uchar *dest, uchar data)
{
	vu_char *addr = (vu_char *)dest;
	ulong status;
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((BS(*addr) & data) != data) {
		return 2;
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	*addr = BS(0x40);		/* write setup */
	*addr = data;

	/* re-enable interrupts if necessary */
	if (flag) {
		enable_interrupts();
	}

	start = get_timer (0);

	while (((status = BS(*addr)) & BYTEME(0x00800080)) != BYTEME(0x00800080)) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			*addr = BS(0xFF);	/* restore read mode */
			return 1;
		}
	}

	*addr = BS(0xFF);	/* restore read mode */

	return 0;
}

/*-----------------------------------------------------------------------
 */
