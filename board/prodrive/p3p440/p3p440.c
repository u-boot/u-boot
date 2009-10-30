/*
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * Copyright (C) 2002 Scott McNutt <smcnutt@artesyncp.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/processor.h>
#include <command.h>

#include "p3p440.h"

DECLARE_GLOBAL_DATA_PTR;

void set_led(int color)
{
	switch (color) {
	case LED_OFF:
		out32(GPIO0_OR,  in32(GPIO0_OR) & ~CONFIG_SYS_LED_GREEN & ~CONFIG_SYS_LED_RED);
		break;

	case LED_GREEN:
		out32(GPIO0_OR,  (in32(GPIO0_OR) | CONFIG_SYS_LED_GREEN) & ~CONFIG_SYS_LED_RED);
		break;

	case LED_RED:
		out32(GPIO0_OR,  (in32(GPIO0_OR) | CONFIG_SYS_LED_RED) & ~CONFIG_SYS_LED_GREEN);
		break;

	case LED_ORANGE:
		out32(GPIO0_OR,  in32(GPIO0_OR) | CONFIG_SYS_LED_GREEN | CONFIG_SYS_LED_RED);
		break;
	}
}

static int is_monarch(void)
{
	out32(GPIO0_OR,  in32(GPIO0_OR) & ~CONFIG_SYS_GPIO_RDY);
	udelay(1000);

	if (in32(GPIO0_IR) & CONFIG_SYS_MONARCH_IO)
		return 0;
	else
		return 1;
}

static void wait_for_pci_ready(void)
{
	/*
	 * Configure EREADY_IO as input
	 */
	out32(GPIO0_TCR, in32(GPIO0_TCR) & ~CONFIG_SYS_EREADY_IO);
	udelay(1000);

	for (;;) {
		if (in32(GPIO0_IR) & CONFIG_SYS_EREADY_IO)
			return;
	}

}

int board_early_init_f(void)
{
	uint reg;

	/*--------------------------------------------------------------------
	 * Setup the external bus controller/chip selects
	 *-------------------------------------------------------------------*/
	mtdcr(EBC0_CFGADDR, EBC0_CFG);
	reg = mfdcr(EBC0_CFGDATA);
	mtdcr(EBC0_CFGDATA, reg | 0x04000000);	/* Set ATC */

	/*--------------------------------------------------------------------
	 * Setup pin multiplexing (GPIO/IRQ...)
	 *-------------------------------------------------------------------*/
	mtdcr(CPC0_GPIO, 0x03F01F80);

	out32(GPIO0_ODR, 0x00000000);	/* no open drain pins      */
	out32(GPIO0_TCR, CONFIG_SYS_GPIO_RDY | CONFIG_SYS_EREADY_IO | CONFIG_SYS_LED_RED | CONFIG_SYS_LED_GREEN);
	out32(GPIO0_OR,  CONFIG_SYS_GPIO_RDY);

	/*--------------------------------------------------------------------
	 * Setup the interrupt controller polarities, triggers, etc.
	 *-------------------------------------------------------------------*/
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */
	mtdcr(UIC0ER, 0x00000000);	/* disable all */
	mtdcr(UIC0CR, 0x00000001);	/* UIC1 crit is critical */
	mtdcr(UIC0PR, 0xfffffe13);	/* per ref-board manual */
	mtdcr(UIC0TR, 0x01c00008);	/* per ref-board manual */
	mtdcr(UIC0VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(UIC0SR, 0xffffffff);	/* clear all */

	mtdcr(UIC1SR, 0xffffffff);	/* clear all */
	mtdcr(UIC1ER, 0x00000000);	/* disable all */
	mtdcr(UIC1CR, 0x00000000);	/* all non-critical */
	mtdcr(UIC1PR, 0xffffe0ff);	/* per ref-board manual */
	mtdcr(UIC1TR, 0x00ffc000);	/* per ref-board manual */
	mtdcr(UIC1VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr(UIC1SR, 0xffffffff);	/* clear all */

	return 0;
}

int checkboard(void)
{
	char *s = getenv("serial#");

	printf("Board: P3P440");
	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}

	if (is_monarch()) {
		puts(", Monarch");
	} else {
		puts(", None-Monarch");
	}

	putc('\n');

	return (0);
}

int misc_init_r (void)
{
	/*
	 * Adjust flash start and offset to detected values
	 */
	gd->bd->bi_flashstart = 0 - gd->bd->bi_flashsize;
	gd->bd->bi_flashoffset = 0;

	/*
	 * Check if only one FLASH bank is available
	 */
	if (gd->bd->bi_flashsize != CONFIG_SYS_MAX_FLASH_BANKS * (0 - CONFIG_SYS_FLASH0)) {
		mtebc(PB1CR, 0);			/* disable cs */
		mtebc(PB1AP, 0);
		mtebc(PB2CR, 0);			/* disable cs */
		mtebc(PB2AP, 0);
		mtebc(PB3CR, 0);			/* disable cs */
		mtebc(PB3AP, 0);
	}

	return 0;
}

/*************************************************************************
 *  pci_pre_init
 *
 *  This routine is called just prior to registering the hose and gives
 *  the board the opportunity to check things. Returning a value of zero
 *  indicates that things are bad & PCI initialization should be aborted.
 *
 *	Different boards may wish to customize the pci controller structure
 *	(add regions, override default access routines, etc) or perform
 *	certain pre-initialization actions.
 *
 ************************************************************************/
#if defined(CONFIG_PCI)
int pci_pre_init(struct pci_controller *hose)
{
	unsigned long strap;

	/*--------------------------------------------------------------------------+
	 *	The P3P440 board is always configured as the host & requires the
	 *	PCI arbiter to be disabled because it's an PMC module.
	 *--------------------------------------------------------------------------*/
	strap = mfdcr(CPC0_STRP1);
	if (strap & 0x00100000) {
		printf("PCI: CPC0_STRP1[PAE] set.\n");
		return 0;
	}

	return 1;
}
#endif	/* defined(CONFIG_PCI) */

/*************************************************************************
 *  pci_target_init
 *
 *	The bootstrap configuration provides default settings for the pci
 *	inbound map (PIM). But the bootstrap config choices are limited and
 *	may not be sufficient for a given board.
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT)
void pci_target_init(struct pci_controller *hose)
{
	/*--------------------------------------------------------------------------+
	 * Disable everything
	 *--------------------------------------------------------------------------*/
	out32r(PCIL0_PIM0SA, 0);	/* disable */
	out32r(PCIL0_PIM1SA, 0);	/* disable */
	out32r(PCIL0_PIM2SA, 0);	/* disable */
	out32r(PCIL0_EROMBA, 0);	/* disable expansion rom */

	/*--------------------------------------------------------------------------+
	 * Map all of SDRAM to PCI address 0x0000_0000. Note that the 440 strapping
	 * options to not support sizes such as 128/256 MB.
	 *--------------------------------------------------------------------------*/
	out32r(PCIL0_PIM0LAL, CONFIG_SYS_SDRAM_BASE);
	out32r(PCIL0_PIM0LAH, 0);
	out32r(PCIL0_PIM0SA, ~(gd->ram_size - 1) | 1);

	out32r(PCIL0_BAR0, 0);

	/*--------------------------------------------------------------------------+
	 * Program the board's subsystem id/vendor id
	 *--------------------------------------------------------------------------*/
	out16r(PCIL0_SBSYSVID, CONFIG_SYS_PCI_SUBSYS_VENDORID);
	out16r(PCIL0_SBSYSID, CONFIG_SYS_PCI_SUBSYS_DEVICEID);

	out16r(PCIL0_CMD, in16r(PCIL0_CMD) | PCI_COMMAND_MEMORY);
}
#endif				/* defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_TARGET_INIT) */

/*************************************************************************
 *  is_pci_host
 *
 *	This routine is called to determine if a pci scan should be
 *	performed. With various hardware environments (especially cPCI and
 *	PPMC) it's insufficient to depend on the state of the arbiter enable
 *	bit in the strap register, or generic host/adapter assumptions.
 *
 *	Rather than hard-code a bad assumption in the general 440 code, the
 *	440 pci code requires the board to decide at runtime.
 *
 *	Return 0 for adapter mode, non-zero for host (monarch) mode.
 *
 *
 ************************************************************************/
#if defined(CONFIG_PCI)
int is_pci_host(struct pci_controller *hose)
{
	if (is_monarch()) {
		wait_for_pci_ready();
		return 1;		/* return 1 for host controller */
	} else {
		return 0;		/* return 0 for adapter controller */
	}
}
#endif				/* defined(CONFIG_PCI) */
