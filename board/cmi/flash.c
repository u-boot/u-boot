/*
 * (C) Copyright 2003
 * Martin Winistoerfer, martinwinistoerfer@gmx.ch.
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

/*
 * File:		flash.c
 *
 * Discription:		This Driver is for 28F320J3A, 28F640J3A and
 * 			28F128J3A Intel flashs working in 16 Bit mode.
 *			They are single bank flashs.
 *
 *			Most of this code is taken from existing u-boot
 *			source code.
 */


#include <common.h>
#include <mpc5xx.h>

#if defined(CFG_ENV_IS_IN_FLASH)
# ifndef  CFG_ENV_ADDR
#  define CFG_ENV_ADDR	(CFG_FLASH_BASE + CFG_ENV_OFFSET)
# endif
# ifndef  CFG_ENV_SIZE
#  define CFG_ENV_SIZE	CFG_ENV_SECT_SIZE
# endif
# ifndef  CFG_ENV_SECT_SIZE
#  define CFG_ENV_SECT_SIZE  CFG_ENV_SIZE
# endif
#endif

#define	FLASH_ID_MASK			0xFFFF
#define FLASH_BLOCK_SIZE		0x00010000
#define FLASH_CMD_READ_ID		0x0090
#define FLASH_CMD_RESET			0x00ff
#define FLASH_CMD_BLOCK_ERASE		0x0020
#define FLASH_CMD_ERASE_CONFIRM		0x00D0
#define FLASH_CMD_CLEAR_STATUS		0x0050
#define FLASH_CMD_SUSPEND_ERASE		0x00B0
#define FLASH_CMD_WRITE			0x0040
#define FLASH_CMD_PROTECT		0x0060
#define FLASH_CMD_PROTECT_SET		0x0001
#define FLASH_CMD_PROTECT_CLEAR		0x00D0
#define FLASH_STATUS_DONE		0x0080

flash_info_t	flash_info[CFG_MAX_FLASH_BANKS];

/*
 * Local function prototypes
 */
static ulong 	flash_get_size 		(vu_short *addr, flash_info_t *info);
static int 	write_short 		(flash_info_t *info, ulong dest, ushort data);
static void 	flash_get_offsets 	(ulong base, flash_info_t *info);

/*
 * Initialize flash
 */

unsigned long flash_init (void)
{
	unsigned long size_b0;
	int i;

	/* Init: no FLASHes known */
	for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Static FLASH Bank configuration here - FIXME XXX */
#if 1
	debug ("\n## Get flash bank 1 size @ 0x%08x\n",FLASH_BASE0_PRELIM);
#endif
	size_b0 = flash_get_size((vu_short *)FLASH_BASE0_PRELIM, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0: "
			"ID 0x%lx, Size = 0x%08lx = %ld MB\n",
			flash_info[0].flash_id,
			size_b0, size_b0<<20);
	}

	flash_get_offsets (FLASH_BASE0_PRELIM, &flash_info[0]);

	flash_info[0].size = size_b0;

#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CFG_MONITOR_BASE,
		      CFG_MONITOR_BASE+monitor_flash_len-1,
		      &flash_info[0]);
#endif

#ifdef	CFG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CFG_ENV_ADDR,
		      CFG_ENV_ADDR+CFG_ENV_SECT_SIZE-1,
		      &flash_info[0]);
#endif

	return size_b0;
}

/*
 * Compute start adress of each sector (block)
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
		info->start[i] = base + i * FLASH_BLOCK_SIZE;
	    }
	    return;

	default:
	    printf ("Don't know sector offsets for flash type 0x%lx\n",
		info->flash_id);
	    return;
	}
}

/*
 * Print flash information
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
	case FLASH_28F320J3A:	printf ("28F320J3A (32Mbit) 16-Bit\n");
				break;
	case FLASH_28F640J3A:	printf ("28F640J3A (64Mbit) 16-Bit\n");
				break;
	case FLASH_28F128J3A:	printf ("28F128J3A (128Mbit) 16-Bit\n");
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

/*
 * Get size of flash in bytes.
 * The following code cannot be run from FLASH!
 */

static ulong flash_get_size (vu_short *addr, flash_info_t *info)
{
	vu_short value;

	/* Read Manufacturer ID */
	addr[0] = FLASH_CMD_READ_ID;
	value = addr[0];

	switch (value) {
	case (AMD_MANUFACT & FLASH_ID_MASK):
		info->flash_id = FLASH_MAN_AMD;
		break;
	case (FUJ_MANUFACT & FLASH_ID_MASK):
		info->flash_id = FLASH_MAN_FUJ;
		break;
	case (SST_MANUFACT & FLASH_ID_MASK):
		info->flash_id = FLASH_MAN_SST;
		break;
	case (STM_MANUFACT & FLASH_ID_MASK):
		info->flash_id = FLASH_MAN_STM;
		break;
	case (INTEL_MANUFACT & FLASH_ID_MASK):
		info->flash_id = FLASH_MAN_INTEL;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		addr[0] = FLASH_CMD_RESET;	/* restore read mode */
		return (0);			/* no or unknown flash	*/
	}

	value = addr[1];			/* device ID		*/

	switch (value) {
	case (INTEL_ID_28F320J3A  & FLASH_ID_MASK):
		info->flash_id += FLASH_28F320J3A;
		info->sector_count = 32;
		info->size = 0x00400000;
		break;				/* =>  32 MBit		*/

	case (INTEL_ID_28F640J3A & FLASH_ID_MASK):
		info->flash_id += FLASH_28F640J3A;
		info->sector_count = 64;
		info->size = 0x00800000;
		break;				/* => 64 MBit		*/

	case (INTEL_ID_28F128J3A & FLASH_ID_MASK):
		info->flash_id += FLASH_28F128J3A;
		info->sector_count = 128;
		info->size = 0x01000000;
		break;				/* => 128 MBit		*/

	default:
		info->flash_id = FLASH_UNKNOWN;
		addr[0] = FLASH_CMD_RESET;	/* restore read mode */
		return (0);			/* => no or unknown flash */

	}

	if (info->sector_count > CFG_MAX_FLASH_SECT) {
		printf ("** ERROR: sector count %d > max (%d) **\n",
			info->sector_count, CFG_MAX_FLASH_SECT);
		info->sector_count = CFG_MAX_FLASH_SECT;
	}

	addr[0] = FLASH_CMD_RESET;		/* restore read mode */

	return (info->size);
}


/*
 * Erase unprotected sectors
 */

int flash_erase (flash_info_t *info, int s_first, int s_last)
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
		printf ("- Warning: %d protected sectors will not be erased!\n",
			prot);
	} else {
		printf ("\n");
	}

	start = get_timer (0);
	last  = start;

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			vu_short *addr = (vu_short *)(info->start[sect]);
			unsigned long status;

			/* Disable interrupts which might cause a timeout here */
			flag = disable_interrupts();

#ifdef DEBUG
			printf("Erase sector %d at start addr 0x%08X", sect, (unsigned int)info->start[sect]);
#endif

			*addr = FLASH_CMD_CLEAR_STATUS;
			*addr = FLASH_CMD_BLOCK_ERASE;
			*addr = FLASH_CMD_ERASE_CONFIRM;

			/* re-enable interrupts if necessary */
			if (flag)
				enable_interrupts();

			/* wait at least 80us - let's wait 1 ms */
			udelay (1000);

			while (((status = *addr) & FLASH_STATUS_DONE) != FLASH_STATUS_DONE) {
				if ((now=get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
					printf("Flash erase timeout at address %lx\n", info->start[sect]);
					*addr = FLASH_CMD_SUSPEND_ERASE;
					*addr = FLASH_CMD_RESET;
					return 1;
				}

				/* show that we're waiting */
				if ((now - last) > 1000) {	/* every second */
					putc ('.');
					last = now;
				}
			}
			*addr = FLASH_CMD_RESET;
		}
	}
	printf (" done\n");
	return 0;
}

/*
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 4 - Flash not identified
 */

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong cp, wp;
	ushort data;
	int i, rc;

	if (info->flash_id == FLASH_UNKNOWN) {
		return 4;
	}

	wp = (addr & ~1);	/* get lower word aligned address */

	/*
	 * handle unaligned start byte
	 */

	if (addr - wp) {
		data = 0;
		data = (data << 8) | *src++;
		--cnt;
		if ((rc = write_short(info, wp, data)) != 0) {
			return (rc);
		}
		wp += 2;
	}

	/*
	 * handle word aligned part
	 */

	while (cnt >= 2) {
		data = 0;
		for (i=0; i<2; ++i) {
			data = (data << 8) | *src++;
		}

		if ((rc = write_short(info, wp, data)) != 0) {
			return (rc);
		}
		wp  += 2;
		cnt -= 2;
	}

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */

	data = 0;
	for (i=0, cp=wp; i<2 && cnt>0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i<2; ++i, ++cp) {
		data = (data << 8) | (*(uchar *)cp);
	}

	return (write_short(info, wp, data));

}

/*
 * Write 16 bit (short) to flash
 */

static int write_short (flash_info_t *info, ulong dest, ushort data)
{
	vu_short *addr = (vu_short*)(info->start[0]);
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*((vu_short *)dest) & data) != data) {
		return (2);
	}

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	if (!(info->flash_id & FLASH_VENDMASK)) {
		return 4;
	}
	*addr = FLASH_CMD_ERASE_CONFIRM;
	*addr = FLASH_CMD_WRITE;

	*((vu_short *)dest) = data;

	/* re-enable interrupts if necessary */
	if (flag) {
		enable_interrupts();
	}

	/* data polling for D7 */
	start = get_timer (0);

	/* wait for error or finish */
	while(!(addr[0] & FLASH_STATUS_DONE)){
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			addr[0] = FLASH_CMD_RESET;
			return (1);
		}
	}

	*addr = FLASH_CMD_RESET;
	return (0);
}

/*
 * Protects a flash sector
 */

int flash_real_protect(flash_info_t *info, long sector, int prot)
{
	vu_short *addr = (vu_short*)(info->start[sector]);
	ulong start;

	*addr = FLASH_CMD_CLEAR_STATUS;
	*addr = FLASH_CMD_PROTECT;

	if(prot) {
		*addr = FLASH_CMD_PROTECT_SET;
	} else {
		*addr = FLASH_CMD_PROTECT_CLEAR;
	}

	/* wait for error or finish */
	start = get_timer (0);
	while(!(addr[0] & FLASH_STATUS_DONE)){
		if (get_timer(start) > CFG_FLASH_ERASE_TOUT) {
			printf("Flash protect timeout at address %lx\n",  info->start[sector]);
			addr[0] = FLASH_CMD_RESET;
			return (1);
		}
	}
	/* Set software protect flag */
	info->protect[sector] = prot;
	*addr = FLASH_CMD_RESET;
	return (0);
}
