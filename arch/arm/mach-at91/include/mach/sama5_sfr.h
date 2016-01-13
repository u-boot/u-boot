/*
 * Special Function Register (SFR)
 *
 * Copyright (C) 2014 Atmel
 *		      Bo Shen <voice.shen@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SAMA5_SFR_H
#define __SAMA5_SFR_H

struct atmel_sfr {
	u32 reserved1;	/* 0x00 */
	u32 ddrcfg;	/* 0x04: DDR Configuration Register */
	u32 reserved2;	/* 0x08 */
	u32 reserved3;	/* 0x0c */
	u32 ohciicr;	/* 0x10: OHCI Interrupt Configuration Register */
	u32 ohciisr;	/* 0x14: OHCI Interrupt Status Register */
	u32 reserved4[4];	/* 0x18 ~ 0x24 */
	u32 secure;		/* 0x28: Security Configuration Register */
	u32 reserved5[5];	/* 0x2c ~ 0x3c */
	u32 ebicfg;		/* 0x40: EBI Configuration Register */
	u32 reserved6[2];	/* 0x44 ~ 0x48 */
	u32 sn0;		/* 0x4c */
	u32 sn1;		/* 0x50 */
	u32 aicredir;	/* 0x54 */
};

/* Bit field in DDRCFG */
#define ATMEL_SFR_DDRCFG_FDQIEN		0x00010000
#define ATMEL_SFR_DDRCFG_FDQSIEN	0x00020000

/* Bit field in AICREDIR */
#define ATMEL_SFR_AICREDIR_NSAIC	0x00000001

#endif
