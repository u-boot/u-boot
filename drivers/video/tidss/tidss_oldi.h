/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 * Swamil Jain <s-jain1@ti.com>
 *
 * based on the linux tidss_oldi.c, which is
 *
 * Copyright (C) 2024 - Texas Instruments Incorporated
 * Author: Aradhya Bhatia <a-bhatia1@ti.com>
 */

#ifndef __TIDSS_OLDI_H__
#define __TIDSS_OLDI_H__

#include <dm/ofnode.h>
#include <dm/of_access.h>

#include "tidss_drv.h"

/* OLDI PORTS */
#define OLDI_INPUT_PORT    0
#define OLDI_OURPUT_PORT   1

/* Control MMR Registers */

/* Register offsets */
#define OLDI_PD_CTRL            0x100
#define OLDI_LB_CTRL            0x104

/* Power control bits */
#define OLDI_PWRDOWN_TX(n)	BIT(n)

/* LVDS Bandgap reference Enable/Disable */
#define OLDI_PWRDN_BG		BIT(8)

/*
 * OLDI IO_CTRL register offsets. On AM654 the registers are found
 * from CTRL_MMR0, there the syscon regmap should map 0x14 bytes from
 * CTRLMMR0P1_OLDI_DAT0_IO_CTRL to CTRLMMR0P1_OLDI_CLK_IO_CTRL
 * register range.
 */
#define OLDI_DAT0_IO_CTRL			0x00
#define OLDI_DAT1_IO_CTRL			0x04
#define OLDI_DAT2_IO_CTRL			0x08
#define OLDI_DAT3_IO_CTRL			0x0C
#define OLDI_CLK_IO_CTRL			0x10

/* Only for AM625 OLDI TX */
#define OLDI_PD_CTRL				0x100
#define OLDI_LB_CTRL				0x104

#define OLDI_BANDGAP_PWR			BIT(8)
#define OLDI_PWRDN_TX				BIT(8)
#define OLDI0_PWRDN_TX				BIT(0)
#define OLDI1_PWRDN_TX				BIT(1)

struct tidss_oldi {
	struct udevice *dev;

	enum tidss_oldi_link_type link_type;
	u32 oldi_instance;
	u32 companion_instance;
	u32 parent_vp;

	struct clk *serial;
	struct regmap *io_ctrl;
};

int tidss_oldi_init(struct udevice *dev);
void dss_oldi_tx_power(struct tidss_drv_priv *priv, bool power);

#endif /* __TIDSS_OLDI_H__ */
