/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/arch/crm_regs.h>

void init_aips(void)
{
	struct aipstz_regs *aips1, *aips2, *aips3;

	aips1 = (struct aipstz_regs *)AIPS1_BASE_ADDR;
	aips2 = (struct aipstz_regs *)AIPS2_BASE_ADDR;
	aips3 = (struct aipstz_regs *)AIPS3_BASE_ADDR;

	/*
	 * Set all MPROTx to be non-bufferable, trusted for R/W,
	 * not forced to user-mode.
	 */
	writel(0x77777777, &aips1->mprot0);
	writel(0x77777777, &aips1->mprot1);
	writel(0x77777777, &aips2->mprot0);
	writel(0x77777777, &aips2->mprot1);

	/*
	 * Set all OPACRx to be non-bufferable, not require
	 * supervisor privilege level for access,allow for
	 * write access and untrusted master access.
	 */
	writel(0x00000000, &aips1->opacr0);
	writel(0x00000000, &aips1->opacr1);
	writel(0x00000000, &aips1->opacr2);
	writel(0x00000000, &aips1->opacr3);
	writel(0x00000000, &aips1->opacr4);
	writel(0x00000000, &aips2->opacr0);
	writel(0x00000000, &aips2->opacr1);
	writel(0x00000000, &aips2->opacr2);
	writel(0x00000000, &aips2->opacr3);
	writel(0x00000000, &aips2->opacr4);

	if (is_cpu_type(MXC_CPU_MX6SX) || is_soc_type(MXC_SOC_MX7))
	{
		/*
		 * Set all MPROTx to be non-bufferable, trusted for R/W,
		 * not forced to user-mode.
		 */
		writel(0x77777777, &aips3->mprot0);
		writel(0x77777777, &aips3->mprot1);

		/*
		 * Set all OPACRx to be non-bufferable, not require
		 * supervisor privilege level for access,allow for
		 * write access and untrusted master access.
		 */
		writel(0x00000000, &aips3->opacr0);
		writel(0x00000000, &aips3->opacr1);
		writel(0x00000000, &aips3->opacr2);
		writel(0x00000000, &aips3->opacr3);
		writel(0x00000000, &aips3->opacr4);
	}
}

#define SRC_SCR_WARM_RESET_ENABLE	0

void init_src(void)
{
	struct src *src_regs = (struct src *)SRC_BASE_ADDR;
	u32 val;

	/*
	 * force warm reset sources to generate cold reset
	 * for a more reliable restart
	 */
	val = readl(&src_regs->scr);
	val &= ~(1 << SRC_SCR_WARM_RESET_ENABLE);
	writel(val, &src_regs->scr);
}

void boot_mode_apply(unsigned cfg_val)
{
	unsigned reg;
	struct src *psrc = (struct src *)SRC_BASE_ADDR;
	writel(cfg_val, &psrc->gpr9);
	reg = readl(&psrc->gpr10);
	if (cfg_val)
		reg |= 1 << 28;
	else
		reg &= ~(1 << 28);
	writel(reg, &psrc->gpr10);
}
