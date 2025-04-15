// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Rockchip Electronics Co., Ltd
 */

#define LOG_CATEGORY LOGC_ARCH

#include <dm.h>
#include <misc.h>
#include <asm/armv8/mmu.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/grf_rk3568.h>
#include <asm/arch-rockchip/hardware.h>
#include <dt-bindings/clock/rk3568-cru.h>

#define PMUGRF_BASE			0xfdc20000
#define GRF_BASE			0xfdc60000
#define GRF_GPIO1B_DS_2			0x218
#define GRF_GPIO1B_DS_3			0x21c
#define GRF_GPIO1C_DS_0			0x220
#define GRF_GPIO1C_DS_1			0x224
#define GRF_GPIO1C_DS_2			0x228
#define GRF_GPIO1C_DS_3			0x22c
#define SGRF_BASE			0xFDD18000
#define SGRF_SOC_CON4			0x10
#define EMMC_HPROT_SECURE_CTRL		0x03
#define SDMMC0_HPROT_SECURE_CTRL	0x01

#define PMU_BASE_ADDR		0xfdd90000
#define PMU_NOC_AUTO_CON0	(0x70)
#define PMU_NOC_AUTO_CON1	(0x74)
#define PMU_PWR_GATE_SFTCON	(0xa0)
#define PMU_PD_VO_DWN_ENA	BIT(7)
#define EDP_PHY_GRF_BASE	0xfdcb0000
#define EDP_PHY_GRF_CON0	(EDP_PHY_GRF_BASE + 0x00)
#define EDP_PHY_GRF_CON10	(EDP_PHY_GRF_BASE + 0x28)
#define CPU_GRF_BASE		0xfdc30000
#define GRF_CORE_PVTPLL_CON0	(0x10)

/* PMU_GRF_GPIO0D_IOMUX_L */
enum {
	GPIO0D1_SHIFT		= 4,
	GPIO0D1_MASK		= GENMASK(6, 4),
	GPIO0D1_GPIO		= 0,
	GPIO0D1_UART2_TXM0,

	GPIO0D0_SHIFT		= 0,
	GPIO0D0_MASK		= GENMASK(2, 0),
	GPIO0D0_GPIO		= 0,
	GPIO0D0_UART2_RXM0,
};

/* GRF_IOFUNC_SEL3 */
enum {
	UART2_IO_SEL_SHIFT	= 10,
	UART2_IO_SEL_MASK	= GENMASK(11, 10),
	UART2_IO_SEL_M0		= 0,
};

static struct mm_region rk3568_mem_map[] = {
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
	}, {
		.virt = 0x300000000,
		.phys = 0x300000000,
		.size = 0x0c0c00000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/mmc@fe310000",
	[BROM_BOOTSOURCE_SPINOR] = "/spi@fe300000/flash@0",
	[BROM_BOOTSOURCE_SD] = "/mmc@fe2b0000",
};

struct mm_region *mem_map = rk3568_mem_map;

void board_debug_uart_init(void)
{
	static struct rk3568_pmugrf * const pmugrf = (void *)PMUGRF_BASE;
	static struct rk3568_grf * const grf = (void *)GRF_BASE;

	/* UART2 M0 */
	rk_clrsetreg(&grf->iofunc_sel3, UART2_IO_SEL_MASK,
		     UART2_IO_SEL_M0 << UART2_IO_SEL_SHIFT);

	/* Switch iomux */
	rk_clrsetreg(&pmugrf->pmu_gpio0d_iomux_l,
		     GPIO0D1_MASK | GPIO0D0_MASK,
		     GPIO0D1_UART2_TXM0 << GPIO0D1_SHIFT |
		     GPIO0D0_UART2_RXM0 << GPIO0D0_SHIFT);
}

int arch_cpu_init(void)
{
#ifdef CONFIG_XPL_BUILD
	/*
	 * When perform idle operation, corresponding clock can
	 * be opened or gated automatically.
	 */
	writel(0xffffffff, PMU_BASE_ADDR + PMU_NOC_AUTO_CON0);
	writel(0x000f000f, PMU_BASE_ADDR + PMU_NOC_AUTO_CON1);

	/* Disable eDP phy by default */
	writel(0x00070007, EDP_PHY_GRF_CON10);
	writel(0x0ff10ff1, EDP_PHY_GRF_CON0);

	/* Set core pvtpll ring length */
	writel(0x00ff002b, CPU_GRF_BASE + GRF_CORE_PVTPLL_CON0);

	/* Set the emmc sdmmc0 to secure */
	rk_clrreg(SGRF_BASE + SGRF_SOC_CON4, (EMMC_HPROT_SECURE_CTRL << 11
		| SDMMC0_HPROT_SECURE_CTRL << 4));
	/* set the emmc driver strength to level 2 */
	writel(0x3f3f0707, GRF_BASE + GRF_GPIO1B_DS_2);
	writel(0x3f3f0707, GRF_BASE + GRF_GPIO1B_DS_3);
	writel(0x3f3f0707, GRF_BASE + GRF_GPIO1C_DS_0);
	writel(0x3f3f0707, GRF_BASE + GRF_GPIO1C_DS_1);
	writel(0x3f3f0707, GRF_BASE + GRF_GPIO1C_DS_2);
	writel(0x3f3f0707, GRF_BASE + GRF_GPIO1C_DS_3);

	/* Enable VO power domain for display */
	writel((PMU_PD_VO_DWN_ENA << 16),
	       PMU_BASE_ADDR + PMU_PWR_GATE_SFTCON);
#endif
	return 0;
}

#define RK3568_OTP_CPU_CODE_OFFSET		0x02
#define RK3568_OTP_SPECIFICATION_OFFSET		0x07
#define RK3568_OTP_PERFORMANCE_OFFSET		0x22

int checkboard(void)
{
	u8 cpu_code[2], specification, package, performance;
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

	/* cpu-code: SoC model, e.g. 0x35 0x66 or 0x35 0x68 */
	ret = misc_read(dev, RK3568_OTP_CPU_CODE_OFFSET, cpu_code, 2);
	if (ret < 0) {
		log_debug("Could not read cpu-code, ret=%d\n", ret);
		return 0;
	}

	/* specification: SoC variant, e.g. 0x2 for RK3568B2 and 0xA for RK3568J */
	ret = misc_read(dev, RK3568_OTP_SPECIFICATION_OFFSET, &specification, 1);
	if (ret < 0) {
		log_debug("Could not read specification, ret=%d\n", ret);
		return 0;
	}
	/* package: likely SoC variant revision, 0x2 for RK3568B2 */
	package = specification >> 5;
	specification &= 0x1f;

	/* performance: used to identify RK3566T SoC variant */
	ret = misc_read(dev, RK3568_OTP_PERFORMANCE_OFFSET, &performance, 1);
	if (ret < 0) {
		log_debug("Could not read performance, ret=%d\n", ret);
		return 0;
	}
	if (performance & 0x0f)
		specification = 0x14; /* T-variant */

	/* for RK3568J i.e. '@' + 0xA = 'J' */
	suffix[0] = specification > 1 ? '@' + specification : '\0';
	/* for RK3568B2 i.e. '0' + 0x2 = '2' */
	suffix[1] = package > 1 ? '0' + package : '\0';
	suffix[2] = '\0';

	printf("SoC:   RK%02x%02x%s\n", cpu_code[0], cpu_code[1], suffix);

	return 0;
}
