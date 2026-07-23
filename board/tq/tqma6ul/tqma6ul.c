// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Marco Felsch, Nora Schiffer
 *
 */

#include <env.h>
#include <fdt_support.h>
#include <mmc.h>
#include <mtd_node.h>
#include <spi_flash.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>

#include "../common/tq_bb.h"
#include "tqma6ul.h"

int tq_bb_board_early_init_f(void)
{
	if (CONFIG_IS_ENABLED(FSL_QSPI))
		enable_qspi_clk(0);

	return 0;
}

/**
 * Checks if CPU (imx6ul or ima6ull) matches the one set for the image.
 */
static const char *check_cpu_variant(void)
{
	const char *cpu;

	if (is_mx6ul()) {
		cpu = "ul";
		if (!IS_ENABLED(CONFIG_MX6UL))
			printf("*** ERROR: image not compiled for i.MX6UL!\n");
	} else if (is_mx6ull()) {
		cpu = "ull";
		if (!IS_ENABLED(CONFIG_MX6ULL))
			printf("*** ERROR: image not compiled for i.MX6ULL!\n");
	} else {
		printf("unknown CPU\n");
		return NULL;
	}

	return cpu;
}

/**
 * Checks configuration for TQMa6UL SoM module variant.
 */
enum tqma6ul_som_type check_tqma6ul_variant(void)
{
	if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_STANDARD))
		return tqma6ul_som_type_ca;

	if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_LGA))
		return tqma6ul_som_type_lga;

	printf("unknown SoM variant\n");

	return tqma6ul_som_type_unknown;
}

/**
 * Adjusts device tree name based on CPU variant.
 */
enum tqma6ul_som_type set_tqma6ul_dt_name(char *dt, size_t dtsize, const char *mb)
{
	const char *tqma6ul_cpu, *tqma6ul_variant;
	enum tqma6ul_som_type somtype;
	u8 mx6ul_variant;

	tqma6ul_cpu = check_cpu_variant();
	if (!tqma6ul_cpu)
		return tqma6ul_som_type_unknown;

	/* MX6UL1 vs MX6UL2 */
	mx6ul_variant = check_module_fused(MODULE_ENET2) ? 1 : 2;

	somtype = check_tqma6ul_variant();
	switch (somtype) {
	case tqma6ul_som_type_ca:
		tqma6ul_variant = "";
		break;
	case tqma6ul_som_type_lga:
		tqma6ul_variant = "l";
		break;
	default:
		return tqma6ul_som_type_unknown;
	}

	snprintf(dt, dtsize, "imx6%s-tqma6%s%u%s-%s",
		 tqma6ul_cpu, tqma6ul_cpu, mx6ul_variant, tqma6ul_variant, mb);

	return somtype;
}

#if !IS_ENABLED(CONFIG_SPL_BUILD)
int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

const char *tq_som_get_modulename(void)
{
	if (is_mx6ul()) {
		if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_STANDARD))
			return "TQMa6ULx";

		if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_LGA))
			return "TQMa6ULxL";
	}

	if (is_mx6ull()) {
		if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_STANDARD))
			return "TQMa6ULLx";

		if (IS_ENABLED(CONFIG_TQMA6UL_VARIANT_LGA))
			return "TQMa6ULLxL";
	}

	return "Unknown";
}

int checkboard(void)
{
	printf("Board: %s on %s\n", tq_som_get_modulename(),
	       tq_bb_get_boardname());

	return tq_bb_checkboard();
}

#if IS_ENABLED(CONFIG_CMD_BMODE)
static const struct boot_mode tqma6ul_board_boot_modes[] = {
	/* 4 bit bus width */
	{"sd", MAKE_CFGVAL(0x42, 0x20, 0x00, 0x00)},
	{"emmc", MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
	{"qspi", MAKE_CFGVAL(0x10, 0x00, 0x00, 0x00)},
	{NULL, 0},
};
#endif

int tq_bb_board_late_init(void)
{
	if (IS_ENABLED(CONFIG_CMD_BMODE))
		add_board_boot_modes(tqma6ul_board_boot_modes);

	env_set_runtime("board_name", tq_som_get_modulename());

	return 0;
}

int tq_bb_checkboard(void)
{
	if (is_mx6ul()) {
		if (!IS_ENABLED(CONFIG_MX6UL))
			printf("*** ERROR: image not compiled for i.MX6UL!\n");
	} else if (is_mx6ull()) {
		if (!IS_ENABLED(CONFIG_MX6ULL))
			printf("*** ERROR: image not compiled for i.MX6ULL!\n");
	} else {
		printf("*** ERROR: unknown CPU variant!\n");
	}

	return 0;
}

/*
 * Device Tree Support
 */
#if IS_ENABLED(CONFIG_OF_BOARD_SETUP) && IS_ENABLED(CONFIG_OF_LIBFDT)
int tq_bb_ft_board_setup(void *blob, struct bd_info *bd)
{
	return 0;
}
#endif /* IS_ENABLED(CONFIG_OF_BOARD_SETUP) && IS_ENABLED(CONFIG_OF_LIBFDT) */

#endif /* !IS_ENABLED(CONFIG_SPL_BUILD) */
