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

#include "tqma6_emmc.h"
#include "../common/tq_bb.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Rev. 0200 and newer optionally implements GIGE errata fix.
 * Use a global var to signal presence of the fix. This should be checked
 * early. If fix is present, system I2C is I2C1 - otherwise I2C3
 */
static int has_enet_workaround = -1;

int tqma6_has_enet_workaround(void)
{
	return has_enet_workaround;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

#define GPIO_REVDET_PAD_CTRL  (PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_LOW | \
			       PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

static const iomux_v3_cfg_t tqma6_revdet_pads[] = {
	MX6_PAD_GPIO_6__GPIO1_IO06 | MUX_PAD_CTRL(GPIO_REVDET_PAD_CTRL),
};

void tqma6_detect_enet_workaround(void)
{
	int ret;
	struct gpio_desc desc;

	imx_iomux_v3_setup_multiple_pads(tqma6_revdet_pads,
					 ARRAY_SIZE(tqma6_revdet_pads));

	ret = dm_gpio_lookup_name("GPIO1_6", &desc);
	if (ret) {
		pr_err("error: gpio lookup for enet workaround %d\n", ret);
		return;
	}

	ret = dm_gpio_get_value(&desc);
	if (ret == 0)
		has_enet_workaround = 1;
	else if (ret > 0)
		has_enet_workaround = 0;

	dm_gpio_free(NULL, &desc);
}

int board_early_init_f(void)
{
	return tq_bb_board_early_init_f();
}

int board_init(void)
{
	struct mmc *mmc = find_mmc_device(0);

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	tqma6_mmc_detect_card_type(mmc);

	tq_bb_board_init();

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

	tqma6_detect_enet_workaround();

	tq_bb_board_late_init();

	printf("Board: %s on a %s\n", tqma6_get_boardname(),
	       tq_bb_get_boardname());

	puts("Enet workaround: ");
	switch (tqma6_has_enet_workaround()) {
	case 0:
		puts("absent");
		break;
	case 1:
		puts("implemented");
		break;
	default:
		puts("Unknown");
		break;
	};
	puts("\n");

	return tq_bb_checkboard();
}

/*
 * Device Tree Support
 */
#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
#define MODELSTRLEN 32u
int ft_board_setup(void *blob, struct bd_info *bd)
{
	struct mmc *mmc = find_mmc_device(0);
	char modelstr[MODELSTRLEN];

	snprintf(modelstr, MODELSTRLEN, "TQ %s on %s", tqma6_get_boardname(),
		 tq_bb_get_boardname());
	do_fixup_by_path_string(blob, "/", "model", modelstr);
	fdt_fixup_memory(blob, (u64)PHYS_SDRAM, (u64)gd->ram_size);

	/* bring in eMMC dsr settings if needed */
	if (mmc && (!mmc_init(mmc))) {
		if (tqma6_emmc_need_dsr(mmc) > 0) {
			tqma6_ft_fixup_emmc_dsr(blob,
						"/soc/bus@2100000/mmc@2198000",
						TQMA6_EMMC_DSR);
		}
	} else {
		puts("eMMC: not present?\n");
	}

	return 0;
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */
