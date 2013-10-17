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
#  define CONFIG_ENV_ADDR  (CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
# endif
# ifndef  CONFIG_ENV_SIZE
#  define CONFIG_ENV_SIZE  CONFIG_ENV_SECT_SIZE
# endif
# ifndef  CONFIG_ENV_SECT_SIZE
#  define CONFIG_ENV_SECT_SIZE  CONFIG_ENV_SIZE
# endif
#endif


/*---------------------------------------------------------------------*/
#define DEBUG_FLASH

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

#define BS(b)     (b)
#define BYTEME(x) ((x) & 0xFF)

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	unsigned long flash_banks[CONFIG_SYS_MAX_FLASH_BANKS] = CONFIG_SYS_FLASH_BANKS;
	unsigned long size, size_b[CONFIG_SYS_MAX_FLASH_BANKS];

	int i;

	/* Init: no FLASHes known */
	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i)
	{
		flash_info[i].flash_id = FLASH_UNKNOWN;

		DEBUGF("Get flash bank %d @ 0x%08lx\n", i, flash_banks[i]);
/*
		size_b[i] = flash_get_size((vu_char *)flash_banks[i], &flash_info[i]);
*/
		size_b[i] = flash_get_size((vu_char *) 0xff800000 , &flash_info[i]);

		if (flash_info[i].flash_id == FLASH_UNKNOWN)
		{
			printf ("## Unknown FLASH on Bank %d: "
				"ID 0x%lx, Size = 0x%08lx = %ld MB\n",
				i, flash_info[i].flash_id,
				size_b[i], size_b[i]<<20);
		}
		else
		{
			DEBUGF("## Flash bank %d at 0x%08lx sizes: 0x%08lx \n",
				i, flash_banks[i], size_b[i]);

			flash_get_offsets (flash_banks[i], &flash_info[i]);
			flash_info[i].size = size_b[i];
		}
	}


#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
	DEBUGF("protect monitor %x @ %x\n", CONFIG_SYS_MONITOR_BASE, CONFIG_SYS_MONITOR_LEN);
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE+CONFIG_SYS_MONITOR_LEN-1,
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

	size = 0;
	DEBUGF("## Final Flash bank sizes: ");
	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i)
	{
		DEBUGF("%08lx ", size_b[i]);
		size += size_b[i];
	}
	DEBUGF("\n");
	return (size);
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
	case FLASH_28F320J3A:
			printf ("28F320J3A (32Mbit = 128K x 32)\n");
			break;
	case FLASH_28F640J3A:
			printf ("28F640J3A (64Mbit = 128K x 64)\n");
			break;
	case FLASH_28F128J3A:
			printf ("28F128J3A (128Mbit = 128K x 128)\n");
			break;
	default:
			printf ("Unknown Chip Type\n");
			break;
	}

#if 1
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
#endif
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
	DEBUGF("Manuf. ID @ 0x%08lx: 0x%08x\n", (ulong)addr, manuf);

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

	DEBUGF("Device ID @ 0x%08lx: 0x%08x\n", (ulong)(&addr[1]), device);

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
