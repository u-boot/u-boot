/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
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

/* #define DEBUG */

#include <common.h>
#include <environment.h>

static ulong flash_get_size (vu_long *addr, flash_info_t *info);

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];


#define CMD_READ_ARRAY		0x00F000F0
#define CMD_UNLOCK1		0x00AA00AA
#define CMD_UNLOCK2		0x00550055
#define CMD_ERASE_SETUP		0x00800080
#define CMD_ERASE_CONFIRM	0x00300030
#define CMD_PROGRAM		0x00A000A0
#define CMD_UNLOCK_BYPASS	0x00200020
#define CMD_READ_MANF_ID	0x00900090
#define CMD_UNLOCK_BYPASS_RES1	0x00900090
#define CMD_UNLOCK_BYPASS_RES2	0x00000000

#define MEM_FLASH_ADDR		(*(volatile u32 *)CFG_FLASH_BASE)
#define MEM_FLASH_ADDR1		(*(volatile u32 *)(CFG_FLASH_BASE + (0x00000555 << 2)))
#define MEM_FLASH_ADDR2		(*(volatile u32 *)(CFG_FLASH_BASE + (0x000002AA << 2)))

#define BIT_ERASE_DONE		0x00800080
#define BIT_RDY_MASK		0x00800080
#define BIT_PROGRAM_ERROR	0x00200020
#define BIT_TIMEOUT		0x80000000	/* our flag */

#define READY 1
#define ERR   2
#define TMO   4

/*-----------------------------------------------------------------------
 */

ulong flash_init (void)
{
	int i, j;
	ulong size = 0;

	for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
		ulong flashbase = 0;
		flash_info_t *info = &flash_info[i];

		/* Init: no FLASHes known */
		info->flash_id = FLASH_UNKNOWN;

		size += flash_get_size (CFG_FLASH_BASE, info);

		if (i == 0)
			flashbase = CFG_FLASH_BASE;
		else
			panic ("configured too many flash banks!\n");
		for (j = 0; j < info->sector_count; j++) {

			info->protect[j] = 0;
			info->start[j] = flashbase;

			switch (info->flash_id & FLASH_TYPEMASK) {
			case (FLASH_AM320B & FLASH_TYPEMASK):
			case (FLASH_MXLV320B & FLASH_TYPEMASK):
				/* Boot sector type: 8 x 8 + N x 128 kB */
				flashbase += (j < 8) ? 0x4000 : 0x20000;
				break;
			case (FLASH_AM640U & FLASH_TYPEMASK):
				/* Uniform sector type: 128 kB */
				flashbase += 0x20000;
				break;
			default:
				printf ("## Bad flash chip type 0x%04lX\n",
					info->flash_id & FLASH_TYPEMASK);
			}
		}
	}

	/*
	 * Protect monitor and environment sectors
	 */
	flash_protect ( FLAG_PROTECT_SET,
			CFG_FLASH_BASE,
			CFG_FLASH_BASE + monitor_flash_len - 1,
			&flash_info[0]);

	flash_protect ( FLAG_PROTECT_SET,
			CFG_ENV_ADDR,
			CFG_ENV_ADDR + CFG_ENV_SIZE - 1, &flash_info[0]);

#ifdef CFG_ENV_ADDR_REDUND
	flash_protect ( FLAG_PROTECT_SET,
			CFG_ENV_ADDR_REDUND,
			CFG_ENV_ADDR_REDUND + CFG_ENV_SIZE_REDUND - 1,
			&flash_info[0]);
#endif

	return size;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info (flash_info_t * info)
{
	int i;

	switch (info->flash_id & FLASH_VENDMASK) {
	case (FLASH_MAN_AMD & FLASH_VENDMASK):
			printf ("AMD ");		break;
	case (FLASH_MAN_FUJ & FLASH_VENDMASK):
			printf ("FUJITSU ");		break;
	case (FLASH_MAN_MX  & FLASH_VENDMASK):
			printf ("MACRONIX ");		break;
	default:	printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case (FLASH_AM320B & FLASH_TYPEMASK):
		printf ("2x Am29LV320DB (32Mbit)\n");
		break;
	case (FLASH_MXLV320B & FLASH_TYPEMASK):
		printf ("2x MX29LV320DB (32Mbit)\n");
		break;
	case (FLASH_AM640U & FLASH_TYPEMASK):
		printf ("2x Am29LV640D (64Mbit)\n");
		break;
	default:
		printf ("Unknown Chip Type\n");
		goto Done;
		break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
			info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; i++) {
		if ((i % 5) == 0) {
			printf ("\n   ");
		}
		printf (" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");

Done:	;
}

/*-----------------------------------------------------------------------
 */

int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	ulong result;

#if 0
	int cflag;
#endif
	int iflag, prot, sect;
	int rc = ERR_OK;
	int chip1, chip2;

	debug ("flash_erase: s_first %d  s_last %d\n", s_first, s_last);

	/* first look for protection bits */

	if (info->flash_id == FLASH_UNKNOWN)
		return ERR_UNKNOWN_FLASH_TYPE;

	if ((s_first < 0) || (s_first > s_last)) {
		return ERR_INVAL;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case (FLASH_MAN_AMD & FLASH_VENDMASK):	break;	/* OK */
	case (FLASH_MAN_FUJ & FLASH_VENDMASK):	break;	/* OK */
	case (FLASH_MAN_MX  & FLASH_VENDMASK):	break;	/* OK */
	default:
		debug ("## flash_erase: unknown manufacturer\n");
		return (ERR_UNKNOWN_FLASH_VENDOR);
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
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

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */
#if 0
	cflag = icache_status ();
	icache_disable ();
#endif
	iflag = disable_interrupts ();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last && !ctrlc (); sect++) {

		debug ("Erasing sector %2d @ %08lX... ",
			sect, info->start[sect]);

		/* arm simple, non interrupt dependent timer */
		reset_timer_masked ();

		if (info->protect[sect] == 0) {	/* not protected */
			vu_long *addr = (vu_long *) (info->start[sect]);

			MEM_FLASH_ADDR1 = CMD_UNLOCK1;
			MEM_FLASH_ADDR2 = CMD_UNLOCK2;
			MEM_FLASH_ADDR1 = CMD_ERASE_SETUP;

			MEM_FLASH_ADDR1 = CMD_UNLOCK1;
			MEM_FLASH_ADDR2 = CMD_UNLOCK2;
			*addr = CMD_ERASE_CONFIRM;

			/* wait until flash is ready */
			chip1 = chip2 = 0;

			do {
				result = *addr;

				/* check timeout */
				if (get_timer_masked () > CFG_FLASH_ERASE_TOUT) {
					MEM_FLASH_ADDR1 = CMD_READ_ARRAY;
					chip1 = TMO;
					break;
				}

				if (!chip1 && (result & 0xFFFF) & BIT_ERASE_DONE)
					chip1 = READY;

				if (!chip1 && (result & 0xFFFF) & BIT_PROGRAM_ERROR)
					chip1 = ERR;

				if (!chip2 && (result >> 16) & BIT_ERASE_DONE)
					chip2 = READY;

				if (!chip2 && (result >> 16) & BIT_PROGRAM_ERROR)
					chip2 = ERR;

			} while (!chip1 || !chip2);

			MEM_FLASH_ADDR1 = CMD_READ_ARRAY;

			if (chip1 == ERR || chip2 == ERR) {
				rc = ERR_PROG_ERROR;
				printf ("Flash erase error\n");
				goto outahere;
			}
			if (chip1 == TMO) {
				rc = ERR_TIMOUT;
				printf ("Flash erase timeout error\n");
				goto outahere;
			}
		}
	}

outahere:
	/* allow flash to settle - wait 10 ms */
	udelay_masked (10000);

	if (iflag)
		enable_interrupts ();

#if 0
	if (cflag)
		icache_enable ();
#endif
	return rc;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash
 */

static int write_word (flash_info_t * info, ulong dest, ulong data)
{
	vu_long *addr = (vu_long *) dest;
	ulong result;
	int rc = ERR_OK;

#if 0
	int cflag;
#endif
	int iflag;
	int chip1, chip2;

	/*
	 * Check if Flash is (sufficiently) erased
	 */
	result = *addr;
	if ((result & data) != data)
		return ERR_NOT_ERASED;

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */
#if 0
	cflag = icache_status ();
	icache_disable ();
#endif
	iflag = disable_interrupts ();

	MEM_FLASH_ADDR1 = CMD_UNLOCK1;
	MEM_FLASH_ADDR2 = CMD_UNLOCK2;
	MEM_FLASH_ADDR1 = CMD_PROGRAM;
	*addr = data;

	/* arm simple, non interrupt dependent timer */
	reset_timer_masked ();

	/* wait until flash is ready */
	chip1 = chip2 = 0;
	do {
		result = *addr;

		/* check timeout */
		if (get_timer_masked () > CFG_FLASH_WRITE_TOUT) {
			chip1 = ERR | TMO;
			break;
		}
		if (!chip1 && ((result & 0x80) == (data & 0x80)))
			chip1 = READY;

		if (!chip1 && ((result & 0xFFFF) & BIT_PROGRAM_ERROR)) {
			result = *addr;

			if ((result & 0x80) == (data & 0x80))
				chip1 = READY;
			else
				chip1 = ERR;
		}

		if (!chip2 && ((result & (0x80 << 16)) == (data & (0x80 << 16))))
			chip2 = READY;

		if (!chip2 && ((result >> 16) & BIT_PROGRAM_ERROR)) {
			result = *addr;

			if ((result & (0x80 << 16)) == (data & (0x80 << 16)))
				chip2 = READY;
			else
				chip2 = ERR;
		}

	} while (!chip1 || !chip2);

	*addr = CMD_READ_ARRAY;

	if (chip1 == ERR || chip2 == ERR || *addr != data) {
		rc = ERR_PROG_ERROR;
		printf ("Flash program error\n");
		debug ("chip1: %#x, chip2: %#x, addr: %#lx *addr: %#lx, "
		       "data: %#lx\n",
		       chip1, chip2, addr, *addr, data);
	}

	if (iflag)
		enable_interrupts ();

#if 0
	if (cflag)
		icache_enable ();
#endif

	return rc;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash.
 */

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
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
		for (i = 0, cp = wp; i < l; ++i, ++cp) {
			data = (data >> 8) | (*(uchar *) cp << 24);
		}
		for (; i < 4 && cnt > 0; ++i) {
			data = (data >> 8) | (*src++ << 24);
			--cnt;
			++cp;
		}
		for (; cnt == 0 && i < 4; ++i, ++cp) {
			data = (data >> 8) | (*(uchar *) cp << 24);
		}

		if ((rc = write_word (info, wp, data)) != 0) {
			goto Done;
		}
		wp += 4;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= 4) {
		if (((ulong)src) & 0x3) {
			for (i = 0; i < 4; i++) {
				((char *)&data)[i] = ((vu_char *)src)[i];
			}
		}
		else {
			data = *((vu_long *) src);
		}

		if ((rc = write_word (info, wp, data)) != 0) {
			goto Done;
		}
		src += 4;
		wp += 4;
		cnt -= 4;
	}

	if (cnt == 0) {
		rc = ERR_OK;
		goto Done;
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i = 0, cp = wp; i < 4 && cnt > 0; ++i, ++cp) {
		data = (data >> 8) | (*src++ << 24);
		--cnt;
	}
	for (; i < 4; ++i, ++cp) {
		data = (data >> 8) | (*(uchar *) cp << 24);
	}

	rc = write_word (info, wp, data);

	Done:

	return (rc);
}

/*-----------------------------------------------------------------------
 */

static ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
	ulong value;

	/* Write auto select command sequence and read Manufacturer ID */
	addr[0x0555] = CMD_UNLOCK1;
	addr[0x02AA] = CMD_UNLOCK2;
	addr[0x0555] = CMD_READ_MANF_ID;

	value = addr[0];

	debug ("Manuf. ID @ 0x%08lx: 0x%08lx\n", (ulong)addr, value);

	switch (value) {
	case AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
		break;
	case FUJ_MANUFACT:
		info->flash_id = FLASH_MAN_FUJ;
		break;
	case MX_MANUFACT:
		info->flash_id = FLASH_MAN_MX;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		addr[0] = CMD_READ_ARRAY;	/* restore read mode */
		debug ("## flash_init: unknown manufacturer\n");
		return (0);			/* no or unknown flash	*/
	}

	value = addr[1];			/* device ID		*/

	debug ("Device ID @ 0x%08lx: 0x%08lx\n", (ulong)(&addr[1]), value);

	switch (value) {
	case AMD_ID_LV320B:
		info->flash_id += FLASH_AM320B;
		info->sector_count = 71;
		info->size = 0x00800000;

		addr[0] = CMD_READ_ARRAY;	/* restore read mode */
		break;				/* =>  8 MB		*/

	case AMD_ID_LV640U:
		info->flash_id += FLASH_AM640U;
		info->sector_count = 128;
		info->size = 0x01000000;

		addr[0] = CMD_READ_ARRAY;	/* restore read mode */
		break;				/* => 16 MB		*/

	case MX_ID_LV320B:
		info->flash_id += FLASH_MXLV320B;
		info->sector_count = 71;
		info->size = 0x00800000;

		addr[0] = CMD_READ_ARRAY;	/* restore read mode */
		break;				/* =>  8 MB		*/

	default:
		debug ("## flash_init: unknown flash chip\n");
		info->flash_id = FLASH_UNKNOWN;
		addr[0] = CMD_READ_ARRAY;	/* restore read mode */
		return (0);			/* => no or unknown flash */

	}

	if (info->sector_count > CFG_MAX_FLASH_SECT) {
		printf ("** ERROR: sector count %d > max (%d) **\n",
			info->sector_count, CFG_MAX_FLASH_SECT);
		info->sector_count = CFG_MAX_FLASH_SECT;
	}

	return (info->size);
}
