// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2012-2017 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <asm/io.h>
#include <errno.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <altera.h>
#include <miiphy.h>
#include <netdev.h>
#include <watchdog.h>
#include <asm/arch/misc.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/scan_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/nic301.h>
#include <asm/arch/scu.h>
#include <asm/pl310.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SYS_L2_PL310
static const struct pl310_regs *const pl310 =
	(struct pl310_regs *)CONFIG_SYS_PL310_BASE;
#endif

struct bsel bsel_str[] = {
	{ "rsvd", "Reserved", },
	{ "fpga", "FPGA (HPS2FPGA Bridge)", },
	{ "nand", "NAND Flash (1.8V)", },
	{ "nand", "NAND Flash (3.0V)", },
	{ "sd", "SD/MMC External Transceiver (1.8V)", },
	{ "sd", "SD/MMC Internal Transceiver (3.0V)", },
	{ "qspi", "QSPI Flash (1.8V)", },
	{ "qspi", "QSPI Flash (3.0V)", },
};

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
}

void enable_caches(void)
{
#ifndef CONFIG_SYS_ICACHE_OFF
	icache_enable();
#endif
#ifndef CONFIG_SYS_DCACHE_OFF
	dcache_enable();
#endif
}

#ifdef CONFIG_SYS_L2_PL310
void v7_outer_cache_enable(void)
{
	/* Disable the L2 cache */
	clrbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);

	writel(0x111, &pl310->pl310_tag_latency_ctrl);
	writel(0x121, &pl310->pl310_data_latency_ctrl);

	/* enable BRESP, instruction and data prefetch, full line of zeroes */
	setbits_le32(&pl310->pl310_aux_ctrl,
		     L310_AUX_CTRL_DATA_PREFETCH_MASK |
		     L310_AUX_CTRL_INST_PREFETCH_MASK |
		     L310_SHARED_ATT_OVERRIDE_ENABLE);

	/* Enable the L2 cache */
	setbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);
}

void v7_outer_cache_disable(void)
{
	/* Disable the L2 cache */
	clrbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);
}
#endif

#if defined(CONFIG_SYS_CONSOLE_IS_IN_ENV) && \
defined(CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE)
int overwrite_console(void)
{
	return 0;
}
#endif

#ifdef CONFIG_FPGA
/* add device descriptor to FPGA device table */
void socfpga_fpga_add(void *fpga_desc)
{
	fpga_init();
	fpga_add(fpga_altera, fpga_desc);
}
#endif

int arch_cpu_init(void)
{
#ifdef CONFIG_HW_WATCHDOG
	/*
	 * In case the watchdog is enabled, make sure to (re-)configure it
	 * so that the defined timeout is valid. Otherwise the SPL (Perloader)
	 * timeout value is still active which might too short for Linux
	 * booting.
	 */
	hw_watchdog_init();
#else
	/*
	 * If the HW watchdog is NOT enabled, make sure it is not running,
	 * for example because it was enabled in the preloader. This might
	 * trigger a watchdog-triggered reboot of Linux kernel later.
	 * Toggle watchdog reset, so watchdog in not running state.
	 */
	socfpga_per_reset(SOCFPGA_RESET(L4WD0), 1);
	socfpga_per_reset(SOCFPGA_RESET(L4WD0), 0);
#endif

	return 0;
}

#ifndef CONFIG_SPL_BUILD
static int do_bridge(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;

	argv++;

	switch (*argv[0]) {
	case 'e':	/* Enable */
		do_bridge_reset(1);
		break;
	case 'd':	/* Disable */
		do_bridge_reset(0);
		break;
	default:
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(bridge, 2, 1, do_bridge,
	   "SoCFPGA HPS FPGA bridge control",
	   "enable  - Enable HPS-to-FPGA, FPGA-to-HPS, LWHPS-to-FPGA bridges\n"
	   "bridge disable - Enable HPS-to-FPGA, FPGA-to-HPS, LWHPS-to-FPGA bridges\n"
	   ""
);

#endif
