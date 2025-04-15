// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2018 Intel Corporation <www.intel.com>
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 *
 */

#include <altera.h>
#include <env.h>
#include <errno.h>
#include <init.h>
#include <log.h>
#include <asm/arch/board.h>
#include <asm/arch/mailbox_s10.h>
#include <asm/arch/misc.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/global_data.h>
#include <mach/clock_manager.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * FPGA programming support for SoC FPGA Stratix 10
 */
static Altera_desc altera_fpga[] = {
	{
		/* Family */
		Intel_FPGA_SDM_Mailbox,
		/* Interface type */
		secure_device_manager_mailbox,
		/* No limitation as additional data will be ignored */
		-1,
		/* No device function table */
		NULL,
		/* Base interface address specified in driver */
		NULL,
		/* No cookie implementation */
		0
	},
};

/*
 * The Agilex5 platform has enabled the bloblist feature, and the bloblist
 * address and size are initialized based on the defconfig settings.
 * During the SPL phase, this function is used to prevent the bloblist
 * from initializing its address and size with the saved boot parameters,
 * which may have been incorrectly set.
 */
void save_boot_params(unsigned long r0, unsigned long r1, unsigned long r2,
		      unsigned long r3)
{
	save_boot_params_ret();
}

/*
 * Print CPU information
 */
#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	printf("CPU: Intel FPGA SoCFPGA Platform (ARMv8 64bit Cortex-%s)\n",
	       IS_ENABLED(CONFIG_TARGET_SOCFPGA_AGILEX5) ? "A55/A76" : "A53");

	return 0;
}
#endif

#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
	char qspi_string[13];
	unsigned long id;

	sprintf(qspi_string, "<0x%08x>", cm_get_qspi_controller_clk_hz());
	env_set("qspi_clock", qspi_string);

	/* Export board_id as environment variable */
	id = socfpga_get_board_id();
	env_set_ulong("board_id", id);

	return 0;
}
#endif

int arch_early_init_r(void)
{
	socfpga_fpga_add(&altera_fpga[0]);

	return 0;
}

/* Return 1 if FPGA is ready otherwise return 0 */
int is_fpga_config_ready(void)
{
	return (readl(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_FPGA_CONFIG) &
		SYSMGR_FPGACONFIG_READY_MASK) == SYSMGR_FPGACONFIG_READY_MASK;
}

void do_bridge_reset(int enable, unsigned int mask)
{
	/* Check FPGA status before bridge enable */
	if (!is_fpga_config_ready()) {
		puts("FPGA not ready. Bridge reset aborted!\n");
		return;
	}

	socfpga_bridges_reset(enable, mask);
}
