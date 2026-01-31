// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright Contributors to the U-Boot project.

#define LOG_CATEGORY LOGC_ARCH

#include <dm.h>
#include <misc.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/hardware.h>

#define SGRF_BASE			0xff210000

#define FIREWALL_DDR_BASE		0xff5f0000
#define FW_DDR_MST1_REG			0x24
#define FW_DDR_MST2_REG			0x28
#define FW_DDR_MST3_REG			0x2c

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/soc/mmc@ff480000",
	[BROM_BOOTSOURCE_SD] = "/soc/mmc@ff480000",
};

void board_debug_uart_init(void)
{
}

int arch_cpu_init(void)
{
	u32 val;

	if (!IS_ENABLED(CONFIG_SPL_BUILD))
		return 0;

	/* Select non-secure OTPC */
	rk_clrreg(SGRF_BASE + 0x100, BIT(1));

	/* Set the sdmmc/emmc to access ddr memory */
	val = readl(FIREWALL_DDR_BASE + FW_DDR_MST1_REG);
	writel(val & 0xffff00ff, FIREWALL_DDR_BASE + FW_DDR_MST1_REG);

	/* Set the fspi to access ddr memory */
	val = readl(FIREWALL_DDR_BASE + FW_DDR_MST1_REG);
	writel(val & 0xff00ffff, FIREWALL_DDR_BASE + FW_DDR_MST1_REG);

	/* Set the mac0 to access ddr memory */
	val = readl(FIREWALL_DDR_BASE + FW_DDR_MST1_REG);
	writel(val & 0xf0ffffff, FIREWALL_DDR_BASE + FW_DDR_MST1_REG);

	/* Set the mac1 to access ddr memory */
	val = readl(FIREWALL_DDR_BASE + FW_DDR_MST2_REG);
	writel(val & 0xfffffff0, FIREWALL_DDR_BASE + FW_DDR_MST2_REG);

	/* Set the otg1 to access ddr memory */
	val = readl(FIREWALL_DDR_BASE + FW_DDR_MST3_REG);
	writel(val & 0xfff0ffff, FIREWALL_DDR_BASE + FW_DDR_MST3_REG);

	return 0;
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

	asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (CONFIG_COUNTER_FREQUENCY));
	writel(0xffffffff, HP_TIMER_BASE + HP_LOAD_COUNT0_REG);
	writel(0xffffffff, HP_TIMER_BASE + HP_LOAD_COUNT1_REG);
	writel((TIMER_EN << 16) | TIMER_EN, HP_TIMER_BASE + HP_CTRL_REG);
}

#define RK3506_OTP_CPU_CODE_OFFSET		0x02
#define RK3506_OTP_SPECIFICATION_OFFSET		0x08

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

	/* cpu-code: SoC model, e.g. 0x35 0x06 */
	ret = misc_read(dev, RK3506_OTP_CPU_CODE_OFFSET, cpu_code, 2);
	if (ret < 0) {
		log_debug("Could not read cpu-code, ret=%d\n", ret);
		return 0;
	}

	/* specification: SoC variant, e.g. 0xA for RK3506J */
	ret = misc_read(dev, RK3506_OTP_SPECIFICATION_OFFSET, &specification, 1);
	if (ret < 0) {
		log_debug("Could not read specification, ret=%d\n", ret);
		return 0;
	}
	specification &= 0x1f;

	/* for RK3506J i.e. '@' + 0xA = 'J' */
	suffix[0] = specification > 1 ? '@' + specification : '\0';
	suffix[1] = '\0';

	printf("SoC:   RK%02x%02x%s\n", cpu_code[0], cpu_code[1], suffix);

	return 0;
}
