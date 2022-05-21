// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <spl.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/arch/ddr.h>

#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>

#include <power/pmic.h>
#include <power/pca9450.h>

#include "lpddr4_timing.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_FSEL1)
#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_ODE | PAD_CTL_PUE | PAD_CTL_PE)

static const iomux_v3_cfg_t uart_pads[] = {
	MX8MP_PAD_SAI2_RXFS__UART1_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX8MP_PAD_SAI2_RXC__UART1_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static const iomux_v3_cfg_t wdog_pads[] = {
	MX8MP_PAD_GPIO1_IO02__WDOG1_WDOG_B  | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

static void dh_imx8mp_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));

	set_wdog_reset(wdog);

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
}

static int dh_imx8mp_board_power_init(void)
{
	struct udevice *dev;
	int ret;

	ret = pmic_get("pmic@25", &dev);
	if (ret == -ENODEV) {
		puts("Failed to get PMIC\n");
		return 0;
	}
	if (ret != 0)
		return ret;

	/* BUCKxOUT_DVS0/1 control BUCK123 output. */
	pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

	/* Increase VDD_SOC to typical value 0.95V before first DRAM access. */
	if (IS_ENABLED(CONFIG_IMX8M_VDD_SOC_850MV))
		/* Set DVS0 to 0.85V for special case. */
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x14);
	else
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x1c);

	/* Set DVS1 to 0.85v for suspend. */
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x14);

	/*
	 * Enable DVS control through PMIC_STBY_REQ and
	 * set B1_ENMODE=1 (ON by PMIC_ON_REQ=H).
	 */
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	/* Kernel uses OD/OD frequency for SoC. */

	/* To avoid timing risk from SoC to ARM, increase VDD_ARM to OD voltage 0.95V */
	pmic_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 0x1c);

	/* Set WDOG_B_CFG to cold reset. */
	pmic_reg_write(dev, PCA9450_RESET_CTRL, 0xA1);

	/* Set LDO4 and CONFIG2 to enable the I2C level translator. */
	pmic_reg_write(dev, PCA9450_LDO4CTRL, 0x59);
	pmic_reg_write(dev, PCA9450_CONFIG2, 0x1);

	return 0;
}

static struct dram_timing_info *dram_timing_info[8] = {
	NULL,					/* 512 MiB */
	NULL,					/* 1024 MiB */
	NULL,					/* 1536 MiB */
	NULL,					/* 2048 MiB */
	NULL,					/* 3072 MiB */
	&dh_imx8mp_dhcom_dram_timing_32g_x32,	/* 4096 MiB */
	NULL,					/* 6144 MiB */
	NULL,					/* 8192 MiB */
};

static void spl_dram_init(void)
{
	const u16 size[] = { 512, 1024, 1536, 2048, 3072, 4096, 6144, 8192 };
	u8 memcfg = dh_get_memcfg();
	int i;

	printf("DDR:   %d MiB [0x%x]\n", size[memcfg], memcfg);

	if (!dram_timing_info[memcfg]) {
		printf("Unsupported DRAM strapping, trying lowest supported. MEMCFG=0x%x\n",
		       memcfg);
		for (i = 0; i < ARRAY_SIZE(dram_timing_info); i++)
			if (dram_timing_info[i])	/* Configuration found */
				break;
	}

	ddr_init(dram_timing_info[memcfg]);
}

void spl_board_init(void)
{
	/*
	 * Set GIC clock to 500 MHz for OD VDD_SOC. Kernel driver does not
	 * allow to change it. Should set the clock after PMIC setting done.
	 * Default is 400 MHz (system_pll1_800m with div = 2) set by ROM for
	 * ND VDD_SOC.
	 */
	clock_enable(CCGR_GIC, 0);
	clock_set_target_val(GIC_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(5));
	clock_enable(CCGR_GIC, 1);
}

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	return BOOT_DEVICE_BOOTROM;
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

	arch_cpu_init();

	init_uart_clk(0);

	dh_imx8mp_early_init_f();

	preloader_console_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device_by_name(UCLASS_CLK,
					"clock-controller@30380000",
					&dev);
	if (ret < 0) {
		printf("Failed to find clock node. Check device tree\n");
		hang();
	}

	enable_tzc380();

	dh_imx8mp_board_power_init();

	/* DDR initialization */
	spl_dram_init();

	board_init_r(NULL, 0);
}
