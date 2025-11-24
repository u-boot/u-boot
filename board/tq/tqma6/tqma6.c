// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2012 Freescale Semiconductor, Inc.
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * ported SabreSD to TQMa6x
 * Copyright (c) 2013-2014 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/sys_proto.h>
#include <env.h>
#include <fdt_support.h>
#include <asm/global_data.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/libfdt.h>
#include <mmc.h>
#include <power/pfuze100_pmic.h>
#include <power/pmic.h>

#include "tqma6_bb.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

static const uint16_t tqma6_emmc_dsr = 0x0100;

int board_early_init_f(void)
{
	return tqma6_bb_board_early_init_f();
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	tqma6_bb_board_init();

	return 0;
}

static const char *tqma6_get_boardname(void)
{
	u32 cpurev = get_cpu_rev();

	switch ((cpurev & 0xFF000) >> 12) {
	case MXC_CPU_MX6SOLO:
		return "TQMa6S";
	case MXC_CPU_MX6DL:
		return "TQMa6DL";
	case MXC_CPU_MX6D:
		return "TQMa6D";
	case MXC_CPU_MX6Q:
		return "TQMa6Q";
	default:
		return "??";
	};
}

#if CONFIG_IS_ENABLED(DM_PMIC)
/* setup board specific PMIC */
int power_init_board(void)
{
	struct udevice *dev;
	u32 reg, rev;
	int ret;

	ret = pmic_get("pmic@8", &dev);
	if (ret < 0)
		return 0;

	reg = pmic_reg_read(dev, PFUZE100_DEVICEID);
	rev = pmic_reg_read(dev, PFUZE100_REVID);

	printf("PMIC:  PFUZE100 ID=0x%02x REV=0x%02x\n", reg, rev);
	return 0;
}
#endif

int board_late_init(void)
{
	env_set("board_name", tqma6_get_boardname());

	tqma6_bb_board_late_init();

	printf("Board: %s on a %s\n", tqma6_get_boardname(),
	       tqma6_bb_get_boardname());
	return 0;
}

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
#define MODELSTRLEN 32u
int ft_board_setup(void *blob, struct bd_info *bd)
{
	char modelstr[MODELSTRLEN];

	snprintf(modelstr, MODELSTRLEN, "TQ %s on %s", tqma6_get_boardname(),
		 tqma6_bb_get_boardname());
	do_fixup_by_path_string(blob, "/", "model", modelstr);
	fdt_fixup_memory(blob, (u64)PHYS_SDRAM, (u64)gd->ram_size);
	/* bring in eMMC dsr settings */
	do_fixup_by_path_u32(blob,
			     "/soc/aips-bus@02100000/usdhc@02198000",
			     "dsr", tqma6_emmc_dsr, 2);
	tqma6_bb_ft_board_setup(blob, bd);

	return 0;
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
