/*
 * SPDX-License-Identifier:    GPL-2.0+
 */

#ifndef XIL_IO_H /* prevent circular inclusions */
#define XIL_IO_H

/* FIXME remove this when vivado is fixed */
#include <asm/io.h>

#define xil_printf(...)

void Xil_ICacheEnable(void)
{}

void Xil_DCacheEnable(void)
{}

void Xil_ICacheDisable(void)
{}

void Xil_DCacheDisable(void)
{}

void Xil_Out32(unsigned long addr, unsigned long val)
{
	writel(val, addr);
}

int Xil_In32(unsigned long addr)
{
	return readl(addr);
}

__weak void prog_reg(unsigned long addr, unsigned long mask,
		     unsigned long shift, unsigned long value)
{
	int rdata = 0;

	rdata = Xil_In32(addr);
	rdata = rdata & (~mask);
	rdata = rdata | (value << shift);
	Xil_Out32(addr,rdata);
}

#endif /* XIL_IO_H */
