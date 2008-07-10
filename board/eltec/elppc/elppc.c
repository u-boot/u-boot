/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
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
#include <command.h>
#include <mpc106.h>
#include <video_fb.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */

int checkboard (void)
{
	puts ("Board: ELTEC PowerPC\n");
	return (0);
}

/* ------------------------------------------------------------------------- */

int checkflash (void)
{
	/* TODO */
	printf ("Test not implemented !\n");
	return (0);
}

/* ------------------------------------------------------------------------- */

static unsigned int mpc106_read_cfg_dword (unsigned int reg)
{
	unsigned int reg_addr = MPC106_REG | (reg & 0xFFFFFFFC);

	out32r (MPC106_REG_ADDR, reg_addr);

	return (in32r (MPC106_REG_DATA | (reg & 0x3)));
}

/* ------------------------------------------------------------------------- */

long int dram_size (int board_type)
{
	/*
	 * No actual initialisation to do - done when setting up
	 * PICRs MCCRs ME/SARs etc in asm_init.S.
	 */

	register unsigned long i, msar1, mear1, memSize;

#if defined(CFG_MEMTEST)
	register unsigned long reg;

	printf ("Testing DRAM\n");

	/* write each mem addr with it's address */
	for (reg = CFG_MEMTEST_START; reg < CFG_MEMTEST_END; reg += 4)
		*reg = reg;

	for (reg = CFG_MEMTEST_START; reg < CFG_MEMTEST_END; reg += 4) {
		if (*reg != reg)
			return -1;
	}
#endif

	/*
	 * Since MPC107 memory controller chip has already been set to
	 * control all memory, just read and interpret its memory boundery register.
	 */
	memSize = 0;
	msar1 = mpc106_read_cfg_dword (MPC106_MSAR1);
	mear1 = mpc106_read_cfg_dword (MPC106_MEAR1);
	i = mpc106_read_cfg_dword (MPC106_MBER) & 0xf;

	do {
		if (i & 0x01)			/* is bank enabled ? */
			memSize += (mear1 & 0xff) - (msar1 & 0xff) + 1;
		msar1 >>= 8;
		mear1 >>= 8;
		i >>= 1;
	} while (i);

	return (memSize * 0x100000);
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	return dram_size (board_type);
}

/* ------------------------------------------------------------------------- */

/*
 * The BAB 911 can be reset by writing bit 0 of the Processor Initialization
 * Register PI in the MPC 107 (at offset 0x41090 of the Embedded Utilities
 * Memory Block).
 */
int do_reset (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	out8 (MPC107_EUMB_PI, 1);
	return (0);
}

/* ------------------------------------------------------------------------- */

#if defined(CONFIG_WATCHDOG)

/*
 * Since the 7xx CPUs don't have an internal watchdog, this function is
 * board specific.
 */
void watchdog_reset (void)
{
}
#endif							/* CONFIG_WATCHDOG */

/* ------------------------------------------------------------------------- */

void after_reloc (ulong dest_addr)
{
	/*
	 * Jump to the main U-Boot board init code
	 */
	board_init_r ((gd_t *)gd, dest_addr);
}

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_CONSOLE_EXTRA_INFO
extern GraphicDevice smi;

void video_get_info_str (int line_number, char *info)
{
	/* init video info strings for graphic console */
	switch (line_number) {
	case 1:
		sprintf (info, " MPC7xx V%d.%d at %d / %d MHz",
				 (get_pvr () >> 8) & 0xFF, get_pvr () & 0xFF, 400, 100);
		return;
	case 2:
		sprintf (info, " ELTEC ELPPC with %ld MB DRAM and %ld MB FLASH",
				 dram_size (0) / 0x100000, flash_init () / 0x100000);
		return;
	case 3:
		sprintf (info, " %s", smi.modeIdent);
		return;
	}

	/* no more info lines */
	*info = 0;
	return;
}
#endif
