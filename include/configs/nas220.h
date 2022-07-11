/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Evgeni Dobrev <evgeni@studio-punkt.com>
 *
 * based on work from:
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 */

#ifndef _CONFIG_NAS220_H
#define _CONFIG_NAS220_H

/* power-on led, regulator, sata0, sata1 */
#define NAS220_GE_OE_VAL_LOW ((1 << 12)|(1 << 14)|(1 << 24)|(1 << 28))
#define NAS220_GE_OE_VAL_HIGH (0)
#define NAS220_GE_OE_LOW (~((1 << 12)|(1 << 14)|(1 << 24)|(1 << 28)))
#define NAS220_GE_OE_HIGH (~(0))

/* PHY related */
#define MV88E1116_LED_FCTRL_REG		10
#define MV88E1116_CPRSP_CR3_REG		21
#define MV88E1116_MAC_CTRL_REG		21
#define MV88E1116_PGADR_REG		22
#define MV88E1116_RGMII_TXTM_CTRL	(1 << 4)
#define MV88E1116_RGMII_RXTM_CTRL	(1 << 5)

#include "mv-common.h"

/*
 *  Environment variables configurations
 */

/*
 * Default environment variables
 */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootargs=console=ttyS0,115200\0" \
	"autostart=no\0"

/*
 * Ethernet Driver configuration
 */
#ifdef CONFIG_CMD_NET
#define CONFIG_MVGBE_PORTS {1, 0}	/* enable port 0 only */
#define CONFIG_PHY_BASE_ADR 8
#endif /* CONFIG_CMD_NET */

/*
 * EFI partition
 */

#endif /* _CONFIG_NAS220_H */
