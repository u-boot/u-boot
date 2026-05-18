// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Amarula Solutions(India)
 * Copyright (C) 2016 Engicam S.r.l.
 *
 * Authors:
 * Manoj Sai <abbaraju.manojsai@amarulasolutions.com>
 * Jagan Teki <jagan@amarulasolutions.com>
 */

#include <hang.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/arch/ddr.h>
#include <power/pmic.h>
#include <power/pca9450.h>

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	return BOOT_DEVICE_BOOTROM;
}

void spl_dram_init(void)
{
	ddr_init(&dram_timing);
}

#if CONFIG_IS_ENABLED(DM_PMIC_PCA9450)
int power_init_board(void)
{
	struct udevice *dev;
	int ret;

	ret = pmic_get("pmic@25", &dev);
	if (ret == -ENODEV) {
		puts("No pmic@25\n");
		return 0;
	}
	if (ret < 0)
		return ret;

	/* BUCKxOUT_DVS0/1 control BUCK123 output */
	pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

#ifdef CONFIG_IMX8M_LPDDR4
	/*
	 * increase VDD_SOC to typical value 0.95V before first
	 * DRAM access, set DVS1 to 0.85v for suspend.
	 * Enable DVS control through PMIC_STBY_REQ and
	 * set B1_ENMODE=1 (ON by PMIC_ON_REQ=H)
	 */
#ifdef CONFIG_IMX8M_VDD_SOC_850MV
	/* set DVS0 to 0.85v for special case*/
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x14);
#else
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x1C);
#endif
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x14);
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	/* Kernel uses OD/OD freq for SOC */
	/* To avoid timing risk from SOC to ARM,increase VDD_ARM to OD voltage 0.95v */
	pmic_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 0x1C);
#elif defined(CONFIG_IMX8M_DDR4)
	/* DDR4 runs at 3200MTS, uses default ND 0.85v for VDD_SOC and VDD_ARM */
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	/* Set NVCC_DRAM to 1.2v for DDR4 */
	pmic_reg_write(dev, PCA9450_BUCK6OUT, 0x18);
#endif

	return 0;
}
#endif

void spl_board_init(void)
{
	/* Set GIC clock to 500Mhz for OD VDD_SOC. Kernel driver does not allow to change it.
	 * Should set the clock after PMIC setting done.
	 * Default is 400Mhz (system_pll1_800m with div = 2) set by ROM for ND VDD_SOC
	 */
	clock_enable(CCGR_GIC, 0);
	clock_set_target_val(GIC_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(5));
	clock_enable(CCGR_GIC, 1);

	puts("Normal Boot\n");
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

void board_init_f(ulong dummy)
{
	int ret;

	arch_cpu_init();

	init_uart_clk(1);

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	preloader_console_init();

	enable_tzc380();

	power_init_board();

	/* DDR initialization */
	spl_dram_init();
}
