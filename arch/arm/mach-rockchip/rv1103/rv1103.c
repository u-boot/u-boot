// SPDX-License-Identifier:     GPL-2.0+
// Copyright (c) 2024 Rockchip Electronics Co., Ltd

#include <dm.h>
#include <spl.h>
#include <asm/io.h>
#include <image.h>

DECLARE_GLOBAL_DATA_PTR;

#define PERI_CRU_BASE			0x20000000
#define PERICRU_PERISOFTRST_CON10	0x0a28

#define PMU0_CRU_BASE			0x20070000
#define PMUCRU_PMUSOFTRST_CON02		0x0a08

#define GRF_SYS_BASE			0x20150000
#define GRF_SYS_HPMCU_CACHE_MISC	0x0214

#define GPIO0_IOC_BASE			0x201B0000
#define GPIO0A_IOMUX_SEL_H		0x04
#define GPIO0_BASE			0x20520000
#define GPIO_SWPORT_DR_L		0x00
#define GPIO_SWPORT_DDR_L		0x08

#define GPIO1_IOC_BASE			0x20170000
#define GPIO1A_IOMUX_SEL_0		0x20
#define GPIO1A_IOMUX_SEL_1_0		0x24
#define GPIO1A_IOMUX_SEL_1_1		0x10024
#define GPIO1B_IOMUX_SEL_0		0x10028
#define GPIO1B_IOMUX_SEL_1		0x1002c
#define GPIO1_IOC_GPIO1A_PULL_0		0x210
#define GPIO1_IOC_GPIO1A_PULL_1		0x10210
#define GPIO1_IOC_GPIO1B_PULL		0x10214
#define GPIO1_IOC_JTAG_M2_CON		0x10810

#define GPIO2_IOC_BASE			0x20840000
#define GPIO2A_IOMUX_SEL_1_1		0x44

#define SGRF_SYS_BASE			0x20250000
#define SGRF_SYS_SOC_CON2		0x0008
#define SGRF_SYS_SOC_CON3		0x000c
#define SGRF_SYS_OTP_CON		0x0018
#define FIREWALL_CON0			0x0020
#define FIREWALL_CON1			0x0024
#define FIREWALL_CON2			0x0028
#define FIREWALL_CON3			0x002c
#define FIREWALL_CON4			0x0030
#define FIREWALL_CON5			0x0034
#define FIREWALL_CON7			0x003c
#define SGRF_SYS_HPMCU_BOOT_DDR		0x0080

#define SGRF_PMU_BASE			0x20260000
#define SGRF_PMU_SOC_CON0		0x0000
#define SGRF_PMU_PMUMCU_BOOT_ADDR	0x0020

#define SYS_GRF_BASE			0x20150000
#define GRF_SYS_PERI_CON2		0x08
#define GRF_SYS_USBPHY_CON0		0x50

#define TOP_CRU_BASE			0x20060000
#define TOPCRU_CRU_GLB_RST_CON		0xc10

#define USBPHY_APB_BASE			0x20e10000
#define USBPHY_FSLS_DIFF_RECEIVER	0x0100

#define RV1103_WDT_BASE			0x208d0000
#define RV1103_WDT_CR			0x00

void board_debug_uart_init(void)
{
	/* No need to change uart */
}

#ifdef CONFIG_SPL_BUILD
void rockchip_stimer_init(void)
{
	/* If Timer already enabled, don't re-init it */
	u32 reg = readl(CONFIG_ROCKCHIP_STIMER_BASE + 0x4);

	if (reg & 0x1)
		return;
	writel(0x00010000, CONFIG_ROCKCHIP_STIMER_BASE + 0x4);

	asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r"(CONFIG_COUNTER_FREQUENCY));
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + 0x14);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + 0x18);
	writel(0x00010001, CONFIG_ROCKCHIP_STIMER_BASE + 0x4);
}
#endif

#ifndef CONFIG_TPL_BUILD
int arch_cpu_init(void)
{
	/* Stop any watchdog left running by BootROM/Boot1. */
	writel(0, RV1103_WDT_BASE + RV1103_WDT_CR);

#if defined(CONFIG_SPL_BUILD)
	/* Set all devices to Non-secure */
	writel(0xffff0000, SGRF_SYS_BASE + FIREWALL_CON0);
	writel(0xffff0000, SGRF_SYS_BASE + FIREWALL_CON1);
	writel(0xffff0000, SGRF_SYS_BASE + FIREWALL_CON2);
	writel(0xffff0000, SGRF_SYS_BASE + FIREWALL_CON3);
	writel(0xffff0000, SGRF_SYS_BASE + FIREWALL_CON4);
	writel(0xffff0000, SGRF_SYS_BASE + FIREWALL_CON5);
	writel(0x01f00000, SGRF_SYS_BASE + FIREWALL_CON7);
	/* Set OTP to none secure mode */
	writel(0x00020000, SGRF_SYS_BASE + SGRF_SYS_OTP_CON);

	/* no-secure WDT reset output will reset SoC system. */
	writel(0x00010001, SYS_GRF_BASE + GRF_SYS_PERI_CON2);
	/* secure WDT reset output will reset SoC system. */
	writel(0x00010001, SGRF_SYS_BASE + SGRF_SYS_SOC_CON2);
	/*
	 * enable tsadc trigger global reset and select first reset.
	 * enable global reset and wdt trigger pmu reset.
	 * select first reset trigger pmu reset.
	 */
	writel(0x0000ffdf, TOP_CRU_BASE + TOPCRU_CRU_GLB_RST_CON);

	/*
	 * Set the USB2 PHY in suspend mode and turn off the
	 * USB2 PHY FS/LS differential receiver to save power:
	 * VCC1V8_USB : reduce 3.8 mA
	 * VDD_0V9 : reduce 4.4 mA
	 */
	writel(0x01ff01d1, SYS_GRF_BASE + GRF_SYS_USBPHY_CON0);
	writel(0x00000000, USBPHY_APB_BASE + USBPHY_FSLS_DIFF_RECEIVER);
#endif

	return 0;
}
#endif
