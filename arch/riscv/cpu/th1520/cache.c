// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025 Yao Zi <ziyao@disroot.org>
 */

#include <asm/io.h>
#include <cpu_func.h>
#include <linux/bitops.h>

#define CSR_MHCR		0x7c1
#define  CSR_MHCR_IE		BIT(0)
#define  CSR_MHCR_DE		BIT(1)

#if CONFIG_IS_ENABLED(RISCV_MMODE)
void icache_enable(void)
{
	csr_write(CSR_MHCR, csr_read(CSR_MHCR) | CSR_MHCR_IE);
}

void dcache_enable(void)
{
	csr_write(CSR_MHCR, csr_read(CSR_MHCR) | CSR_MHCR_DE);
}

int icache_status(void)
{
	return (csr_read(CSR_MHCR) & CSR_MHCR_IE) != 0;
}

int dcache_status(void)
{
	return (csr_read(CSR_MHCR) & CSR_MHCR_DE) != 0;
}
#endif /* CONFIG_IS_ENABLED(RISCV_MMODE) */
