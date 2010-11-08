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
#include <asm/4xx_pci.h>

DECLARE_GLOBAL_DATA_PTR;

extern int alpr_fpga_init(void);

int board_early_init_f (void)
{
	/*-------------------------------------------------------------------------
	 * Initialize EBC CONFIG
	 *-------------------------------------------------------------------------*/
	mtebc(EBC0_CFG, EBC_CFG_LE_UNLOCK |
	      EBC_CFG_PTD_DISABLE | EBC_CFG_RTC_64PERCLK |
	      EBC_CFG_ATC_PREVIOUS | EBC_CFG_DTC_PREVIOUS |
	      EBC_CFG_CTC_PREVIOUS | EBC_CFG_EMC_NONDEFAULT |
	      EBC_CFG_PME_DISABLE | EBC_CFG_PR_32);

	/*--------------------------------------------------------------------
	 * Setup the interrupt controller polarities, triggers, etc.
	 *-------------------------------------------------------------------*/
	/*
	 * Because of the interrupt handling rework to handle 440GX interrupts
	 * with the common code, we needed to change names of the UIC registers.
	 * Here the new relationship:
	 *
	 * U-Boot name	440GX name
	 * -----------------------
	 * UIC0		UICB0
	 * UIC1		UIC0
	 * UIC2		UIC1
	 * UIC3		UIC2
	 */
	mtdcr (UIC1SR, 0xffffffff);	/* clear all */
	mtdcr (UIC1ER, 0x00000000);	/* disable all */
	mtdcr (UIC1CR, 0x00000009);	/* SMI & UIC1 crit are critical */
	mtdcr (UIC1PR, 0xfffffe03);	/* per manual */
	mtdcr (UIC1TR, 0x01c00000);	/* per manual */
	mtdcr (UIC1VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (UIC1SR, 0xffffffff);	/* clear all */

	mtdcr (UIC2SR, 0xffffffff);	/* clear all */
	mtdcr (UIC2ER, 0x00000000);	/* disable all */
	mtdcr (UIC2CR, 0x00000000);	/* all non-critical */
	mtdcr (UIC2PR, 0xffffe0ff);	/* per ref-board manual */
	mtdcr (UIC2TR, 0x00ffc000);	/* per ref-board manual */
	mtdcr (UIC2VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (UIC2SR, 0xffffffff);	/* clear all */

	mtdcr (UIC3SR, 0xffffffff);	/* clear all */
	mtdcr (UIC3ER, 0x00000000);	/* disable all */
	mtdcr (UIC3CR, 0x00000000);	/* all non-critical */
	mtdcr (UIC3PR, 0xffffffff);	/* per ref-board manual */
	mtdcr (UIC3TR, 0x00ff8c0f);	/* per ref-board manual */
	mtdcr (UIC3VR, 0x00000001);	/* int31 highest, base=0x000 */
	mtdcr (UIC3SR, 0xffffffff);	/* clear all */

	mtdcr (UIC0SR, 0xfc000000); /* clear all */
	mtdcr (UIC0ER, 0x00000000); /* disable all */
	mtdcr (UIC0CR, 0x00000000); /* all non-critical */
	mtdcr (UIC0PR, 0xfc000000); /* */
	mtdcr (UIC0TR, 0x00000000); /* */
	mtdcr (UIC0VR, 0x00000001); /* */

	/* Setup shutdown/SSD empty interrupt as inputs */
	out32(GPIO0_TCR, in32(GPIO0_TCR) & ~(CONFIG_SYS_GPIO_SHUTDOWN | CONFIG_SYS_GPIO_SSD_EMPTY));
	out32(GPIO0_ODR, in32(GPIO0_ODR) & ~(CONFIG_SYS_GPIO_SHUTDOWN | CONFIG_SYS_GPIO_SSD_EMPTY));

	/* Setup GPIO/IRQ multiplexing */
	mtsdr(SDR0_PFC0, 0x01a33e00);

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
	out32(GPIO0_TCR, in32(GPIO0_TCR) & ~(CONFIG_SYS_GPIO_REV0 | CONFIG_SYS_GPIO_REV1));
	out32(GPIO0_ODR, in32(GPIO0_ODR) & ~(CONFIG_SYS_GPIO_REV0 | CONFIG_SYS_GPIO_REV1));

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

#if defined(CONFIG_PCI)
/*
 * Override weak pci_pre_init()
 */
int pci_pre_init(struct pci_controller *hose)
{
	if (__pci_pre_init(hose) == 0)
		return 0;

	/* FPGA Init */
	alpr_fpga_init();

	return 1;
}

/*************************************************************************
 * Override weak is_pci_host()
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
static void wait_for_pci_ready(void)
{
	/*
	 * Configure EREADY as input
	 */
	out32(GPIO0_TCR, in32(GPIO0_TCR) & ~CONFIG_SYS_GPIO_EREADY);
	udelay(1000);

	for (;;) {
		if (in32(GPIO0_IR) & CONFIG_SYS_GPIO_EREADY)
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
#if defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_MASTER_INIT)
void pci_master_init(struct pci_controller *hose)
{
	/*--------------------------------------------------------------------------+
	  | PowerPC440 PCI Master configuration.
	  | Map PLB/processor addresses to PCI memory space.
	  |   PLB address 0xA0000000-0xCFFFFFFF ==> PCI address 0x80000000-0xCFFFFFFF
	  |   Use byte reversed out routines to handle endianess.
	  | Make this region non-prefetchable.
	  +--------------------------------------------------------------------------*/
	out32r( PCIL0_POM0SA, 0 ); /* disable */
	out32r( PCIL0_POM1SA, 0 ); /* disable */
	out32r( PCIL0_POM2SA, 0 ); /* disable */

	out32r(PCIL0_POM0LAL, CONFIG_SYS_PCI_MEMBASE);	/* PMM0 Local Address */
	out32r(PCIL0_POM0LAH, 0x00000003);	/* PMM0 Local Address */
	out32r(PCIL0_POM0PCIAL, CONFIG_SYS_PCI_MEMBASE);	/* PMM0 PCI Low Address */
	out32r(PCIL0_POM0PCIAH, 0x00000000);	/* PMM0 PCI High Address */
	out32r(PCIL0_POM0SA, ~(0x10000000 - 1) | 1);	/* 256MB + enable region */

	out32r(PCIL0_POM1LAL, CONFIG_SYS_PCI_MEMBASE2);	/* PMM0 Local Address */
	out32r(PCIL0_POM1LAH, 0x00000003);	/* PMM0 Local Address */
	out32r(PCIL0_POM1PCIAL, CONFIG_SYS_PCI_MEMBASE2);	/* PMM0 PCI Low Address */
	out32r(PCIL0_POM1PCIAH, 0x00000000);	/* PMM0 PCI High Address */
	out32r(PCIL0_POM1SA, ~(0x10000000 - 1) | 1);	/* 256MB + enable region */
}
#endif				/* defined(CONFIG_PCI) && defined(CONFIG_SYS_PCI_MASTER_INIT) */
