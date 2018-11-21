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
/*
 * FPGA programming support for SoC FPGA Cyclone V
 */
static Altera_desc altera_fpga[] = {
	{
		/* Family */
		Altera_SoCFPGA,
		/* Interface type */
		fast_passive_parallel,
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

/* add device descriptor to FPGA device table */
void socfpga_fpga_add(void)
{
	int i;
	fpga_init();
	for (i = 0; i < ARRAY_SIZE(altera_fpga); i++)
		fpga_add(fpga_altera, &altera_fpga[i]);
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

#ifdef CONFIG_ETH_DESIGNWARE
static int dwmac_phymode_to_modereg(const char *phymode, u32 *modereg)
{
	if (!phymode)
		return -EINVAL;

	if (!strcmp(phymode, "mii") || !strcmp(phymode, "gmii")) {
		*modereg = SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_GMII_MII;
		return 0;
	}

	if (!strcmp(phymode, "rgmii")) {
		*modereg = SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RGMII;
		return 0;
	}

	if (!strcmp(phymode, "rmii")) {
		*modereg = SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RMII;
		return 0;
	}

	return -EINVAL;
}

int socfpga_eth_reset_common(void (*resetfn)(const u8 of_reset_id,
					     const u8 phymode))
{
	const void *fdt = gd->fdt_blob;
	struct fdtdec_phandle_args args;
	const char *phy_mode;
	u32 phy_modereg;
	int nodes[2];	/* Max. two GMACs */
	int ret, count;
	int i, node;

	count = fdtdec_find_aliases_for_id(fdt, "ethernet",
					   COMPAT_ALTERA_SOCFPGA_DWMAC,
					   nodes, ARRAY_SIZE(nodes));
	for (i = 0; i < count; i++) {
		node = nodes[i];
		if (node <= 0)
			continue;

		ret = fdtdec_parse_phandle_with_args(fdt, node, "resets",
						     "#reset-cells", 1, 0,
						     &args);
		if (ret || (args.args_count != 1)) {
			debug("GMAC%i: Failed to parse DT 'resets'!\n", i);
			continue;
		}

		phy_mode = fdt_getprop(fdt, node, "phy-mode", NULL);
		ret = dwmac_phymode_to_modereg(phy_mode, &phy_modereg);
		if (ret) {
			debug("GMAC%i: Failed to parse DT 'phy-mode'!\n", i);
			continue;
		}

		resetfn(args.args[0], phy_modereg);
	}

	return 0;
}
#endif

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
