/*
 * Copyright (c) 2014 Google, Inc
 *
 * From Coreboot src/southbridge/intel/bd82x6x/pch.h
 *
 * Copyright (C) 2008-2009 coresystems GmbH
 * Copyright (C) 2012 The Chromium OS Authors.  All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _ASM_ARCH_PCH_H
#define _ASM_ARCH_PCH_H

#include <pci.h>

/* PCI Configuration Space (D31:F0): LPC */
#define PCH_LPC_DEV		PCI_BDF(0, 0x1f, 0)

#define LPC_IO_DEC		0x80 /* IO Decode Ranges Register */
#define LPC_EN			0x82 /* LPC IF Enables Register */
#define  CNF2_LPC_EN		(1 << 13) /* 0x4e/0x4f */
#define  CNF1_LPC_EN		(1 << 12) /* 0x2e/0x2f */
#define  MC_LPC_EN		(1 << 11) /* 0x62/0x66 */
#define  KBC_LPC_EN		(1 << 10) /* 0x60/0x64 */
#define  GAMEH_LPC_EN		(1 << 9)  /* 0x208/0x20f */
#define  GAMEL_LPC_EN		(1 << 8)  /* 0x200/0x207 */
#define  FDD_LPC_EN		(1 << 3)  /* LPC_IO_DEC[12] */
#define  LPT_LPC_EN		(1 << 2)  /* LPC_IO_DEC[9:8] */
#define  COMB_LPC_EN		(1 << 1)  /* LPC_IO_DEC[6:4] */
#define  COMA_LPC_EN		(1 << 0)  /* LPC_IO_DEC[3:2] */
#define LPC_GEN1_DEC		0x84 /* LPC IF Generic Decode Range 1 */
#define LPC_GEN2_DEC		0x88 /* LPC IF Generic Decode Range 2 */
#define LPC_GEN3_DEC		0x8c /* LPC IF Generic Decode Range 3 */
#define LPC_GEN4_DEC		0x90 /* LPC IF Generic Decode Range 4 */
#define LPC_GENX_DEC(x)		(0x84 + 4 * (x))

/**
 * lpc_early_init() - set up LPC serial ports and other early things
 *
 * @blob:	Device tree blob
 * @node:	Offset of LPC node
 * @dev:	PCH PCI device containing the LPC
 * @return 0 if OK, -ve on error
 */
int lpc_early_init(const void *blob, int node, pci_dev_t dev);

#endif
