/*
 * Copyright (C) 2003 ETC s.r.o.
 *
 * This code was inspired by Marius Groeger and Kyle Harris code
 * available in other board ports for U-Boot
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
 *
 * Written by Peter Figuli <peposh@etc.sk>, 2003.
 *
 */

#include <common.h>
#include "intel.h"


/*
 * This code should handle CFI FLASH memory device. This code is very
 * minimalistic approach without many essential error handling code as well.
 * Because U-Boot actually is missing smart handling of FLASH device,
 * we just set flash_id to anything else to FLASH_UNKNOW, so common code
 * can call us without any restrictions.
 * TODO: Add CFI Query, to be able to determine FLASH device.
 * TODO: Add error handling code
 * NOTE: This code was tested with BUS_WIDTH 4 and ITERLEAVE 2 only, but
 *       hopefully may work with other configurations.
 */

#if ( WEP_FLASH_BUS_WIDTH == 1 )
#  define FLASH_BUS vu_char
#  define FLASH_BUS_RET u_char
#  if ( WEP_FLASH_INTERLEAVE == 1 )
#    define FLASH_CMD( x ) x
#  else
#    error "With 8bit bus only one chip is allowed"
#  endif


#elif ( WEP_FLASH_BUS_WIDTH == 2 )
#  define FLASH_BUS vu_short
#  define FLASH_BUS_RET u_short
#  if ( WEP_FLASH_INTERLEAVE == 1 )
#    define FLASH_CMD( x ) x
#  elif ( WEP_FLASH_INTERLEAVE == 2 )
#    define FLASH_CMD( x ) (( x << 8 )| x )
#  else
#    error "With 16bit bus only 1 or 2 chip(s) are allowed"
#  endif


#elif ( WEP_FLASH_BUS_WIDTH == 4 )
#  define FLASH_BUS vu_long
#  define FLASH_BUS_RET u_long
#  if ( WEP_FLASH_INTERLEAVE == 1 )
#    define FLASH_CMD( x ) x
#  elif ( WEP_FLASH_INTERLEAVE == 2 )
#    define FLASH_CMD( x ) (( x << 16 )| x )
#  elif ( WEP_FLASH_INTERLEAVE == 4 )
#    define FLASH_CMD( x ) (( x << 24 )|( x << 16 ) ( x << 8 )| x )
#  else
#    error "With 32bit bus only 1,2 or 4 chip(s) are allowed"
#  endif

#else
#  error "Flash bus width might be 1,2,4 for 8,16,32 bit configuration"
#endif


flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

static FLASH_BUS_RET flash_status_reg (void)
{

	FLASH_BUS *addr = (FLASH_BUS *) 0;

	*addr = FLASH_CMD (CFI_INTEL_CMD_READ_STATUS_REGISTER);

	return *addr;
}

static int flash_ready (ulong timeout)
{
	int ok = 1;

	reset_timer_masked ();
	while ((flash_status_reg () & FLASH_CMD (CFI_INTEL_SR_READY)) !=
		   FLASH_CMD (CFI_INTEL_SR_READY)) {
		if (get_timer_masked () > timeout && timeout != 0) {
			ok = 0;
			break;
		}
	}
	return ok;
}

#if ( CONFIG_SYS_MAX_FLASH_BANKS != 1 )
#  error "WEP platform has only one flash bank!"
#endif


ulong flash_init (void)
{
	int i;
	FLASH_BUS address = WEP_FLASH_BASE;

	flash_info[0].size = WEP_FLASH_BANK_SIZE;
	flash_info[0].sector_count = CONFIG_SYS_MAX_FLASH_SECT;
	flash_info[0].flash_id = INTEL_MANUFACT;
	memset (flash_info[0].protect, 0, CONFIG_SYS_MAX_FLASH_SECT);

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_SECT; i++) {
		flash_info[0].start[i] = address;
#ifdef WEP_FLASH_UNLOCK
		/* Some devices are hw locked after start. */
		*((FLASH_BUS *) address) = FLASH_CMD (CFI_INTEL_CMD_LOCK_SETUP);
		*((FLASH_BUS *) address) = FLASH_CMD (CFI_INTEL_CMD_UNLOCK_BLOCK);
		flash_ready (0);
		*((FLASH_BUS *) address) = FLASH_CMD (CFI_INTEL_CMD_READ_ARRAY);
#endif
		address += WEP_FLASH_SECT_SIZE;
	}

	flash_protect (FLAG_PROTECT_SET,
				   CONFIG_SYS_FLASH_BASE,
				   CONFIG_SYS_FLASH_BASE + monitor_flash_len - 1,
				   &flash_info[0]);

	flash_protect (FLAG_PROTECT_SET,
				   CONFIG_ENV_ADDR,
				   CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1, &flash_info[0]);

	return WEP_FLASH_BANK_SIZE;
}

void flash_print_info (flash_info_t * info)
{
	int i;

	printf (" Intel vendor\n");
	printf ("  Size: %ld MB in %d Sectors\n",
			info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; i++) {
		if (!(i % 5)) {
			printf ("\n");
		}

		printf (" %08lX%s", info->start[i],
				info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");
}


int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int flag, non_protected = 0, sector;
	int rc = ERR_OK;

	FLASH_BUS *address;

	for (sector = s_first; sector <= s_last; sector++) {
		if (!info->protect[sector]) {
			non_protected++;
		}
	}

	if (!non_protected) {
		return ERR_PROTECTED;
	}

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */
	flag = disable_interrupts ();


	/* Start erase on unprotected sectors */
	for (sector = s_first; sector <= s_last && !ctrlc (); sector++) {
		if (info->protect[sector]) {
			printf ("Protected sector %2d skipping...\n", sector);
			continue;
		} else {
			printf ("Erasing sector %2d ... ", sector);
		}

		address = (FLASH_BUS *) (info->start[sector]);

		*address = FLASH_CMD (CFI_INTEL_CMD_BLOCK_ERASE);
		*address = FLASH_CMD (CFI_INTEL_CMD_CONFIRM);
		if (flash_ready (CONFIG_SYS_FLASH_ERASE_TOUT)) {
			*address = FLASH_CMD (CFI_INTEL_CMD_CLEAR_STATUS_REGISTER);
			printf ("ok.\n");
		} else {
			*address = FLASH_CMD (CFI_INTEL_CMD_SUSPEND);
			rc = ERR_TIMOUT;
			printf ("timeout! Aborting...\n");
			break;
		}
		*address = FLASH_CMD (CFI_INTEL_CMD_READ_ARRAY);
	}
	if (ctrlc ())
		printf ("User Interrupt!\n");

	/* allow flash to settle - wait 10 ms */
	udelay_masked (10000);
	if (flag) {
		enable_interrupts ();
	}

	return rc;
}

static int write_data (flash_info_t * info, ulong dest, FLASH_BUS data)
{
	FLASH_BUS *address = (FLASH_BUS *) dest;
	int rc = ERR_OK;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*address & data) != data) {
		return ERR_NOT_ERASED;
	}

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */

	flag = disable_interrupts ();

	*address = FLASH_CMD (CFI_INTEL_CMD_CLEAR_STATUS_REGISTER);
	*address = FLASH_CMD (CFI_INTEL_CMD_PROGRAM1);
	*address = data;

	if (!flash_ready (CONFIG_SYS_FLASH_WRITE_TOUT)) {
		*address = FLASH_CMD (CFI_INTEL_CMD_SUSPEND);
		rc = ERR_TIMOUT;
		printf ("timeout! Aborting...\n");
	}

	*address = FLASH_CMD (CFI_INTEL_CMD_READ_ARRAY);
	if (flag) {
		enable_interrupts ();
	}

	return rc;
}

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong read_addr, write_addr;
	FLASH_BUS data;
	int i, result = ERR_OK;


	read_addr = addr & ~(sizeof (FLASH_BUS) - 1);
	write_addr = read_addr;
	if (read_addr != addr) {
		data = 0;
		for (i = 0; i < sizeof (FLASH_BUS); i++) {
			if (read_addr < addr || cnt == 0) {
				data |= *((uchar *) read_addr) << i * 8;
			} else {
				data |= (*src++) << i * 8;
				cnt--;
			}
			read_addr++;
		}
		if ((result = write_data (info, write_addr, data)) != ERR_OK) {
			return result;
		}
		write_addr += sizeof (FLASH_BUS);
	}
	for (; cnt >= sizeof (FLASH_BUS); cnt -= sizeof (FLASH_BUS)) {
		if ((result = write_data (info, write_addr,
								  *((FLASH_BUS *) src))) != ERR_OK) {
			return result;
		}
		write_addr += sizeof (FLASH_BUS);
		src += sizeof (FLASH_BUS);
	}
	if (cnt > 0) {
		read_addr = write_addr;
		data = 0;
		for (i = 0; i < sizeof (FLASH_BUS); i++) {
			if (cnt > 0) {
				data |= (*src++) << i * 8;
				cnt--;
			} else {
				data |= *((uchar *) read_addr) << i * 8;
			}
			read_addr++;
		}
		if ((result = write_data (info, write_addr, data)) != 0) {
			return result;
		}
	}
	return ERR_OK;
}
