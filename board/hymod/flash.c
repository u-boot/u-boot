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
 *
 * Hacked for the Hymod board by Murray.Jensen@cmst.csiro.au, 20-Oct-00
 */

#include <common.h>
#include <mpc8260.h>
#include <board/hymod/flash.h>

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];	/* info for FLASH chips */

/*-----------------------------------------------------------------------
 * Protection Flags:
 */
#define FLAG_PROTECT_SET	0x01
#define FLAG_PROTECT_CLEAR	0x02

/*-----------------------------------------------------------------------
 * Functions
 */
#if 0
static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static void flash_get_offsets (ulong base, flash_info_t *info);
#endif
static int write_word (flash_info_t *info, ulong dest, ulong data);

/*-----------------------------------------------------------------------
 */

/*
 * probe for the existence of flash at bank word address "addr"
 * 0 = yes, 1 = bad Manufacturer's Id, 2 = bad Device Id
 */
static int
bank_probe_word(bank_addr_t addr)
{
	int retval;

	/* reset the flash */
	*addr = BANK_CMD_RST;

	/* check the manufacturer id */
	*addr = BANK_CMD_RD_ID;
	if (*BANK_ADDR_REG_MAN(addr) != BANK_RD_ID_MAN) {
		retval = -1;
		goto out;
	}

	/* check the device id */
	*addr = BANK_CMD_RD_ID;
	if (*BANK_ADDR_REG_DEV(addr) != BANK_RD_ID_DEV) {
		retval = -2;
		goto out;
	}

	retval = CFG_FLASH_TYPE;

out:
	/* reset the flash again */
	*addr = BANK_CMD_RST;

	return retval;
}

/*
 * probe for flash banks at address "base" and store info for any found
 * into flash_info entry "fip". Must find at least one bank.
 */
static void
bank_probe(flash_info_t *fip, bank_addr_t base)
{
	bank_addr_t addr, eaddr;
	int nbanks;

	fip->flash_id = FLASH_UNKNOWN;
	fip->size = 0L;
	fip->sector_count = 0;

	addr = base;
	eaddr = BANK_ADDR_BASE(addr, MAX_BANKS);
	nbanks = 0;

	while (addr < eaddr) {
		bank_addr_t addrw, eaddrw, addrb;
		int i, osc, nsc, curtype = -1;

		addrw = addr;
		eaddrw = BANK_ADDR_NEXT_WORD(addrw);

		while (addrw < eaddrw) {
			int thistype;

#ifdef FLASH_DEBUG
			printf("  probing for flash at addr 0x%08lx\n",
				(unsigned long)addrw);
#endif
			if ((thistype = bank_probe_word(addrw++)) < 0)
				goto out;

			if (curtype < 0)
				curtype = thistype;
			else {
				if (thistype != curtype) {
					printf("Differing flash type found!\n");
					goto out;
				}
			}
		}

		if (curtype < 0)
			goto out;

		/* bank exists - append info for this bank to *fip */
		fip->flash_id = FLASH_MAN_INTEL|curtype;
		fip->size += BANK_SIZE;
		osc = fip->sector_count;
		fip->sector_count += BANK_NBLOCKS;
		if ((nsc = fip->sector_count) >= CFG_MAX_FLASH_SECT)
			panic("Too many sectors in flash at address 0x%08lx\n",
				(unsigned long)base);

		addrb = addr;
		for (i = osc; i < nsc; i++) {
			fip->start[i] = (ulong)addrb;
			fip->protect[i] = 0;
			addrb = BANK_ADDR_NEXT_BLK(addrb);
		}

		addr = BANK_ADDR_NEXT_BANK(addr);
		nbanks++;
	}

out:
	if (nbanks == 0)
		panic("ERROR: no flash found at address 0x%08lx\n",
			(unsigned long)base);
}

static void
bank_reset(flash_info_t *info, int sect)
{
	bank_addr_t addrw, eaddrw;

	addrw = (bank_addr_t)info->start[sect];
	eaddrw = BANK_ADDR_NEXT_WORD(addrw);

	while (addrw < eaddrw) {
#ifdef FLASH_DEBUG
		printf("  writing reset cmd to addr 0x%08lx\n",
			(unsigned long)addrw);
#endif
		*addrw = BANK_CMD_RST;
		addrw++;
	}
}

static void
bank_erase_init(flash_info_t *info, int sect)
{
	bank_addr_t addrw, saddrw, eaddrw;
	int flag;

#ifdef FLASH_DEBUG
	printf("0x%08lx BANK_CMD_PROG\n", BANK_CMD_PROG);
	printf("0x%08lx BANK_CMD_ERASE1\n", BANK_CMD_ERASE1);
	printf("0x%08lx BANK_CMD_ERASE2\n", BANK_CMD_ERASE2);
	printf("0x%08lx BANK_CMD_CLR_STAT\n", BANK_CMD_CLR_STAT);
	printf("0x%08lx BANK_CMD_RST\n", BANK_CMD_RST);
	printf("0x%08lx BANK_STAT_RDY\n", BANK_STAT_RDY);
	printf("0x%08lx BANK_STAT_ERR\n", BANK_STAT_ERR);
#endif

	saddrw = (bank_addr_t)info->start[sect];
	eaddrw = BANK_ADDR_NEXT_WORD(saddrw);

#ifdef FLASH_DEBUG
	printf("erasing sector %d, start addr = 0x%08lx "
		"(bank next word addr = 0x%08lx)\n", sect,
		(unsigned long)saddrw, (unsigned long)eaddrw);
#endif

	/* Disable intrs which might cause a timeout here */
	flag = disable_interrupts();

	for (addrw = saddrw; addrw < eaddrw; addrw++) {
#ifdef FLASH_DEBUG
		printf("  writing erase cmd to addr 0x%08lx\n",
			(unsigned long)addrw);
#endif
		*addrw = BANK_CMD_ERASE1;
		*addrw = BANK_CMD_ERASE2;
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();
}

static int
bank_erase_poll(flash_info_t *info, int sect)
{
	bank_addr_t addrw, saddrw, eaddrw;
	int sectdone, haderr;

	saddrw = (bank_addr_t)info->start[sect];
	eaddrw = BANK_ADDR_NEXT_WORD(saddrw);

	sectdone = 1;
	haderr = 0;

	for (addrw = saddrw; addrw < eaddrw; addrw++) {
		bank_word_t stat = *addrw;

#ifdef FLASH_DEBUG
		printf("  checking status at addr "
			"0x%08lx [0x%08lx]\n",
			(unsigned long)addrw, stat);
#endif
		if ((stat & BANK_STAT_RDY) != BANK_STAT_RDY)
			sectdone = 0;
		else if ((stat & BANK_STAT_ERR) != 0) {
			printf(" failed on sector %d "
				"(stat = 0x%08lx) at "
				"address 0x%08lx\n",
				sect, stat,
				(unsigned long)addrw);
			*addrw = BANK_CMD_CLR_STAT;
			haderr = 1;
		}
	}

	if (haderr)
		return (-1);
	else
		return (sectdone);
}

static int
bank_write_word(bank_addr_t addr, bank_word_t value)
{
	bank_word_t stat;
	ulong start;
	int flag, retval;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	*addr = BANK_CMD_PROG;

	*addr = value;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	retval = 0;

	/* data polling for D7 */
	start = get_timer (0);
	do {
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			retval = 1;
			goto done;
		}
		stat = *addr;
	} while ((stat & BANK_STAT_RDY) != BANK_STAT_RDY);

	if ((stat & BANK_STAT_ERR) != 0) {
		printf("flash program failed (stat = 0x%08lx) "
			"at address 0x%08lx\n", (ulong)stat, (ulong)addr);
		*addr = BANK_CMD_CLR_STAT;
		retval = 3;
	}

done:
	/* reset to read mode */
	*addr = BANK_CMD_RST;

	return (retval);
}

/*-----------------------------------------------------------------------
 */

unsigned long
flash_init(void)
{
	int i;

	/* Init: no FLASHes known */
	for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	bank_probe(&flash_info[0], (bank_addr_t)CFG_FLASH_BASE);

	/*
	 * protect monitor and environment sectors
	 */

#if CFG_MONITOR_BASE == CFG_FLASH_BASE
	(void)flash_protect(FLAG_PROTECT_SET,
		      CFG_MONITOR_BASE,
		      CFG_MONITOR_BASE+CFG_MONITOR_LEN-1,
		      &flash_info[0]);
#endif

#if defined(CFG_FLASH_ENV_ADDR)
	(void)flash_protect(FLAG_PROTECT_SET,
		      CFG_FLASH_ENV_ADDR,
#if defined(CFG_FLASH_ENV_BUF)
		      CFG_FLASH_ENV_ADDR + CFG_FLASH_ENV_BUF - 1,
#else
		      CFG_FLASH_ENV_ADDR + CFG_FLASH_ENV_SIZE - 1,
#endif
		      &flash_info[0]);
#endif

	return flash_info[0].size;
}

/*-----------------------------------------------------------------------
 */
#if 0
static void
flash_get_offsets(ulong base, flash_info_t *info)
{
	int i;

	/* set up sector start adress table */
	if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block type	*/
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00008000;
		info->start[2] = base + 0x0000C000;
		info->start[3] = base + 0x00010000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00020000) - 0x00060000;
		}
	} else {
		/* set sector offsets for top boot block type		*/
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00008000;
		info->start[i--] = base + info->size - 0x0000C000;
		info->start[i--] = base + info->size - 0x00010000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00020000;
		}
	}

}
#endif	/* 0 */

/*-----------------------------------------------------------------------
 */
void
flash_print_info(flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_INTEL:	printf ("INTEL ");		break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F320J5:	printf ("28F320J5 (32 Mbit, 2 x 16bit)\n");
				break;
	default:		printf ("Unknown Chip Type\n");
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

/*
 * The following code cannot be run from FLASH!
 */
#if 0
static ulong
flash_get_size(vu_long *addr, flash_info_t *info)
{
	short i;
	ulong value;
	ulong base = (ulong)addr;


	/* Write auto select command: read Manufacturer ID */
	addr[0x0555] = 0x00AA00AA;
	addr[0x02AA] = 0x00550055;
	addr[0x0555] = 0x00900090;

	value = addr[0];

	switch (value) {
	case AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
		break;
	case FUJ_MANUFACT:
		info->flash_id = FLASH_MAN_FUJ;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);			/* no or unknown flash	*/
	}

	value = addr[1];			/* device ID		*/

	switch (value) {
	case AMD_ID_LV400T:
		info->flash_id += FLASH_AM400T;
		info->sector_count = 11;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/

	case AMD_ID_LV400B:
		info->flash_id += FLASH_AM400B;
		info->sector_count = 11;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/

	case AMD_ID_LV800T:
		info->flash_id += FLASH_AM800T;
		info->sector_count = 19;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/

	case AMD_ID_LV800B:
		info->flash_id += FLASH_AM800B;
		info->sector_count = 19;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/

	case AMD_ID_LV160T:
		info->flash_id += FLASH_AM160T;
		info->sector_count = 35;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/

	case AMD_ID_LV160B:
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/
#if 0	/* enable when device IDs are available */
	case AMD_ID_LV320T:
		info->flash_id += FLASH_AM320T;
		info->sector_count = 67;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/

	case AMD_ID_LV320B:
		info->flash_id += FLASH_AM320B;
		info->sector_count = 67;
		info->size = 0x00800000;
		break;				/* => 8 MB		*/
#endif
	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */

	}

	/* set up sector start adress table */
	if (info->flash_id & FLASH_BTYPE) {
		/* set sector offsets for bottom boot block type	*/
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00008000;
		info->start[2] = base + 0x0000C000;
		info->start[3] = base + 0x00010000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00020000) - 0x00060000;
		}
	} else {
		/* set sector offsets for top boot block type		*/
		i = info->sector_count - 1;
		info->start[i--] = base + info->size - 0x00008000;
		info->start[i--] = base + info->size - 0x0000C000;
		info->start[i--] = base + info->size - 0x00010000;
		for (; i >= 0; i--) {
			info->start[i] = base + i * 0x00020000;
		}
	}

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr = (volatile unsigned long *)(info->start[i]);
		info->protect[i] = addr[2] & 1;
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr = (volatile unsigned long *)info->start[0];

		*addr = 0x00F000F0;	/* reset bank */
	}

	return (info->size);
}
#endif /* 0 */


/*-----------------------------------------------------------------------
 */

int
flash_erase(flash_info_t *info, int s_first, int s_last)
{
	int prot, sect, haderr;
	ulong start, now, last;
	int rcode = 0;

#ifdef FLASH_DEBUG
	printf("\nflash_erase: erase %d sectors (%d to %d incl.) from\n"
		"  Bank # %d: ", s_last - s_first + 1, s_first, s_last,
		(info - flash_info) + 1);
	flash_print_info(info);
#endif

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	prot = 0;
	for (sect=s_first; sect<=s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf("- Warning: %d protected sector%s will not be erased!\n",
			prot, (prot > 1 ? "s" : ""));
	}

	start = get_timer (0);
	last = 0;
	haderr = 0;

	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			ulong estart;
			int sectdone;

			bank_erase_init(info, sect);

			/* wait at least 80us - let's wait 1 ms */
			udelay (1000);

			estart = get_timer(start);

			do {
				now = get_timer(start);

				if (now - estart > CFG_FLASH_ERASE_TOUT) {
					printf ("Timeout (sect %d)\n", sect);
					haderr = 1;
					rcode = 1;
					break;
				}

#ifndef FLASH_DEBUG
				/* show that we're waiting */
				if ((now - last) > 1000) { /* every second */
					putc ('.');
					last = now;
				}
#endif

				sectdone = bank_erase_poll(info, sect);

				if (sectdone < 0) {
					haderr = 1;
					rcode = 1;
					break;
				}

			} while (!sectdone);

			if (haderr)
				break;
		}
	}

	if (haderr > 0)
		printf (" failed\n");
	else
		printf (" done\n");

	/* reset to read mode */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			bank_reset(info, sect);
		}
	}
	return rcode;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

int
write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
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
static int
write_word(flash_info_t *info, ulong dest, ulong data)
{
	int retval;

	/* Check if Flash is (sufficiently) erased */
	if ((*(ulong *)dest & data) != data) {
		return (2);
	}

	retval = bank_write_word((bank_addr_t)dest, (bank_word_t)data);

	return (retval);
}

/*-----------------------------------------------------------------------
 */
