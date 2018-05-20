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
#include <asm/arch/scu.h>
#include <asm/arch/nic301.h>
#include <asm/sections.h>
#include <fdtdec.h>
#include <watchdog.h>
#if defined(CONFIG_TARGET_SOCFPGA_ARRIA10)
#include <asm/arch/pinmux.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
static struct pl310_regs *const pl310 =
	(struct pl310_regs *)CONFIG_SYS_PL310_BASE;
static struct scu_registers *scu_regs =
	(struct scu_registers *)SOCFPGA_MPUSCU_ADDRESS;
static struct nic301_registers *nic301_regs =
	(struct nic301_registers *)SOCFPGA_L3REGS_ADDRESS;
#endif

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
		socfpga_per_reset(SOCFPGA_RESET(NAND), 0);
		return BOOT_DEVICE_NAND;
	case 0x4:	/* SD/MMC External Transceiver (1.8V) */
	case 0x5:	/* SD/MMC Internal Transceiver (3.0V) */
		socfpga_per_reset(SOCFPGA_RESET(SDMMC), 0);
		socfpga_per_reset(SOCFPGA_RESET(DMA), 0);
		return BOOT_DEVICE_MMC1;
	case 0x6:	/* QSPI Flash (1.8V) */
	case 0x7:	/* QSPI Flash (3.0V) */
		socfpga_per_reset(SOCFPGA_RESET(QSPI), 0);
		return BOOT_DEVICE_SPI;
	default:
		printf("Invalid boot device (bsel=%08x)!\n", bsel);
		hang();
	}
}

#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
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
	const struct cm_config *cm_default_cfg = cm_get_default_config();
	unsigned long sdram_size;
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

	/* Configure simple malloc base pointer into RAM. */
	gd->malloc_base = CONFIG_SYS_TEXT_BASE + (1024 * 1024);
}
#elif defined(CONFIG_TARGET_SOCFPGA_ARRIA10)
void spl_board_init(void)
{
	/* configuring the clock based on handoff */
	cm_basic_init(gd->fdt_blob);
	WATCHDOG_RESET();

	config_dedicated_pins(gd->fdt_blob);
	WATCHDOG_RESET();

	/* Release UART from reset */
	socfpga_reset_uart(0);

	/* enable console uart printing */
	preloader_console_init();

	WATCHDOG_RESET();

	/* Add device descriptor to FPGA device table */
	socfpga_fpga_add();
}

void board_init_f(ulong dummy)
{
	/*
	 * Configure Clock Manager to use intosc clock instead external osc to
	 * ensure success watchdog operation. We do it as early as possible.
	 */
	cm_use_intosc();

	socfpga_watchdog_disable();

	arch_early_init_r();

#ifdef CONFIG_HW_WATCHDOG
	/* release osc1 watchdog timer 0 from reset */
	socfpga_reset_deassert_osc1wd0();

	/* reconfigure and enable the watchdog */
	hw_watchdog_init();
	WATCHDOG_RESET();
#endif /* CONFIG_HW_WATCHDOG */
}
#endif
