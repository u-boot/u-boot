/*
 * (C) Copyright 2005
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
 */

#include <asm/mmu.h>
#include <common.h>
#include <pci.h>

#ifdef CONFIG_PCI

/* System RAM mapped to PCI space */
#define CONFIG_PCI_SYS_MEM_BUS	CFG_SDRAM_BASE
#define CONFIG_PCI_SYS_MEM_PHYS	CFG_SDRAM_BASE
#define CONFIG_PCI_SYS_MEM_SIZE	(1024 * 1024 * 1024)

#ifndef CONFIG_PCI_PNP
static struct pci_config_table pci_tqm834x_config_table[] = {
	{PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	 PCI_IDSEL_NUMBER, PCI_ANY_ID,
 	 pci_cfgfunc_config_device, {PCI_ENET0_IOADDR,
				     PCI_ENET0_MEMADDR,
				     PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER
		}
	},
	{}
};
#endif

static struct pci_controller pci1_hose = {
#ifndef CONFIG_PCI_PNP
	config_table:pci_tqm834x_config_table,
#endif
};


/**************************************************************************
 * pci_init_board()
 *
 * NOTICE: MPC8349 internally has two PCI controllers (PCI1 and PCI2) but since
 * per TQM834x design physical connections to external devices (PCI sockets)
 * are routed only to the PCI1 we do not account for the second one - this code
 * supports PCI1 module only. Should support for the PCI2 be required in the
 * future it needs a separate pci_controller structure (above) and handling -
 * please refer to other boards' implementation for dual PCI host controllers,
 * for example board/Marvell/db64360/pci.c, pci_init_board()
 *
 */
void
pci_init_board(void)
{
	volatile immap_t *	immr;
	volatile clk8349_t *	clk;
	volatile law8349_t *	pci_law;
	volatile pot8349_t *	pci_pot;
	volatile pcictrl8349_t *	pci_ctrl;
	volatile pciconf8349_t *	pci_conf;
	u16 reg16;
	u32 reg32;
	struct	pci_controller * hose;

	immr = (immap_t *)CFG_IMMRBAR;
	clk = (clk8349_t *)&immr->clk;
	pci_law = immr->sysconf.pcilaw;
	pci_pot = immr->ios.pot;
	pci_ctrl = immr->pci_ctrl;
	pci_conf = immr->pci_conf;

	hose = &pci1_hose;

	/*
	 * Configure PCI controller and PCI_CLK_OUTPUT
	 */

	/*
	 * WARNING! only PCI_CLK_OUTPUT1 is enabled here as this is the one
	 * line actually used for clocking all external PCI devices in TQM83xx.
	 * Enabling other PCI_CLK_OUTPUT lines may lead to board's hang for
	 * unknown reasons - particularly PCI_CLK_OUTPUT6 and PCI_CLK_OUTPUT7
	 * are known to hang the board; this issue is under investigation
	 * (13 oct 05)
	 */
	reg32 = OCCR_PCICOE1;
#if 0
	/* enabling all PCI_CLK_OUTPUT lines HANGS the board... */
	reg32 = 0xff000000;
#endif
	if (clk->spmr & SPMR_CKID) {
		/* PCI Clock is half CONFIG_83XX_CLKIN so need to set up OCCR
		 * fields accordingly */
		reg32 |= (OCCR_PCI1CR | OCCR_PCI2CR);

		reg32 |= (OCCR_PCICD0 | OCCR_PCICD1 | OCCR_PCICD2 \
			  | OCCR_PCICD3 | OCCR_PCICD4 | OCCR_PCICD5 \
			  | OCCR_PCICD6 | OCCR_PCICD7);
	}

	clk->occr = reg32;
	udelay(2000);

	/*
	 * Release PCI RST Output signal
	 */
	pci_ctrl[0].gcr = 0;
	udelay(2000);
	pci_ctrl[0].gcr = 1;
	udelay(2000);

	/*
	 * Configure PCI Local Access Windows
	 */
	pci_law[0].bar = CFG_PCI1_MEM_PHYS & LAWBAR_BAR;
	pci_law[0].ar = LAWAR_EN | LAWAR_SIZE_512M;

	pci_law[1].bar = CFG_PCI1_IO_PHYS & LAWBAR_BAR;
	pci_law[1].ar = LAWAR_EN | LAWAR_SIZE_16M;

	/*
	 * Configure PCI Outbound Translation Windows
	 */

	/* PCI1 mem space */
	pci_pot[0].potar = (CFG_PCI1_MEM_BASE >> 12) & POTAR_TA_MASK;
	pci_pot[0].pobar = (CFG_PCI1_MEM_PHYS >> 12) & POBAR_BA_MASK;
	pci_pot[0].pocmr = POCMR_EN | (POCMR_CM_512M & POCMR_CM_MASK);

	/* PCI1 IO space */
	pci_pot[1].potar = (CFG_PCI1_IO_BASE >> 12) & POTAR_TA_MASK;
	pci_pot[1].pobar = (CFG_PCI1_IO_PHYS >> 12) & POBAR_BA_MASK;
	pci_pot[1].pocmr = POCMR_EN | POCMR_IO | (POCMR_CM_16M & POCMR_CM_MASK);

	/*
	 * Configure PCI Inbound Translation Windows
	 */

	/* we need RAM mapped to PCI space for the devices to
	 * access main memory */
	pci_ctrl[0].pitar1 = 0x0;
	pci_ctrl[0].pibar1 = 0x0;
	pci_ctrl[0].piebar1 = 0x0;
	pci_ctrl[0].piwar1 = PIWAR_EN | PIWAR_PF | PIWAR_RTT_SNOOP | PIWAR_WTT_SNOOP | PIWAR_IWS_256M;

	hose->first_busno = 0;
	hose->last_busno = 0xff;

	/* PCI memory space */
	pci_set_region(hose->regions + 0,
		       CFG_PCI1_MEM_BASE,
		       CFG_PCI1_MEM_PHYS,
		       CFG_PCI1_MEM_SIZE,
		       PCI_REGION_MEM);

	/* PCI IO space */
	pci_set_region(hose->regions + 1,
		       CFG_PCI1_IO_BASE,
		       CFG_PCI1_IO_PHYS,
		       CFG_PCI1_IO_SIZE,
		       PCI_REGION_IO);

	/* System memory space */
	pci_set_region(hose->regions + 2,
		       CONFIG_PCI_SYS_MEM_BUS,
		       CONFIG_PCI_SYS_MEM_PHYS,
		       CONFIG_PCI_SYS_MEM_SIZE,
		       PCI_REGION_MEM | PCI_REGION_MEMORY);

	hose->region_count = 3;

	pci_setup_indirect(hose,
			   (CFG_IMMRBAR+0x8300),
			   (CFG_IMMRBAR+0x8304));

	pci_register_hose(hose);

	/*
	 * Write to Command register
	 */
	reg16 = 0xff;
	pci_hose_read_config_word (hose, PCI_BDF(0,0,0), PCI_COMMAND,
					&reg16);
	reg16 |= PCI_COMMAND_SERR | PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY;
	pci_hose_write_config_word(hose, PCI_BDF(0,0,0), PCI_COMMAND,
					reg16);

	/*
	 * Clear non-reserved bits in status register.
	 */
	pci_hose_write_config_word(hose, PCI_BDF(0,0,0), PCI_STATUS,
					0xffff);
	pci_hose_write_config_byte(hose, PCI_BDF(0,0,0), PCI_LATENCY_TIMER,
					0x80);

#ifdef CONFIG_PCI_SCAN_SHOW
	printf("PCI:   Bus Dev VenId DevId Class Int\n");
#endif
	/*
	 * Hose scan.
	 */
	hose->last_busno = pci_hose_scan(hose);
}
#endif /* CONFIG_PCI */
