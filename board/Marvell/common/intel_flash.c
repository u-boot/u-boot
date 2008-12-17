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
 * Hacked for the marvell db64360 eval board by
 * Ingo Assmus <ingo.assmus@keymile.com>
 */

#include <common.h>
#include <mpc8xx.h>
#include "../include/mv_gen_reg.h"
#include "../include/memory.h"
#include "intel_flash.h"


/*-----------------------------------------------------------------------
 * Protection Flags:
 */
#define FLAG_PROTECT_SET	0x01
#define FLAG_PROTECT_CLEAR	0x02

static void bank_reset (flash_info_t * info, int sect)
{
	bank_addr_t addrw, eaddrw;

	addrw = (bank_addr_t) info->start[sect];
	eaddrw = BANK_ADDR_NEXT_WORD (addrw);

	while (addrw < eaddrw) {
#ifdef FLASH_DEBUG
		printf ("  writing reset cmd to addr 0x%08lx\n",
			(unsigned long) addrw);
#endif
		*addrw = BANK_CMD_RST;
		addrw++;
	}
}

static void bank_erase_init (flash_info_t * info, int sect)
{
	bank_addr_t addrw, saddrw, eaddrw;
	int flag;

#ifdef FLASH_DEBUG
	printf ("0x%08x BANK_CMD_PROG\n", BANK_CMD_PROG);
	printf ("0x%08x BANK_CMD_ERASE1\n", BANK_CMD_ERASE1);
	printf ("0x%08x BANK_CMD_ERASE2\n", BANK_CMD_ERASE2);
	printf ("0x%08x BANK_CMD_CLR_STAT\n", BANK_CMD_CLR_STAT);
	printf ("0x%08x BANK_CMD_RST\n", BANK_CMD_RST);
	printf ("0x%08x BANK_STAT_RDY\n", BANK_STAT_RDY);
	printf ("0x%08x BANK_STAT_ERR\n", BANK_STAT_ERR);
#endif

	saddrw = (bank_addr_t) info->start[sect];
	eaddrw = BANK_ADDR_NEXT_WORD (saddrw);

#ifdef FLASH_DEBUG
	printf ("erasing sector %d, start addr = 0x%08lx "
		"(bank next word addr = 0x%08lx)\n", sect,
		(unsigned long) saddrw, (unsigned long) eaddrw);
#endif

	/* Disable intrs which might cause a timeout here */
	flag = disable_interrupts ();

	for (addrw = saddrw; addrw < eaddrw; addrw++) {
#ifdef FLASH_DEBUG
		printf ("  writing erase cmd to addr 0x%08lx\n",
			(unsigned long) addrw);
#endif
		*addrw = BANK_CMD_ERASE1;
		*addrw = BANK_CMD_ERASE2;
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();
}

static int bank_erase_poll (flash_info_t * info, int sect)
{
	bank_addr_t addrw, saddrw, eaddrw;
	int sectdone, haderr;

	saddrw = (bank_addr_t) info->start[sect];
	eaddrw = BANK_ADDR_NEXT_WORD (saddrw);

	sectdone = 1;
	haderr = 0;

	for (addrw = saddrw; addrw < eaddrw; addrw++) {
		bank_word_t stat = *addrw;

#ifdef FLASH_DEBUG
		printf ("  checking status at addr "
			"0x%08x [0x%08x]\n", (unsigned long) addrw, stat);
#endif
		if ((stat & BANK_STAT_RDY) != BANK_STAT_RDY)
			sectdone = 0;
		else if ((stat & BANK_STAT_ERR) != 0) {
			printf (" failed on sector %d "
				"(stat = 0x%08x) at "
				"address 0x%p\n", sect, stat, addrw);
			*addrw = BANK_CMD_CLR_STAT;
			haderr = 1;
		}
	}

	if (haderr)
		return (-1);
	else
		return (sectdone);
}

int write_word_intel (bank_addr_t addr, bank_word_t value)
{
	bank_word_t stat;
	ulong start;
	int flag, retval;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	*addr = BANK_CMD_PROG;

	*addr = value;

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();

	retval = 0;

	/* data polling for D7 */
	start = get_timer (0);
	do {
		if (get_timer (start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			retval = 1;
			goto done;
		}
		stat = *addr;
	} while ((stat & BANK_STAT_RDY) != BANK_STAT_RDY);

	if ((stat & BANK_STAT_ERR) != 0) {
		printf ("flash program failed (stat = 0x%08lx) "
			"at address 0x%08lx\n", (ulong) stat, (ulong) addr);
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

int flash_erase_intel (flash_info_t * info, int s_first, int s_last)
{
	int prot, sect, haderr;
	ulong start, now, last;

#ifdef FLASH_DEBUG
	printf ("\nflash_erase: erase %d sectors (%d to %d incl.) from\n"
		"  Bank # %d: ", s_last - s_first + 1, s_first, s_last,
		(info - flash_info) + 1);
	flash_print_info (info);
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
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf ("- Warning: %d protected sector%s will not be erased!\n", prot, (prot > 1 ? "s" : ""));
	}

	start = get_timer (0);
	last = 0;
	haderr = 0;

	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			ulong estart;
			int sectdone;

			bank_erase_init (info, sect);

			/* wait at least 80us - let's wait 1 ms */
			udelay (1000);

			estart = get_timer (start);

			do {
				now = get_timer (start);

				if (now - estart > CONFIG_SYS_FLASH_ERASE_TOUT) {
					printf ("Timeout (sect %d)\n", sect);
					haderr = 1;
					break;
				}
#ifndef FLASH_DEBUG
				/* show that we're waiting */
				if ((now - last) > 1000) {	/* every second */
					putc ('.');
					last = now;
				}
#endif

				sectdone = bank_erase_poll (info, sect);

				if (sectdone < 0) {
					haderr = 1;
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
			bank_reset (info, sect);
		}
	}
	return haderr;
}
