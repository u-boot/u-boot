/*
 * multiverse.h
 *
 * VME driver for Multiverse
 *
 * Author : Sangmoon Kim
 *	    dogoil@etinsys.com
 *
 * Copyright 2005 ETIN SYSTEMS Co.,Ltd.
 *
 * This program is free software; you can redistribute	it and/or modify it
 * under  the terms of	the GNU General Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#ifndef __MULTIVERSE_H__
#define __MULTIVERSE_H__

#define VME_A32_MSTR_BUS	0x90000000
#define VME_A32_MSTR_SIZE	0x01000000

#define VME_A32_SLV_SIZE	0x01000000

#define VME_A32_SLV_BUS		0x90000000
#define VME_A24_SLV_BUS		0x00000000
#define VME_A16_SLV_BUS		0x00000000

#define VME_A32_SLV_LOCAL	0x00000000
#define VME_A24_SLV_LOCAL	0x00000000
#define VME_A16_SLV_LOCAL	0x00000000

#define A32_SLV_WINDOW
#undef	A24_SLV_WINDOW
#undef	A16_SLV_WINDOW
#undef	REG_SLV_WINDOW

/* PCI Registers */

#define P_IMG_CTRL0		0x100
#define P_BA0			0x104
#define P_AM0			0x108
#define P_TA0			0x10C
#define P_IMG_CTRL1		0x110
#define P_BA1			0x114
#define P_AM1			0x118
#define P_TA1			0x11C
#define P_IMG_CTRL2		0x120
#define P_BA2			0x124
#define P_AM2			0x128
#define P_TA2			0x12C
#define P_IMG_CTRL3		0x130
#define P_BA3			0x134
#define P_AM3			0x138
#define P_TA3			0x13C
#define P_IMG_CTRL4		0x140
#define P_BA4			0x144
#define P_AM4			0x148
#define P_TA4			0x14C
#define P_IMG_CTRL5		0x150
#define P_BA5			0x154
#define P_AM5			0x158
#define P_TA5			0x15C
#define P_ERR_CS		0x160
#define P_ERR_ADDR		0x164
#define P_ERR_DATA		0x168

#define WB_CONF_SPC_BAR		0x180
#define W_IMG_CTRL1		0x184
#define W_BA1			0x188
#define W_AM1			0x18C
#define W_TA1			0x190
#define W_IMG_CTRL2		0x194
#define W_BA2			0x198
#define W_AM2			0x19C
#define W_TA2			0x1A0
#define W_IMG_CTRL3		0x1A4
#define W_BA3			0x1A8
#define W_AM3			0x1AC
#define W_TA3			0x1B0
#define W_IMG_CTRL4		0x1B4
#define W_BA4			0x1B8
#define W_AM4			0x1BC
#define W_TA4			0x1C0
#define W_IMG_CTRL5		0x1C4
#define W_BA5			0x1C8
#define W_AM5			0x1CC
#define W_TA5			0x1D0
#define W_ERR_CS		0x1D4
#define W_ERR_ADDR		0x1D8
#define W_ERR_DATA		0x1DC
#define CNF_ADDR		0x1E0
#define CNF_DATA		0x1E4
#define INT_ACK			0x1E8
#define ICR			0x1EC
#define ISR			0x1F0

/* VME registers */

#define VME_SLAVE32_AM		0x03
#define VME_SLAVE24_AM		0x02
#define VME_SLAVE16_AM		0x01
#define VME_SLAVE_REG_AM	0x00
#define VME_SLAVE32_A		0x07
#define VME_SLAVE24_A		0x06
#define VME_SLAVE16_A		0x05
#define VME_SLAVE_REG_A		0x04
#define VME_SLAVE32_MASK	0x0B
#define VME_SLAVE24_MASK	0x0A
#define VME_SLAVE16_MASK	0x09
#define VME_SLAVE_REG_MASK	0x08
#define VME_SLAVE32_EN		0x0F
#define VME_SLAVE24_EN		0x0E
#define VME_SLAVE16_EN		0x0D
#define VME_SLAVE_REG_EN	0x0C
#define VME_MASTER32_AM		0x13
#define VME_MASTER24_AM		0x12
#define VME_MASTER16_AM		0x11
#define VME_MASTER_REG_AM	0x10
#define VME_RMW_ADRS		0x14
#define VME_MBOX		0x18
#define VME_STATUS		0x1E
#define VME_CTRL		0x1C
#define VME_IRQ			0x20
#define VME_INT_EN		0x21
#define VME_INT			0x22
#define VME_IRQ1_REG		0x24
#define VME_IRQ2_REG		0x28
#define VME_IRQ3_REG		0x2C
#define VME_IRQ4_REG		0x30
#define VME_IRQ5_REG		0x34
#define VME_IRQ6_REG		0x38
#define VME_IRQ7_REG		0x3C

/* VME control register */

#define VME_CTRL_BRDRST		0x01
#define VME_CTRL_SYSRST		0x02
#define VME_CTRL_RMW		0x04
#define VME_CTRL_SHORT_D	0x08
#define VME_CTRL_SYSFAIL	0x10
#define VME_CTRL_VOWN		0x20
#define VME_CTRL_A16_REG_MODE	0x40

/* VME status register */

#define VME_STATUS_SYSCON	0x01
#define VME_STATUS_SYSFAIL	0x02
#define VME_STATUS_ACFAIL	0x04
#define VME_STATUS_SYSRST	0x08
#define VME_STATUS_VOWN		0x10

/* Interrupt types */

#define LVL1			0x0002
#define LVL2			0x0004
#define LVL3			0x0008
#define LVL4			0x0010
#define LVL5			0x0020
#define LVL6			0x0040
#define LVL7			0x0080
#define MULTIVERSE_INTI_INT	0x0100
#define MULTIVERSE_WB_INT	0x0200
#define MULTIVERSE_PCI_INT	0x0400

/* interrupt acknowledge */

#define VME_IACK1		0x04
#define VME_IACK2		0x08
#define VME_IACK3		0x0c
#define VME_IACK4		0x10
#define VME_IACK5		0x14
#define VME_IACK6		0x18
#define VME_IACK7		0x1c

#endif /* __MULTIVERSE_H__ */
