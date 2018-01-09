/*
 * (C) Copyright 2016 - Beniamino Galvani <b.galvani@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __GXBB_H__
#define __GXBB_H__

#define GXBB_FIRMWARE_MEM_SIZE	0x1000000

#define GXBB_AOBUS_BASE		0xc8100000
#define GXBB_PERIPHS_BASE	0xc8834400
#define GXBB_HIU_BASE		0xc883c000
#define GXBB_ETH_BASE		0xc9410000

/* Always-On Peripherals registers */
#define GXBB_AO_ADDR(off)	(GXBB_AOBUS_BASE + ((off) << 2))

#define GXBB_AO_SEC_GP_CFG0	GXBB_AO_ADDR(0x90)
#define GXBB_AO_SEC_GP_CFG3	GXBB_AO_ADDR(0x93)
#define GXBB_AO_SEC_GP_CFG4	GXBB_AO_ADDR(0x94)
#define GXBB_AO_SEC_GP_CFG5	GXBB_AO_ADDR(0x95)

#define GXBB_AO_MEM_SIZE_MASK	0xFFFF0000
#define GXBB_AO_MEM_SIZE_SHIFT	16
#define GXBB_AO_BL31_RSVMEM_SIZE_MASK	0xFFFF0000
#define GXBB_AO_BL31_RSVMEM_SIZE_SHIFT	16
#define GXBB_AO_BL32_RSVMEM_SIZE_MASK	0xFFFF

/* Peripherals registers */
#define GXBB_PERIPHS_ADDR(off)	(GXBB_PERIPHS_BASE + ((off) << 2))

/* GPIO registers 0 to 6 */
#define _GXBB_GPIO_OFF(n)	((n) == 6 ? 0x08 : 0x0c + 3 * (n))
#define GXBB_GPIO_EN(n)		GXBB_PERIPHS_ADDR(_GXBB_GPIO_OFF(n) + 0)
#define GXBB_GPIO_IN(n)		GXBB_PERIPHS_ADDR(_GXBB_GPIO_OFF(n) + 1)
#define GXBB_GPIO_OUT(n)	GXBB_PERIPHS_ADDR(_GXBB_GPIO_OFF(n) + 2)

#define GXBB_ETH_REG_0		GXBB_PERIPHS_ADDR(0x50)
#define GXBB_ETH_REG_1		GXBB_PERIPHS_ADDR(0x51)
#define GXBB_ETH_REG_2		GXBB_PERIPHS_ADDR(0x56)
#define GXBB_ETH_REG_3		GXBB_PERIPHS_ADDR(0x57)

#define GXBB_ETH_REG_0_PHY_INTF		BIT(0)
#define GXBB_ETH_REG_0_TX_PHASE(x)	(((x) & 3) << 5)
#define GXBB_ETH_REG_0_TX_RATIO(x)	(((x) & 7) << 7)
#define GXBB_ETH_REG_0_PHY_CLK_EN	BIT(10)
#define GXBB_ETH_REG_0_INVERT_RMII_CLK	BIT(11)
#define GXBB_ETH_REG_0_CLK_EN		BIT(12)

/* HIU registers */
#define GXBB_HIU_ADDR(off)	(GXBB_HIU_BASE + ((off) << 2))

#define GXBB_MEM_PD_REG_0	GXBB_HIU_ADDR(0x40)

/* Ethernet memory power domain */
#define GXBB_MEM_PD_REG_0_ETH_MASK	(BIT(2) | BIT(3))

/* Clock gates */
#define GXBB_GCLK_MPEG_0	GXBB_HIU_ADDR(0x50)
#define GXBB_GCLK_MPEG_1	GXBB_HIU_ADDR(0x51)
#define GXBB_GCLK_MPEG_2	GXBB_HIU_ADDR(0x52)
#define GXBB_GCLK_MPEG_OTHER	GXBB_HIU_ADDR(0x53)
#define GXBB_GCLK_MPEG_AO	GXBB_HIU_ADDR(0x54)

#define GXBB_GCLK_MPEG_0_I2C   BIT(9)
#define GXBB_GCLK_MPEG_1_ETH	BIT(3)

#endif /* __GXBB_H__ */
