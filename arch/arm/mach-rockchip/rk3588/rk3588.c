// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021 Rockchip Electronics Co., Ltd
 * Copyright (c) 2022 Edgeble AI Technologies Pvt. Ltd.
 */

#define LOG_CATEGORY LOGC_ARCH

#include <dm.h>
#include <misc.h>
#include <spl.h>
#include <asm/armv8/mmu.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/grf_rk3588.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/ioc_rk3588.h>

#define USB_GRF_BASE			0xfd5ac000
#define USB3OTG0_CON1			0x001c
#define USB3OTG1_CON1			0x0034

#define FIREWALL_DDR_BASE		0xfe030000
#define FW_DDR_MST5_REG			0x54
#define FW_DDR_MST13_REG		0x74
#define FW_DDR_MST21_REG		0x94
#define FW_DDR_MST26_REG		0xa8
#define FW_DDR_MST27_REG		0xac
#define FIREWALL_SYSMEM_BASE		0xfe038000
#define FW_SYSM_MST5_REG		0x54
#define FW_SYSM_MST13_REG		0x74
#define FW_SYSM_MST21_REG		0x94
#define FW_SYSM_MST26_REG		0xa8
#define FW_SYSM_MST27_REG		0xac

#define BUS_IOC_GPIO2A_IOMUX_SEL_L	0x40
#define BUS_IOC_GPIO2B_IOMUX_SEL_L	0x48
#define BUS_IOC_GPIO2D_IOMUX_SEL_L	0x58
#define BUS_IOC_GPIO2D_IOMUX_SEL_H	0x5c
#define BUS_IOC_GPIO3A_IOMUX_SEL_L	0x60

#define SYS_GRF_FORCE_JTAG		BIT(14)

/**
 * Boot-device identifiers used by the BROM on RK3588 when device is booted
 * from SPI flash. IOMUX used for SPI flash affect the value used by the BROM
 * and not the type of SPI flash used.
 */
enum {
	BROM_BOOTSOURCE_FSPI_M0 = 3,
	BROM_BOOTSOURCE_FSPI_M1 = 4,
	BROM_BOOTSOURCE_FSPI_M2 = 6,
};

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/mmc@fe2e0000",
	[BROM_BOOTSOURCE_FSPI_M0] = "/spi@fe2b0000/flash@0",
	[BROM_BOOTSOURCE_FSPI_M1] = "/spi@fe2b0000/flash@0",
	[BROM_BOOTSOURCE_FSPI_M2] = "/spi@fe2b0000/flash@0",
	[BROM_BOOTSOURCE_SD] = "/mmc@fe2c0000",
};

static struct mm_region rk3588_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0xf0000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0xf0000000UL,
		.phys = 0xf0000000UL,
		.size = 0x10000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},  {
		.virt = 0x900000000,
		.phys = 0x900000000,
		.size = 0x150000000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},  {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = rk3588_mem_map;

/* GPIO0B_IOMUX_SEL_H */
enum {
	GPIO0B5_SHIFT		= 4,
	GPIO0B5_MASK		= GENMASK(7, 4),
	GPIO0B5_REFER		= 8,
	GPIO0B5_UART2_TX_M0	= 10,

	GPIO0B6_SHIFT		= 8,
	GPIO0B6_MASK		= GENMASK(11, 8),
	GPIO0B6_REFER		= 8,
	GPIO0B6_UART2_RX_M0	= 10,
};

void board_debug_uart_init(void)
{
	__maybe_unused static struct rk3588_bus_ioc * const bus_ioc = (void *)BUS_IOC_BASE;
	static struct rk3588_pmu2_ioc * const pmu2_ioc = (void *)PMU2_IOC_BASE;

	/* Refer to BUS_IOC */
	rk_clrsetreg(&pmu2_ioc->gpio0b_iomux_sel_h,
		     GPIO0B6_MASK | GPIO0B5_MASK,
		     GPIO0B6_REFER << GPIO0B6_SHIFT |
		     GPIO0B5_REFER << GPIO0B5_SHIFT);

	/* UART2_M0 Switch iomux */
	rk_clrsetreg(&bus_ioc->gpio0b_iomux_sel_h,
		     GPIO0B6_MASK | GPIO0B5_MASK,
		     GPIO0B6_UART2_RX_M0 << GPIO0B6_SHIFT |
		     GPIO0B5_UART2_TX_M0 << GPIO0B5_SHIFT);
}

#ifdef CONFIG_XPL_BUILD

#define HP_TIMER_BASE			CONFIG_ROCKCHIP_STIMER_BASE
#define HP_CTRL_REG			0x04
#define TIMER_EN			BIT(0)
#define HP_LOAD_COUNT0_REG		0x14
#define HP_LOAD_COUNT1_REG		0x18

void rockchip_stimer_init(void)
{
	/* If Timer already enabled, don't re-init it */
	u32 reg = readl(HP_TIMER_BASE + HP_CTRL_REG);

	if (reg & TIMER_EN)
		return;

	asm volatile("msr cntfrq_el0, %0" : : "r" (CONFIG_COUNTER_FREQUENCY));
	writel(0xffffffff, HP_TIMER_BASE + HP_LOAD_COUNT0_REG);
	writel(0xffffffff, HP_TIMER_BASE + HP_LOAD_COUNT1_REG);
	writel(TIMER_EN, HP_TIMER_BASE + HP_CTRL_REG);
}
#endif

#ifndef CONFIG_TPL_BUILD
int arch_cpu_init(void)
{
#ifdef CONFIG_XPL_BUILD
#ifdef CONFIG_ROCKCHIP_DISABLE_FORCE_JTAG
	static struct rk3588_sysgrf * const sys_grf = (void *)SYS_GRF_BASE;
#endif
	int secure_reg;

	/* Set the SDMMC eMMC crypto_ns FSPI access secure area */
	secure_reg = readl(FIREWALL_DDR_BASE + FW_DDR_MST5_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_DDR_BASE + FW_DDR_MST5_REG);
	secure_reg = readl(FIREWALL_DDR_BASE + FW_DDR_MST13_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_DDR_BASE + FW_DDR_MST13_REG);
	secure_reg = readl(FIREWALL_DDR_BASE + FW_DDR_MST21_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_DDR_BASE + FW_DDR_MST21_REG);
	secure_reg = readl(FIREWALL_DDR_BASE + FW_DDR_MST26_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_DDR_BASE + FW_DDR_MST26_REG);
	secure_reg = readl(FIREWALL_DDR_BASE + FW_DDR_MST27_REG);
	secure_reg &= 0xffff0000;
	writel(secure_reg, FIREWALL_DDR_BASE + FW_DDR_MST27_REG);

	secure_reg = readl(FIREWALL_SYSMEM_BASE + FW_SYSM_MST5_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_SYSMEM_BASE + FW_SYSM_MST5_REG);
	secure_reg = readl(FIREWALL_SYSMEM_BASE + FW_SYSM_MST13_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_SYSMEM_BASE + FW_SYSM_MST13_REG);
	secure_reg = readl(FIREWALL_SYSMEM_BASE + FW_SYSM_MST21_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_SYSMEM_BASE + FW_SYSM_MST21_REG);
	secure_reg = readl(FIREWALL_SYSMEM_BASE + FW_SYSM_MST26_REG);
	secure_reg &= 0xffff;
	writel(secure_reg, FIREWALL_SYSMEM_BASE + FW_SYSM_MST26_REG);
	secure_reg = readl(FIREWALL_SYSMEM_BASE + FW_SYSM_MST27_REG);
	secure_reg &= 0xffff0000;
	writel(secure_reg, FIREWALL_SYSMEM_BASE + FW_SYSM_MST27_REG);

#ifdef CONFIG_ROCKCHIP_DISABLE_FORCE_JTAG
	/* Disable JTAG exposed on SDMMC */
	rk_clrreg(&sys_grf->soc_con[6], SYS_GRF_FORCE_JTAG);
#endif

	/* Disable USB3OTG U3 ports, later enabled by USBDP PHY driver */
	writel(0xffff0188, USB_GRF_BASE + USB3OTG0_CON1);
	writel(0xffff0188, USB_GRF_BASE + USB3OTG1_CON1);
#endif

	return 0;
}
#endif

#define RK3588_OTP_CPU_CODE_OFFSET		0x02
#define RK3588_OTP_SPECIFICATION_OFFSET		0x06

int checkboard(void)
{
	u8 cpu_code[2], specification, package;
	struct udevice *dev;
	char suffix[3];
	int ret;

	if (!IS_ENABLED(CONFIG_ROCKCHIP_OTP) || !CONFIG_IS_ENABLED(MISC))
		return 0;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(rockchip_otp), &dev);
	if (ret) {
		log_debug("Could not find otp device, ret=%d\n", ret);
		return 0;
	}

	/* cpu-code: SoC model, e.g. 0x35 0x82 or 0x35 0x88 */
	ret = misc_read(dev, RK3588_OTP_CPU_CODE_OFFSET, cpu_code, 2);
	if (ret < 0) {
		log_debug("Could not read cpu-code, ret=%d\n", ret);
		return 0;
	}

	/* specification: SoC variant, e.g. 0xA for RK3588J and 0x13 for RK3588S */
	ret = misc_read(dev, RK3588_OTP_SPECIFICATION_OFFSET, &specification, 1);
	if (ret < 0) {
		log_debug("Could not read specification, ret=%d\n", ret);
		return 0;
	}
	/* package: likely SoC variant revision, 0x2 for RK3588S2 */
	package = specification >> 5;
	specification &= 0x1f;

	/* for RK3588J i.e. '@' + 0xA = 'J' */
	suffix[0] = specification > 1 ? '@' + specification : '\0';
	/* for RK3588S2 i.e. '0' + 0x2 = '2' */
	suffix[1] = package > 1 ? '0' + package : '\0';
	suffix[2] = '\0';

	printf("SoC:   RK%02x%02x%s\n", cpu_code[0], cpu_code[1], suffix);

	return 0;
}
