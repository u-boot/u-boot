/*
 * Copyright (c) 2016 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef __ASM_INTEL_REGS_H
#define __ASM_INTEL_REGS_H

/* Access the memory-controller hub */
#define MCH_BASE_ADDRESS	0xfed10000
#define MCH_BASE_SIZE		0x8000
#define MCHBAR_REG(reg)		(MCH_BASE_ADDRESS + (reg))

#endif
