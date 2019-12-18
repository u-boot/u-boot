/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2019 Google LLC
 */

#ifndef _ASM_ARCH_CPU_H
#define _ASM_ARCH_CPU_H

/* Common Timer Copy (CTC) frequency - 19.2MHz */
#define CTC_FREQ		19200000

#define MAX_PCIE_PORTS		6
#define CLKREQ_DISABLED		0xf

#ifndef __ASSEMBLY__
/* Flush L1D to L2 */
void cpu_flush_l1d_to_l2(void);
#endif

#endif /* _ASM_ARCH_CPU_H */
