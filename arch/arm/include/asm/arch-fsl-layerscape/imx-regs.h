/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#ifndef __ASM_ARCH_FSL_LAYERSCAPE_IMX_REGS_H__
#define __ASM_ARCH_FSL_LAYERSCAPE_IMX_REGS_H__

#define I2C_QUIRK_REG	/* enable 8-bit driver */

#ifdef CONFIG_FSL_LPUART
#ifdef CONFIG_LPUART_32B_REG
struct lpuart_fsl {
	u32 baud;
	u32 stat;
	u32 ctrl;
	u32 data;
	u32 match;
	u32 modir;
	u32 fifo;
	u32 water;
};
#else
struct lpuart_fsl {
	u8 ubdh;
	u8 ubdl;
	u8 uc1;
	u8 uc2;
	u8 us1;
	u8 us2;
	u8 uc3;
	u8 ud;
	u8 uma1;
	u8 uma2;
	u8 uc4;
	u8 uc5;
	u8 ued;
	u8 umodem;
	u8 uir;
	u8 reserved;
	u8 upfifo;
	u8 ucfifo;
	u8 usfifo;
	u8 utwfifo;
	u8 utcfifo;
	u8 urwfifo;
	u8 urcfifo;
	u8 rsvd[28];
};
#endif
#endif	/* CONFIG_FSL_LPUART */

#endif /* __ASM_ARCH_FSL_LAYERSCAPE_IMX_REGS_H__ */
