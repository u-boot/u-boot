// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018, 2021 NXP
 *
 */

#include <config.h>
#include <hang.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/ddr.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <asm/sections.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <spl.h>
#include "../common/pfuze.h"

extern struct dram_timing_info dram_timing_b0;

static void spl_dram_init(void)
{
	/* ddr init */
	if (soc_rev() >= CHIP_REV_2_1)
		ddr_init(&dram_timing);
	else
		ddr_init(&dram_timing_b0);
}

int power_init_board(void)
{
	struct udevice *dev;
	int reg;
	int ret;

	ret = pmic_get("pmic@8", &dev);
	if (ret == -ENODEV) {
		puts("No pmic@8\n");
		return 0;
	}
	if (ret < 0)
		return ret;

	reg = pmic_reg_read(dev, PFUZE100_DEVICEID);
	printf("PMIC:  PFUZE100 ID=0x%02x\n", reg);

	reg = pmic_reg_read(dev, PFUZE100_SW3AVOL);
	if ((reg & 0x3f) != 0x18) {
		reg &= ~0x3f;
		reg |= 0x18;
		pmic_reg_write(dev, PFUZE100_SW3AVOL, reg);
	}

	ret = pfuze_mode_init(dev, APS_PFM);
	if (ret < 0)
		return ret;

	/* set SW3A standby mode to off */
	reg = pmic_reg_read(dev, PFUZE100_SW3AMODE);
	reg &= ~0xf;
	reg |= APS_OFF;
	pmic_reg_write(dev, PFUZE100_SW3AMODE, reg);

	return 0;
}

void spl_board_init(void)
{
	if (IS_ENABLED(CONFIG_FSL_CAAM)) {
		struct udevice *dev;
		int ret;

		ret = uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(caam_jr), &dev);
		if (ret)
			printf("Failed to initialize caam_jr: %d\n", ret);
	}
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

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	arch_cpu_init();

	init_uart_clk(0);

	board_early_init_f();

	timer_init();

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

	init_clk_usdhc(0);
	init_clk_usdhc(1);

	board_init_r(NULL, 0);
}
