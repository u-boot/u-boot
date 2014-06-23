/*
 * Copyright 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <netdev.h>
#include <fsl_ifc.h>
#include <fsl_ddr.h>
#include <asm/io.h>
#include <fdt_support.h>
#include <libfdt.h>
#include <fsl_mc.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	init_final_memctl_regs();
	return 0;
}

int board_early_init_f(void)
{
	init_early_memctl_regs();	/* tighten IFC timing */

	return 0;
}

int dram_init(void)
{
	printf("DRAM:  ");
	gd->ram_size = initdram(0);

	return 0;
}

int timer_init(void)
{
	u32 __iomem *cntcr = (u32 *)CONFIG_SYS_FSL_TIMER_ADDR;
	u32 __iomem *cltbenr = (u32 *)CONFIG_SYS_FSL_PMU_CLTBENR;

	out_le32(cltbenr, 0x1);		/* enable cluster0 timebase */
	out_le32(cntcr, 0x1);		/* enable clock for timer */

	return 0;
}

/*
 * Board specific reset that is system reset.
 */
void reset_cpu(ulong addr)
{
}

int board_eth_init(bd_t *bis)
{
	int error = 0;

#ifdef CONFIG_SMC91111
	error = smc91111_initialize(0, CONFIG_SMC91111_BASE);
#endif

#ifdef CONFIG_FSL_MC_ENET
	error = cpu_eth_init(bis);
#endif
	return error;
}

#ifdef CONFIG_FSL_MC_ENET
void fdt_fixup_board_enet(void *fdt)
{
	int offset;

	offset = fdt_path_offset(fdt, "/fsl,dprc@0");
	if (get_mc_boot_status() == 0)
		fdt_status_okay(fdt, offset);
	else
		fdt_status_fail(fdt, offset);
}
#endif

#ifdef CONFIG_OF_BOARD_SETUP
void ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	/* limit the memory size to bank 1 until Linux can handle 40-bit PA */
	base = getenv_bootm_low();
	size = getenv_bootm_size();
	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_FSL_MC_ENET
	fdt_fixup_board_enet(blob);
#endif
}
#endif
