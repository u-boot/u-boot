/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __RD6281A_H
#define __RD6281A_H

#define RD6281A_OE_LOW			(~(1 << 7))
#define RD6281A_OE_HIGH			(~(1 << 2 | 1 << 12))
#define RD6281A_OE_VAL_LOW		(0)
#define RD6281A_OE_VAL_HIGH		(1 << 12)

/* PHY related */
#define MV88E1116_LED_FCTRL_REG		10
#define MV88E1116_CPRSP_CR3_REG		21
#define MV88E1116_MAC_CTRL_REG		21
#define MV88E1116_PGADR_REG		22
#define MV88E1116_RGMII_TXTM_CTRL	(1 << 4)
#define MV88E1116_RGMII_RXTM_CTRL	(1 << 5)

#endif /* __RD6281A_H */
