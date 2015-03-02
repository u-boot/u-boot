/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_ARCH_SYS_PROTO_H
#define _ASM_ARCH_SYS_PROTO_H

int zynq_sdhci_init(unsigned long regbase);

unsigned int zynqmp_get_silicon_version(void);

#endif /* _ASM_ARCH_SYS_PROTO_H */
