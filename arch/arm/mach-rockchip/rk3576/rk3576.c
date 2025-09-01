// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2024 Rockchip Electronics Co., Ltd
 */

#define LOG_CATEGORY LOGC_ARCH

#include <dm.h>
#include <misc.h>
#include <asm/armv8/mmu.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/hardware.h>

#define SYS_GRF_BASE		0x2600A000
#define SYS_GRF_SOC_CON2	0x0008
#define SYS_GRF_SOC_CON7	0x001c
#define SYS_GRF_SOC_CON11	0x002c
#define SYS_GRF_SOC_CON12	0x0030

#define GPIO0_IOC_BASE		0x26040000
#define GPIO0B_PULL_L		0x0024
#define GPIO0B_IE_L		0x002C

#define SYS_SGRF_BASE		0x26004000
#define SYS_SGRF_SOC_CON14	0x0058
#define SYS_SGRF_SOC_CON15	0x005C
#define SYS_SGRF_SOC_CON20	0x0070

#define FW_SYS_SGRF_BASE	0x26005000
#define SGRF_DOMAIN_CON1	0x4
#define SGRF_DOMAIN_CON2	0x8
#define SGRF_DOMAIN_CON3	0xc
#define SGRF_DOMAIN_CON4	0x10
#define SGRF_DOMAIN_CON5	0x14

#define USB_GRF_BASE		0x2601E000
#define USB3OTG0_CON1		0x0030

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/soc/mmc@2a330000",
	[BROM_BOOTSOURCE_SD] = "/soc/mmc@2a310000",
};

static struct mm_region rk3576_mem_map[] = {
	{
		/* I/O area */
		.virt = 0x20000000UL,
		.phys = 0x20000000UL,
		.size = 0xb080000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* PMU_SRAM, CBUF, SYSTEM_SRAM */
		.virt = 0x3fe70000UL,
		.phys = 0x3fe70000UL,
		.size = 0x190000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* MSCH_DDR_PORT */
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0x400000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* PCIe 0+1 */
		.virt = 0x900000000UL,
		.phys = 0x900000000UL,
		.size = 0x100800000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = rk3576_mem_map;

void board_debug_uart_init(void)
{
}

#define HP_TIMER_BASE			CONFIG_ROCKCHIP_STIMER_BASE
#define HP_CTRL_REG			0x04
#define TIMER_EN			BIT(0)
#define HP_LOAD_COUNT0_REG		0x14
#define HP_LOAD_COUNT1_REG		0x18

void rockchip_stimer_init(void)
{
	u32 reg;

	if (!IS_ENABLED(CONFIG_XPL_BUILD))
		return;

	reg = readl(HP_TIMER_BASE + HP_CTRL_REG);
	if (reg & TIMER_EN)
		return;

	asm volatile("msr cntfrq_el0, %0" : : "r" (CONFIG_COUNTER_FREQUENCY));
	writel(0xffffffff, HP_TIMER_BASE + HP_LOAD_COUNT0_REG);
	writel(0xffffffff, HP_TIMER_BASE + HP_LOAD_COUNT1_REG);
	writel((TIMER_EN << 16) | TIMER_EN, HP_TIMER_BASE + HP_CTRL_REG);
}

int arch_cpu_init(void)
{
	u32 val;

	if (!IS_ENABLED(CONFIG_SPL_BUILD))
		return 0;

	/* Set the emmc to access ddr memory */
	val = readl(FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON2);
	writel(val | 0x7, FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON2);

	/* Set the sdmmc0 to access ddr memory */
	val = readl(FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON5);
	writel(val | 0x700, FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON5);

	/* Set the UFS to access ddr memory */
	val = readl(FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON3);
	writel(val | 0x70000, FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON3);

	/* Set the fspi0 and fspi1 to access ddr memory */
	val = readl(FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON4);
	writel(val | 0x7700, FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON4);

	/* Set the decom to access ddr memory */
	val = readl(FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON1);
	writel(val | 0x700, FW_SYS_SGRF_BASE + SGRF_DOMAIN_CON1);

	/*
	 * Set the GPIO0B0~B3 pull up and input enable.
	 * Keep consistent with other IO.
	 */
	writel(0x00ff00ff, GPIO0_IOC_BASE + GPIO0B_PULL_L);
	writel(0x000f000f, GPIO0_IOC_BASE + GPIO0B_IE_L);

	/*
	 * Set SYS_GRF_SOC_CON2[12](input of pwm2_ch0) as 0,
	 * keep consistent with other pwm.
	 */
	writel(0x10000000, SYS_GRF_BASE + SYS_GRF_SOC_CON2);

	/* Enable noc slave response timeout */
	writel(0x80008000, SYS_GRF_BASE + SYS_GRF_SOC_CON11);
	writel(0xffffffe0, SYS_GRF_BASE + SYS_GRF_SOC_CON12);

	/*
	 * Enable cci channels for below module AXI R/W
	 * Module: GMAC0/1, MMU0/1(PCIe, SATA, USB3)
	 */
	writel(0xffffff00, SYS_SGRF_BASE + SYS_SGRF_SOC_CON20);

	/* Disable USB3OTG0 U3 port, later enabled by USBDP PHY driver */
	writel(0xffff0188, USB_GRF_BASE + USB3OTG0_CON1);

	return 0;
}

#define RK3576_OTP_CPU_CODE_OFFSET		0x02
#define RK3576_OTP_SPECIFICATION_OFFSET		0x08

int checkboard(void)
{
	u8 cpu_code[2], specification;
	struct udevice *dev;
	char suffix[2];
	int ret;

	if (!IS_ENABLED(CONFIG_ROCKCHIP_OTP) || !CONFIG_IS_ENABLED(MISC))
		return 0;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(rockchip_otp), &dev);
	if (ret) {
		log_debug("Could not find otp device, ret=%d\n", ret);
		return 0;
	}

	/* cpu-code: SoC model, e.g. 0x35 0x76 */
	ret = misc_read(dev, RK3576_OTP_CPU_CODE_OFFSET, cpu_code, 2);
	if (ret < 0) {
		log_debug("Could not read cpu-code, ret=%d\n", ret);
		return 0;
	}

	/* specification: SoC variant, e.g. 0xA for RK3576J */
	ret = misc_read(dev, RK3576_OTP_SPECIFICATION_OFFSET, &specification, 1);
	if (ret < 0) {
		log_debug("Could not read specification, ret=%d\n", ret);
		return 0;
	}
	specification &= 0x1f;

	/* for RK3576J i.e. '@' + 0xA = 'J' */
	suffix[0] = specification > 1 ? '@' + specification : '\0';
	suffix[1] = '\0';

	printf("SoC:   RK%02x%02x%s\n", cpu_code[0], cpu_code[1], suffix);

	return 0;
}
