/*
 * Keystone2: Common SoC definitions, structures etc.
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include <config.h>

#ifndef __ASSEMBLY__

#include <linux/sizes.h>
#include <asm/io.h>

#define	REG(addr)        (*(volatile unsigned int *)(addr))
#define REG_P(addr)      ((volatile unsigned int *)(addr))

typedef volatile unsigned int   dv_reg;
typedef volatile unsigned int   *dv_reg_p;

#endif

#define		BIT(x)	(1 << (x))

#define KS2_DDRPHY_PIR_OFFSET           0x04
#define KS2_DDRPHY_PGCR0_OFFSET         0x08
#define KS2_DDRPHY_PGCR1_OFFSET         0x0C
#define KS2_DDRPHY_PGSR0_OFFSET         0x10
#define KS2_DDRPHY_PGSR1_OFFSET         0x14
#define KS2_DDRPHY_PLLCR_OFFSET         0x18
#define KS2_DDRPHY_PTR0_OFFSET          0x1C
#define KS2_DDRPHY_PTR1_OFFSET          0x20
#define KS2_DDRPHY_PTR2_OFFSET          0x24
#define KS2_DDRPHY_PTR3_OFFSET          0x28
#define KS2_DDRPHY_PTR4_OFFSET          0x2C
#define KS2_DDRPHY_DCR_OFFSET           0x44

#define KS2_DDRPHY_DTPR0_OFFSET         0x48
#define KS2_DDRPHY_DTPR1_OFFSET         0x4C
#define KS2_DDRPHY_DTPR2_OFFSET         0x50

#define KS2_DDRPHY_MR0_OFFSET           0x54
#define KS2_DDRPHY_MR1_OFFSET           0x58
#define KS2_DDRPHY_MR2_OFFSET           0x5C
#define KS2_DDRPHY_DTCR_OFFSET          0x68
#define KS2_DDRPHY_PGCR2_OFFSET         0x8C

#define KS2_DDRPHY_ZQ0CR1_OFFSET        0x184
#define KS2_DDRPHY_ZQ1CR1_OFFSET        0x194
#define KS2_DDRPHY_ZQ2CR1_OFFSET        0x1A4
#define KS2_DDRPHY_ZQ3CR1_OFFSET        0x1B4

#define KS2_DDRPHY_DATX8_8_OFFSET       0x3C0

#define IODDRM_MASK                     0x00000180
#define ZCKSEL_MASK                     0x01800000
#define CL_MASK                         0x00000072
#define WR_MASK                         0x00000E00
#define BL_MASK                         0x00000003
#define RRMODE_MASK                     0x00040000
#define UDIMM_MASK                      0x20000000
#define BYTEMASK_MASK                   0x0003FC00
#define MPRDQ_MASK                      0x00000080
#define PDQ_MASK                        0x00000070
#define NOSRA_MASK                      0x08000000
#define ECC_MASK                        0x00000001

/* DDR3 definitions */
#define KS2_DDR3A_EMIF_CTRL_BASE	0x21010000
#define KS2_DDR3A_EMIF_DATA_BASE	0x80000000
#define KS2_DDR3A_DDRPHYC		0x02329000

#define KS2_DDR3_MIDR_OFFSET            0x00
#define KS2_DDR3_STATUS_OFFSET          0x04
#define KS2_DDR3_SDCFG_OFFSET           0x08
#define KS2_DDR3_SDRFC_OFFSET           0x10
#define KS2_DDR3_SDTIM1_OFFSET          0x18
#define KS2_DDR3_SDTIM2_OFFSET          0x1C
#define KS2_DDR3_SDTIM3_OFFSET          0x20
#define KS2_DDR3_SDTIM4_OFFSET          0x28
#define KS2_DDR3_PMCTL_OFFSET           0x38
#define KS2_DDR3_ZQCFG_OFFSET           0xC8

#define KS2_DDR3_PLLCTRL_PHY_RESET	0x80000000

#define KS2_UART0_BASE                	0x02530c00
#define KS2_UART1_BASE                	0x02531000

/* Boot Config */
#define KS2_DEVICE_STATE_CTRL_BASE	0x02620000
#define KS2_JTAG_ID_REG			(KS2_DEVICE_STATE_CTRL_BASE + 0x18)
#define KS2_DEVSTAT			(KS2_DEVICE_STATE_CTRL_BASE + 0x20)

/* PSC */
#define KS2_PSC_BASE			0x02350000
#define KS2_LPSC_GEM_0			15
#define KS2_LPSC_TETRIS			52
#define KS2_TETRIS_PWR_DOMAIN		31

/* Chip configuration unlock codes and registers */
#define KS2_KICK0			(KS2_DEVICE_STATE_CTRL_BASE + 0x38)
#define KS2_KICK1			(KS2_DEVICE_STATE_CTRL_BASE + 0x3c)
#define KS2_KICK0_MAGIC			0x83e70b13
#define KS2_KICK1_MAGIC			0x95a4f1e0

/* PLL control registers */
#define KS2_MAINPLLCTL0			(KS2_DEVICE_STATE_CTRL_BASE + 0x350)
#define KS2_MAINPLLCTL1			(KS2_DEVICE_STATE_CTRL_BASE + 0x354)
#define KS2_PASSPLLCTL0			(KS2_DEVICE_STATE_CTRL_BASE + 0x358)
#define KS2_PASSPLLCTL1			(KS2_DEVICE_STATE_CTRL_BASE + 0x35C)
#define KS2_DDR3APLLCTL0		(KS2_DEVICE_STATE_CTRL_BASE + 0x360)
#define KS2_DDR3APLLCTL1		(KS2_DEVICE_STATE_CTRL_BASE + 0x364)
#define KS2_ARMPLLCTL0			(KS2_DEVICE_STATE_CTRL_BASE + 0x370)
#define KS2_ARMPLLCTL1			(KS2_DEVICE_STATE_CTRL_BASE + 0x374)

#define KS2_PLL_CNTRL_BASE		0x02310000
#define KS2_CLOCK_BASE			KS2_PLL_CNTRL_BASE
#define KS2_RSTCTRL_RSTYPE		(KS2_PLL_CNTRL_BASE + 0xe4)
#define KS2_RSTCTRL			(KS2_PLL_CNTRL_BASE + 0xe8)
#define KS2_RSTCTRL_RSCFG		(KS2_PLL_CNTRL_BASE + 0xec)
#define KS2_RSTCTRL_KEY			0x5a69
#define KS2_RSTCTRL_MASK		0xffff0000
#define KS2_RSTCTRL_SWRST		0xfffe0000
#define KS2_RSTYPE_PLL_SOFT		BIT(13)

/* SPI */
#define KS2_SPI0_BASE			0x21000400
#define KS2_SPI1_BASE			0x21000600
#define KS2_SPI2_BASE			0x21000800
#define KS2_SPI_BASE			KS2_SPI0_BASE

/* AEMIF */
#define KS2_AEMIF_CNTRL_BASE       	0x21000a00
#define DAVINCI_ASYNC_EMIF_CNTRL_BASE   KS2_AEMIF_CNTRL_BASE

/* Flag from ks2_debug options to check if DSPs need to stay ON */
#define DBG_LEAVE_DSPS_ON		0x1

/* Device speed */
#define KS2_REV1_DEVSPEED		(KS2_DEVICE_STATE_CTRL_BASE + 0xc98)
#define KS2_EFUSE_BOOTROM		(KS2_DEVICE_STATE_CTRL_BASE + 0xc90)

/* Queue manager */
#define KS2_QM_MANAGER_BASE		0x02a02000
#define KS2_QM_DESC_SETUP_BASE		0x02a03000
#define KS2_QM_MANAGER_QUEUES_BASEi	0x02a80000
#define KS2_QM_MANAGER_Q_PROXY_BASE	0x02ac0000
#define KS2_QM_QUEUE_STATUS_BASE	0x02a40000

/* MSMC control */
#define KS2_MSMC_CTRL_BASE		0x0bc00000

#ifdef CONFIG_SOC_K2HK
#include <asm/arch/hardware-k2hk.h>
#endif

#ifdef CONFIG_SOC_K2E
#include <asm/arch/hardware-k2e.h>
#endif

#ifndef __ASSEMBLY__
static inline int cpu_is_k2hk(void)
{
	unsigned int jtag_id	= __raw_readl(KS2_JTAG_ID_REG);
	unsigned int part_no	= (jtag_id >> 12) & 0xffff;

	return (part_no == 0xb981) ? 1 : 0;
}

static inline int cpu_is_k2e(void)
{
	unsigned int jtag_id    = __raw_readl(KS2_JTAG_ID_REG);
	unsigned int part_no    = (jtag_id >> 12) & 0xffff;

	return (part_no == 0xb9a6) ? 1 : 0;
}

static inline int cpu_revision(void)
{
	unsigned int jtag_id	= __raw_readl(KS2_JTAG_ID_REG);
	unsigned int rev	= (jtag_id >> 28) & 0xf;

	return rev;
}

int cpu_to_bus(u32 *ptr, u32 length);
void sdelay(unsigned long);

#endif

#endif /* __ASM_ARCH_HARDWARE_H */
