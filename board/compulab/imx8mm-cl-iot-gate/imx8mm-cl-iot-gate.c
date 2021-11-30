// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 * Copyright 2020 Linaro
 */

#include <common.h>
#include <env.h>
#include <hang.h>
#include <init.h>
#include <miiphy.h>
#include <netdev.h>

#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>

#include "ddr/ddr.h"

DECLARE_GLOBAL_DATA_PTR;

int board_phys_sdram_size(phys_size_t *size)
{
	struct lpddr4_tcm_desc *lpddr4_tcm_desc =
		(struct lpddr4_tcm_desc *)TCM_DATA_CFG;

	switch (lpddr4_tcm_desc->size) {
	case 4096:
	case 2048:
	case 1024:
		*size = (1L << 20) * lpddr4_tcm_desc->size;
		break;
	default:
		printf("%s: DRAM size %uM is not supported\n",
		       __func__,
		       lpddr4_tcm_desc->size);
		hang();
		break;
	};

	return 0;
}

static int setup_fec(void)
{
	if (IS_ENABLED(CONFIG_FEC_MXC)) {
		struct iomuxc_gpr_base_regs *gpr =
			(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

		/* Use 125M anatop REF_CLK1 for ENET1, not from external */
		clrsetbits_le32(&gpr->gpr[1], 0x2000, 0);
	}

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	if (IS_ENABLED(CONFIG_FEC_MXC)) {
		/* enable rgmii rxc skew and phy mode select to RGMII copper */
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x8);

		phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x00);
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x82ee);
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x05);
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

		if (phydev->drv->config)
			phydev->drv->config(phydev);
	}
	return 0;
}

int board_init(void)
{
	if (IS_ENABLED(CONFIG_FEC_MXC))
		setup_fec();

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		env_set("board_name", "IOT-GATE-IMX8");
		env_set("board_rev", "SBC-IOTMX8");
	}

	return 0;
}
