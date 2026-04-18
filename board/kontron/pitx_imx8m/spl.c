// SPDX-License-Identifier: GPL-2.0+

#include <config.h>
#include <errno.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/arch/ddr.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <asm/sections.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>

#include "pitx_misc.h"

extern struct dram_timing_info dram_timing_2gb;
extern struct dram_timing_info dram_timing_4gb;

static void spl_dram_init(void)
{
	struct dram_timing_info *dram_timing;
	int variant = 0, size;

	variant = get_pitx_board_variant();

	switch(variant) {
	case 2:
		dram_timing = &dram_timing_2gb;
		size = 2;
		break;
	case 3:
		dram_timing = &dram_timing_4gb;
		size = 4;
		break;
	default:
		printf("Unknown DDR type (%d)\n", variant);
		return;
	};

	/* ddr init */
	ddr_init(dram_timing);
}

const char *spl_board_loader_name(u32 boot_device)
{
	switch (boot_device) {
	case BOOT_DEVICE_MMC1:
		return "eMMC";
	case BOOT_DEVICE_MMC2:
		return "SD card";
	default:
		return NULL;
	}
}

static int pfuze_mode_init(struct udevice *dev, u32 mode)
{
	unsigned char offset, i, switch_num;
	u32 id;
	int ret;

	id = pmic_reg_read(dev, PFUZE100_DEVICEID);
	id = id & 0xf;

	if (id == 0) {
		switch_num = 6;
		offset = PFUZE100_SW1CMODE;
	} else if (id == 1) {
		switch_num = 4;
		offset = PFUZE100_SW2MODE;
	} else {
		printf("Not supported, id=%d\n", id);
		return -EINVAL;
	}

	ret = pmic_reg_write(dev, PFUZE100_SW1ABMODE, mode);
	if (ret < 0) {
		printf("Set SW1AB mode error!\n");
		return ret;
	}

	for (i = 0; i < switch_num - 1; i++) {
		ret = pmic_reg_write(dev, offset + i * SWITCH_SIZE, mode);
		if (ret < 0) {
			printf("Set switch 0x%x mode error!\n",
			       offset + i * SWITCH_SIZE);
			return ret;
		}
	}

	return ret;
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

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	arch_cpu_init();

	init_uart_clk(2);

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

	spl_dram_init();

	init_clk_usdhc(0);
	init_clk_usdhc(1);

	board_init_r(NULL, 0);
}
