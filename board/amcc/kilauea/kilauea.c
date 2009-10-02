/*
 * (C) Copyright 2007
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <ppc4xx.h>
#include <ppc405.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/errno.h>

#if defined(CONFIG_PCI)
#include <pci.h>
#include <asm/4xx_pcie.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

extern flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/*
 * Board early initialization function
 */
int board_early_init_f (void)
{
	u32 val;

	/*--------------------------------------------------------------------+
	 | Interrupt controller setup for the AMCC 405EX(r) PINE evaluation board.
	 +--------------------------------------------------------------------+
	+---------------------------------------------------------------------+
	|Interrupt| Source                            | Pol.  | Sensi.| Crit. |
	+---------+-----------------------------------+-------+-------+-------+
	| IRQ 00  | UART0                             | High  | Level | Non   |
	| IRQ 01  | UART1                             | High  | Level | Non   |
	| IRQ 02  | IIC0                              | High  | Level | Non   |
	| IRQ 03  | TBD                               | High  | Level | Non   |
	| IRQ 04  | TBD                               | High  | Level | Non   |
	| IRQ 05  | EBM                               | High  | Level | Non   |
	| IRQ 06  | BGI                               | High  | Level | Non   |
	| IRQ 07  | IIC1                              | Rising| Edge  | Non   |
	| IRQ 08  | SPI                               | High  | Lvl/ed| Non   |
	| IRQ 09  | External IRQ 0 - (PCI-Express)    | pgm H | Pgm   | Non   |
	| IRQ 10  | MAL TX EOB                        | High  | Level | Non   |
	| IRQ 11  | MAL RX EOB                        | High  | Level | Non   |
	| IRQ 12  | DMA Channel 0 FIFO Full           | High  | Level | Non   |
	| IRQ 13  | DMA Channel 0 Stat FIFO           | High  | Level | Non   |
	| IRQ 14  | DMA Channel 1 FIFO Full           | High  | Level | Non   |
	| IRQ 15  | DMA Channel 1 Stat FIFO           | High  | Level | Non   |
	| IRQ 16  | PCIE0 AL                          | high  | Level | Non   |
	| IRQ 17  | PCIE0 VPD access                  | rising| Edge  | Non   |
	| IRQ 18  | PCIE0 hot reset request           | rising| Edge  | Non   |
	| IRQ 19  | PCIE0 hot reset request           | faling| Edge  | Non   |
	| IRQ 20  | PCIE0 TCR                         | High  | Level | Non   |
	| IRQ 21  | PCIE0 MSI level0                  | High  | Level | Non   |
	| IRQ 22  | PCIE0 MSI level1                  | High  | Level | Non   |
	| IRQ 23  | Security EIP-94                   | High  | Level | Non   |
	| IRQ 24  | EMAC0 interrupt                   | High  | Level | Non   |
	| IRQ 25  | EMAC1 interrupt                   | High  | Level | Non   |
	| IRQ 26  | PCIE0 MSI level2                  | High  | Level | Non   |
	| IRQ 27  | External IRQ 4                    | pgm H | Pgm   | Non   |
	| IRQ 28  | UIC2 Non-critical Int.            | High  | Level | Non   |
	| IRQ 29  | UIC2 Critical Interrupt           | High  | Level | Crit. |
	| IRQ 30  | UIC1 Non-critical Int.            | High  | Level | Non   |
	| IRQ 31  | UIC1 Critical Interrupt           | High  | Level | Crit. |
	|----------------------------------------------------------------------
	| IRQ 32  | MAL Serr                          | High  | Level | Non   |
	| IRQ 33  | MAL Txde                          | High  | Level | Non   |
	| IRQ 34  | MAL Rxde                          | High  | Level | Non   |
	| IRQ 35  | PCIE0 bus master VC0              |falling| Edge  | Non   |
	| IRQ 36  | PCIE0 DCR Error                   | High  | Level | Non   |
	| IRQ 37  | EBC                               | High  |Lvl Edg| Non   |
	| IRQ 38  | NDFC                              | High  | Level | Non   |
	| IRQ 39  | GPT Compare Timer 8               | Risin | Edge  | Non   |
	| IRQ 40  | GPT Compare Timer 9               | Risin | Edge  | Non   |
	| IRQ 41  | PCIE1 AL                          | high  | Level | Non   |
	| IRQ 42  | PCIE1 VPD access                  | rising| edge  | Non   |
	| IRQ 43  | PCIE1 hot reset request           | rising| Edge  | Non   |
	| IRQ 44  | PCIE1 hot reset request           | faling| Edge  | Non   |
	| IRQ 45  | PCIE1 TCR                         | High  | Level | Non   |
	| IRQ 46  | PCIE1 bus master VC0              |falling| Edge  | Non   |
	| IRQ 47  | GPT Compare Timer 3               | Risin | Edge  | Non   |
	| IRQ 48  | GPT Compare Timer 4               | Risin | Edge  | Non   |
	| IRQ 49  | Ext. IRQ 7                        |pgm/Fal|pgm/Lvl| Non   |
	| IRQ 50  | Ext. IRQ 8 -                      |pgm (H)|pgm/Lvl| Non   |
	| IRQ 51  | Ext. IRQ 9                        |pgm (H)|pgm/Lvl| Non   |
	| IRQ 52  | GPT Compare Timer 5               | high  | Edge  | Non   |
	| IRQ 53  | GPT Compare Timer 6               | high  | Edge  | Non   |
	| IRQ 54  | GPT Compare Timer 7               | high  | Edge  | Non   |
	| IRQ 55  | Serial ROM                        | High  | Level | Non   |
	| IRQ 56  | GPT Decrement Pulse               | High  | Level | Non   |
	| IRQ 57  | Ext. IRQ 2                        |pgm/Fal|pgm/Lvl| Non   |
	| IRQ 58  | Ext. IRQ 5                        |pgm/Fal|pgm/Lvl| Non   |
	| IRQ 59  | Ext. IRQ 6                        |pgm/Fal|pgm/Lvl| Non   |
	| IRQ 60  | EMAC0 Wake-up                     | High  | Level | Non   |
	| IRQ 61  | Ext. IRQ 1                        |pgm/Fal|pgm/Lvl| Non   |
	| IRQ 62  | EMAC1 Wake-up                     | High  | Level | Non   |
	|----------------------------------------------------------------------
	| IRQ 64  | PE0 AL                            | High  | Level | Non   |
	| IRQ 65  | PE0 VPD Access                    | Risin | Edge  | Non   |
	| IRQ 66  | PE0 Hot Reset Request             | Risin | Edge  | Non   |
	| IRQ 67  | PE0 Hot Reset Request             | Falli | Edge  | Non   |
	| IRQ 68  | PE0 TCR                           | High  | Level | Non   |
	| IRQ 69  | PE0 BusMaster VCO                 | Falli | Edge  | Non   |
	| IRQ 70  | PE0 DCR Error                     | High  | Level | Non   |
	| IRQ 71  | Reserved                          | N/A   | N/A   | Non   |
	| IRQ 72  | PE1 AL                            | High  | Level | Non   |
	| IRQ 73  | PE1 VPD Access                    | Risin | Edge  | Non   |
	| IRQ 74  | PE1 Hot Reset Request             | Risin | Edge  | Non   |
	| IRQ 75  | PE1 Hot Reset Request             | Falli | Edge  | Non   |
	| IRQ 76  | PE1 TCR                           | High  | Level | Non   |
	| IRQ 77  | PE1 BusMaster VCO                 | Falli | Edge  | Non   |
	| IRQ 78  | PE1 DCR Error                     | High  | Level | Non   |
	| IRQ 79  | Reserved                          | N/A   | N/A   | Non   |
	| IRQ 80  | PE2 AL                            | High  | Level | Non   |
	| IRQ 81  | PE2 VPD Access                    | Risin | Edge  | Non   |
	| IRQ 82  | PE2 Hot Reset Request             | Risin | Edge  | Non   |
	| IRQ 83  | PE2 Hot Reset Request             | Falli | Edge  | Non   |
	| IRQ 84  | PE2 TCR                           | High  | Level | Non   |
	| IRQ 85  | PE2 BusMaster VCO                 | Falli | Edge  | Non   |
	| IRQ 86  | PE2 DCR Error                     | High  | Level | Non   |
	| IRQ 87  | Reserved                          | N/A   | N/A   | Non   |
	| IRQ 88  | External IRQ(5)                   | Progr | Progr | Non   |
	| IRQ 89  | External IRQ 4 - Ethernet         | Progr | Progr | Non   |
	| IRQ 90  | External IRQ 3 - PCI-X            | Progr | Progr | Non   |
	| IRQ 91  | External IRQ 2 - PCI-X            | Progr | Progr | Non   |
	| IRQ 92  | External IRQ 1 - PCI-X            | Progr | Progr | Non   |
	| IRQ 93  | External IRQ 0 - PCI-X            | Progr | Progr | Non   |
	| IRQ 94  | Reserved                          | N/A   | N/A   | Non   |
	| IRQ 95  | Reserved                          | N/A   | N/A   | Non   |
	|---------------------------------------------------------------------
	+---------+-----------------------------------+-------+-------+------*/
	/*--------------------------------------------------------------------+
	 | Initialise UIC registers.  Clear all interrupts.  Disable all
	 | interrupts.
	 | Set critical interrupt values.  Set interrupt polarities.  Set
	 | interrupt trigger levels.  Make bit 0 High  priority.  Clear all
	 | interrupts again.
	 +-------------------------------------------------------------------*/

	mtdcr (UIC2SR, 0xffffffff);	/* Clear all interrupts */
	mtdcr (UIC2ER, 0x00000000);	/* disable all interrupts */
	mtdcr (UIC2CR, 0x00000000);	/* Set Critical / Non Critical interrupts */
	mtdcr (UIC2PR, 0xf7ffffff);	/* Set Interrupt Polarities */
	mtdcr (UIC2TR, 0x01e1fff8);	/* Set Interrupt Trigger Levels */
	mtdcr (UIC2VR, 0x00000001);	/* Set Vect base=0,INT31 Highest priority */
	mtdcr (UIC2SR, 0x00000000);	/* clear all interrupts */
	mtdcr (UIC2SR, 0xffffffff);	/* clear all interrupts */

	mtdcr (UIC1SR, 0xffffffff);	/* Clear all interrupts */
	mtdcr (UIC1ER, 0x00000000);	/* disable all interrupts */
	mtdcr (UIC1CR, 0x00000000);	/* Set Critical / Non Critical interrupts */
	mtdcr (UIC1PR, 0xfffac785);	/* Set Interrupt Polarities */
	mtdcr (UIC1TR, 0x001d0040);	/* Set Interrupt Trigger Levels */
	mtdcr (UIC1VR, 0x00000001);	/* Set Vect base=0,INT31 Highest priority */
	mtdcr (UIC1SR, 0x00000000);	/* clear all interrupts */
	mtdcr (UIC1SR, 0xffffffff);	/* clear all interrupts */

	mtdcr (UIC0SR, 0xffffffff);	/* Clear all interrupts */
	mtdcr (UIC0ER, 0x0000000a);	/* Disable all interrupts */
					/* Except cascade UIC0 and UIC1 */
	mtdcr (UIC0CR, 0x00000000);	/* Set Critical / Non Critical interrupts */
	mtdcr (UIC0PR, 0xffbfefef);	/* Set Interrupt Polarities */
	mtdcr (UIC0TR, 0x00007000);	/* Set Interrupt Trigger Levels */
	mtdcr (UIC0VR, 0x00000001);	/* Set Vect base=0,INT31 Highest priority */
	mtdcr (UIC0SR, 0x00000000);	/* clear all interrupts */
	mtdcr (UIC0SR, 0xffffffff);	/* clear all interrupts */

	/*
	 * Note: Some cores are still in reset when the chip starts, so
	 * take them out of reset
	 */
	mtsdr(SDR0_SRST, 0);

	/* Configure 405EX for NAND usage */
	val = SDR0_CUST0_MUX_NDFC_SEL |
		SDR0_CUST0_NDFC_ENABLE |
		SDR0_CUST0_NDFC_BW_8_BIT |
		SDR0_CUST0_NRB_BUSY |
		(0x80000000 >> (28 + CONFIG_SYS_NAND_CS));
	mtsdr(SDR0_CUST0, val);

	/*
	 * Configure PFC (Pin Function Control) registers
	 * -> Enable USB
	 */
	val = SDR0_PFC1_USBEN | SDR0_PFC1_USBBIGEN | SDR0_PFC1_GPT_FREQ;
	mtsdr(SDR0_PFC1, val);

	/*
	 * Configure FPGA register with PCIe reset
	 */
	out_be32((void *)CONFIG_SYS_FPGA_BASE, 0xff570cc4);	/* assert PCIe reset */
	mdelay(50);
	out_be32((void *)CONFIG_SYS_FPGA_BASE, 0xff570cc7);	/* deassert PCIe reset */

	return 0;
}

int misc_init_r(void)
{
#ifdef CONFIG_ENV_IS_IN_FLASH
	/* Monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      -CONFIG_SYS_MONITOR_LEN,
		      0xffffffff,
		      &flash_info[0]);
#endif

	return 0;
}

static int is_405exr(void)
{
	u32 pvr = get_pvr();

	if (pvr & 0x00000004)
		return 0;		/* bit 2 set -> 405EX */

	return 1;			/* bit 2 cleared -> 405EXr */
}

int board_emac_count(void)
{
	/*
	 * 405EXr only has one EMAC interface, 405EX has two
	 */
	if (is_405exr())
		return 1;
	else
		return 2;
}

static int board_pcie_count(void)
{
	/*
	 * 405EXr only has one EMAC interface, 405EX has two
	 */
	if (is_405exr())
		return 1;
	else
		return 2;
}

int checkboard (void)
{
	char *s = getenv("serial#");

	if (is_405exr())
		printf("Board: Haleakala - AMCC PPC405EXr Evaluation Board");
	else
		printf("Board: Kilauea - AMCC PPC405EX Evaluation Board");

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}
	putc('\n');

	return (0);
}

/*************************************************************************
 *  pci_pre_init
 *
 *  This routine is called just prior to registering the hose and gives
 *  the board the opportunity to check things. Returning a value of zero
 *  indicates that things are bad & PCI initialization should be aborted.
 *
 *      Different boards may wish to customize the pci controller structure
 *      (add regions, override default access routines, etc) or perform
 *      certain pre-initialization actions.
 *
 ************************************************************************/
#if defined(CONFIG_PCI)
int pci_pre_init(struct pci_controller * hose )
{
	return 0;
}
#endif  /* defined(CONFIG_PCI) */

#ifdef CONFIG_PCI
static struct pci_controller pcie_hose[2] = {{0},{0}};

void pcie_setup_hoses(int busno)
{
	struct pci_controller *hose;
	int i, bus;
	int ret = 0;
	bus = busno;
	char *env;
	unsigned int delay;

	for (i = 0; i < board_pcie_count(); i++) {

		if (is_end_point(i))
			ret = ppc4xx_init_pcie_endport(i);
		else
			ret = ppc4xx_init_pcie_rootport(i);
		if (ret == -ENODEV)
			continue;
		if (ret) {
			printf("PCIE%d: initialization as %s failed\n", i,
			       is_end_point(i) ? "endpoint" : "root-complex");
			continue;
		}

		hose = &pcie_hose[i];
		hose->first_busno = bus;
		hose->last_busno = bus;
		hose->current_busno = bus;

		/* setup mem resource */
		pci_set_region(hose->regions + 0,
			       CONFIG_SYS_PCIE_MEMBASE + i * CONFIG_SYS_PCIE_MEMSIZE,
			       CONFIG_SYS_PCIE_MEMBASE + i * CONFIG_SYS_PCIE_MEMSIZE,
			       CONFIG_SYS_PCIE_MEMSIZE,
			       PCI_REGION_MEM);
		hose->region_count = 1;
		pci_register_hose(hose);

		if (is_end_point(i)) {
			ppc4xx_setup_pcie_endpoint(hose, i);
			/*
			 * Reson for no scanning is endpoint can not generate
			 * upstream configuration accesses.
			 */
		} else {
			ppc4xx_setup_pcie_rootpoint(hose, i);
			env = getenv ("pciscandelay");
			if (env != NULL) {
				delay = simple_strtoul(env, NULL, 10);
				if (delay > 5)
					printf("Warning, expect noticable delay before "
					       "PCIe scan due to 'pciscandelay' value!\n");
				mdelay(delay * 1000);
			}

			/*
			 * Config access can only go down stream
			 */
			hose->last_busno = pci_hose_scan(hose);
			bus = hose->last_busno + 1;
		}
	}
}
#endif

#if defined(CONFIG_POST)
/*
 * Returns 1 if keys pressed to start the power-on long-running tests
 * Called from board_init_f().
 */
int post_hotkeys_pressed(void)
{
	return 0;	/* No hotkeys supported */
}
#endif /* CONFIG_POST */
