/*
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
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

#ifndef __ASM_ARCH_MX31_REGS_H
#define __ASM_ARCH_MX31_REGS_H

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>

/* Clock control module registers */
struct clock_control_regs {
	u32 ccmr;
	u32 pdr0;
	u32 pdr1;
	u32 rcsr;
	u32 mpctl;
	u32 upctl;
	u32 spctl;
	u32 cosr;
	u32 cgr0;
	u32 cgr1;
	u32 cgr2;
	u32 wimr0;
	u32 ldc;
	u32 dcvr0;
	u32 dcvr1;
	u32 dcvr2;
	u32 dcvr3;
	u32 ltr0;
	u32 ltr1;
	u32 ltr2;
	u32 ltr3;
	u32 ltbr0;
	u32 ltbr1;
	u32 pmcr0;
	u32 pmcr1;
	u32 pdr2;
};

/* Bit definitions for RCSR register in CCM */
#define CCM_RCSR_NF16B	(1 << 31)
#define CCM_RCSR_NFMS	(1 << 30)

#endif

#define __REG(x)     (*((volatile u32 *)(x)))
#define __REG16(x)   (*((volatile u16 *)(x)))
#define __REG8(x)    (*((volatile u8 *)(x)))

#define CCM_BASE	0x53f80000
#define CCM_CCMR	(CCM_BASE + 0x00)
#define CCM_PDR0	(CCM_BASE + 0x04)
#define CCM_PDR1	(CCM_BASE + 0x08)
#define CCM_RCSR	(CCM_BASE + 0x0c)
#define CCM_MPCTL	(CCM_BASE + 0x10)
#define CCM_UPCTL	(CCM_BASE + 0x14)
#define CCM_SPCTL	(CCM_BASE + 0x18)
#define CCM_COSR	(CCM_BASE + 0x1C)
#define CCM_CGR0	(CCM_BASE + 0x20)
#define CCM_CGR1	(CCM_BASE + 0x24)
#define CCM_CGR2	(CCM_BASE + 0x28)

#define CCMR_MDS	(1 << 7)
#define CCMR_SBYCS	(1 << 4)
#define CCMR_MPE	(1 << 3)
#define CCMR_PRCS_MASK	(3 << 1)
#define CCMR_FPM	(1 << 1)
#define CCMR_CKIH	(2 << 1)

#define PDR0_CSI_PODF(x)	(((x) & 0x1ff) << 23)
#define PDR0_PER_PODF(x)	(((x) & 0x1f) << 16)
#define PDR0_HSP_PODF(x)	(((x) & 0x7) << 11)
#define PDR0_NFC_PODF(x)	(((x) & 0x7) << 8)
#define PDR0_IPG_PODF(x)	(((x) & 0x3) << 6)
#define PDR0_MAX_PODF(x)	(((x) & 0x7) << 3)
#define PDR0_MCU_PODF(x)	((x) & 0x7)

#define PLL_PD(x)		(((x) & 0xf) << 26)
#define PLL_MFD(x)		(((x) & 0x3ff) << 16)
#define PLL_MFI(x)		(((x) & 0xf) << 10)
#define PLL_MFN(x)		(((x) & 0x3ff) << 0)

#define WEIM_ESDCTL0	0xB8001000
#define WEIM_ESDCFG0	0xB8001004
#define WEIM_ESDCTL1	0xB8001008
#define WEIM_ESDCFG1	0xB800100C
#define WEIM_ESDMISC	0xB8001010

#define ESDCTL_SDE			(1 << 31)
#define ESDCTL_CMD_RW			(0 << 28)
#define ESDCTL_CMD_PRECHARGE		(1 << 28)
#define ESDCTL_CMD_AUTOREFRESH		(2 << 28)
#define ESDCTL_CMD_LOADMODEREG		(3 << 28)
#define ESDCTL_CMD_MANUALREFRESH	(4 << 28)
#define ESDCTL_ROW_13			(2 << 24)
#define ESDCTL_ROW(x)			((x) << 24)
#define ESDCTL_COL_9			(1 << 20)
#define ESDCTL_COL(x)			((x) << 20)
#define ESDCTL_DSIZ(x)			((x) << 16)
#define ESDCTL_SREFR(x)			((x) << 13)
#define ESDCTL_PWDT(x)			((x) << 10)
#define ESDCTL_FP(x)			((x) << 8)
#define ESDCTL_BL(x)			((x) << 7)
#define ESDCTL_PRCT(x)			((x) << 0)

#define WEIM_BASE	0xb8002000
#define CSCR_U(x)	(WEIM_BASE + (x) * 0x10)
#define CSCR_L(x)	(WEIM_BASE + 4 + (x) * 0x10)
#define CSCR_A(x)	(WEIM_BASE + 8 + (x) * 0x10)

#define IOMUXC_BASE	0x43FAC000
#define IOMUXC_GPR	(IOMUXC_BASE + 0x8)
#define IOMUXC_SW_MUX_CTL(x)	(IOMUXC_BASE + 0xc + (x) * 4)
#define IOMUXC_SW_PAD_CTL(x)	(IOMUXC_BASE + 0x154 + (x) * 4)

#define IPU_BASE		0x53fc0000
#define IPU_CONF		IPU_BASE

#define IPU_CONF_PXL_ENDIAN	(1<<8)
#define IPU_CONF_DU_EN		(1<<7)
#define IPU_CONF_DI_EN		(1<<6)
#define IPU_CONF_ADC_EN		(1<<5)
#define IPU_CONF_SDC_EN		(1<<4)
#define IPU_CONF_PF_EN		(1<<3)
#define IPU_CONF_ROT_EN		(1<<2)
#define IPU_CONF_IC_EN		(1<<1)
#define IPU_CONF_SCI_EN		(1<<0)

#define ARM_PPMRR		0x40000015

#define WDOG_BASE		0x53FDC000

/*
 * GPIO
 */
#define GPIO1_BASE	0x53FCC000
#define GPIO2_BASE	0x53FD0000
#define GPIO3_BASE	0x53FA4000
#define GPIO_DR		0x00000000	/* data register */
#define GPIO_GDIR	0x00000004	/* direction register */
#define GPIO_PSR	0x00000008	/* pad status register */

/*
 * Signal Multiplexing (IOMUX)
 */

/* bits in the SW_MUX_CTL registers */
#define MUX_CTL_OUT_GPIO_DR	(0 << 4)
#define MUX_CTL_OUT_FUNC	(1 << 4)
#define MUX_CTL_OUT_ALT1	(2 << 4)
#define MUX_CTL_OUT_ALT2	(3 << 4)
#define MUX_CTL_OUT_ALT3	(4 << 4)
#define MUX_CTL_OUT_ALT4	(5 << 4)
#define MUX_CTL_OUT_ALT5	(6 << 4)
#define MUX_CTL_OUT_ALT6	(7 << 4)
#define MUX_CTL_IN_NONE		(0 << 0)
#define MUX_CTL_IN_GPIO		(1 << 0)
#define MUX_CTL_IN_FUNC		(2 << 0)
#define MUX_CTL_IN_ALT1		(4 << 0)
#define MUX_CTL_IN_ALT2		(8 << 0)

#define MUX_CTL_FUNC		(MUX_CTL_OUT_FUNC | MUX_CTL_IN_FUNC)
#define MUX_CTL_ALT1		(MUX_CTL_OUT_ALT1 | MUX_CTL_IN_ALT1)
#define MUX_CTL_ALT2		(MUX_CTL_OUT_ALT2 | MUX_CTL_IN_ALT2)
#define MUX_CTL_GPIO		(MUX_CTL_OUT_GPIO_DR | MUX_CTL_IN_GPIO)

/* Register offsets based on IOMUXC_BASE */
/* 0x00 .. 0x7b */
#define MUX_CTL_RTS1		0x7c
#define MUX_CTL_CTS1		0x7d
#define MUX_CTL_DTR_DCE1	0x7e
#define MUX_CTL_DSR_DCE1	0x7f
#define MUX_CTL_CSPI2_SCLK	0x80
#define MUX_CTL_CSPI2_SPI_RDY	0x81
#define MUX_CTL_RXD1		0x82
#define MUX_CTL_TXD1		0x83
#define MUX_CTL_CSPI2_MISO	0x84
#define MUX_CTL_CSPI2_SS0	0x85
#define MUX_CTL_CSPI2_SS1	0x86
#define MUX_CTL_CSPI2_SS2	0x87
#define MUX_CTL_CSPI1_SS2	0x88
#define MUX_CTL_CSPI1_SCLK	0x89
#define MUX_CTL_CSPI1_SPI_RDY	0x8a
#define MUX_CTL_CSPI2_MOSI	0x8b
#define MUX_CTL_CSPI1_MOSI	0x8c
#define MUX_CTL_CSPI1_MISO	0x8d
#define MUX_CTL_CSPI1_SS0	0x8e
#define MUX_CTL_CSPI1_SS1	0x8f

/*
 * Helper macros for the MUX_[contact name]__[pin function] macros
 */
#define IOMUX_MODE_POS 9
#define IOMUX_MODE(contact, mode) (((mode) << IOMUX_MODE_POS) | (contact))

/*
 * These macros can be used in mx31_gpio_mux() and have the form
 * MUX_[contact name]__[pin function]
 */
#define MUX_RXD1__UART1_RXD_MUX	IOMUX_MODE(MUX_CTL_RXD1, MUX_CTL_FUNC)
#define MUX_TXD1__UART1_TXD_MUX	IOMUX_MODE(MUX_CTL_TXD1, MUX_CTL_FUNC)
#define MUX_RTS1__UART1_RTS_B	IOMUX_MODE(MUX_CTL_RTS1, MUX_CTL_FUNC)
#define MUX_CTS1__UART1_CTS_B	IOMUX_MODE(MUX_CTL_CTS1, MUX_CTL_FUNC)

#define MUX_CSPI2_SS0__CSPI2_SS0_B IOMUX_MODE(MUX_CTL_CSPI2_SS0, MUX_CTL_FUNC)
#define MUX_CSPI2_SS1__CSPI2_SS1_B IOMUX_MODE(MUX_CTL_CSPI2_SS1, MUX_CTL_FUNC)
#define MUX_CSPI2_SS2__CSPI2_SS2_B IOMUX_MODE(MUX_CTL_CSPI2_SS2, MUX_CTL_FUNC)
#define MUX_CSPI2_MOSI__CSPI2_MOSI IOMUX_MODE(MUX_CTL_CSPI2_MOSI, MUX_CTL_FUNC)
#define MUX_CSPI2_MISO__CSPI2_MISO IOMUX_MODE(MUX_CTL_CSPI2_MISO, MUX_CTL_FUNC)
#define MUX_CSPI2_SPI_RDY__CSPI2_DATAREADY_B \
	IOMUX_MODE(MUX_CTL_CSPI2_SPI_RDY, MUX_CTL_FUNC)
#define MUX_CSPI2_SCLK__CSPI2_CLK IOMUX_MODE(MUX_CTL_CSPI2_SCLK, MUX_CTL_FUNC)

#define MUX_CSPI1_SS0__CSPI1_SS0_B IOMUX_MODE(MUX_CTL_CSPI1_SS0, MUX_CTL_FUNC)
#define MUX_CSPI1_SS1__CSPI1_SS1_B IOMUX_MODE(MUX_CTL_CSPI1_SS1, MUX_CTL_FUNC)
#define MUX_CSPI1_SS2__CSPI1_SS2_B IOMUX_MODE(MUX_CTL_CSPI1_SS2, MUX_CTL_FUNC)
#define MUX_CSPI1_MOSI__CSPI1_MOSI IOMUX_MODE(MUX_CTL_CSPI1_MOSI, MUX_CTL_FUNC)
#define MUX_CSPI1_MISO__CSPI1_MISO IOMUX_MODE(MUX_CTL_CSPI1_MISO, MUX_CTL_FUNC)
#define MUX_CSPI1_SPI_RDY__CSPI1_DATAREADY_B \
	IOMUX_MODE(MUX_CTL_CSPI1_SPI_RDY, MUX_CTL_FUNC)
#define MUX_CSPI1_SCLK__CSPI1_CLK IOMUX_MODE(MUX_CTL_CSPI1_SCLK, MUX_CTL_FUNC)

#define MUX_CSPI2_MOSI__I2C2_SCL IOMUX_MODE(MUX_CTL_CSPI2_MOSI, MUX_CTL_ALT1)
#define MUX_CSPI2_MISO__I2C2_SDA IOMUX_MODE(MUX_CTL_CSPI2_MISO, MUX_CTL_ALT1)

/* PAD control registers for SDR/DDR */
#define IOMUXC_SW_PAD_CTL_SDCKE1_SDCLK_SDCLK_B	(IOMUXC_BASE + 0x26C)
#define IOMUXC_SW_PAD_CTL_CAS_SDWE_SDCKE0	(IOMUXC_BASE + 0x270)
#define IOMUXC_SW_PAD_CTL_BCLK_RW_RAS		(IOMUXC_BASE + 0x274)
#define IOMUXC_SW_PAD_CTL_CS5_ECB_LBA		(IOMUXC_BASE + 0x278)
#define IOMUXC_SW_PAD_CTL_CS2_CS3_CS4		(IOMUXC_BASE + 0x27C)
#define IOMUXC_SW_PAD_CTL_OE_CS0_CS1		(IOMUXC_BASE + 0x280)
#define IOMUXC_SW_PAD_CTL_DQM3_EB0_EB1		(IOMUXC_BASE + 0x284)
#define IOMUXC_SW_PAD_CTL_DQM0_DQM1_DQM2	(IOMUXC_BASE + 0x288)
#define IOMUXC_SW_PAD_CTL_SD29_SD30_SD31	(IOMUXC_BASE + 0x28C)
#define IOMUXC_SW_PAD_CTL_SD26_SD27_SD28	(IOMUXC_BASE + 0x290)
#define IOMUXC_SW_PAD_CTL_SD23_SD24_SD25	(IOMUXC_BASE + 0x294)
#define IOMUXC_SW_PAD_CTL_SD20_SD21_SD22	(IOMUXC_BASE + 0x298)
#define IOMUXC_SW_PAD_CTL_SD17_SD18_SD19	(IOMUXC_BASE + 0x29C)
#define IOMUXC_SW_PAD_CTL_SD14_SD15_SD16	(IOMUXC_BASE + 0x2A0)
#define IOMUXC_SW_PAD_CTL_SD11_SD12_SD13	(IOMUXC_BASE + 0x2A4)
#define IOMUXC_SW_PAD_CTL_SD8_SD9_SD10		(IOMUXC_BASE + 0x2A8)
#define IOMUXC_SW_PAD_CTL_SD5_SD6_SD7		(IOMUXC_BASE + 0x2AC)
#define IOMUXC_SW_PAD_CTL_SD2_SD3_SD4		(IOMUXC_BASE + 0x2B0)
#define IOMUXC_SW_PAD_CTL_SDBA0_SD0_SD1		(IOMUXC_BASE + 0x2B4)
#define IOMUXC_SW_PAD_CTL_A24_A25_SDBA1		(IOMUXC_BASE + 0x2B8)
#define IOMUXC_SW_PAD_CTL_A21_A22_A23		(IOMUXC_BASE + 0x2BC)
#define IOMUXC_SW_PAD_CTL_A18_A19_A20		(IOMUXC_BASE + 0x2C0)
#define IOMUXC_SW_PAD_CTL_A15_A16_A17		(IOMUXC_BASE + 0x2C4)
#define IOMUXC_SW_PAD_CTL_A12_A13_A14		(IOMUXC_BASE + 0x2C8)
#define IOMUXC_SW_PAD_CTL_A10_MA10_A11		(IOMUXC_BASE + 0x2CC)
#define IOMUXC_SW_PAD_CTL_A7_A8_A9		(IOMUXC_BASE + 0x2D0)
#define IOMUXC_SW_PAD_CTL_A4_A5_A6		(IOMUXC_BASE + 0x2D4)
#define IOMUXC_SW_PAD_CTL_A1_A2_A3		(IOMUXC_BASE + 0x2D8)
#define IOMUXC_SW_PAD_CTL_VPG0_VPG1_A0		(IOMUXC_BASE + 0x2DC)

/*
 * Memory regions and CS
 */
#define IPU_MEM_BASE	0x70000000
#define CSD0_BASE	0x80000000
#define CSD1_BASE	0x90000000
#define CS0_BASE	0xA0000000
#define CS1_BASE	0xA8000000
#define CS2_BASE	0xB0000000
#define CS3_BASE	0xB2000000
#define CS4_BASE	0xB4000000
#define CS4_PSRAM_BASE	0xB5000000
#define CS5_BASE	0xB6000000
#define PCMCIA_MEM_BASE	0xC0000000

/*
 * NAND controller
 */
#define NFC_BASE_ADDR	0xB8000000

#endif /* __ASM_ARCH_MX31_REGS_H */
