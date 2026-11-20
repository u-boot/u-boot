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

#include <dm.h>
#include <env.h>
#include <fdt_support.h>
#include <mmc.h>
#include <sysinfo.h>
#include <power/pfuze100_pmic.h>
#include <power/pmic.h>

#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/mxc_i2c.h>

#include "tqma6_emmc.h"
#include "../common/tq_bb.h"
#include "../common/tq_sysinfo.h"

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

#define GPIO_REVDET_PAD_CTRL  (PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_LOW | \
			       PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

static const iomux_v3_cfg_t tqma6_revdet_pads[] = {
	MX6_PAD_GPIO_6__GPIO1_IO06 | MUX_PAD_CTRL(GPIO_REVDET_PAD_CTRL),
};

static void tqma6_i2c_idle_bus(int seq)
{
	struct mxc_i2c_bus *i2c_bus;
	struct udevice *bus;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C, seq, &bus);
	if (ret) {
		printf("Failed to get I2C bus %d: %d\n", seq, ret);
		return;
	}

	i2c_bus = dev_get_priv(bus);
	if (!i2c_bus) {
		printf("Failed to get private data for I2C bus %d\n", seq);
		return;
	}

	ret = i2c_idle_bus(i2c_bus);
	if (ret) {
		printf("Failed to recover I2C bus %d: %d\n",
		       seq, ret);
		return;
	}
}

#define TQMA6_REVDET_GPIO IMX_GPIO_NR(1, 6)

void tqma6_detect_enet_workaround(void)
{
	int ret;

	imx_iomux_v3_setup_multiple_pads(tqma6_revdet_pads,
					 ARRAY_SIZE(tqma6_revdet_pads));

	if (IS_ENABLED(CONFIG_SPL_BUILD)) {
		ret = gpio_request(TQMA6_REVDET_GPIO, "tqma6-revdet");
		if (ret) {
			pr_err("error: gpio request for enet workaround %d\n", ret);
			return;
		}

		if (gpio_get_value(TQMA6_REVDET_GPIO) == 0)
			has_enet_workaround = 1;
		else if (gpio_get_value(TQMA6_REVDET_GPIO) > 0)
			has_enet_workaround = 0;

		gpio_free(TQMA6_REVDET_GPIO);
	} else {
		struct gpio_desc desc;

		ret = dm_gpio_lookup_name("GPIO1_6", &desc);
		if (ret) {
			pr_err("error: gpio lookup for enet workaround %d\n", ret);
			return;
		}

		ret = dm_gpio_request(&desc, "tqma6-revdet");
		if (ret) {
			if (ret == -EBUSY) {
				has_enet_workaround = 0;
				tqma6_i2c_idle_bus(2);
			} else {
				pr_err("error: gpio request for enet workaround %d\n", ret);
			}
			return;
		}

		ret = dm_gpio_get_value(&desc);
		if (ret == 0)
			has_enet_workaround = 1;
		else if (ret > 0)
			has_enet_workaround = 0;

		dm_gpio_free(NULL, &desc);
	}
}

const char *tqma6_get_fdt_configuration(void)
{
	tqma6_detect_enet_workaround();

	if (is_mx6dq())
		return !tqma6_has_enet_workaround() ? "imx6q-mba6b" : "imx6q-mba6a";
	if (is_mx6dqp())
		return "imx6qp-mba6b";
	if (is_mx6sdl())
		return !tqma6_has_enet_workaround() ? "imx6dl-mba6b" : "imx6dl-mba6a";

	return NULL;
}

int board_early_init_f(void)
{
	tq_bb_board_early_init_f();

	return 0;
}

#if !IS_ENABLED(CONFIG_SPL_BUILD)

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
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
	case MXC_CPU_MX6DP:
		return "TQMa6DP";
	case MXC_CPU_MX6Q:
		return "TQMa6Q";
	case MXC_CPU_MX6QP:
		return "TQMa6QP";
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
#define FDTFILE_STRLEN 32u
int board_late_init(void)
{
	char fdtfile[FDTFILE_STRLEN];
	const char *config = tqma6_get_fdt_configuration();
	int ret;

	env_set("board_name", tqma6_get_boardname());

	if (!env_get("fdtfile")) {
		if (config) {
			snprintf(fdtfile, FDTFILE_STRLEN, "%s.dtb", config);
			env_set_runtime("fdtfile", fdtfile);
		} else {
			pr_err("ENV: Could not set kernel devicetree, ${fdtfile} remains unset\n");
		}
	}

	ret = tq_common_sysinfo_setup();
	if (ret)
		pr_err("Failed to get sysinfo data: %d\n", ret);

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
#if IS_ENABLED(CONFIG_OF_BOARD_SETUP) && IS_ENABLED(CONFIG_OF_LIBFDT)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	struct mmc *mmc = find_mmc_device(0);

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
#endif /* IS_ENABLED(CONFIG_OF_BOARD_SETUP) && IS_ENABLED(CONFIG_OF_LIBFDT) */

#endif /* !IS_ENABLED(CONFIG_SPL_BUILD) */
