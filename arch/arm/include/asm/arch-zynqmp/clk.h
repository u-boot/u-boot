/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_ARCH_CLK_H_
#define _ASM_ARCH_CLK_H_

unsigned long get_uart_clk(int dev_id);
unsigned long zynqmp_get_system_timer_freq(void);

#endif /* _ASM_ARCH_CLK_H_ */
