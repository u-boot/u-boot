// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/pl310.h>
#include <asm/u-boot.h>
#include <asm/utils.h>
#include <image.h>
#include <asm/arch/reset_manager.h>
#include <spl.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/freeze_controller.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/misc.h>
#include <asm/arch/scan_manager.h>
#include <asm/arch/sdram.h>
#include <asm/sections.h>
#include <debug_uart.h>
#include <fdtdec.h>
#include <watchdog.h>
#include <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

static struct pl310_regs *const pl310 =
	(struct pl310_regs *)CONFIG_SYS_PL310_BASE;
static const struct socfpga_system_manager *sysmgr_regs =
	(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;

u32 spl_boot_device(void)
{
	const u32 bsel = readl(&sysmgr_regs->bootinfo);

	switch (SYSMGR_GET_BOOTINFO_BSEL(bsel)) {
	case 0x1:	/* FPGA (HPS2FPGA Bridge) */
		return BOOT_DEVICE_RAM;
	case 0x2:	/* NAND Flash (1.8V) */
	case 0x3:	/* NAND Flash (3.0V) */
		return BOOT_DEVICE_NAND;
	case 0x4:	/* SD/MMC External Transceiver (1.8V) */
	case 0x5:	/* SD/MMC Internal Transceiver (3.0V) */
		return BOOT_DEVICE_MMC1;
	case 0x6:	/* QSPI Flash (1.8V) */
	case 0x7:	/* QSPI Flash (3.0V) */
		return BOOT_DEVICE_SPI;
	default:
		printf("Invalid boot device (bsel=%08x)!\n", bsel);
		hang();
	}
}

#ifdef CONFIG_SPL_MMC_SUPPORT
u32 spl_boot_mode(const u32 boot_device)
{
#if defined(CONFIG_SPL_FS_FAT) || defined(CONFIG_SPL_FS_EXT4)
	return MMCSD_MODE_FS;
#else
	return MMCSD_MODE_RAW;
#endif
}
#endif

static void socfpga_pl310_clear(void)
{
	u32 mask = 0xff, ena = 0;

	icache_enable();

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

void board_init_f(ulong dummy)
{
	const struct cm_config *cm_default_cfg = cm_get_default_config();
	unsigned long reg;
	int ret;
	struct udevice *dev;

	/*
	 * First C code to run. Clear fake OCRAM ECC first as SBE
	 * and DBE might triggered during power on
	 */
	reg = readl(&sysmgr_regs->eccgrp_ocram);
	if (reg & SYSMGR_ECC_OCRAM_SERR)
		writel(SYSMGR_ECC_OCRAM_SERR | SYSMGR_ECC_OCRAM_EN,
		       &sysmgr_regs->eccgrp_ocram);
	if (reg & SYSMGR_ECC_OCRAM_DERR)
		writel(SYSMGR_ECC_OCRAM_DERR  | SYSMGR_ECC_OCRAM_EN,
		       &sysmgr_regs->eccgrp_ocram);

	memset(__bss_start, 0, __bss_end - __bss_start);

	socfpga_sdram_remap_zero();
	socfpga_pl310_clear();

	debug("Freezing all I/O banks\n");
	/* freeze all IO banks */
	sys_mgr_frzctrl_freeze_req();

	/* Put everything into reset but L4WD0. */
	socfpga_per_reset_all();

	if (!socfpga_is_booting_from_fpga()) {
		/* Put FPGA bridges into reset too. */
		socfpga_bridges_reset(1);
	}

	socfpga_per_reset(SOCFPGA_RESET(OSC1TIMER0), 0);
	timer_init();

	debug("Reconfigure Clock Manager\n");
	/* reconfigure the PLLs */
	if (cm_basic_init(cm_default_cfg))
		hang();

	/* Enable bootrom to configure IOs. */
	sysmgr_config_warmrstcfgio(1);

	/* configure the IOCSR / IO buffer settings */
	if (scan_mgr_configure_iocsr())
		hang();

	sysmgr_config_warmrstcfgio(0);

	/* configure the pin muxing through system manager */
	sysmgr_config_warmrstcfgio(1);
	sysmgr_pinmux_init();
	sysmgr_config_warmrstcfgio(0);

	/* De-assert reset for bridges based on handoff */
	socfpga_bridges_reset(0);

	debug("Unfreezing/Thaw all I/O banks\n");
	/* unfreeze / thaw all IO banks */
	sys_mgr_frzctrl_thaw_req();

#ifdef CONFIG_DEBUG_UART
	socfpga_per_reset(SOCFPGA_RESET(UART0), 0);
	debug_uart_init();
#endif

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device(UCLASS_RESET, 0, &dev);
	if (ret)
		debug("Reset init failed: %d\n", ret);

	/* enable console uart printing */
	preloader_console_init();

	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		hang();
	}

	if (!socfpga_is_booting_from_fpga())
		socfpga_bridges_reset(1);
}
