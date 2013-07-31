/*
 * Copyright (C) 2010
 * Reinhard Meyer, reinhard.meyer@emk-elektronik.de
 *
 * Shutdown Controller
 * Based on AT91SAM9XE datasheet
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef AT91_SHDWN_H
#define AT91_SHDWN_H

#ifndef __ASSEMBLY__

struct at91_shdwn {
	u32	cr;	/* Control Rer.    WO */
	u32	mr;	/* Mode Register   RW 0x00000003 */
	u32	sr;	/* Status Register RO 0x00000000 */
};

#endif /* __ASSEMBLY__ */

#define AT91_SHDW_CR_KEY	0xa5000000
#define AT91_SHDW_CR_SHDW	0x00000001

#define AT91_SHDW_MR_RTTWKEN	0x00010000
#define AT91_SHDW_MR_CPTWK0	0x000000f0
#define AT91_SHDW_MR_WKMODE0H2L	0x00000002
#define AT91_SHDW_MR_WKMODE0L2H	0x00000001

#define AT91_SHDW_SR_RTTWK	0x00010000
#define AT91_SHDW_SR_WAKEUP0	0x00000001

#endif
