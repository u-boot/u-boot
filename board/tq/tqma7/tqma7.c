// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel, Steffen Doster
 */

#include <env.h>
#include <fdt_support.h>
#include <mtd_node.h>
#include <spi_flash.h>
#include <asm/bootm.h>
#include <asm/setup.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>

#include "../common/tq_bb.h"
#include "../common/tq_som.h"

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

#if (!IS_ENABLED(CONFIG_SPL_BUILD))

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	if (IS_ENABLED(CONFIG_FSL_QSPI))
		set_clk_qspi();

	return tq_bb_board_init();
}

static const char *tqma7_get_boardname(void)
{
	switch (get_cpu_type()) {
	case MXC_CPU_MX7S:
		return "TQMa7S";
	case MXC_CPU_MX7D:
		return "TQMa7D";
	default:
		return "??";
	};
}

int board_late_init(void)
{
	const char *bname = tqma7_get_boardname();

	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		struct tag_serialnr serialnr;

		get_board_serial(&serialnr);

		printf("UID:   %08x%08x\n", serialnr.high, serialnr.low);
	}

	env_set_runtime("board_name", bname);

	return tq_bb_board_late_init();
}

static u32 tqma7_get_board_rev(void)
{
	/* REV.0100 is unsupported */
	return 200;
}

int checkboard(void)
{
	printf("Board: %s REV.%04u\n", tq_bb_get_boardname(), tqma7_get_board_rev());
	return 0;
}

/*
 * Device Tree Support
 */
#if IS_ENABLED(CONFIG_OF_BOARD_SETUP) && IS_ENABLED(CONFIG_OF_LIBFDT)

int ft_board_setup(void *blob, struct bd_info *bd)
{
	tq_bb_ft_board_setup(blob, bd);

	return 0;
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */

#endif /* !IS_ENABLED(CONFIG_SPL_BUILD) */
