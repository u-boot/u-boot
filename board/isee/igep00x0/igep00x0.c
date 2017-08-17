/*
 * (C) Copyright 2010
 * ISEE 2007 SL, <www.iseebcn.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <status_led.h>
#include <dm.h>
#include <ns16550.h>
#include <twl4030.h>
#include <netdev.h>
#include <spl.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/mem.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-types.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/onenand.h>
#include <jffs2/load_kernel.h>
#include <mtd_node.h>
#include <fdt_support.h>
#include "igep00x0.h"

static const struct ns16550_platdata igep_serial = {
	.base = OMAP34XX_UART3,
	.reg_shift = 2,
	.clock = V_NS16550_CLK,
	.fcr = UART_FCR_DEFVAL,
};

U_BOOT_DEVICE(igep_uart) = {
	"ns16550_serial",
	&igep_serial
};

int onenand_board_init(struct mtd_info *mtd)
{
	if (gpmc_cs0_flash == MTD_DEV_TYPE_ONENAND) {
		struct onenand_chip *this = mtd->priv;
		this->base = (void *)CONFIG_SYS_ONENAND_BASE;
		return 0;
	}
	return 1;
}

#if defined(CONFIG_CMD_NET)
static void reset_net_chip(int gpio)
{
	if (!gpio_request(gpio, "eth nrst")) {
		gpio_direction_output(gpio, 1);
		udelay(1);
		gpio_set_value(gpio, 0);
		udelay(40);
		gpio_set_value(gpio, 1);
		mdelay(10);
	}
}

/*
 * Routine: setup_net_chip
 * Description: Setting up the configuration GPMC registers specific to the
 *		Ethernet hardware.
 */
static void setup_net_chip(void)
{
	struct ctrl *ctrl_base = (struct ctrl *)OMAP34XX_CTRL_BASE;
	static const u32 gpmc_lan_config[] = {
		NET_LAN9221_GPMC_CONFIG1,
		NET_LAN9221_GPMC_CONFIG2,
		NET_LAN9221_GPMC_CONFIG3,
		NET_LAN9221_GPMC_CONFIG4,
		NET_LAN9221_GPMC_CONFIG5,
		NET_LAN9221_GPMC_CONFIG6,
	};

	enable_gpmc_cs_config(gpmc_lan_config, &gpmc_cfg->cs[5],
			CONFIG_SMC911X_BASE, GPMC_SIZE_16M);

	/* Enable off mode for NWE in PADCONF_GPMC_NWE register */
	writew(readw(&ctrl_base->gpmc_nwe) | 0x0E00, &ctrl_base->gpmc_nwe);
	/* Enable off mode for NOE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_noe) | 0x0E00, &ctrl_base->gpmc_noe);
	/* Enable off mode for ALE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_nadv_ale) | 0x0E00,
		&ctrl_base->gpmc_nadv_ale);

	reset_net_chip(64);
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_SMC911X
	return smc911x_initialize(0, CONFIG_SMC911X_BASE);
#else
	return 0;
#endif
}
#else
static inline void setup_net_chip(void) {}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
static int ft_enable_by_compatible(void *blob, char *compat, int enable)
{
	int off = fdt_node_offset_by_compatible(blob, -1, compat);
	if (off < 0)
		return off;

	if (enable)
		fdt_status_okay(blob, off);
	else
		fdt_status_disabled(blob, off);

	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_FDT_FIXUP_PARTITIONS
	static struct node_info nodes[] = {
		{ "ti,omap2-nand", MTD_DEV_TYPE_NAND, },
		{ "ti,omap2-onenand", MTD_DEV_TYPE_ONENAND, },
	};

	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
#endif
	ft_enable_by_compatible(blob, "ti,omap2-nand",
				gpmc_cs0_flash == MTD_DEV_TYPE_NAND);
	ft_enable_by_compatible(blob, "ti,omap2-onenand",
				gpmc_cs0_flash == MTD_DEV_TYPE_ONENAND);

	return 0;
}
#endif

void set_fdt(void)
{
	switch (gd->bd->bi_arch_number) {
	case MACH_TYPE_IGEP0020:
		env_set("fdtfile", "omap3-igep0020.dtb");
		break;
	case MACH_TYPE_IGEP0030:
		env_set("fdtfile", "omap3-igep0030.dtb");
		break;
	}
}

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	twl4030_power_init();

	setup_net_chip();

	omap_die_id_display();

	set_fdt();

	return 0;
}

void board_mtdparts_default(const char **mtdids, const char **mtdparts)
{
	struct mtd_info *mtd = get_mtd_device(NULL, 0);
	if (mtd) {
		static char ids[24];
		static char parts[48];
		const char *linux_name = "omap2-nand";
		if (strncmp(mtd->name, "onenand0", 8) == 0)
			linux_name = "omap2-onenand";
		snprintf(ids, sizeof(ids), "%s=%s", mtd->name, linux_name);
		snprintf(parts, sizeof(parts), "mtdparts=%s:%dk(SPL),-(UBI)",
		         linux_name, 4 * mtd->erasesize >> 10);
		*mtdids = ids;
		*mtdparts = parts;
	}
}
