/*
 * (C) Copyright 2009-2010
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/ppc4xx.h>
#include <i2c.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <netdev.h>
#include <video.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/ppc4xx-gpio.h>
#include <asm/4xx_pcie.h>
#include <linux/errno.h>
#include <asm/mmu.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	unsigned long mfr;

	/*
	 * Interrupt controller setup for the ICON 440SPe board.
	 *
	 *--------------------------------------------------------------------
	 * IRQ    | Source                            | Pol.  | Sensi.| Crit.
	 *--------+-----------------------------------+-------+-------+-------
	 * IRQ 00 | UART0                             | High  | Level | Non
	 * IRQ 01 | UART1                             | High  | Level | Non
	 * IRQ 02 | IIC0                              | High  | Level | Non
	 * IRQ 03 | IIC1                              | High  | Level | Non
	 * IRQ 04 | PCI0X0 MSG IN                     | High  | Level | Non
	 * IRQ 05 | PCI0X0 CMD Write                  | High  | Level | Non
	 * IRQ 06 | PCI0X0 Power Mgt                  | High  | Level | Non
	 * IRQ 07 | PCI0X0 VPD Access                 | Rising| Edge  | Non
	 * IRQ 08 | PCI0X0 MSI level 0                | High  | Lvl/ed| Non
	 * IRQ 09 | External IRQ 15 - (PCI-Express)   | pgm H | Pgm   | Non
	 * IRQ 10 | UIC2 Non-critical Int.            | NA    | NA    | Non
	 * IRQ 11 | UIC2 Critical Interrupt           | NA    | NA    | Crit
	 * IRQ 12 | PCI Express MSI Level 0           | Rising| Edge  | Non
	 * IRQ 13 | PCI Express MSI Level 1           | Rising| Edge  | Non
	 * IRQ 14 | PCI Express MSI Level 2           | Rising| Edge  | Non
	 * IRQ 15 | PCI Express MSI Level 3           | Rising| Edge  | Non
	 * IRQ 16 | UIC3 Non-critical Int.            | NA    | NA    | Non
	 * IRQ 17 | UIC3 Critical Interrupt           | NA    | NA    | Crit
	 * IRQ 18 | External IRQ 14 - (PCI-Express)   | Pgm   | Pgm   | Non
	 * IRQ 19 | DMA Channel 0 FIFO Full           | High  | Level | Non
	 * IRQ 20 | DMA Channel 0 Stat FIFO           | High  | Level | Non
	 * IRQ 21 | DMA Channel 1 FIFO Full           | High  | Level | Non
	 * IRQ 22 | DMA Channel 1 Stat FIFO           | High  | Level | Non
	 * IRQ 23 | I2O Inbound Doorbell              | High  | Level | Non
	 * IRQ 24 | Inbound Post List FIFO Not Empt   | High  | Level | Non
	 * IRQ 25 | I2O Region 0 LL PLB Write         | High  | Level | Non
	 * IRQ 26 | I2O Region 1 LL PLB Write         | High  | Level | Non
	 * IRQ 27 | I2O Region 0 HB PLB Write         | High  | Level | Non
	 * IRQ 28 | I2O Region 1 HB PLB Write         | High  | Level | Non
	 * IRQ 29 | GPT Down Count Timer              | Rising| Edge  | Non
	 * IRQ 30 | UIC1 Non-critical Int.            | NA    | NA    | Non
	 * IRQ 31 | UIC1 Critical Interrupt           | NA    | NA    | Crit.
	 *--------------------------------------------------------------------
	 * IRQ 32 | Ext. IRQ 13 - (PCI-Express)       |pgm (H)|pgm/Lvl| Non
	 * IRQ 33 | MAL Serr                          | High  | Level | Non
	 * IRQ 34 | MAL Txde                          | High  | Level | Non
	 * IRQ 35 | MAL Rxde                          | High  | Level | Non
	 * IRQ 36 | DMC CE or DMC UE                  | High  | Level | Non
	 * IRQ 37 | EBC or UART2                      | High  |Lvl Edg| Non
	 * IRQ 38 | MAL TX EOB                        | High  | Level | Non
	 * IRQ 39 | MAL RX EOB                        | High  | Level | Non
	 * IRQ 40 | PCIX0 MSI Level 1                 | High  |Lvl Edg| Non
	 * IRQ 41 | PCIX0 MSI level 2                 | High  |Lvl Edg| Non
	 * IRQ 42 | PCIX0 MSI level 3                 | High  |Lvl Edg| Non
	 * IRQ 43 | L2 Cache                          | Risin | Edge  | Non
	 * IRQ 44 | GPT Compare Timer 0               | Risin | Edge  | Non
	 * IRQ 45 | GPT Compare Timer 1               | Risin | Edge  | Non
	 * IRQ 46 | GPT Compare Timer 2               | Risin | Edge  | Non
	 * IRQ 47 | GPT Compare Timer 3               | Risin | Edge  | Non
	 * IRQ 48 | GPT Compare Timer 4               | Risin | Edge  | Non
	 * IRQ 49 | Ext. IRQ 12 - PCI-X               |pgm/Fal|pgm/Lvl| Non
	 * IRQ 50 | Ext. IRQ 11 -                     |pgm (H)|pgm/Lvl| Non
	 * IRQ 51 | Ext. IRQ 10 -                     |pgm (H)|pgm/Lvl| Non
	 * IRQ 52 | Ext. IRQ 9                        |pgm (H)|pgm/Lvl| Non
	 * IRQ 53 | Ext. IRQ 8                        |pgm (H)|pgm/Lvl| Non
	 * IRQ 54 | DMA Error                         | High  | Level | Non
	 * IRQ 55 | DMA I2O Error                     | High  | Level | Non
	 * IRQ 56 | Serial ROM                        | High  | Level | Non
	 * IRQ 57 | PCIX0 Error                       | High  | Edge  | Non
	 * IRQ 58 | Ext. IRQ 7-                       |pgm (H)|pgm/Lvl| Non
	 * IRQ 59 | Ext. IRQ 6-                       |pgm (H)|pgm/Lvl| Non
	 * IRQ 60 | EMAC0 Interrupt                   | High  | Level | Non
	 * IRQ 61 | EMAC0 Wake-up                     | High  | Level | Non
	 * IRQ 62 | Reserved                          | High  | Level | Non
	 * IRQ 63 | XOR                               | High  | Level | Non
	 *--------------------------------------------------------------------
	 * IRQ 64 | PE0 AL                            | High  | Level | Non
	 * IRQ 65 | PE0 VPD Access                    | Risin | Edge  | Non
	 * IRQ 66 | PE0 Hot Reset Request             | Risin | Edge  | Non
	 * IRQ 67 | PE0 Hot Reset Request             | Falli | Edge  | Non
	 * IRQ 68 | PE0 TCR                           | High  | Level | Non
	 * IRQ 69 | PE0 BusMaster VCO                 | Falli | Edge  | Non
	 * IRQ 70 | PE0 DCR Error                     | High  | Level | Non
	 * IRQ 71 | Reserved                          | N/A   | N/A   | Non
	 * IRQ 72 | PE1 AL                            | High  | Level | Non
	 * IRQ 73 | PE1 VPD Access                    | Risin | Edge  | Non
	 * IRQ 74 | PE1 Hot Reset Request             | Risin | Edge  | Non
	 * IRQ 75 | PE1 Hot Reset Request             | Falli | Edge  | Non
	 * IRQ 76 | PE1 TCR                           | High  | Level | Non
	 * IRQ 77 | PE1 BusMaster VCO                 | Falli | Edge  | Non
	 * IRQ 78 | PE1 DCR Error                     | High  | Level | Non
	 * IRQ 79 | Reserved                          | N/A   | N/A   | Non
	 * IRQ 80 | PE2 AL                            | High  | Level | Non
	 * IRQ 81 | PE2 VPD Access                    | Risin | Edge  | Non
	 * IRQ 82 | PE2 Hot Reset Request             | Risin | Edge  | Non
	 * IRQ 83 | PE2 Hot Reset Request             | Falli | Edge  | Non
	 * IRQ 84 | PE2 TCR                           | High  | Level | Non
	 * IRQ 85 | PE2 BusMaster VCO                 | Falli | Edge  | Non
	 * IRQ 86 | PE2 DCR Error                     | High  | Level | Non
	 * IRQ 87 | Reserved                          | N/A   | N/A   | Non
	 * IRQ 88 | External IRQ(5)                   | Progr | Progr | Non
	 * IRQ 89 | External IRQ 4 - Ethernet         | Progr | Progr | Non
	 * IRQ 90 | External IRQ 3 - PCI-X            | Progr | Progr | Non
	 * IRQ 91 | External IRQ 2 - PCI-X            | Progr | Progr | Non
	 * IRQ 92 | External IRQ 1 - PCI-X            | Progr | Progr | Non
	 * IRQ 93 | External IRQ 0 - PCI-X            | Progr | Progr | Non
	 * IRQ 94 | Reserved                          | N/A   | N/A   | Non
	 * IRQ 95 | Reserved                          | N/A   | N/A   | Non
	 *--------------------------------------------------------------------
	 * IRQ 96 | PE0 INTA                          | High  | Level | Non
	 * IRQ 97 | PE0 INTB                          | High  | Level | Non
	 * IRQ 98 | PE0 INTC                          | High  | Level | Non
	 * IRQ 99 | PE0 INTD                          | High  | Level | Non
	 * IRQ 100| PE1 INTA                          | High  | Level | Non
	 * IRQ 101| PE1 INTB                          | High  | Level | Non
	 * IRQ 102| PE1 INTC                          | High  | Level | Non
	 * IRQ 103| PE1 INTD                          | High  | Level | Non
	 * IRQ 104| PE2 INTA                          | High  | Level | Non
	 * IRQ 105| PE2 INTB                          | High  | Level | Non
	 * IRQ 106| PE2 INTC                          | High  | Level | Non
	 * IRQ 107| PE2 INTD                          | Risin | Edge  | Non
	 * IRQ 108| PCI Express MSI Level 4           | Risin | Edge  | Non
	 * IRQ 109| PCI Express MSI Level 5           | Risin | Edge  | Non
	 * IRQ 110| PCI Express MSI Level 6           | Risin | Edge  | Non
	 * IRQ 111| PCI Express MSI Level 7           | Risin | Edge  | Non
	 * IRQ 116| PCI Express MSI Level 12          | Risin | Edge  | Non
	 * IRQ 112| PCI Express MSI Level 8           | Risin | Edge  | Non
	 * IRQ 113| PCI Express MSI Level 9           | Risin | Edge  | Non
	 * IRQ 114| PCI Express MSI Level 10          | Risin | Edge  | Non
	 * IRQ 115| PCI Express MSI Level 11          | Risin | Edge  | Non
	 * IRQ 117| PCI Express MSI Level 13          | Risin | Edge  | Non
	 * IRQ 118| PCI Express MSI Level 14          | Risin | Edge  | Non
	 * IRQ 119| PCI Express MSI Level 15          | Risin | Edge  | Non
	 * IRQ 120| PCI Express MSI Level 16          | Risin | Edge  | Non
	 * IRQ 121| PCI Express MSI Level 17          | Risin | Edge  | Non
	 * IRQ 122| PCI Express MSI Level 18          | Risin | Edge  | Non
	 * IRQ 123| PCI Express MSI Level 19          | Risin | Edge  | Non
	 * IRQ 124| PCI Express MSI Level 20          | Risin | Edge  | Non
	 * IRQ 125| PCI Express MSI Level 21          | Risin | Edge  | Non
	 * IRQ 126| PCI Express MSI Level 22          | Risin | Edge  | Non
	 * IRQ 127| PCI Express MSI Level 23          | Risin | Edge  | Non
	 */

	/*
	 * Put UICs in PowerPC 440SPe mode.
	 * Initialise UIC registers. Clear all interrupts. Disable all
	 * interrupts. Set critical interrupt values. Set interrupt polarities.
	 * Set interrupt trigger levels. Make bit 0 High priority. Clear all
	 * interrupts again.
	 */
	mtdcr(UIC3SR, 0xffffffff);	/* Clear all interrupts */
	mtdcr(UIC3ER, 0x00000000);	/* disable all interrupts */
	mtdcr(UIC3CR, 0x00000000);	/* Set Critical / Non Critical IRQs */
	mtdcr(UIC3PR, 0xffffffff);	/* Set Interrupt Polarities*/
	mtdcr(UIC3TR, 0x001fffff);	/* Set Interrupt Trigger Levels */
	mtdcr(UIC3VR, 0x00000001);	/* Set Vect base=0,INT31 Highest prio */
	mtdcr(UIC3SR, 0x00000000);	/* clear all  interrupts*/
	mtdcr(UIC3SR, 0xffffffff);	/* clear all  interrupts*/

	mtdcr(UIC2SR, 0xffffffff);	/* Clear all interrupts */
	mtdcr(UIC2ER, 0x00000000);	/* disable all interrupts*/
	mtdcr(UIC2CR, 0x00000000);	/* Set Critical / Non Critical IRQs */
	mtdcr(UIC2PR, 0xebebebff);	/* Set Interrupt Polarities*/
	mtdcr(UIC2TR, 0x74747400);	/* Set Interrupt Trigger Levels */
	mtdcr(UIC2VR, 0x00000001);	/* Set Vect base=0,INT31 Highest prio */
	mtdcr(UIC2SR, 0x00000000);	/* clear all interrupts */
	mtdcr(UIC2SR, 0xffffffff);	/* clear all interrupts */

	mtdcr(UIC1SR, 0xffffffff);	/* Clear all interrupts*/
	mtdcr(UIC1ER, 0x00000000);	/* disable all interrupts*/
	mtdcr(UIC1CR, 0x00000000);	/* Set Critical / Non Critical IRQs */
	mtdcr(UIC1PR, 0xffffffff);	/* Set Interrupt Polarities */
	mtdcr(UIC1TR, 0x001f8040);	/* Set Interrupt Trigger Levels*/
	mtdcr(UIC1VR, 0x00000001);	/* Set Vect base=0,INT31 Highest prio */
	mtdcr(UIC1SR, 0x00000000);	/* clear all interrupts*/
	mtdcr(UIC1SR, 0xffffffff);	/* clear all interrupts*/

	mtdcr(UIC0SR, 0xffffffff);	/* Clear all interrupts */
	mtdcr(UIC0ER, 0x00000000);	/* disable all int. excepted cascade */
	mtdcr(UIC0CR, 0x00104001);	/* Set Critical / Non Critical IRQs */
	mtdcr(UIC0PR, 0xffffffff);	/* Set Interrupt Polarities*/
	mtdcr(UIC0TR, 0x010f0004);	/* Set Interrupt Trigger Levels */
	mtdcr(UIC0VR, 0x00000001);	/* Set Vect base=0,INT31 Highest prio */
	mtdcr(UIC0SR, 0x00000000);	/* clear all interrupts*/
	mtdcr(UIC0SR, 0xffffffff);	/* clear all interrupts*/

	mfsdr(SDR0_MFR, mfr);
	mfr |= SDR0_MFR_FIXD;		/* Workaround for PCI/DMA */
	mtsdr(SDR0_MFR, mfr);

	mtsdr(SDR0_PFC0, CONFIG_SYS_PFC0);

	out_be32((void *)GPIO0_OR, CONFIG_SYS_GPIO_OR);
	out_be32((void *)GPIO0_ODR, CONFIG_SYS_GPIO_ODR);
	out_be32((void *)GPIO0_TCR, CONFIG_SYS_GPIO_TCR);

	return 0;
}

int board_early_init_r(void)
{
	/*
	 * ICON has 64MBytes of NOR FLASH (Spansion 29GL512), but the
	 * boot EBC mapping only supports a maximum of 16MBytes
	 * (4.ff00.0000 - 4.ffff.ffff).
	 * To solve this problem, the FLASH has to get remapped to another
	 * EBC address which accepts bigger regions:
	 *
	 * 0xfc00.0000 -> 4.ec00.0000
	 */

	/* Remap the NOR FLASH to 0xec00.0000 ... 0xefff.ffff */
	mtebc(PB0CR, CONFIG_SYS_FLASH_BASE_PHYS_L | 0xda000);

	/* Remove TLB entry of boot EBC mapping */
	remove_tlb(CONFIG_SYS_BOOT_BASE_ADDR, 16 << 20);

	/* Add TLB entry for 0xfc00.0000 -> 0x4.ec00.0000 */
	program_tlb(CONFIG_SYS_FLASH_BASE_PHYS, CONFIG_SYS_FLASH_BASE,
		    CONFIG_SYS_FLASH_SIZE, TLB_WORD2_I_ENABLE);

	/*
	 * Now accessing of the whole 64Mbytes of NOR FLASH at virtual address
	 * 0xfc00.0000 is possible
	 */

	/*
	 * Clear potential errors resulting from auto-calibration.
	 * If not done, then we could get an interrupt later on when
	 * exceptions are enabled.
	 */
	set_mcsr(get_mcsr());

	return 0;
}

int checkboard(void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

	printf("Board: ICON");
	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	putc('\n');

	return 0;
}

/*
 * Override the default functions in cpu/ppc4xx/44x_spd_ddr2.c with
 * board specific values.
 *
 * Tested successfully with the following SODIMM:
 * Crucial CT6464AC667.4FE - 512MB SO-DIMM (single rank)
 *
 * Tests with Micron MT4HTF6464HZ-667H1 showed problems in "cold" state,
 * directly after power-up. Only after running for more than 10 minutes
 * real stable auto-calibration windows could be found.
 */
u32 ddr_wrdtr(u32 default_val)
{
	return SDRAM_WRDTR_LLWP_1_CYC | SDRAM_WRDTR_WTR_90_DEG_ADV;
}

u32 ddr_clktr(u32 default_val)
{
	return SDRAM_CLKTR_CLKP_180_DEG_ADV;
}

/*
 * Override the weak default implementation and return the
 * last PCIe slot number (max number - 1).
 */
int board_pcie_last(void)
{
	/* Only 2 PCIe ports used on ICON, so the last one is 1 */
	return 1;
}

/*
 * Video
 */
#ifdef CONFIG_VIDEO_SM501
#include <sm501.h>

#define DISPLAY_WIDTH   640
#define DISPLAY_HEIGHT  480

static const SMI_REGS sm502_init_regs[] = {
	{0x00004, 0x0},
	{0x00040, 0x00021847},
	{0x00044, 0x091a0a01}, /* 24 MHz pixclk */
	{0x00054, 0x0},
	{0x00048, 0x00021847},
	{0x0004C, 0x091a0a01},
	{0x00054, 0x1},
	{0x80004, 0xc428bb17},
	{0x8000C, 0x00000000},
	{0x80010, 0x0a000a00},
	{0x80014, 0x02800000},
	{0x80018, 0x01e00000},
	{0x8001C, 0x00000000},
	{0x80020, 0x01e00280},
	{0x80024, 0x02fa027f},
	{0x80028, 0x004a0280},
	{0x8002C, 0x020c01df},
	{0x80030, 0x000201e7},
	{0x80200, 0x00010000},
	{0x00008, 0x20000000}, /* gpio29 is pwm0, LED_PWM */
	{0x0000C, 0x3f000000}, /* gpio56 - gpio61 as flat panel data pins */
	{0x10020, 0x25725728}, /* 20 kHz pwm0, 50 % duty cycle, disabled */
	{0x80000, 0x0f010106}, /* vsync & hsync pos, disp on */
	{0, 0}
};

/*
 * Return a pointer to the register initialization table.
 */
const SMI_REGS *board_get_regs(void)
{
	return sm502_init_regs;
}

int board_get_width(void)
{
	return DISPLAY_WIDTH;
}

int board_get_height(void)
{
	return DISPLAY_HEIGHT;
}

#ifdef CONFIG_CONSOLE_EXTRA_INFO
/*
 * Return text to be printed besides the logo.
 */
void video_get_info_str(int line_number, char *info)
{
	if (line_number == 1)
		strcpy(info, " Board: ICON");
	else
		info[0] = '\0';
}
#endif

#endif /* CONFIG_VIDEO_SM501 */
