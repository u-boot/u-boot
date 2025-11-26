/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 - Beniamino Galvani <b.galvani@gmail.com>
 */

#ifndef __GX_H__
#define __GX_H__

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#define GX_FIRMWARE_MEM_SIZE	0x1000000

#define GX_MB_SRAM_BASE		0xd9013800
#define GX_AOBUS_BASE		0xc8100000
#define GX_SEC_HIU_MB_BASE	0xda83c400
#define GX_SEC_AOBUS_BASE	0xda100000
#define GX_PERIPHS_BASE	0xc8834400
#define GX_HIU_BASE		0xc883c000
#define GX_ETH_BASE		0xc9410000

/* Always-On Peripherals registers */
#define GX_AO_ADDR(off)	(GX_AOBUS_BASE + ((off) << 2))

#define GX_AO_SEC_GP_CFG0	GX_AO_ADDR(0x90)
#define GX_AO_SEC_GP_CFG3	GX_AO_ADDR(0x93)
#define GX_AO_SEC_GP_CFG4	GX_AO_ADDR(0x94)
#define GX_AO_SEC_GP_CFG5	GX_AO_ADDR(0x95)
#define GX_AO_SEC_SD_CFG15	GX_AO_ADDR(0x8f)

#define GX_SEC_AO_ADDR(off)	(GX_SEC_AOBUS_BASE + ((off) << 2))
#define GX_SEC_AO_SEC_GP_CFG0	GX_SEC_AO_ADDR(0x90)

#define GX_AO_BOOT_DEVICE	0xF
#define GX_AO_MEM_SIZE_MASK	0xFFFF0000
#define GX_AO_MEM_SIZE_SHIFT	16
#define GX_AO_BL31_RSVMEM_SIZE_MASK	0xFFFF0000
#define GX_AO_BL31_RSVMEM_SIZE_SHIFT	16
#define GX_AO_BL32_RSVMEM_SIZE_MASK	0xFFFF

/* Peripherals registers */
#define GX_PERIPHS_ADDR(off)	(GX_PERIPHS_BASE + ((off) << 2))

/* GPIO registers 0 to 6 */
#define _GX_GPIO_OFF(n)	((n) == 6 ? 0x08 : 0x0c + 3 * (n))
#define GX_GPIO_EN(n)		GX_PERIPHS_ADDR(_GX_GPIO_OFF(n) + 0)
#define GX_GPIO_IN(n)		GX_PERIPHS_ADDR(_GX_GPIO_OFF(n) + 1)
#define GX_GPIO_OUT(n)	GX_PERIPHS_ADDR(_GX_GPIO_OFF(n) + 2)

/* Mailbox registers */
#define GX_SEC_HIU_MB_ADDR(off)	(GX_SEC_HIU_MB_BASE + ((off) << 2))
#define GX_SEC_HIU_MAILBOX_SET_0	GX_SEC_HIU_MB_ADDR(0x01)
#define GX_SEC_HIU_MAILBOX_STAT_0	GX_SEC_HIU_MB_ADDR(0x02)
#define GX_SEC_HIU_MAILBOX_CLR_0	GX_SEC_HIU_MB_ADDR(0x03)

/* Mailbox commands */
#define GX_MB_CMD_SHA	0xc0de0001
#define GX_MB_CMD_DATA	0xc0dec0de
#define GX_MB_CMD_END	0xe00de00d
#define GX_MB_CMD_OP_SHA	0xc0de0002
#define GX_MB_CMD_DATA_LEN	0xc0dec0d0

/* PIN_MUX registers */
#define GX_PIN_MUX_REG1		(0xda834400 + (0x2d << 2))
#define GX_PIN_MUX_REG2		(0xda834400 + (0x2e << 2))
#define GX_PIN_MUX_REG3		(0xda834400 + (0x2f << 2))
#define GX_PIN_MUX_REG7		(0xda834400 + (0x33 << 2))

/* PWM registers */
#define GX_PWM_PWM_B	(0xc1100000 + (0x2155 << 2))
#define GX_PWM_PWM_D	(0xc1100000 + (0x2195 << 2))
#define GX_PWM_MISC_REG_CD	(0xc1100000 + (0x2196 << 2))
#define GX_PWM_MISC_REG_AB	(0xc1100000 + (0x2156 << 2))

/* Non-DM MMC init */
#if CONFIG_IS_ENABLED(MMC) && !CONFIG_IS_ENABLED(DM_MMC)
struct mmc *meson_mmc_init(int mmc_no);
#endif

#if !CONFIG_IS_ENABLED(WDT_MESON_GXBB) && defined(CONFIG_SPL_BUILD)
#define GX_WDT_CTRL_REG		0xc11098d0
#endif

#endif /* __GX_H__ */
