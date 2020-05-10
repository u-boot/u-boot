// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2013-2017 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <command.h>
#include <init.h>
#include <wait_bit.h>
#include <asm/io.h>
#include <asm/arch/clock_manager.h>

DECLARE_GLOBAL_DATA_PTR;

void cm_wait_for_lock(u32 mask)
{
	u32 inter_val;
	u32 retry = 0;
	do {
#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
		inter_val = readl(socfpga_get_clkmgr_addr() +
				  CLKMGR_INTER) & mask;
#else
		inter_val = readl(socfpga_get_clkmgr_addr() +
				  CLKMGR_STAT) & mask;
#endif
		/* Wait for stable lock */
		if (inter_val == mask)
			retry++;
		else
			retry = 0;
		if (retry >= 10)
			break;
	} while (1);
}

/* function to poll in the fsm busy bit */
int cm_wait_for_fsm(void)
{
	return wait_for_bit_le32((const void *)(socfpga_get_clkmgr_addr() +
				 CLKMGR_STAT), CLKMGR_STAT_BUSY, false, 20000,
				 false);
}

int set_cpu_clk_info(void)
{
#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
	/* Calculate the clock frequencies required for drivers */
	cm_get_l4_sp_clk_hz();
	cm_get_mmc_controller_clk_hz();
#endif

	gd->bd->bi_arm_freq = cm_get_mpu_clk_hz() / 1000000;
	gd->bd->bi_dsp_freq = 0;

#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
	gd->bd->bi_ddr_freq = cm_get_sdram_clk_hz() / 1000000;
#else
	gd->bd->bi_ddr_freq = 0;
#endif

	return 0;
}

#ifndef CONFIG_SPL_BUILD
static int do_showclocks(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
{
	cm_print_clock_quick_summary();
	return 0;
}

U_BOOT_CMD(
	clocks,	CONFIG_SYS_MAXARGS, 1, do_showclocks,
	"display clocks",
	""
);
#endif
