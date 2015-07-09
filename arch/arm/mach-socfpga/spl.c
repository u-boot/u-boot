/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
#include <asm/arch/scan_manager.h>
#include <asm/arch/sdram.h>
#include <asm/arch/scu.h>
#include <asm/arch/nic301.h>

DECLARE_GLOBAL_DATA_PTR;

static struct pl310_regs *const pl310 =
	(struct pl310_regs *)CONFIG_SYS_PL310_BASE;
static struct scu_registers *scu_regs =
	(struct scu_registers *)SOCFPGA_MPUSCU_ADDRESS;
static struct nic301_registers *nic301_regs =
	(struct nic301_registers *)SOCFPGA_L3REGS_ADDRESS;

static void socfpga_nic301_slave_ns(void)
{
	writel(0x1, &nic301_regs->lwhps2fpgaregs);
	writel(0x1, &nic301_regs->hps2fpgaregs);
	writel(0x1, &nic301_regs->acp);
	writel(0x1, &nic301_regs->rom);
	writel(0x1, &nic301_regs->ocram);
	writel(0x1, &nic301_regs->sdrdata);
}

void board_init_f(ulong dummy)
{
	struct socfpga_system_manager *sysmgr_regs =
		(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;
	unsigned long reg;
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

	socfpga_nic301_slave_ns();

	/* Configure ARM MPU SNSAC register. */
	setbits_le32(&scu_regs->sacr, 0xfff);

	/* Remap SDRAM to 0x0 */
	writel(0x1, &nic301_regs->remap);	/* remap.mpuzero */
	writel(0x1, &pl310->pl310_addr_filter_start);

	board_init_r(NULL, 0);
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_RAM;
}

/*
 * Board initialization after bss clearance
 */
void spl_board_init(void)
{
	unsigned long sdram_size;
#ifndef CONFIG_SOCFPGA_VIRTUAL_TARGET
	const struct cm_config *cm_default_cfg = cm_get_default_config();
#endif

	debug("Freezing all I/O banks\n");
	/* freeze all IO banks */
	sys_mgr_frzctrl_freeze_req();

	/* Put everything into reset but L4WD0. */
	socfpga_per_reset_all();
	/* Put FPGA bridges into reset too. */
	socfpga_bridges_reset(1);

	socfpga_per_reset(SOCFPGA_RESET(SDR), 0);
	socfpga_per_reset(SOCFPGA_RESET(UART0), 0);
	socfpga_per_reset(SOCFPGA_RESET(OSC1TIMER0), 0);

	timer_init();

	debug("Reconfigure Clock Manager\n");
	/* reconfigure the PLLs */
	cm_basic_init(cm_default_cfg);

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

#endif /* CONFIG_SOCFPGA_VIRTUAL_TARGET */

	/* De-assert reset for peripherals and bridges based on handoff */
	reset_deassert_peripherals_handoff();
	socfpga_bridges_reset(0);

	debug("Unfreezing/Thaw all I/O banks\n");
	/* unfreeze / thaw all IO banks */
	sys_mgr_frzctrl_thaw_req();

	/* enable console uart printing */
	preloader_console_init();

	if (sdram_mmr_init_full(0xffffffff) != 0) {
		puts("SDRAM init failed.\n");
		hang();
	}

	debug("SDRAM: Calibrating PHY\n");
	/* SDRAM calibration */
	if (sdram_calibration_full() == 0) {
		puts("SDRAM calibration failed.\n");
		hang();
	}

	sdram_size = sdram_calculate_size();
	debug("SDRAM: %ld MiB\n", sdram_size >> 20);

	/* Sanity check ensure correct SDRAM size specified */
	if (get_ram_size(0, sdram_size) != sdram_size) {
		puts("SDRAM size check failed!\n");
		hang();
	}

	socfpga_bridges_reset(1);
}
