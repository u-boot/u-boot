/*
 * TNETV107X: Hardware information
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#ifndef __ASSEMBLY__

#include <asm/sizes.h>

#define ASYNC_EMIF_NUM_CS		4
#define ASYNC_EMIF_MODE_NOR		0
#define ASYNC_EMIF_MODE_NAND		1
#define ASYNC_EMIF_MODE_ONENAND		2
#define ASYNC_EMIF_PRESERVE		-1

struct async_emif_config {
	unsigned mode;
	unsigned select_strobe;
	unsigned extend_wait;
	unsigned wr_setup;
	unsigned wr_strobe;
	unsigned wr_hold;
	unsigned rd_setup;
	unsigned rd_strobe;
	unsigned rd_hold;
	unsigned turn_around;
	enum {
		ASYNC_EMIF_8	= 0,
		ASYNC_EMIF_16	= 1,
		ASYNC_EMIF_32	= 2,
	} width;
};

void init_async_emif(int num_cs, struct async_emif_config *config);

int wdt_start(unsigned long msecs);
int wdt_stop(void);
int wdt_kick(void);

#endif

/* Chip configuration unlock codes and registers */
#define TNETV107X_KICK0		(TNETV107X_CHIP_CONFIG_SYS_BASE+0x38)
#define TNETV107X_KICK1		(TNETV107X_CHIP_CONFIG_SYS_BASE+0x3c)
#define TNETV107X_PINMUX(n)	(TNETV107X_CHIP_CONFIG_SYS_BASE+0x150+(n)*4)
#define TNETV107X_KICK0_MAGIC	0x83e70b13
#define TNETV107X_KICK1_MAGIC	0x95a4f1e0

/* Module base addresses */
#define TNETV107X_TPCC_BASE			0x01C00000
#define TNETV107X_TPTC0_BASE			0x01C10000
#define TNETV107X_TPTC1_BASE			0x01C10400
#define TNETV107X_INTC_BASE			0x03000000
#define TNETV107X_LCD_CONTROLLER_BASE		0x08030000
#define TNETV107X_INTD_BASE			0x08038000
#define TNETV107X_INTD_IPC_BASE			0x08038000
#define TNETV107X_INTD_FAST_BASE		0x08039000
#define TNETV107X_INTD_ASYNC_BASE		0x0803A000
#define TNETV107X_INTD_SLOW_BASE		0x0803B000
#define TNETV107X_PKA_BASE			0x08040000
#define TNETV107X_RNG_BASE			0x08044000
#define TNETV107X_TIMER0_BASE			0x08086500
#define TNETV107X_TIMER1_BASE			0x08086600
#define TNETV107X_WDT0_ARM_BASE			0x08086700
#define TNETV107X_WDT1_DSP_BASE			0x08086800
#define TNETV107X_CHIP_CONFIG_SYS_BASE		0x08087000
#define TNETV107X_GPIO_BASE			0x08088000
#define TNETV107X_UART1_BASE			0x08088400
#define TNETV107X_TOUCHSCREEN_BASE		0x08088500
#define TNETV107X_SDIO0_BASE			0x08088700
#define TNETV107X_SDIO1_BASE			0x08088800
#define TNETV107X_MDIO_BASE			0x08088900
#define TNETV107X_KEYPAD_BASE			0x08088A00
#define TNETV107X_SSP_BASE			0x08088C00
#define TNETV107X_CLOCK_CONTROL_BASE		0x0808A000
#define TNETV107X_PSC_BASE			0x0808B000
#define TNETV107X_TDM0_BASE			0x08100000
#define TNETV107X_TDM1_BASE			0x08100100
#define TNETV107X_MCDMA_BASE			0x08108000
#define TNETV107X_UART0_DMA_BASE		0x08108200
#define TNETV107X_USBSS_BASE			0x08120000
#define TNETV107X_VLYNQ_CONTROL_BASE		0x0810D000
#define TNETV107X_ASYNC_EMIF_CNTRL_BASE		0x08200000
#define TNETV107X_VLYNQ_MEM_MAP_BASE		0x0C000000
#define TNETV107X_IMCOP_BASE			0x01CC0000
#define TNETV107X_MBX_LITE_BASE			0x07000000
#define TNETV107X_ETHSS_BASE			0x0803C000
#define TNETV107X_CPSW_BASE			0x0803C000
#define TNETV107X_SPF_BASE			0x0803C800
#define TNETV107X_IOPU_ETHSS_BASE		0x0803D000
#define TNETV107X_VTP_CNTRL_0			0x0803D800
#define TNETV107X_VTP_CNTRL_1			0x0803D900
#define TNETV107X_UART2_DMA_BASE		0x08108400
#define TNETV107X_INTERNAL_MEMORY		0x20000000
#define TNETV107X_ASYNC_EMIF_DATA_CE0_BASE	0x30000000
#define TNETV107X_ASYNC_EMIF_DATA_CE1_BASE	0x40000000
#define TNETV107X_ASYNC_EMIF_DATA_CE2_BASE	0x44000000
#define TNETV107X_ASYNC_EMIF_DATA_CE3_BASE	0x48000000
#define TNETV107X_DDR_EMIF_DATA_BASE		0x80000000
#define TNETV107X_DDR_EMIF_CONTROL_BASE		0x90000000

/* LPSC module definitions */
#define TNETV107X_LPSC_ARM			0
#define TNETV107X_LPSC_GEM			1
#define TNETV107X_LPSC_DDR2_PHY			2
#define TNETV107X_LPSC_TPCC			3
#define TNETV107X_LPSC_TPTC0			4
#define TNETV107X_LPSC_TPTC1			5
#define TNETV107X_LPSC_RAM			6
#define TNETV107X_LPSC_MBX_LITE			7
#define TNETV107X_LPSC_LCD			8
#define TNETV107X_LPSC_ETHSS			9
#define TNETV107X_LPSC_AEMIF			10
#define TNETV107X_LPSC_CHIP_CFG			11
#define TNETV107X_LPSC_TSC			12
#define TNETV107X_LPSC_ROM			13
#define TNETV107X_LPSC_UART2			14
#define TNETV107X_LPSC_PKTSEC			15
#define TNETV107X_LPSC_SECCTL			16
#define TNETV107X_LPSC_KEYMGR			17
#define TNETV107X_LPSC_KEYPAD			18
#define TNETV107X_LPSC_GPIO			19
#define TNETV107X_LPSC_MDIO			20
#define TNETV107X_LPSC_SDIO0			21
#define TNETV107X_LPSC_UART0			22
#define TNETV107X_LPSC_UART1			23
#define TNETV107X_LPSC_TIMER0			24
#define TNETV107X_LPSC_TIMER1			25
#define TNETV107X_LPSC_WDT_ARM			26
#define TNETV107X_LPSC_WDT_DSP			27
#define TNETV107X_LPSC_SSP			28
#define TNETV107X_LPSC_TDM0			29
#define TNETV107X_LPSC_VLYNQ			30
#define TNETV107X_LPSC_MCDMA			31
#define TNETV107X_LPSC_USB0			32
#define TNETV107X_LPSC_TDM1			33
#define TNETV107X_LPSC_DEBUGSS			34
#define TNETV107X_LPSC_ETHSS_RGMII		35
#define TNETV107X_LPSC_SYSTEM			36
#define TNETV107X_LPSC_IMCOP			37
#define TNETV107X_LPSC_SPARE			38
#define TNETV107X_LPSC_SDIO1			39
#define TNETV107X_LPSC_USB1			40
#define TNETV107X_LPSC_USBSS			41
#define TNETV107X_LPSC_DDR2_EMIF1_VRST		42
#define TNETV107X_LPSC_DDR2_EMIF2_VCTL_RST	43
#define TNETV107X_LPSC_MAX			44

/* Interrupt controller */
#define INTC_GLB_EN			(TNETV107X_INTC_BASE + 0x10)
#define INTC_HINT_EN			(TNETV107X_INTC_BASE + 0x1500)
#define INTC_EN_CLR0			(TNETV107X_INTC_BASE + 0x380)

#endif /* __ASM_ARCH_HARDWARE_H */
