// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Technexion Ltd.
 *
 * Author: Richard Hu <richard.hu@technexion.com>
 */

#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch-mx7/mx7-ddr.h>
#include <asm/gpio.h>
#include <spl.h>

#if defined(CONFIG_SPL_BUILD)

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	return 0;
}
#endif

static struct ddrc ddrc_regs_val = {
	.mstr		= 0x01040001,
	.rfshtmg	= 0x00400046,
	.init1		= 0x00690000,
	.init0		= 0x00020083,
	.init3		= 0x09300004,
	.init4		= 0x04080000,
	.init5		= 0x00100004,
	.rankctl	= 0x0000033F,
	.dramtmg0	= 0x09081109,
	.dramtmg1	= 0x0007020d,
	.dramtmg2	= 0x03040407,
	.dramtmg3	= 0x00002006,
	.dramtmg4	= 0x04020205,
	.dramtmg5	= 0x03030202,
	.dramtmg8	= 0x00000803,
	.zqctl0		= 0x00800020,
	.dfitmg0	= 0x02098204,
	.dfitmg1	= 0x00030303,
	.dfiupd0	= 0x80400003,
	.dfiupd1	= 0x00100020,
	.dfiupd2	= 0x80100004,
	.addrmap4	= 0x00000F0F,
	.odtcfg		= 0x06000604,
	.odtmap		= 0x00000001,
	.rfshtmg	= 0x00400046,
	.dramtmg0	= 0x09081109,
	.addrmap0	= 0x0000001f,
	.addrmap1	= 0x00080808,
	.addrmap4	= 0x00000f0f,
	.addrmap5	= 0x07070707,
	.addrmap6	= 0x0f0f0707,
};

static struct ddrc_mp ddrc_mp_val = {
	.pctrl_0	= 0x00000001,
};

static struct ddr_phy ddr_phy_regs_val = {
	.phy_con0	= 0x17420f40,
	.phy_con1	= 0x10210100,
	.phy_con4	= 0x00060807,
	.mdll_con0	= 0x1010007e,
	.drvds_con0	= 0x00000d6e,
	.cmd_sdll_con0	= 0x00000010,
	.offset_lp_con0	= 0x0000000f,
	.offset_rd_con0	= 0x08080808,
	.offset_wr_con0	= 0x08080808,
};

static struct mx7_calibration calib_param = {
	.num_val	= 5,
	.values		= {
		0x0E407304,
		0x0E447304,
		0x0E447306,
		0x0E447304,
		0x0E447304,
	},
};

static void gpr_init(void)
{
	struct iomuxc_gpr_base_regs *gpr_regs =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;
	writel(0x4F400005, &gpr_regs->gpr[1]);
}

static bool is_1g(void)
{
	gpio_direction_input(IMX_GPIO_NR(1, 12));
	return !gpio_get_value(IMX_GPIO_NR(1, 12));
}

static void ddr_init(void)
{
	if (is_1g())
		ddrc_regs_val.addrmap6	= 0x0f070707;

	mx7_dram_cfg(&ddrc_regs_val, &ddrc_mp_val, &ddr_phy_regs_val,
		     &calib_param);
}

void board_init_f(ulong dummy)
{
	arch_cpu_init();
	gpr_init();
	board_early_init_f();
	timer_init();
	preloader_console_init();
	ddr_init();
	memset(__bss_start, 0, __bss_end - __bss_start);
	board_init_r(NULL, 0);
}

void reset_cpu(ulong addr)
{
}
#endif
