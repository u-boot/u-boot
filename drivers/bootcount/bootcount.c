/*
 * (C) Copyright 2010-2012
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <bootcount.h>
#include <linux/compiler.h>

/*
 * Only override CONFIG_SYS_BOOTCOUNT_ADDR if not already defined. This
 * way, some boards can define it directly in their config header.
 */
#if !defined(CONFIG_SYS_BOOTCOUNT_ADDR)

#if defined(CONFIG_MPC5xxx)
#define CONFIG_SYS_BOOTCOUNT_ADDR	(MPC5XXX_CDM_BRDCRMB)
#define CONFIG_SYS_BOOTCOUNT_SINGLEWORD
#endif /* defined(CONFIG_MPC5xxx) */

#if defined(CONFIG_MPC512X)
#define CONFIG_SYS_BOOTCOUNT_ADDR	(&((immap_t *)CONFIG_SYS_IMMR)->clk.bcr)
#define CONFIG_SYS_BOOTCOUNT_SINGLEWORD
#endif /* defined(CONFIG_MPC512X) */

#if defined(CONFIG_8xx)
#define CONFIG_SYS_BOOTCOUNT_ADDR (((immap_t *)CONFIG_SYS_IMMR)->im_cpm.cp_dpmem + \
				CPM_BOOTCOUNT_ADDR)
#endif /* defined(CONFIG_8xx) */

#if defined(CONFIG_MPC8260)
#include <asm/cpm_8260.h>

#define CONFIG_SYS_BOOTCOUNT_ADDR	(CONFIG_SYS_IMMR + CPM_BOOTCOUNT_ADDR)
#endif /* defined(CONFIG_MPC8260) */

#if defined(CONFIG_QE)
#include <linux/immap_qe.h>

#define CONFIG_SYS_BOOTCOUNT_ADDR	(CONFIG_SYS_IMMR + 0x110000 + \
					 QE_MURAM_SIZE - 2 * sizeof(u32))
#endif /* defined(CONFIG_MPC8360) */

#if defined(CONFIG_4xx)
#define CONFIG_SYS_BOOTCOUNT_ADDR	(CONFIG_SYS_OCM_DATA_ADDR + \
				CONFIG_SYS_BOOTCOUNT_ADDR)
#endif /* defined(CONFIG_4xx) */

#endif /* !defined(CONFIG_SYS_BOOTCOUNT_ADDR) */

/* Now implement the generic default functions */
#if defined(CONFIG_SYS_BOOTCOUNT_ADDR)
__weak void bootcount_store(ulong a)
{
	void *reg = (void *)CONFIG_SYS_BOOTCOUNT_ADDR;

#if defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD)
	raw_bootcount_store(reg, (BOOTCOUNT_MAGIC & 0xffff0000) | a);
#else
	raw_bootcount_store(reg, a);
	raw_bootcount_store(reg + 4, BOOTCOUNT_MAGIC);
#endif
}

__weak ulong bootcount_load(void)
{
	void *reg = (void *)CONFIG_SYS_BOOTCOUNT_ADDR;

#if defined(CONFIG_SYS_BOOTCOUNT_SINGLEWORD)
	u32 tmp = raw_bootcount_load(reg);

	if ((tmp & 0xffff0000) != (BOOTCOUNT_MAGIC & 0xffff0000))
		return 0;
	else
		return (tmp & 0x0000ffff);
#else
	if (raw_bootcount_load(reg + 4) != BOOTCOUNT_MAGIC)
		return 0;
	else
		return raw_bootcount_load(reg);
#endif
}
#endif
