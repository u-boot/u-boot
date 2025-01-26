/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Sunxi H6 Power Management Unit register definition.
 *
 * (C) Copyright 2020 Jernej Skrabec <jernej.skrabec@siol.net>
 */

#ifndef _SUN50I_PRCM_H
#define _SUN50I_PRCM_H

#ifndef __ASSEMBLY__

#define CCU_PRCM_I2C_GATE_RESET		0x19c
#define CCU_PRCM_PLL_LDO_CFG		0x244
#define CCU_PRCM_SYS_PWROFF_GATING	0x250
#define CCU_PRCM_RES_CAL_CTRL		0x310
#define CCU_PRCM_OHMS240		0x318

#define PRCM_TWI_GATE		(1 << 0)
#define PRCM_TWI_RESET		(1 << 16)

#endif /* __ASSEMBLY__ */
#endif /* _PRCM_H */
