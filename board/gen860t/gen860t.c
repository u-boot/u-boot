/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Keith Outwater, keith_outwater@mvis.com
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

#include <virtex2.h>
#include <common.h>
#include <mpc8xx.h>
#include <asm/8xx_immap.h>
#include "beeper.h"
#include "fpga.h"
#include "ioport.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_STATUS_LED
#include <status_led.h>
#endif

#if defined(CONFIG_CMD_MII) && defined(CONFIG_MII)
#include <net.h>
#endif

#if 0
#define GEN860T_DEBUG
#endif

#ifdef GEN860T_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define	PRINTF(fmt,args...)
#endif

/*
 * The following UPM init tables were generated automatically by
 * Motorola's MCUINIT program. See the README file for UPM to
 * SDRAM pin assignments if you want to type this data into
 * MCUINIT in order to reverse engineer the waveforms.
 */

/*
 * UPM initialization tables for MICRON MT48LC16M16A2TG SDRAM devices
 * (UPMA) and Virtex FPGA SelectMap interface (UPMB).
 * NOTE that unused areas of the table are used to hold NOP, precharge
 * and mode register set sequences.
 *
 */
#define	UPMA_NOP_ADDR			0x5
#define	UPMA_PRECHARGE_ADDR		0x6
#define UPMA_MRS_ADDR			0x12

#define UPM_SINGLE_READ_ADDR	0x00
#define UPM_BURST_READ_ADDR		0x08
#define UPM_SINGLE_WRITE_ADDR	0x18
#define UPM_BURST_WRITE_ADDR	0x20
#define	UPM_REFRESH_ADDR		0x30

const uint sdram_upm_table[] = {
	/* single read   (offset 0x00 in upm ram) */
	0x0e0fdc04, 0x01adfc04, 0x0fbffc00, 0x1fff5c05,
	0xffffffff, 0x0fffffcd, 0x0fff0fce, 0xefcfffff,
	/* burst read    (offset 0x08 in upm ram) */
	0x0f0fdc04, 0x00fdfc04, 0xf0fffc00, 0xf0fffc00,
	0xf1fffc00, 0xfffffc00, 0xfffffc05, 0xffffffff,
	0xffffffff, 0xffffffff, 0x0ffffff4, 0x1f3d5ff4,
	0xfffffff4, 0xfffffff5, 0xffffffff, 0xffffffff,
	/* single write  (offset 0x18 in upm ram) */
	0x0f0fdc04, 0x00ad3c00, 0x1fff5c05, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	/* burst write   (offset 0x20 in upm ram) */
	0x0f0fdc00, 0x10fd7c00, 0xf0fffc00, 0xf0fffc00,
	0xf1fffc04, 0xfffffc05, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xfffff7ff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	/* refresh       (offset 0x30 in upm ram) */
	0x1ffddc84, 0xfffffc04, 0xfffffc04, 0xfffffc84,
	0xfffffc05, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	/* exception     (offset 0x3C in upm ram) */
};

const uint selectmap_upm_table[] = {
	/* single read   (offset 0x00 in upm ram) */
	0x88fffc06, 0x00fff404, 0x00fffc04, 0x33fffc00,
	0xfffffc05, 0xffffffff, 0xffffffff, 0xffffffff,
	/* burst read    (offset 0x08 in upm ram) */
	0xfffffc04, 0xfffffc05, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	/* single write  (offset 0x18 in upm ram) */
	0x88fffc04, 0x00fff400, 0x77fffc05, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	/* burst write   (offset 0x20 in upm ram) */
	0xfffffc04, 0xfffffc05, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	/* refresh       (offset 0x30 in upm ram) */
	0xfffffc04, 0xfffffc05, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	/* exception     (offset 0x3C in upm ram) */
	0xfffffc05, 0xffffffff, 0xffffffff, 0xffffffff
};

/*
 * Check board identity.  Always successful (gives information only)
 */
int checkboard (void)
{
	char *s;
	char buf[64];
	int i;

	i = getenv_r ("board_id", buf, sizeof (buf));
	s = (i > 0) ? buf : NULL;

	if (s) {
		printf ("%s ", s);
	} else {
		printf ("<unknown> ");
	}

	i = getenv_r ("serial#", buf, sizeof (buf));
	s = (i > 0) ? buf : NULL;

	if (s) {
		printf ("S/N %s\n", s);
	} else {
		printf ("S/N <unknown>\n");
	}

	printf ("CPU at %s MHz, ", strmhz (buf, gd->cpu_clk));
	printf ("local bus at %s MHz\n", strmhz (buf, gd->bus_clk));
	return (0);
}

/*
 * Initialize SDRAM
 */
phys_size_t initdram (int board_type)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immr->im_memctl;

	upmconfig (UPMA,
		   (uint *) sdram_upm_table,
		   sizeof (sdram_upm_table) / sizeof (uint)
		);

	/*
	 * Setup MAMR register
	 */
	memctl->memc_mptpr = CONFIG_SYS_MPTPR_1BK_8K;
	memctl->memc_mamr = CONFIG_SYS_MAMR_8COL & (~(MAMR_PTAE));	/* no refresh yet */

	/*
	 * Map CS1* to SDRAM bank
	 */
	memctl->memc_or1 = CONFIG_SYS_OR1;
	memctl->memc_br1 = CONFIG_SYS_BR1;

	/*
	 * Perform SDRAM initialization sequence:
	 * 1. Apply at least one NOP command
	 * 2. 100 uS delay (JEDEC standard says 200 uS)
	 * 3. Issue 4 precharge commands
	 * 4. Perform two refresh cycles
	 * 5. Program mode register
	 *
	 * Program SDRAM for standard operation, sequential burst, burst length
	 * of 4, CAS latency of 2.
	 */
	memctl->memc_mar = 0x00000000;
	memctl->memc_mcr = MCR_UPM_A | MCR_OP_RUN | MCR_MB_CS1 |
		MCR_MLCF (0) | UPMA_NOP_ADDR;
	udelay (200);
	memctl->memc_mar = 0x00000000;
	memctl->memc_mcr = MCR_UPM_A | MCR_OP_RUN | MCR_MB_CS1 |
		MCR_MLCF (4) | UPMA_PRECHARGE_ADDR;

	memctl->memc_mar = 0x00000000;
	memctl->memc_mcr = MCR_UPM_A | MCR_OP_RUN | MCR_MB_CS1 |
		MCR_MLCF (2) | UPM_REFRESH_ADDR;

	memctl->memc_mar = 0x00000088;
	memctl->memc_mcr = MCR_UPM_A | MCR_OP_RUN | MCR_MB_CS1 |
		MCR_MLCF (1) | UPMA_MRS_ADDR;

	memctl->memc_mar = 0x00000000;
	memctl->memc_mcr = MCR_UPM_A | MCR_OP_RUN | MCR_MB_CS1 |
		MCR_MLCF (0) | UPMA_NOP_ADDR;
	/*
	 * Enable refresh
	 */
	memctl->memc_mamr |= MAMR_PTAE;

	return (SDRAM_SIZE);
}

/*
 * Disk On Chip (DOC) Millenium initialization.
 * The DOC lives in the CS2* space
 */
#if defined(CONFIG_CMD_DOC)
void doc_init (void)
{
	printf ("Probing at 0x%.8x: ", DOC_BASE);
	doc_probe (DOC_BASE);
}
#endif

/*
 * Miscellaneous intialization
 */
int misc_init_r (void)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immr->im_memctl;

	/*
	 * Set up UPMB to handle the Virtex FPGA SelectMap interface
	 */
	upmconfig (UPMB, (uint *) selectmap_upm_table,
		   sizeof (selectmap_upm_table) / sizeof (uint));

	memctl->memc_mbmr = 0x0;

	config_mpc8xx_ioports (immr);

#if defined(CONFIG_CMD_MII)
	mii_init ();
#endif

#if defined(CONFIG_FPGA)
	gen860t_init_fpga ();
#endif
	return 0;
}

/*
 * Final init hook before entering command loop.
 */
int last_stage_init (void)
{
#if !defined(CONFIG_SC)
	char buf[256];
	int i;

	/*
	 * Turn the beeper volume all the way down in case this is a warm boot.
	 */
	set_beeper_volume (-64);
	init_beeper ();

	/*
	 * Read the environment to see what to do with the beeper
	 */
	i = getenv_r ("beeper", buf, sizeof (buf));
	if (i > 0) {
		do_beeper (buf);
	}
#endif
	return 0;
}

/*
 * Stub to make POST code happy.  Can't self-poweroff, so just hang.
 */
void board_poweroff (void)
{
	puts ("### Please power off the board ###\n");
	while (1);
}
