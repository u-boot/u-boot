/*
 * (C) Copyright 2000
 * Marius Groeger <mgroeger@sysgo.de>
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Flash Routines for AM290[48]0B devices
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

/* flash hardware ids */
#define VENDOR_AMD     0x0001
#define AMD_29DL323C_B 0x2253

/* Define this to include autoselect sequence in flash_init(). Does NOT
 * work when executing from flash itself, so this should be turned
 * on only when debugging the RAM version.
 */
#undef WITH_AUTOSELECT

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

#if 1
#define D(x)
#else
#define D(x) printf x
#endif

/*-----------------------------------------------------------------------
 * Functions
 */

static unsigned char write_ull(flash_info_t *info,
			       unsigned long address,
			       volatile unsigned long long data);

/* from flash_asm.S */
extern void ull_write(unsigned long long volatile *address,
		      unsigned long long volatile *data);
extern void ull_read(unsigned long long volatile *address,
		     unsigned long long volatile *data);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
    int i;
    ulong addr;

#ifdef WITH_AUTOSELECT
	{
		unsigned long long *f_addr = (unsigned long long *)PHYS_FLASH;
		unsigned long long f_command, vendor, device;
		/* Perform Autoselect */
		f_command	= 0x00AA00AA00AA00AAULL;
		ull_write(&f_addr[0x555], &f_command);
		f_command	= 0x0055005500550055ULL;
		ull_write(&f_addr[0x2AA], &f_command);
		f_command	= 0x0090009000900090ULL;
		ull_write(&f_addr[0x555], &f_command);
		ull_read(&f_addr[0], &vendor);
		vendor &= 0xffff;
		ull_read(&f_addr[1], &device);
		device &= 0xffff;
		f_command	= 0x00F000F000F000F0ULL;
		ull_write(&f_addr[0x555], &f_command);
		if (vendor != VENDOR_AMD || device != AMD_29DL323C_B)
		  return 0;
	}
#endif

    /* Init: no FLASHes known */
    for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
    }

	/* 1st bank: 8 x 32 KB sectors */
	flash_info[0].flash_id = VENDOR_AMD << 16 | AMD_29DL323C_B;
	flash_info[0].sector_count = 8;
	flash_info[0].size = flash_info[0].sector_count * 32 * 1024;
	addr = PHYS_FLASH;
    for(i = 0; i < flash_info[0].sector_count; i++) {
		flash_info[0].start[i] = addr;
		addr += flash_info[0].size / flash_info[0].sector_count;
	}
	/* 1st bank: 63 x 256 KB sectors */
	flash_info[1].flash_id = VENDOR_AMD << 16 | AMD_29DL323C_B;
	flash_info[1].sector_count = 63;
	flash_info[1].size = flash_info[1].sector_count * 256 * 1024;
    for(i = 0; i < flash_info[1].sector_count; i++) {
		flash_info[1].start[i] = addr;
		addr += flash_info[1].size / flash_info[1].sector_count;
	}

	/*
	 * protect monitor and environment sectors
	 */

#if CONFIG_SYS_MONITOR_BASE >= PHYS_FLASH
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE+monitor_flash_len-1,
		      &flash_info[0]);
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE+monitor_flash_len-1,
		      &flash_info[1]);
#endif

#if defined(CONFIG_ENV_IS_IN_FLASH) && defined(CONFIG_ENV_ADDR)
# ifndef  CONFIG_ENV_SIZE
#  define CONFIG_ENV_SIZE	CONFIG_ENV_SECT_SIZE
# endif
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_ENV_ADDR,
		      CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1,
		      &flash_info[0]);
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_ENV_ADDR,
		      CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1,
		      &flash_info[1]);
#endif

    return flash_info[0].size + flash_info[1].size;
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

    switch (info->flash_id >> 16) {
    case VENDOR_AMD:
		printf ("AMD ");
		break;
    default:
		printf ("Unknown Vendor ");
		break;
    }

    switch (info->flash_id & FLASH_TYPEMASK) {
    case AMD_29DL323C_B:
		printf ("AM29DL323CB (32 Mbit)\n");
		break;
    default:
		printf ("Unknown Chip Type\n");
		break;
    }

    printf ("  Size: %ld MB in %d Sectors\n",
			info->size >> 20, info->sector_count);

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

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
    int flag, prot, sect, l_sect;
    ulong start;
	unsigned long long volatile *f_addr;
	unsigned long long volatile f_command;

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

	f_addr	= (unsigned long long *)info->start[0];
	f_command	= 0x00AA00AA00AA00AAULL;
	ull_write(&f_addr[0x555], &f_command);
	f_command	= 0x0055005500550055ULL;
	ull_write(&f_addr[0x2AA], &f_command);
	f_command	= 0x0080008000800080ULL;
	ull_write(&f_addr[0x555], &f_command);
	f_command	= 0x00AA00AA00AA00AAULL;
	ull_write(&f_addr[0x555], &f_command);
	f_command	= 0x0055005500550055ULL;
	ull_write(&f_addr[0x2AA], &f_command);

    /* Disable interrupts which might cause a timeout here */
    flag = disable_interrupts();

    /* Start erase on unprotected sectors */
    for (l_sect = -1, sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */

			f_addr	=
			  (unsigned long long *)(info->start[sect]);
			f_command	= 0x0030003000300030ULL;
			ull_write(f_addr, &f_command);
			l_sect = sect;
		}
    }

    /* re-enable interrupts if necessary */
    if (flag)
      enable_interrupts();

	start = get_timer (0);
	do
	{
		if (get_timer(start) > CONFIG_SYS_FLASH_ERASE_TOUT)
		{	/* write reset command, command address is unimportant */
			/* this command turns the flash back to read mode     */
			f_addr =
			  (unsigned long long *)(info->start[l_sect]);
			f_command	= 0x00F000F000F000F0ULL;
			ull_write(f_addr, &f_command);
			printf (" timeout\n");
			return 1;
	}
	} while(*f_addr != 0xFFFFFFFFFFFFFFFFULL);

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
    unsigned long cp, wp;
	unsigned long long data;
    int i, l, rc;

    wp = (addr & ~7);	/* get lower long long aligned address */

    /*
     * handle unaligned start bytes
     */
    if ((l = addr - wp) != 0) {
		data = 0;
		for (i=0, cp=wp; i<l; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}
		for (; i<8 && cnt>0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt==0 && i<8; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}

		if ((rc = write_ull(info, wp, data)) != 0) {
			return rc;
		}
		wp += 4;
    }

    /*
     * handle long long aligned part
     */
    while (cnt >= 8) {
		data = 0;
		for (i=0; i<8; ++i) {
			data = (data << 8) | *src++;
		}
		if ((rc = write_ull(info, wp, data)) != 0) {
			return rc;
		}
		wp  += 8;
		cnt -= 8;
    }

    if (cnt == 0) {
		return ERR_OK;
    }

    /*
     * handle unaligned tail bytes
     */
    data = 0;
    for (i=0, cp=wp; i<8 && cnt>0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
    }
    for (; i<8; ++i, ++cp) {
		data = (data << 8) | (*(uchar *)cp);
    }

    return write_ull(info, wp, data);
}

/*---------------------------------------------------------------------------
*
* FUNCTION NAME:  write_ull
*
* DESCRIPTION:   writes 8 bytes to flash
*
* EXTERNAL EFFECT: nothing
*
* PARAMETERS:   32 bit long pointer to address, 64 bit long pointer to data
*
* RETURNS:	0 if OK, 1 if timeout, 4 if parameter error
*--------------------------------------------------------------------------*/

static unsigned char write_ull(flash_info_t *info,
							   unsigned long address,
							   volatile unsigned long long data)
{
	static unsigned long long f_command;
	static unsigned long long *f_addr;
	ulong start;

	/* address muss be 8-aligned! */
	if (address & 0x7)
	  return ERR_ALIGN;

	f_addr	= (unsigned long long *)info->start[0];
	f_command	= 0x00AA00AA00AA00AAULL;
	ull_write(&f_addr[0x555], &f_command);
	f_command	= 0x0055005500550055ULL;
	ull_write(&f_addr[0x2AA], &f_command);
	f_command	= 0x00A000A000A000A0ULL;
	ull_write(&f_addr[0x555], &f_command);

	f_addr	= (unsigned long long *)address;
	f_command	= data;
	ull_write(f_addr, &f_command);

	start = get_timer (0);
	do
	{
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT)
		{
			/* write reset command, command address is unimportant */
			/* this command turns the flash back to read mode     */
			f_addr	= (unsigned long long *)info->start[0];
			f_command	= 0x00F000F000F000F0ULL;
			ull_write(f_addr, &f_command);
			return ERR_TIMOUT;
		}
	} while(*((unsigned long long *)address) != data);

	return 0;
}
