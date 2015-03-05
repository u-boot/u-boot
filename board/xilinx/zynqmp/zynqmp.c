/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

int board_early_init_r(void)
{
	u32 val;

	val = readl(&crlapb_base->timestamp_ref_ctrl);
	val |= ZYNQMP_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT;
	writel(val, &crlapb_base->timestamp_ref_ctrl);

	/* Program freq register in System counter and enable system counter */
	writel(gd->cpu_clk, &iou_scntr->base_frequency_id_register);
	writel(ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_HDBG |
	       ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_EN,
	       &iou_scntr->counter_control_register);

	return 0;
}

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

int timer_init(void)
{
	return 0;
}

void reset_cpu(ulong addr)
{
}

#ifdef CONFIG_CMD_MMC
int board_mmc_init(bd_t *bd)
{
	int ret = 0;

#if defined(CONFIG_ZYNQ_SDHCI)
# if defined(CONFIG_ZYNQ_SDHCI0)
	ret = zynq_sdhci_init(ZYNQ_SDHCI_BASEADDR0);
# endif
# if defined(CONFIG_ZYNQ_SDHCI1)
	ret |= zynq_sdhci_init(ZYNQ_SDHCI_BASEADDR1);
# endif
#endif

	return ret;
}
#endif

int board_late_init(void)
{
	u32 reg = 0;
	u8 bootmode;

	reg = readl(&crlapb_base->boot_mode);
	bootmode = reg & BOOT_MODES_MASK;

	switch (bootmode) {
	case SD_MODE:
		setenv("modeboot", "sdboot");
		break;
	default:
		printf("Invalid Boot Mode:0x%x\n", bootmode);
		break;
	}

	return 0;
}
