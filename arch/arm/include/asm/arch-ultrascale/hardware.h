/*
 * (C) Copyright 2014 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_ARCH_HARDWARE_H
#define _ASM_ARCH_HARDWARE_H

/* Generic Interrupt Controller Definitions */
#define GICD_BASE	0xFD3FF000
#define GICC_BASE	0xFD3FE100

#define ZYNQ_SERIAL_BASEADDR0	0xFF000000
#define ZYNQ_SERIAL_BASEADDR1	0xFF001000

#define ZYNQ_GEM_BASEADDR0	0xFF009000
#define ZYNQ_GEM_BASEADDR1	0xFF00A000
#define ZYNQ_GEM_BASEADDR2	0xFF00B000
#define ZYNQ_GEM_BASEADDR3	0xFF00C000

#define ZYNQ_QSPI_BASEADDR	0xFF00D000

#endif /* _ASM_ARCH_HARDWARE_H */
