/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/ppc4xx.h>
#include <asm/ppc405.h>
#include <libfdt.h>
#include <asm/processor.h>
#include <asm/ppc4xx-gpio.h>
#include <asm/io.h>
#include <fdt_support.h>
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

	/* Reset PCIe slots */
	gpio_write_bit(CONFIG_SYS_GPIO_PCIE_RST, 0);
	udelay(100);
	gpio_write_bit(CONFIG_SYS_GPIO_PCIE_RST, 1);

	/*
	 * Configure PFC (Pin Function Control) registers
	 * -> Enable USB
	 */
	val = SDR0_PFC1_USBEN | SDR0_PFC1_USBBIGEN | SDR0_PFC1_GPT_FREQ;
	mtsdr(SDR0_PFC1, val);

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

int checkboard (void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

	printf("Board: Makalu - AMCC PPC405EX Evaluation Board");

	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

	return (0);
}
