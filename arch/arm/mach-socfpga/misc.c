/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <altera.h>
#include <miiphy.h>
#include <netdev.h>
#include <watchdog.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/dwmmc.h>
#include <asm/arch/nic301.h>
#include <asm/arch/scu.h>
#include <asm/pl310.h>

DECLARE_GLOBAL_DATA_PTR;

static struct pl310_regs *const pl310 =
	(struct pl310_regs *)CONFIG_SYS_PL310_BASE;
static struct socfpga_system_manager *sysmgr_regs =
	(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;
static struct socfpga_reset_manager *reset_manager_base =
	(struct socfpga_reset_manager *)SOCFPGA_RSTMGR_ADDRESS;
static struct nic301_registers *nic301_regs =
	(struct nic301_registers *)SOCFPGA_L3REGS_ADDRESS;
static struct scu_registers *scu_regs =
	(struct scu_registers *)SOCFPGA_MPUSCU_ADDRESS;

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
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

/*
 * DesignWare Ethernet initialization
 */
#ifdef CONFIG_ETH_DESIGNWARE
int cpu_eth_init(bd_t *bis)
{
#if CONFIG_EMAC_BASE == SOCFPGA_EMAC0_ADDRESS
	const int physhift = SYSMGR_EMACGRP_CTRL_PHYSEL0_LSB;
#elif CONFIG_EMAC_BASE == SOCFPGA_EMAC1_ADDRESS
	const int physhift = SYSMGR_EMACGRP_CTRL_PHYSEL1_LSB;
#else
#error "Incorrect CONFIG_EMAC_BASE value!"
#endif

	/* Initialize EMAC. This needs to be done at least once per boot. */

	/*
	 * Putting the EMAC controller to reset when configuring the PHY
	 * interface select at System Manager
	 */
	socfpga_emac_reset(1);

	/* Clearing emac0 PHY interface select to 0 */
	clrbits_le32(&sysmgr_regs->emacgrp_ctrl,
		     SYSMGR_EMACGRP_CTRL_PHYSEL_MASK << physhift);

	/* configure to PHY interface select choosed */
	setbits_le32(&sysmgr_regs->emacgrp_ctrl,
		     SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RGMII << physhift);

	/* Release the EMAC controller from reset */
	socfpga_emac_reset(0);

	/* initialize and register the emac */
	return designware_initialize(CONFIG_EMAC_BASE,
				     CONFIG_PHY_INTERFACE_MODE);
}
#endif

#ifdef CONFIG_DWMMC
/*
 * Initializes MMC controllers.
 * to override, implement board_mmc_init()
 */
int cpu_mmc_init(bd_t *bis)
{
	return socfpga_dwmmc_init(SOCFPGA_SDMMC_ADDRESS,
				  CONFIG_HPS_SDMMC_BUSWIDTH, 0);
}
#endif

#if defined(CONFIG_DISPLAY_CPUINFO)
/*
 * Print CPU information
 */
int print_cpuinfo(void)
{
	puts("CPU:   Altera SoCFPGA Platform\n");
	return 0;
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
static void socfpga_fpga_add(void)
{
	int i;
	fpga_init();
	for (i = 0; i < ARRAY_SIZE(altera_fpga); i++)
		fpga_add(fpga_altera, &altera_fpga[i]);
}
#else
static inline void socfpga_fpga_add(void) {}
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
	 */
	socfpga_watchdog_reset();
#endif

	return 0;
}

/*
 * Convert all NIC-301 AMBA slaves from secure to non-secure
 */
static void socfpga_nic301_slave_ns(void)
{
	writel(0x1, &nic301_regs->lwhps2fpgaregs);
	writel(0x1, &nic301_regs->hps2fpgaregs);
	writel(0x1, &nic301_regs->acp);
	writel(0x1, &nic301_regs->rom);
	writel(0x1, &nic301_regs->ocram);
	writel(0x1, &nic301_regs->sdrdata);
}

static uint32_t iswgrp_handoff[8];

int arch_early_init_r(void)
{
	int i;
	for (i = 0; i < 8; i++)	/* Cache initial SW setting regs */
		iswgrp_handoff[i] = readl(&sysmgr_regs->iswgrp_handoff[i]);

	socfpga_bridges_reset(1);
	socfpga_nic301_slave_ns();

	/*
	 * Private components security:
	 * U-Boot : configure private timer, global timer and cpu component
	 * access as non secure for kernel stage (as required by Linux)
	 */
	setbits_le32(&scu_regs->sacr, 0xfff);

	/* Configure the L2 controller to make SDRAM start at 0 */
#ifdef CONFIG_SOCFPGA_VIRTUAL_TARGET
	writel(0x2, &nic301_regs->remap);
#else
	writel(0x1, &nic301_regs->remap);	/* remap.mpuzero */
	writel(0x1, &pl310->pl310_addr_filter_start);
#endif

	/* Add device descriptor to FPGA device table */
	socfpga_fpga_add();

#ifdef CONFIG_DESIGNWARE_SPI
	/* Get Designware SPI controller out of reset */
	socfpga_spim_enable();
#endif

	return 0;
}

static void socfpga_sdram_apply_static_cfg(void)
{
	const uint32_t staticcfg = SOCFPGA_SDR_ADDRESS + 0x505c;
	const uint32_t applymask = 0x8;
	uint32_t val = readl(staticcfg) | applymask;

	/*
	 * SDRAM staticcfg register specific:
	 * When applying the register setting, the CPU must not access
	 * SDRAM. Luckily for us, we can abuse i-cache here to help us
	 * circumvent the SDRAM access issue. The idea is to make sure
	 * that the code is in one full i-cache line by branching past
	 * it and back. Once it is in the i-cache, we execute the core
	 * of the code and apply the register settings.
	 *
	 * The code below uses 7 instructions, while the Cortex-A9 has
	 * 32-byte cachelines, thus the limit is 8 instructions total.
	 */
	asm volatile(
		".align	5			\n"
		"	b	2f		\n"
		"1:	str	%0,	[%1]	\n"
		"	dsb			\n"
		"	isb			\n"
		"	b	3f		\n"
		"2:	b	1b		\n"
		"3:	nop			\n"
	: : "r"(val), "r"(staticcfg) : "memory", "cc");
}

int do_bridge(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc != 2)
		return CMD_RET_USAGE;

	argv++;

	switch (*argv[0]) {
	case 'e':	/* Enable */
		writel(iswgrp_handoff[2], &sysmgr_regs->fpgaintfgrp_module);
		socfpga_sdram_apply_static_cfg();
		writel(iswgrp_handoff[3], SOCFPGA_SDR_ADDRESS + 0x5080);
		writel(iswgrp_handoff[0], &reset_manager_base->brg_mod_reset);
		writel(iswgrp_handoff[1], &nic301_regs->remap);
		break;
	case 'd':	/* Disable */
		writel(0, &sysmgr_regs->fpgaintfgrp_module);
		writel(0, SOCFPGA_SDR_ADDRESS + 0x5080);
		socfpga_sdram_apply_static_cfg();
		writel(0, &reset_manager_base->brg_mod_reset);
		writel(1, &nic301_regs->remap);
		break;
	default:
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(
	bridge, 2, 1, do_bridge,
	"SoCFPGA HPS FPGA bridge control",
	"enable  - Enable HPS-to-FPGA, FPGA-to-HPS, LWHPS-to-FPGA bridges\n"
	"bridge disable - Enable HPS-to-FPGA, FPGA-to-HPS, LWHPS-to-FPGA bridges\n"
	""
);
