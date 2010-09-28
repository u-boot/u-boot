/*
 * Copyright (c) 2004 Picture Elements, Inc.
 *    Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  <common.h>
# include  <asm/ppc4xx.h>
# include  <asm/processor.h>
# include  <asm/io.h>
# include  "jse_priv.h"

/*
 * This function is run very early, out of flash, and before devices are
 * initialized. It is called by arch/powerpc/lib/board.c:board_init_f by virtue
 * of being in the init_sequence array.
 *
 * The SDRAM has been initialized already -- start.S:start called
 * init.S:init_sdram early on -- but it is not yet being used for
 * anything, not even stack. So be careful.
 */
int board_early_init_f (void)
{
   /*-------------------------------------------------------------------------+
   | Interrupt controller setup for the JSE board.
   | Note: IRQ 0-15  405GP internally generated; active high; level sensitive
   |       IRQ 16    405GP internally generated; active low; level sensitive
   |       IRQ 17-24 RESERVED/UNUSED
   |       IRQ 25 (EXT IRQ 0) PCI SLOT 0; active low; level sensitive
   |       IRQ 26 (EXT IRQ 1) PCI SLOT 1; active low; level sensitive
   |       IRQ 27 (EXT IRQ 2) JP2C CHIP ; active low; level sensitive
   |       IRQ 28 (EXT IRQ 3) PCI bridge; active low; level sensitive
   |       IRQ 29 (EXT IRQ 4) SystemACE IRQ; active high
   |       IRQ 30 (EXT IRQ 5) SystemACE BRdy (unused)
   |       IRQ 31 (EXT IRQ 6) (unused)
   +-------------------------------------------------------------------------*/
	mtdcr (UIC0SR, 0xFFFFFFFF);	/* clear all ints */
	mtdcr (UIC0ER, 0x00000000);	/* disable all ints */
	mtdcr (UIC0CR, 0x00000000);	/* set all to be non-critical */
	mtdcr (UIC0PR, 0xFFFFFF87);	/* set int polarities */
	mtdcr (UIC0TR, 0x10000000);	/* set int trigger levels */
	mtdcr (UIC0SR, 0xFFFFFFFF);	/* clear all ints */

	/* Configure the interface to the SystemACE MCU port.
	   The SystemACE is fast, but there is no reason to have
	   excessivly tight timings. So the settings are slightly
	   generous. */

	/* EBC0_B1AP: BME=1, TWT=2, CSN=0, OEN=1,
	   WBN=0, WBF=1, TH=0,  RE=0,  SOR=0, BEM=0, PEN=0 */
	mtdcr (EBC0_CFGADDR, PB1AP);
	mtdcr (EBC0_CFGDATA, 0x01011000);

	/* EBC0_B1CR: BAS=x, BS=0(1MB), BU=3(R/W), BW=0(8bits) */
	mtdcr (EBC0_CFGADDR, PB1CR);
	mtdcr (EBC0_CFGDATA, CONFIG_SYS_SYSTEMACE_BASE | 0x00018000);

	/* Enable the /PerWE output as /PerWE, instead of /PCIINT. */
	/* CPC0_CR1 |= PCIPW */
	mtdcr (0xb2, mfdcr (0xb2) | 0x00004000);

	return 0;
}

#ifdef CONFIG_BOARD_PRE_INIT
int board_pre_init (void)
{
	return board_early_init_f ();
}

#endif

/*
 * This function is also called by arch/powerpc/lib/board.c:board_init_f (it is
 * also in the init_sequence array) but later. Many more things are
 * configured, but we are still running from flash.
 */
int checkboard (void)
{
	unsigned vers, status;

	/* check that the SystemACE chip is alive. */
	printf ("ACE:   ");
	vers = readw (CONFIG_SYS_SYSTEMACE_BASE + 0x16);
	printf ("SystemACE %u.%u (build %u)",
		(vers >> 12) & 0x0f, (vers >> 8) & 0x0f, vers & 0xff);

	status = readl (CONFIG_SYS_SYSTEMACE_BASE + 0x04);
#ifdef DEBUG
	printf (" STATUS=0x%08x", status);
#endif
	/* If the flash card is present and there is an initial error,
	   then force a restart of the program. */
	if (status & 0x00000010) {
		printf (" CFDETECT");

		if (status & 0x04) {
			/* CONTROLREG = CFGPROG */
			writew (0x1000, CONFIG_SYS_SYSTEMACE_BASE + 0x18);
			udelay (500);
			/* CONTROLREG = CFGRESET */
			writew (0x0080, CONFIG_SYS_SYSTEMACE_BASE + 0x18);
			udelay (500);
			writew (0x0000, CONFIG_SYS_SYSTEMACE_BASE + 0x18);
			/* CONTROLREG = CFGSTART */
			writew (0x0020, CONFIG_SYS_SYSTEMACE_BASE + 0x18);

			status = readl (CONFIG_SYS_SYSTEMACE_BASE + 0x04);
		}
	}

	/* Wait for the SystemACE to program its chain of devices. */
	while ((status & 0x84) == 0x00) {
		udelay (500);
		status = readl (CONFIG_SYS_SYSTEMACE_BASE + 0x04);
	}

	if (status & 0x04)
		printf (" CFG-ERROR");
	if (status & 0x80)
		printf (" CFGDONE");

	printf ("\n");

	/* Force /RTS to active. The board it not wired quite
	   correctly to use cts/rtc flow control, so just force the
	   /RST active and forget about it. */
	writeb (readb (0xef600404) | 0x03, 0xef600404);

	printf ("JSE:   ready\n");

	return 0;
}

/* **** No more functions called by board_init_f. **** */

/*
 * This function is called by arch/powerpc/lib/board.c:board_init_r. At this
 * point, basic setup is done, U-Boot has been moved into SDRAM and
 * PCI has been set up. From here we done late setup.
 */
int misc_init_r (void)
{
	host_bridge_init ();
	return 0;
}
