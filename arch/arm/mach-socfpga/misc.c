// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2012-2025 Altera Corporation <www.altera.com>
 */

#include <config.h>
#include <command.h>
#include <errno.h>
#include <init.h>
#include <handoff.h>
#include <hang.h>
#include <watchdog.h>
#include <fdtdec.h>
#include <dm/ofnode.h>
#include <linux/libfdt.h>
#include <linux/printk.h>
#include <miiphy.h>
#include <netdev.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/cache.h>
#include <asm/pl310.h>
#include <asm/arch/misc.h>
#include <asm/arch/nic301.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/scan_manager.h>
#include <asm/arch/scu.h>
#include <asm/arch/system_manager.h>
#include <altera.h>
#include <bloblist.h>
#include <cpu_func.h>

DECLARE_GLOBAL_DATA_PTR;

phys_addr_t socfpga_clkmgr_base __section(".data");
phys_addr_t socfpga_rstmgr_base __section(".data");
phys_addr_t socfpga_sysmgr_base __section(".data");

#ifdef CONFIG_SYS_L2_PL310
static const struct pl310_regs *const pl310 =
	(struct pl310_regs *)CFG_SYS_PL310_BASE;
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
#if CONFIG_IS_ENABLED(HANDOFF) && IS_ENABLED(CONFIG_TARGET_SOCFPGA_AGILEX5)
	struct spl_handoff *ho;

	ho = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF, sizeof(*ho));
	if (!ho)
		return log_msg_ret("Missing SPL hand-off info", -ENOENT);
	gd->ram_size = ho->ram_bank[0].size;
	gd->ram_base = ho->ram_bank[0].start;
#else
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;
#endif /* HANDOFF && CONFIG_TARGET_SOCFPGA_AGILEX5 */

	return 0;
}

void enable_caches(void)
{
#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
	icache_enable();
#endif
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	dcache_enable();
#endif
}

#ifdef CONFIG_SYS_L2_PL310
void v7_outer_cache_enable(void)
{
	struct udevice *dev;

	if (uclass_get_device(UCLASS_CACHE, 0, &dev))
		pr_err("cache controller driver NOT found!\n");
}

void v7_outer_cache_disable(void)
{
	/* Disable the L2 cache */
	clrbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);
}

void socfpga_pl310_clear(void)
{
	u32 mask = 0xff, ena = 0;

	icache_enable();

	/* Disable the L2 cache */
	clrbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);

	writel(0x0, &pl310->pl310_tag_latency_ctrl);
	writel(0x10, &pl310->pl310_data_latency_ctrl);

	/* enable BRESP, instruction and data prefetch, full line of zeroes */
	setbits_le32(&pl310->pl310_aux_ctrl,
		     L310_AUX_CTRL_DATA_PREFETCH_MASK |
		     L310_AUX_CTRL_INST_PREFETCH_MASK |
		     L310_SHARED_ATT_OVERRIDE_ENABLE);

	/* Enable the L2 cache */
	ena = readl(&pl310->pl310_ctrl);
	ena |= L2X0_CTRL_EN;

	/*
	 * Invalidate the PL310 L2 cache. Keep the invalidation code
	 * entirely in L1 I-cache to avoid any bus traffic through
	 * the L2.
	 */
	asm volatile(
		".align	5			\n"
		"	b	3f		\n"
		"1:	str	%1,	[%4]	\n"
		"	dsb			\n"
		"	isb			\n"
		"	str	%0,	[%2]	\n"
		"	dsb			\n"
		"	isb			\n"
		"2:	ldr	%0,	[%2]	\n"
		"	cmp	%0,	#0	\n"
		"	bne	2b		\n"
		"	str	%0,	[%3]	\n"
		"	dsb			\n"
		"	isb			\n"
		"	b	4f		\n"
		"3:	b	1b		\n"
		"4:	nop			\n"
	: "+r"(mask), "+r"(ena)
	: "r"(&pl310->pl310_inv_way),
	  "r"(&pl310->pl310_cache_sync), "r"(&pl310->pl310_ctrl)
	: "memory", "cc");

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
	socfpga_get_managers_addr();

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

#ifndef CONFIG_XPL_BUILD
static int do_bridge(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	unsigned int mask = ~0;

	if (argc < 2 || argc > 3)
		return CMD_RET_USAGE;

	argv++;

	if (argc == 3)
		mask = hextoul(argv[1], NULL);

	switch (*argv[0]) {
	case 'e':	/* Enable */
		do_bridge_reset(1, mask);
		break;
	case 'd':	/* Disable */
		do_bridge_reset(0, mask);
		break;
	default:
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(bridge, 3, 1, do_bridge,
	   "SoCFPGA HPS FPGA bridge control",
	   "enable [mask] - Enable HPS-to-FPGA (Bit 0), LWHPS-to-FPGA (Bit 1), FPGA-to-HPS (Bit 2) bridges\n"
	   "bridge disable [mask] - Disable HPS-to-FPGA (Bit 0), LWHPS-to-FPGA (Bit 1), FPGA-to-HPS (Bit 2) bridges\n"
	   ""
);

#endif

static int socfpga_get_base_addr(const char *compat, phys_addr_t *base)
{
	const void *blob = gd->fdt_blob;
	struct fdt_resource r;
	int node;
	int ret;

	node = fdt_node_offset_by_compatible(blob, -1, compat);
	if (node < 0)
		return node;

	if (!fdtdec_get_is_enabled(blob, node))
		return -ENODEV;

	ret = fdt_get_resource(blob, node, "reg", 0, &r);
	if (ret)
		return ret;

	*base = (phys_addr_t)r.start;

	return 0;
}

void socfpga_get_managers_addr(void)
{
	int ret;

	ret = socfpga_get_base_addr("altr,rst-mgr", &socfpga_rstmgr_base);
	if (ret)
		hang();

	else if (IS_ENABLED(CONFIG_TARGET_SOCFPGA_N5X))
		ret = socfpga_get_base_addr("intel,n5x-clkmgr",
					    &socfpga_clkmgr_base);
	else if (!IS_ENABLED(CONFIG_TARGET_SOCFPGA_AGILEX) &&
		 !IS_ENABLED(CONFIG_TARGET_SOCFPGA_AGILEX7M) &&
		 !IS_ENABLED(CONFIG_TARGET_SOCFPGA_AGILEX5))
		ret = socfpga_get_base_addr("altr,clk-mgr",
					    &socfpga_clkmgr_base);

	if (ret)
		hang();
}

void socfpga_get_sys_mgr_addr(void)
{
	int ret;
	struct udevice *dev;

	ofnode node = ofnode_get_aliases_node("sysmgr");

	if (!ofnode_valid(node)) {
		printf("'sysmgr' alias not found in device tree\n");
		hang();
	}

	ret = uclass_get_device_by_ofnode(UCLASS_NOP, node, &dev);
	if (ret) {
		printf("Altera system manager init failed: %d\n", ret);
		hang();
	} else {
		socfpga_sysmgr_base = (phys_addr_t)dev_read_addr(dev);
	}
}

phys_addr_t socfpga_get_rstmgr_addr(void)
{
	return socfpga_rstmgr_base;
}

phys_addr_t socfpga_get_sysmgr_addr(void)
{
	return socfpga_sysmgr_base;
}

phys_addr_t socfpga_get_clkmgr_addr(void)
{
	return socfpga_clkmgr_base;
}
