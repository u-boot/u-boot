/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016-2017 Intel Corporation
 */

#ifndef _MISC_H_
#define _MISC_H_

void dwmac_deassert_reset(const unsigned int of_reset_id, const u32 phymode);

struct bsel {
	const char	*mode;
	const char	*name;
};

extern struct bsel bsel_str[];

#ifdef CONFIG_FPGA
void socfpga_fpga_add(void);
#else
static inline void socfpga_fpga_add(void) {}
#endif

#if defined(CONFIG_TARGET_SOCFPGA_ARRIA10)
unsigned int dedicated_uart_com_port(const void *blob);
unsigned int shared_uart_com_port(const void *blob);
unsigned int uart_com_port(const void *blob);
#endif

#endif /* _MISC_H_ */
