/*
 *  Copyright (C) 2012-2017 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <errno.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <altera.h>
#include <miiphy.h>
#include <netdev.h>
#include <watchdog.h>
#include <asm/arch/misc.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/scan_manager.h>
#include <asm/arch/sdram.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/nic301.h>
#include <asm/arch/scu.h>
#include <asm/pl310.h>

#include <dt-bindings/reset/altr,rst-mgr.h>

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
static struct socfpga_sdr_ctrl *sdr_ctrl =
	(struct socfpga_sdr_ctrl *)SDR_CTRLGRP_ADDRESS;

/*
 * DesignWare Ethernet initialization
 */
#ifdef CONFIG_ETH_DESIGNWARE
void dwmac_deassert_reset(const unsigned int of_reset_id,
				 const u32 phymode)
{
	u32 physhift, reset;

	if (of_reset_id == EMAC0_RESET) {
		physhift = SYSMGR_EMACGRP_CTRL_PHYSEL0_LSB;
		reset = SOCFPGA_RESET(EMAC0);
	} else if (of_reset_id == EMAC1_RESET) {
		physhift = SYSMGR_EMACGRP_CTRL_PHYSEL1_LSB;
		reset = SOCFPGA_RESET(EMAC1);
	} else {
		printf("GMAC: Invalid reset ID (%i)!\n", of_reset_id);
		return;
	}

	/* configure to PHY interface select choosed */
	clrsetbits_le32(&sysmgr_regs->emacgrp_ctrl,
			SYSMGR_EMACGRP_CTRL_PHYSEL_MASK << physhift,
			phymode << physhift);

	/* Release the EMAC controller from reset */
	socfpga_per_reset(reset, 0);
}

static u32 dwmac_phymode_to_modereg(const char *phymode, u32 *modereg)
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

static int socfpga_eth_reset(void)
{
	const void *fdt = gd->fdt_blob;
	struct fdtdec_phandle_args args;
	const char *phy_mode;
	u32 phy_modereg;
	int nodes[2];	/* Max. two GMACs */
	int ret, count;
	int i, node;

	/* Put both GMACs into RESET state. */
	socfpga_per_reset(SOCFPGA_RESET(EMAC0), 1);
	socfpga_per_reset(SOCFPGA_RESET(EMAC1), 1);

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

		dwmac_deassert_reset(args.args[0], phy_modereg);
	}

	return 0;
}
#else
static int socfpga_eth_reset(void)
{
	return 0;
};
#endif

static const struct {
	const u16	pn;
	const char	*name;
	const char	*var;
} const socfpga_fpga_model[] = {
	/* Cyclone V E */
	{ 0x2b15, "Cyclone V, E/A2", "cv_e_a2" },
	{ 0x2b05, "Cyclone V, E/A4", "cv_e_a4" },
	{ 0x2b22, "Cyclone V, E/A5", "cv_e_a5" },
	{ 0x2b13, "Cyclone V, E/A7", "cv_e_a7" },
	{ 0x2b14, "Cyclone V, E/A9", "cv_e_a9" },
	/* Cyclone V GX/GT */
	{ 0x2b01, "Cyclone V, GX/C3", "cv_gx_c3" },
	{ 0x2b12, "Cyclone V, GX/C4", "cv_gx_c4" },
	{ 0x2b02, "Cyclone V, GX/C5 or GT/D5", "cv_gx_c5" },
	{ 0x2b03, "Cyclone V, GX/C7 or GT/D7", "cv_gx_c7" },
	{ 0x2b04, "Cyclone V, GX/C9 or GT/D9", "cv_gx_c9" },
	/* Cyclone V SE/SX/ST */
	{ 0x2d11, "Cyclone V, SE/A2 or SX/C2", "cv_se_a2" },
	{ 0x2d01, "Cyclone V, SE/A4 or SX/C4", "cv_se_a4" },
	{ 0x2d12, "Cyclone V, SE/A5 or SX/C5 or ST/D5", "cv_se_a5" },
	{ 0x2d02, "Cyclone V, SE/A6 or SX/C6 or ST/D6", "cv_se_a6" },
	/* Arria V */
	{ 0x2d03, "Arria V, D5", "av_d5" },
};

static int socfpga_fpga_id(const bool print_id)
{
	const u32 altera_mi = 0x6e;
	const u32 id = scan_mgr_get_fpga_id();

	const u32 lsb = id & 0x00000001;
	const u32 mi = (id >> 1) & 0x000007ff;
	const u32 pn = (id >> 12) & 0x0000ffff;
	const u32 version = (id >> 28) & 0x0000000f;
	int i;

	if ((mi != altera_mi) || (lsb != 1)) {
		printf("FPGA:  Not Altera chip ID\n");
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(socfpga_fpga_model); i++)
		if (pn == socfpga_fpga_model[i].pn)
			break;

	if (i == ARRAY_SIZE(socfpga_fpga_model)) {
		printf("FPGA:  Unknown Altera chip, ID 0x%08x\n", id);
		return -EINVAL;
	}

	if (print_id)
		printf("FPGA:  Altera %s, version 0x%01x\n",
		       socfpga_fpga_model[i].name, version);
	return i;
}

/*
 * Print CPU information
 */
#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	const u32 bsel =
		SYSMGR_GET_BOOTINFO_BSEL(readl(&sysmgr_regs->bootinfo));

	puts("CPU:   Altera SoCFPGA Platform\n");
	socfpga_fpga_id(1);

	printf("BOOT:  %s\n", bsel_str[bsel].name);
	return 0;
}
#endif

#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
	const u32 bsel = readl(&sysmgr_regs->bootinfo) & 0x7;
	const int fpga_id = socfpga_fpga_id(0);
	setenv("bootmode", bsel_str[bsel].mode);
	if (fpga_id >= 0)
		setenv("fpgatype", socfpga_fpga_model[fpga_id].var);
	return socfpga_eth_reset();
}
#endif

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

static u32 iswgrp_handoff[8];

int arch_early_init_r(void)
{
	int i;

	/*
	 * Write magic value into magic register to unlock support for
	 * issuing warm reset. The ancient kernel code expects this
	 * value to be written into the register by the bootloader, so
	 * to support that old code, we write it here instead of in the
	 * reset_cpu() function just before resetting the CPU.
	 */
	writel(0xae9efebc, &sysmgr_regs->romcodegrp_warmramgrp_enable);

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
	socfpga_per_reset(SOCFPGA_RESET(SPIM0), 0);
	socfpga_per_reset(SOCFPGA_RESET(SPIM1), 0);
#endif

#ifdef CONFIG_NAND_DENALI
	socfpga_per_reset(SOCFPGA_RESET(NAND), 0);
#endif

	return 0;
}

static void socfpga_sdram_apply_static_cfg(void)
{
	const u32 applymask = 0x8;
	u32 val = readl(&sdr_ctrl->static_cfg) | applymask;

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
	: : "r"(val), "r"(&sdr_ctrl->static_cfg) : "memory", "cc");
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
		writel(iswgrp_handoff[3], &sdr_ctrl->fpgaport_rst);
		writel(iswgrp_handoff[0], &reset_manager_base->brg_mod_reset);
		writel(iswgrp_handoff[1], &nic301_regs->remap);
		break;
	case 'd':	/* Disable */
		writel(0, &sysmgr_regs->fpgaintfgrp_module);
		writel(0, &sdr_ctrl->fpgaport_rst);
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
