/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#include <libfdt.h>
#include <fdt_support.h>
#include <spd_sdram.h>
#include <ppc4xx_enet.h>
#include <miiphy.h>
#include <asm/processor.h>

DECLARE_GLOBAL_DATA_PTR;

extern int alpr_fpga_init(void);

int board_early_init_f (void)
{
	/*-------------------------------------------------------------------------
	 * Initialize EBC CONFIG
	 *-------------------------------------------------------------------------*/
	mtebc(xbcfg, EBC_CFG_LE_UNLOCK |
	      EBC_CFG_PTD_DISABLE | EBC_CFG_RTC_64PERCLK |
	      EBC_CFG_ATC_PREVIOUS | EBC_CFG_DTC_PREVIOUS |
	      EBC_CFG_CTC_PREVIOUS | EBC_CFG_EMC_NONDEFAULT |
	      EBC_CFG_PME_DISABLE | EBC_CFG_PR_32);

	/*--------------------------------------------------------------------
	 * Setup the interrupt controller polarities, triggers, etc.
	 *-------------------------------------------------------------------*/
	mtdcr (uic0sr, 0xffffffff);	/* clear all */
	mtdcr (uic0er, 0x00000000);	/* disable all */
	mtdcr (uic0cr, 0x00000009);	/* SMI & UIC1 crit are critical */
	mtdcr (uic0pr, 0xfffffe03);	/* per manual */
	mtdcr (uic0tr, 0x01c00000);	/* per manual */
	mtdcr (uic0vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (uic0sr, 0xffffffff);	/* clear all */

	mtdcr (uic1sr, 0xffffffff);	/* clear all */
	mtdcr (uic1er, 0x00000000);	/* disable all */
	mtdcr (uic1cr, 0x00000000);	/* all non-critical */
	mtdcr (uic1pr, 0xffffe0ff);	/* per ref-board manual */
	mtdcr (uic1tr, 0x00ffc000);	/* per ref-board manual */
	mtdcr (uic1vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (uic1sr, 0xffffffff);	/* clear all */

	mtdcr (uic2sr, 0xffffffff);	/* clear all */
	mtdcr (uic2er, 0x00000000);	/* disable all */
	mtdcr (uic2cr, 0x00000000);	/* all non-critical */
	mtdcr (uic2pr, 0xffffffff);	/* per ref-board manual */
	mtdcr (uic2tr, 0x00ff8c0f);	/* per ref-board manual */
	mtdcr (uic2vr, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (uic2sr, 0xffffffff);	/* clear all */

	mtdcr (uicb0sr, 0xfc000000); /* clear all */
	mtdcr (uicb0er, 0x00000000); /* disable all */
	mtdcr (uicb0cr, 0x00000000); /* all non-critical */
	mtdcr (uicb0pr, 0xfc000000); /* */
	mtdcr (uicb0tr, 0x00000000); /* */
	mtdcr (uicb0vr, 0x00000001); /* */

	/* Setup shutdown/SSD empty interrupt as inputs */
	out32(GPIO0_TCR, in32(GPIO0_TCR) & ~(CFG_GPIO_SHUTDOWN | CFG_GPIO_SSD_EMPTY));
	out32(GPIO0_ODR, in32(GPIO0_ODR) & ~(CFG_GPIO_SHUTDOWN | CFG_GPIO_SSD_EMPTY));

	/* Setup GPIO/IRQ multiplexing */
	mtsdr(sdr_pfc0, 0x01a33e00);

	return 0;
}

int last_stage_init(void)
{
	unsigned short reg;

	/*
	 * Configure LED's of both Marvell 88E1111 PHY's
	 *
	 * This has to be done after the 4xx ethernet driver is loaded,
	 * so "last_stage_init()" is the right place.
	 */
	miiphy_read("ppc_4xx_eth2", CONFIG_PHY2_ADDR, 0x18, &reg);
	reg |= 0x0001;
	miiphy_write("ppc_4xx_eth2", CONFIG_PHY2_ADDR, 0x18, reg);
	miiphy_read("ppc_4xx_eth3", CONFIG_PHY3_ADDR, 0x18, &reg);
	reg |= 0x0001;
	miiphy_write("ppc_4xx_eth3", CONFIG_PHY3_ADDR, 0x18, reg);

	return 0;
}

static int board_rev(void)
{
	/* Setup as input */
	out32(GPIO0_TCR, in32(GPIO0_TCR) & ~(CFG_GPIO_REV0 | CFG_GPIO_REV1));
	out32(GPIO0_ODR, in32(GPIO0_ODR) & ~(CFG_GPIO_REV0 | CFG_GPIO_REV1));

	return (in32(GPIO0_IR) >> 16) & 0x3;
}

int checkboard (void)
{
	char *s = getenv ("serial#");

	printf ("Board: ALPR");
	if (s != NULL) {
		puts (", serial# ");
		puts (s);
	}
	printf(" (Rev. %d)\n", board_rev());

	return (0);
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
int pci_pre_init(struct pci_controller * hose )
{
	unsigned long strap;

	/*--------------------------------------------------------------------------+
	 *	The ocotea board is always configured as the host & requires the
	 *	PCI arbiter to be enabled.
	 *--------------------------------------------------------------------------*/
	mfsdr(sdr_sdstp1, strap);
	if( (strap & SDR0_SDSTP1_PAE_MASK) == 0 ){
		printf("PCI: SDR0_STRP1[%08lX] - PCI Arbiter disabled.\n",strap);
		return 0;
	}

	/* FPGA Init */
	alpr_fpga_init ();

	return 1;
}
#endif /* defined(CONFIG_PCI) */

/*************************************************************************
 *  pci_target_init
 *
 *	The bootstrap configuration provides default settings for the pci
 *	inbound map (PIM). But the bootstrap config choices are limited and
 *	may not be sufficient for a given board.
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT)
void pci_target_init(struct pci_controller * hose )
{
	/*--------------------------------------------------------------------------+
	 * Disable everything
	 *--------------------------------------------------------------------------*/
	out32r( PCIX0_PIM0SA, 0 ); /* disable */
	out32r( PCIX0_PIM1SA, 0 ); /* disable */
	out32r( PCIX0_PIM2SA, 0 ); /* disable */
	out32r( PCIX0_EROMBA, 0 ); /* disable expansion rom */

	/*--------------------------------------------------------------------------+
	 * Map all of SDRAM to PCI address 0x0000_0000. Note that the 440 strapping
	 * options to not support sizes such as 128/256 MB.
	 *--------------------------------------------------------------------------*/
	out32r( PCIX0_PIM0LAL, CFG_SDRAM_BASE );
	out32r( PCIX0_PIM0LAH, 0 );
	out32r( PCIX0_PIM0SA, ~(gd->ram_size - 1) | 1 );

	out32r( PCIX0_BAR0, 0 );

	/*--------------------------------------------------------------------------+
	 * Program the board's subsystem id/vendor id
	 *--------------------------------------------------------------------------*/
	out16r( PCIX0_SBSYSVID, CFG_PCI_SUBSYS_VENDORID );
	out16r( PCIX0_SBSYSID, CFG_PCI_SUBSYS_DEVICEID );

	out16r( PCIX0_CMD, in16r(PCIX0_CMD) | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
}
#endif /* defined(CONFIG_PCI) && defined(CFG_PCI_TARGET_INIT) */

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

static void wait_for_pci_ready(void)
{
	/*
	 * Configure EREADY as input
	 */
	out32(GPIO0_TCR, in32(GPIO0_TCR) & ~CFG_GPIO_EREADY);
	udelay(1000);

	for (;;) {
		if (in32(GPIO0_IR) & CFG_GPIO_EREADY)
			return;
	}

}

int is_pci_host(struct pci_controller *hose)
{
	wait_for_pci_ready();
	return 1;		/* return 1 for host controller */
}
#endif /* defined(CONFIG_PCI) */

/*************************************************************************
 *  pci_master_init
 *
 ************************************************************************/
#if defined(CONFIG_PCI) && defined(CFG_PCI_MASTER_INIT)
void pci_master_init(struct pci_controller *hose)
{
	/*--------------------------------------------------------------------------+
	  | PowerPC440 PCI Master configuration.
	  | Map PLB/processor addresses to PCI memory space.
	  |   PLB address 0xA0000000-0xCFFFFFFF ==> PCI address 0x80000000-0xCFFFFFFF
	  |   Use byte reversed out routines to handle endianess.
	  | Make this region non-prefetchable.
	  +--------------------------------------------------------------------------*/
	out32r( PCIX0_POM0SA, 0 ); /* disable */
	out32r( PCIX0_POM1SA, 0 ); /* disable */
	out32r( PCIX0_POM2SA, 0 ); /* disable */

	out32r(PCIX0_POM0LAL, CFG_PCI_MEMBASE);	/* PMM0 Local Address */
	out32r(PCIX0_POM0LAH, 0x00000003);	/* PMM0 Local Address */
	out32r(PCIX0_POM0PCIAL, CFG_PCI_MEMBASE);	/* PMM0 PCI Low Address */
	out32r(PCIX0_POM0PCIAH, 0x00000000);	/* PMM0 PCI High Address */
	out32r(PCIX0_POM0SA, ~(0x10000000 - 1) | 1);	/* 256MB + enable region */

	out32r(PCIX0_POM1LAL, CFG_PCI_MEMBASE2);	/* PMM0 Local Address */
	out32r(PCIX0_POM1LAH, 0x00000003);	/* PMM0 Local Address */
	out32r(PCIX0_POM1PCIAL, CFG_PCI_MEMBASE2);	/* PMM0 PCI Low Address */
	out32r(PCIX0_POM1PCIAH, 0x00000000);	/* PMM0 PCI High Address */
	out32r(PCIX0_POM1SA, ~(0x10000000 - 1) | 1);	/* 256MB + enable region */
}
#endif				/* defined(CONFIG_PCI) && defined(CFG_PCI_MASTER_INIT) */

#ifdef CONFIG_POST
/*
 * Returns 1 if keys pressed to start the power-on long-running tests
 * Called from board_init_f().
 */
int post_hotkeys_pressed(void)
{

	return (ctrlc());
}
#endif
