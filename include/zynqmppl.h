/*
 * (C) Copyright 2015 Xilinx, Inc,
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _ZYNQMPPL_H_
#define _ZYNQMPPL_H_

#include <xilinx.h>

#define ZYNQMP_SIP_SVC_CSU_DMA_INFO		0x82002004
#define ZYNQMP_SIP_SVC_CSU_DMA_LOAD		0x82002005
#define ZYNQMP_SIP_SVC_CSU_DMA_DUMP		0x82002006

extern struct xilinx_fpga_op zynqmp_op;

#define XILINX_ZYNQMP_DESC \
{ xilinx_zynqmp, csu_dma, 1, &zynqmp_op, 0, &zynqmp_op, "xczu9eg" }

#endif /* _ZYNQMPPL_H_ */
