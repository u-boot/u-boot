/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Based on:
 *
 * -------------------------------------------------------------------------
 *
 *  linux/include/asm-arm/arch-davinci/hardware.h
 *
 *  Copyright (C) 2006 Texas Instruments.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <linux/sizes.h>

#define	REG(addr)	(*(volatile unsigned int *)(addr))
#define REG_P(addr)	((volatile unsigned int *)(addr))

#ifndef __ASSEMBLY__
typedef volatile unsigned int	dv_reg;
typedef volatile unsigned int *	dv_reg_p;
#endif

/*
 * Base register addresses
 *
 * NOTE:  some of these DM6446-specific addresses DO NOT WORK
 * on other DaVinci chips.  Double check them before you try
 * using the addresses ... or PSC module identifiers, etc.
 */
#ifndef CONFIG_SOC_DA8XX

#define DAVINCI_DMA_3PCC_BASE			(0x01c00000)
#define DAVINCI_DMA_3PTC0_BASE			(0x01c10000)
#define DAVINCI_DMA_3PTC1_BASE			(0x01c10400)
#define DAVINCI_UART0_BASE			(0x01c20000)
#define DAVINCI_UART1_BASE			(0x01c20400)
#define DAVINCI_TIMER3_BASE			(0x01c20800)
#define DAVINCI_I2C_BASE			(0x01c21000)
#define DAVINCI_TIMER0_BASE			(0x01c21400)
#define DAVINCI_TIMER1_BASE			(0x01c21800)
#define DAVINCI_WDOG_BASE			(0x01c21c00)
#define DAVINCI_PWM0_BASE			(0x01c22000)
#define DAVINCI_PWM1_BASE			(0x01c22400)
#define DAVINCI_PWM2_BASE			(0x01c22800)
#define DAVINCI_TIMER4_BASE			(0x01c23800)
#define DAVINCI_SYSTEM_MODULE_BASE		(0x01c40000)
#define DAVINCI_PLL_CNTRL0_BASE			(0x01c40800)
#define DAVINCI_PLL_CNTRL1_BASE			(0x01c40c00)
#define DAVINCI_PWR_SLEEP_CNTRL_BASE		(0x01c41000)
#define DAVINCI_ARM_INTC_BASE			(0x01c48000)
#define DAVINCI_USB_OTG_BASE			(0x01c64000)
#define DAVINCI_CFC_ATA_BASE			(0x01c66000)
#define DAVINCI_SPI_BASE			(0x01c66800)
#define DAVINCI_GPIO_BASE			(0x01c67000)
#define DAVINCI_VPSS_REGS_BASE			(0x01c70000)
#if !defined(CONFIG_SOC_DM646X)
#define DAVINCI_ASYNC_EMIF_DATA_CE0_BASE	(0x02000000)
#define DAVINCI_ASYNC_EMIF_DATA_CE1_BASE	(0x04000000)
#define DAVINCI_ASYNC_EMIF_DATA_CE2_BASE	(0x06000000)
#define DAVINCI_ASYNC_EMIF_DATA_CE3_BASE	(0x08000000)
#endif
#define DAVINCI_DDR_BASE			(0x80000000)

#ifdef CONFIG_SOC_DM644X
#define DAVINCI_UART2_BASE			0x01c20800
#define DAVINCI_UHPI_BASE			0x01c67800
#define DAVINCI_EMAC_CNTRL_REGS_BASE		0x01c80000
#define DAVINCI_EMAC_WRAPPER_CNTRL_REGS_BASE	0x01c81000
#define DAVINCI_EMAC_WRAPPER_RAM_BASE		0x01c82000
#define DAVINCI_MDIO_CNTRL_REGS_BASE		0x01c84000
#define DAVINCI_IMCOP_BASE			0x01cc0000
#define DAVINCI_ASYNC_EMIF_CNTRL_BASE		0x01e00000
#define DAVINCI_VLYNQ_BASE			0x01e01000
#define DAVINCI_ASP_BASE			0x01e02000
#define DAVINCI_MMC_SD_BASE			0x01e10000
#define DAVINCI_MS_BASE				0x01e20000
#define DAVINCI_VLYNQ_REMOTE_BASE		0x0c000000

#elif defined(CONFIG_SOC_DM355)
#define DAVINCI_MMC_SD1_BASE			0x01e00000
#define DAVINCI_ASP0_BASE			0x01e02000
#define DAVINCI_ASP1_BASE			0x01e04000
#define DAVINCI_UART2_BASE			0x01e06000
#define DAVINCI_ASYNC_EMIF_CNTRL_BASE		0x01e10000
#define DAVINCI_MMC_SD0_BASE			0x01e11000

#elif defined(CONFIG_SOC_DM365)
#define DAVINCI_MMC_SD1_BASE			0x01d00000
#define DAVINCI_ASYNC_EMIF_CNTRL_BASE		0x01d10000
#define DAVINCI_MMC_SD0_BASE			0x01d11000
#define DAVINCI_DDR_EMIF_CTRL_BASE		0x20000000
#define DAVINCI_SPI0_BASE			0x01c66000
#define DAVINCI_SPI1_BASE			0x01c66800

#elif defined(CONFIG_SOC_DM646X)
#define DAVINCI_ASYNC_EMIF_CNTRL_BASE		0x20008000
#define DAVINCI_ASYNC_EMIF_DATA_CE0_BASE	0x42000000
#define DAVINCI_ASYNC_EMIF_DATA_CE1_BASE	0x44000000
#define DAVINCI_ASYNC_EMIF_DATA_CE2_BASE	0x46000000
#define DAVINCI_ASYNC_EMIF_DATA_CE3_BASE	0x48000000

#endif

#else /* CONFIG_SOC_DA8XX */

#define DAVINCI_UART0_BASE			0x01c42000
#define DAVINCI_UART1_BASE			0x01d0c000
#define DAVINCI_UART2_BASE			0x01d0d000
#define DAVINCI_I2C0_BASE			0x01c22000
#define DAVINCI_I2C1_BASE			0x01e28000
#define DAVINCI_TIMER0_BASE			0x01c20000
#define DAVINCI_TIMER1_BASE			0x01c21000
#define DAVINCI_WDOG_BASE			0x01c21000
#define DAVINCI_RTC_BASE			0x01c23000
#define DAVINCI_PLL_CNTRL0_BASE			0x01c11000
#define DAVINCI_PLL_CNTRL1_BASE			0x01e1a000
#define DAVINCI_PSC0_BASE			0x01c10000
#define DAVINCI_PSC1_BASE			0x01e27000
#define DAVINCI_SPI0_BASE			0x01c41000
#define DAVINCI_USB_OTG_BASE			0x01e00000
#define DAVINCI_SPI1_BASE			(cpu_is_da830() ? \
						0x01e12000 : 0x01f0e000)
#define DAVINCI_GPIO_BASE			0x01e26000
#define DAVINCI_EMAC_CNTRL_REGS_BASE		0x01e23000
#define DAVINCI_EMAC_WRAPPER_CNTRL_REGS_BASE	0x01e22000
#define DAVINCI_EMAC_WRAPPER_RAM_BASE		0x01e20000
#define DAVINCI_MDIO_CNTRL_REGS_BASE		0x01e24000
#define DAVINCI_SYSCFG1_BASE			0x01e2c000
#define DAVINCI_MMC_SD0_BASE			0x01c40000
#define DAVINCI_MMC_SD1_BASE			0x01e1b000
#define DAVINCI_TIMER2_BASE			0x01f0c000
#define DAVINCI_TIMER3_BASE			0x01f0d000
#define DAVINCI_ASYNC_EMIF_CNTRL_BASE		0x68000000
#define DAVINCI_ASYNC_EMIF_DATA_CE0_BASE	0x40000000
#define DAVINCI_ASYNC_EMIF_DATA_CE2_BASE	0x60000000
#define DAVINCI_ASYNC_EMIF_DATA_CE3_BASE	0x62000000
#define DAVINCI_ASYNC_EMIF_DATA_CE4_BASE	0x64000000
#define DAVINCI_ASYNC_EMIF_DATA_CE5_BASE	0x66000000
#define DAVINCI_DDR_EMIF_CTRL_BASE		0xb0000000
#define DAVINCI_DDR_EMIF_DATA_BASE		0xc0000000
#define DAVINCI_INTC_BASE			0xfffee000
#define DAVINCI_BOOTCFG_BASE			0x01c14000
#define DAVINCI_LCD_CNTL_BASE			0x01e13000
#define DAVINCI_L3CBARAM_BASE			0x80000000
#define JTAG_ID_REG                            (DAVINCI_BOOTCFG_BASE + 0x18)
#define CHIP_REV_ID_REG				(DAVINCI_BOOTCFG_BASE + 0x24)
#define HOST1CFG				(DAVINCI_BOOTCFG_BASE + 0x44)
#define PSC0_MDCTL				(DAVINCI_PSC0_BASE + 0xa00)

#define GPIO_BANK0_REG_DIR_ADDR			(DAVINCI_GPIO_BASE + 0x10)
#define GPIO_BANK0_REG_OPDATA_ADDR		(DAVINCI_GPIO_BASE + 0x14)
#define GPIO_BANK0_REG_SET_ADDR			(DAVINCI_GPIO_BASE + 0x18)
#define GPIO_BANK0_REG_CLR_ADDR			(DAVINCI_GPIO_BASE + 0x1c)
#define GPIO_BANK2_REG_DIR_ADDR			(DAVINCI_GPIO_BASE + 0x38)
#define GPIO_BANK2_REG_OPDATA_ADDR		(DAVINCI_GPIO_BASE + 0x3c)
#define GPIO_BANK2_REG_SET_ADDR			(DAVINCI_GPIO_BASE + 0x40)
#define GPIO_BANK2_REG_CLR_ADDR			(DAVINCI_GPIO_BASE + 0x44)
#define GPIO_BANK6_REG_DIR_ADDR			(DAVINCI_GPIO_BASE + 0x88)
#define GPIO_BANK6_REG_OPDATA_ADDR		(DAVINCI_GPIO_BASE + 0x8c)
#define GPIO_BANK6_REG_SET_ADDR			(DAVINCI_GPIO_BASE + 0x90)
#define GPIO_BANK6_REG_CLR_ADDR			(DAVINCI_GPIO_BASE + 0x94)
#endif /* CONFIG_SOC_DA8XX */

/* Power and Sleep Controller (PSC) Domains */
#define DAVINCI_GPSC_ARMDOMAIN		0
#define DAVINCI_GPSC_DSPDOMAIN		1

#ifndef CONFIG_SOC_DA8XX

#define DAVINCI_LPSC_VPSSMSTR		0
#define DAVINCI_LPSC_VPSSSLV		1
#define DAVINCI_LPSC_TPCC		2
#define DAVINCI_LPSC_TPTC0		3
#define DAVINCI_LPSC_TPTC1		4
#define DAVINCI_LPSC_EMAC		5
#define DAVINCI_LPSC_EMAC_WRAPPER	6
#define DAVINCI_LPSC_MDIO		7
#define DAVINCI_LPSC_IEEE1394		8
#define DAVINCI_LPSC_USB		9
#define DAVINCI_LPSC_ATA		10
#define DAVINCI_LPSC_VLYNQ		11
#define DAVINCI_LPSC_UHPI		12
#define DAVINCI_LPSC_DDR_EMIF		13
#define DAVINCI_LPSC_AEMIF		14
#define DAVINCI_LPSC_MMC_SD		15
#define DAVINCI_LPSC_MEMSTICK		16
#define DAVINCI_LPSC_McBSP		17
#define DAVINCI_LPSC_I2C		18
#define DAVINCI_LPSC_UART0		19
#define DAVINCI_LPSC_UART1		20
#define DAVINCI_LPSC_UART2		21
#define DAVINCI_LPSC_SPI		22
#define DAVINCI_LPSC_PWM0		23
#define DAVINCI_LPSC_PWM1		24
#define DAVINCI_LPSC_PWM2		25
#define DAVINCI_LPSC_GPIO		26
#define DAVINCI_LPSC_TIMER0		27
#define DAVINCI_LPSC_TIMER1		28
#define DAVINCI_LPSC_TIMER2		29
#define DAVINCI_LPSC_SYSTEM_SUBSYS	30
#define DAVINCI_LPSC_ARM		31
#define DAVINCI_LPSC_SCR2		32
#define DAVINCI_LPSC_SCR3		33
#define DAVINCI_LPSC_SCR4		34
#define DAVINCI_LPSC_CROSSBAR		35
#define DAVINCI_LPSC_CFG27		36
#define DAVINCI_LPSC_CFG3		37
#define DAVINCI_LPSC_CFG5		38
#define DAVINCI_LPSC_GEM		39
#define DAVINCI_LPSC_IMCOP		40
#define DAVINCI_LPSC_VPSSMASTER		47
#define DAVINCI_LPSC_MJCP		50
#define DAVINCI_LPSC_HDVICP		51

#define DAVINCI_DM646X_LPSC_EMAC	14
#define DAVINCI_DM646X_LPSC_UART0	26
#define DAVINCI_DM646X_LPSC_I2C		31
#define DAVINCI_DM646X_LPSC_TIMER0	34

#else /* CONFIG_SOC_DA8XX */

#define DAVINCI_LPSC_TPCC		0
#define DAVINCI_LPSC_TPTC0		1
#define DAVINCI_LPSC_TPTC1		2
#define DAVINCI_LPSC_AEMIF		3
#define DAVINCI_LPSC_SPI0		4
#define DAVINCI_LPSC_MMC_SD		5
#define DAVINCI_LPSC_AINTC		6
#define DAVINCI_LPSC_ARM_RAM_ROM	7
#define DAVINCI_LPSC_SECCTL_KEYMGR	8
#define DAVINCI_LPSC_UART0		9
#define DAVINCI_LPSC_SCR0		10
#define DAVINCI_LPSC_SCR1		11
#define DAVINCI_LPSC_SCR2		12
#define DAVINCI_LPSC_DMAX		13
#define DAVINCI_LPSC_ARM		14
#define DAVINCI_LPSC_GEM		15

/* for LPSCs in PSC1, offset from 32 for differentiation */
#define DAVINCI_LPSC_PSC1_BASE		32
#define DAVINCI_LPSC_USB20		(DAVINCI_LPSC_PSC1_BASE + 1)
#define DAVINCI_LPSC_USB11		(DAVINCI_LPSC_PSC1_BASE + 2)
#define DAVINCI_LPSC_GPIO		(DAVINCI_LPSC_PSC1_BASE + 3)
#define DAVINCI_LPSC_UHPI		(DAVINCI_LPSC_PSC1_BASE + 4)
#define DAVINCI_LPSC_EMAC		(DAVINCI_LPSC_PSC1_BASE + 5)
#define DAVINCI_LPSC_DDR_EMIF		(DAVINCI_LPSC_PSC1_BASE + 6)
#define DAVINCI_LPSC_McASP0		(DAVINCI_LPSC_PSC1_BASE + 7)
#define DAVINCI_LPSC_SPI1		(DAVINCI_LPSC_PSC1_BASE + 10)
#define DAVINCI_LPSC_I2C1		(DAVINCI_LPSC_PSC1_BASE + 11)
#define DAVINCI_LPSC_UART1		(DAVINCI_LPSC_PSC1_BASE + 12)
#define DAVINCI_LPSC_UART2		(DAVINCI_LPSC_PSC1_BASE + 13)
#define DAVINCI_LPSC_LCDC		(DAVINCI_LPSC_PSC1_BASE + 16)
#define DAVINCI_LPSC_ePWM		(DAVINCI_LPSC_PSC1_BASE + 17)
#define DAVINCI_LPSC_MMCSD1		(DAVINCI_LPSC_PSC1_BASE + 18)
#define DAVINCI_LPSC_eCAP		(DAVINCI_LPSC_PSC1_BASE + 20)
#define DAVINCI_LPSC_L3_CBA_RAM		(DAVINCI_LPSC_PSC1_BASE + 31)

/* DA830-specific peripherals */
#define DAVINCI_LPSC_McASP1		(DAVINCI_LPSC_PSC1_BASE + 8)
#define DAVINCI_LPSC_McASP2		(DAVINCI_LPSC_PSC1_BASE + 9)
#define DAVINCI_LPSC_eQEP		(DAVINCI_LPSC_PSC1_BASE + 21)
#define DAVINCI_LPSC_SCR8		(DAVINCI_LPSC_PSC1_BASE + 24)
#define DAVINCI_LPSC_SCR7		(DAVINCI_LPSC_PSC1_BASE + 25)
#define DAVINCI_LPSC_SCR12		(DAVINCI_LPSC_PSC1_BASE + 26)

/* DA850-specific peripherals */
#define DAVINCI_LPSC_TPCC1		(DAVINCI_LPSC_PSC1_BASE + 0)
#define DAVINCI_LPSC_SATA		(DAVINCI_LPSC_PSC1_BASE + 8)
#define DAVINCI_LPSC_VPIF		(DAVINCI_LPSC_PSC1_BASE + 9)
#define DAVINCI_LPSC_McBSP0		(DAVINCI_LPSC_PSC1_BASE + 14)
#define DAVINCI_LPSC_McBSP1		(DAVINCI_LPSC_PSC1_BASE + 15)
#define DAVINCI_LPSC_MMC_SD1		(DAVINCI_LPSC_PSC1_BASE + 18)
#define DAVINCI_LPSC_uPP		(DAVINCI_LPSC_PSC1_BASE + 19)
#define DAVINCI_LPSC_TPTC2		(DAVINCI_LPSC_PSC1_BASE + 21)
#define DAVINCI_LPSC_SCR_F0		(DAVINCI_LPSC_PSC1_BASE + 24)
#define DAVINCI_LPSC_SCR_F1		(DAVINCI_LPSC_PSC1_BASE + 25)
#define DAVINCI_LPSC_SCR_F2		(DAVINCI_LPSC_PSC1_BASE + 26)
#define DAVINCI_LPSC_SCR_F6		(DAVINCI_LPSC_PSC1_BASE + 27)
#define DAVINCI_LPSC_SCR_F7		(DAVINCI_LPSC_PSC1_BASE + 28)
#define DAVINCI_LPSC_SCR_F8		(DAVINCI_LPSC_PSC1_BASE + 29)
#define DAVINCI_LPSC_BR_F7		(DAVINCI_LPSC_PSC1_BASE + 30)

#endif /* CONFIG_SOC_DA8XX */

#ifndef __ASSEMBLY__
void lpsc_on(unsigned int id);
void lpsc_syncreset(unsigned int id);
void lpsc_disable(unsigned int id);
void dsp_on(void);

void davinci_enable_uart0(void);
void davinci_enable_emac(void);
void davinci_enable_i2c(void);
void davinci_errata_workarounds(void);

#ifndef CONFIG_SOC_DA8XX

/* Some PSC defines */
#define PSC_CHP_SHRTSW			(0x01c40038)
#define PSC_GBLCTL			(0x01c41010)
#define PSC_EPCPR			(0x01c41070)
#define PSC_EPCCR			(0x01c41078)
#define PSC_PTCMD			(0x01c41120)
#define PSC_PTSTAT			(0x01c41128)
#define PSC_PDSTAT			(0x01c41200)
#define PSC_PDSTAT1			(0x01c41204)
#define PSC_PDCTL			(0x01c41300)
#define PSC_PDCTL1			(0x01c41304)

#define PSC_MDCTL_BASE			(0x01c41a00)
#define PSC_MDSTAT_BASE			(0x01c41800)

#define VDD3P3V_PWDN			(0x01c40048)
#define UART0_PWREMU_MGMT		(0x01c20030)

#define PSC_SILVER_BULLET		(0x01c41a20)

#else /* CONFIG_SOC_DA8XX */

#define	PSC_ENABLE		0x3
#define	PSC_DISABLE		0x2
#define	PSC_SYNCRESET		0x1
#define	PSC_SWRSTDISABLE	0x0

#define PSC_PSC0_MODULE_ID_CNT		16
#define PSC_PSC1_MODULE_ID_CNT		32

#define UART0_PWREMU_MGMT		(0x01c42030)

struct davinci_psc_regs {
	dv_reg	revid;
	dv_reg	rsvd0[71];
	dv_reg	ptcmd;
	dv_reg	rsvd1;
	dv_reg	ptstat;
	dv_reg	rsvd2[437];
	union {
		struct {
			dv_reg	mdstat[PSC_PSC0_MODULE_ID_CNT];
			dv_reg	rsvd3[112];
			dv_reg	mdctl[PSC_PSC0_MODULE_ID_CNT];
		} psc0;
		struct {
			dv_reg	mdstat[PSC_PSC1_MODULE_ID_CNT];
			dv_reg	rsvd3[96];
			dv_reg	mdctl[PSC_PSC1_MODULE_ID_CNT];
		} psc1;
	};
};

#define davinci_psc0_regs ((struct davinci_psc_regs *)DAVINCI_PSC0_BASE)
#define davinci_psc1_regs ((struct davinci_psc_regs *)DAVINCI_PSC1_BASE)

#endif /* CONFIG_SOC_DA8XX */

#define PSC_MDSTAT_STATE		0x3f
#define PSC_MDCTL_NEXT			0x07

#ifndef CONFIG_SOC_DA8XX

/* Miscellania... */
#define VBPR				(0x20000020)

/* NOTE:  system control modules are *highly* chip-specific, both
 * as to register content (e.g. for muxing) and which registers exist.
 */
#define PINMUX0				0x01c40000
#define PINMUX1				0x01c40004
#define PINMUX2				0x01c40008
#define PINMUX3				0x01c4000c
#define PINMUX4				0x01c40010

struct davinci_uart_ctrl_regs {
	dv_reg	revid1;
	dv_reg	res;
	dv_reg	pwremu_mgmt;
	dv_reg	mdr;
};

#define DAVINCI_UART_CTRL_BASE 0x28

/* UART PWREMU_MGMT definitions */
#define DAVINCI_UART_PWREMU_MGMT_FREE	(1 << 0)
#define DAVINCI_UART_PWREMU_MGMT_URRST	(1 << 13)
#define DAVINCI_UART_PWREMU_MGMT_UTRST	(1 << 14)

#else /* CONFIG_SOC_DA8XX */

struct davinci_pllc_regs {
	dv_reg	revid;
	dv_reg	rsvd1[56];
	dv_reg	rstype;
	dv_reg	rsvd2[6];
	dv_reg	pllctl;
	dv_reg	ocsel;
	dv_reg	rsvd3[2];
	dv_reg	pllm;
	dv_reg	prediv;
	dv_reg	plldiv1;
	dv_reg	plldiv2;
	dv_reg	plldiv3;
	dv_reg	oscdiv;
	dv_reg	postdiv;
	dv_reg	rsvd4[3];
	dv_reg	pllcmd;
	dv_reg	pllstat;
	dv_reg	alnctl;
	dv_reg	dchange;
	dv_reg	cken;
	dv_reg	ckstat;
	dv_reg	systat;
	dv_reg	rsvd5[3];
	dv_reg	plldiv4;
	dv_reg	plldiv5;
	dv_reg	plldiv6;
	dv_reg	plldiv7;
	dv_reg	rsvd6[32];
	dv_reg	emucnt0;
	dv_reg	emucnt1;
};

#define davinci_pllc0_regs ((struct davinci_pllc_regs *)DAVINCI_PLL_CNTRL0_BASE)
#define davinci_pllc1_regs ((struct davinci_pllc_regs *)DAVINCI_PLL_CNTRL1_BASE)
#define DAVINCI_PLLC_DIV_MASK	0x1f

/*
 * A clock ID is a 32-bit number where bit 16 represents the PLL controller
 * (clear is PLLC0, set is PLLC1) and the low 16 bits represent the divisor,
 * counting from 1. Clock IDs may be passed to clk_get().
 */

/* flags to select PLL controller */
#define DAVINCI_PLLC0_FLAG			(0)
#define DAVINCI_PLLC1_FLAG			(1 << 16)

enum davinci_clk_ids {
	/*
	 * Clock IDs for PLL outputs. Each may be switched on/off
	 * independently, and each may map to one or more peripherals.
	 */
	DAVINCI_PLL0_SYSCLK2			= DAVINCI_PLLC0_FLAG | 2,
	DAVINCI_PLL0_SYSCLK4			= DAVINCI_PLLC0_FLAG | 4,
	DAVINCI_PLL0_SYSCLK6			= DAVINCI_PLLC0_FLAG | 6,
	DAVINCI_PLL1_SYSCLK1			= DAVINCI_PLLC1_FLAG | 1,
	DAVINCI_PLL1_SYSCLK2			= DAVINCI_PLLC1_FLAG | 2,

	/* map peripherals to clock IDs */
	DAVINCI_ARM_CLKID			= DAVINCI_PLL0_SYSCLK6,
	DAVINCI_DDR_CLKID			= DAVINCI_PLL1_SYSCLK1,
	DAVINCI_MDIO_CLKID			= DAVINCI_PLL0_SYSCLK4,
	DAVINCI_MMC_CLKID			= DAVINCI_PLL0_SYSCLK2,
	DAVINCI_SPI0_CLKID			= DAVINCI_PLL0_SYSCLK2,
	DAVINCI_MMCSD_CLKID			= DAVINCI_PLL0_SYSCLK2,

	/* special clock ID - output of PLL multiplier */
	DAVINCI_PLLM_CLKID			= 0x0FF,

	/* special clock ID - output of PLL post divisor */
	DAVINCI_PLLC_CLKID			= 0x100,

	/* special clock ID - PLL bypass */
	DAVINCI_AUXCLK_CLKID			= 0x101,
};

#define DAVINCI_UART2_CLKID	(cpu_is_da830() ? DAVINCI_PLL0_SYSCLK2 \
						: get_async3_src())

#define DAVINCI_SPI1_CLKID	(cpu_is_da830() ? DAVINCI_PLL0_SYSCLK2 \
						: get_async3_src())

int clk_get(enum davinci_clk_ids id);

/* Boot config */
struct davinci_syscfg_regs {
	dv_reg	revid;
	dv_reg	rsvd[7];
	dv_reg	bootcfg;
	dv_reg	chiprevidr;
	dv_reg	rsvd2[4];
	dv_reg	kick0;
	dv_reg	kick1;
	dv_reg	rsvd1[52];
	dv_reg	mstpri[3];
	dv_reg  rsvd3;
	dv_reg	pinmux[20];
	dv_reg	suspsrc;
	dv_reg	chipsig;
	dv_reg	chipsig_clr;
	dv_reg	cfgchip0;
	dv_reg	cfgchip1;
	dv_reg	cfgchip2;
	dv_reg	cfgchip3;
	dv_reg	cfgchip4;
};

#define davinci_syscfg_regs \
	((struct davinci_syscfg_regs *)DAVINCI_BOOTCFG_BASE)

enum {
	DAVINCI_NAND8_BOOT	= 0b001110,
	DAVINCI_NAND16_BOOT	= 0b010000,
	DAVINCI_SD_OR_MMC_BOOT	= 0b011100,
	DAVINCI_MMC_ONLY_BOOT	= 0b111100,
	DAVINCI_SPI0_FLASH_BOOT	= 0b001010,
	DAVINCI_SPI1_FLASH_BOOT	= 0b001100,
};

#define pinmux(x)	(&davinci_syscfg_regs->pinmux[x])

/* Emulation suspend bits */
#define DAVINCI_SYSCFG_SUSPSRC_EMAC		(1 << 5)
#define DAVINCI_SYSCFG_SUSPSRC_I2C		(1 << 16)
#define DAVINCI_SYSCFG_SUSPSRC_SPI0		(1 << 21)
#define DAVINCI_SYSCFG_SUSPSRC_SPI1		(1 << 22)
#define DAVINCI_SYSCFG_SUSPSRC_UART0		(1 << 18)
#define DAVINCI_SYSCFG_SUSPSRC_UART1		(1 << 19)
#define DAVINCI_SYSCFG_SUSPSRC_UART2		(1 << 20)
#define DAVINCI_SYSCFG_SUSPSRC_TIMER0		(1 << 27)

struct davinci_syscfg1_regs {
	dv_reg	vtpio_ctl;
	dv_reg	ddr_slew;
	dv_reg	deepsleep;
	dv_reg	pupd_ena;
	dv_reg	pupd_sel;
	dv_reg	rxactive;
	dv_reg	pwrdwn;
};

#define davinci_syscfg1_regs \
	((struct davinci_syscfg1_regs *)DAVINCI_SYSCFG1_BASE)

#define DDR_SLEW_CMOSEN_BIT	4
#define DDR_SLEW_DDR_PDENA_BIT	5

#define VTP_POWERDWN		(1 << 6)
#define VTP_LOCK		(1 << 7)
#define VTP_CLKRZ		(1 << 13)
#define VTP_READY		(1 << 15)
#define VTP_IOPWRDWN		(1 << 14)

#define DV_SYSCFG_KICK0_UNLOCK	0x83e70b13
#define DV_SYSCFG_KICK1_UNLOCK	0x95a4f1e0

/* Interrupt controller */
struct davinci_aintc_regs {
	dv_reg	revid;
	dv_reg	cr;
	dv_reg	dummy0[2];
	dv_reg	ger;
	dv_reg	dummy1[219];
	dv_reg	ecr1;
	dv_reg	ecr2;
	dv_reg	ecr3;
	dv_reg	dummy2[1117];
	dv_reg	hier;
};

#define davinci_aintc_regs ((struct davinci_aintc_regs *)DAVINCI_INTC_BASE)

struct davinci_uart_ctrl_regs {
	dv_reg	revid1;
	dv_reg	revid2;
	dv_reg	pwremu_mgmt;
	dv_reg	mdr;
};

#define DAVINCI_UART_CTRL_BASE 0x28
#define DAVINCI_UART0_CTRL_ADDR (DAVINCI_UART0_BASE + DAVINCI_UART_CTRL_BASE)
#define DAVINCI_UART1_CTRL_ADDR (DAVINCI_UART1_BASE + DAVINCI_UART_CTRL_BASE)
#define DAVINCI_UART2_CTRL_ADDR (DAVINCI_UART2_BASE + DAVINCI_UART_CTRL_BASE)

#define davinci_uart0_ctrl_regs \
	((struct davinci_uart_ctrl_regs *)DAVINCI_UART0_CTRL_ADDR)
#define davinci_uart1_ctrl_regs \
	((struct davinci_uart_ctrl_regs *)DAVINCI_UART1_CTRL_ADDR)
#define davinci_uart2_ctrl_regs \
	((struct davinci_uart_ctrl_regs *)DAVINCI_UART2_CTRL_ADDR)

/* UART PWREMU_MGMT definitions */
#define DAVINCI_UART_PWREMU_MGMT_FREE	(1 << 0)
#define DAVINCI_UART_PWREMU_MGMT_URRST	(1 << 13)
#define DAVINCI_UART_PWREMU_MGMT_UTRST	(1 << 14)

static inline int cpu_is_da830(void)
{
	unsigned int jtag_id	= REG(JTAG_ID_REG);
	unsigned short part_no	= (jtag_id >> 12) & 0xffff;

	return ((part_no == 0xb7df) ? 1 : 0);
}
static inline int cpu_is_da850(void)
{
	unsigned int jtag_id    = REG(JTAG_ID_REG);
	unsigned short part_no  = (jtag_id >> 12) & 0xffff;

	return ((part_no == 0xb7d1) ? 1 : 0);
}

static inline enum davinci_clk_ids get_async3_src(void)
{
	return (REG(&davinci_syscfg_regs->cfgchip3) & 0x10) ?
			DAVINCI_PLL1_SYSCLK2 : DAVINCI_PLL0_SYSCLK2;
}

#endif /* CONFIG_SOC_DA8XX */

#if defined(CONFIG_SOC_DM365)
#include <asm/arch/aintc_defs.h>
#include <asm/arch/ddr2_defs.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pll_defs.h>
#include <asm/arch/psc_defs.h>
#include <asm/arch/syscfg_defs.h>
#include <asm/arch/timer_defs.h>

#define TMPBUF			0x00017ff8
#define TMPSTATUS		0x00017ff0
#define DV_TMPBUF_VAL		0x591b3ed7
#define FLAG_PORRST		0x00000001
#define FLAG_WDTRST		0x00000002
#define FLAG_FLGON		0x00000004
#define FLAG_FLGOFF		0x00000010

#endif
#endif /* !__ASSEMBLY__ */

#endif /* __ASM_ARCH_HARDWARE_H */
