// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include <common.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#include "ctrl_pex.h"
#include "sys_env_lib.h"

__weak void board_pex_config(void)
{
	/* nothing in this weak default implementation */
}

int hws_pex_config(const struct serdes_map *serdes_map, u8 count)
{
	enum serdes_type serdes_type;
	u32 idx, tmp;

	DEBUG_INIT_FULL_S("\n### hws_pex_config ###\n");

	tmp = reg_read(SOC_CONTROL_REG1);
	tmp &= ~0x03;

	for (idx = 0; idx < count; idx++) {
		serdes_type = serdes_map[idx].serdes_type;
		if ((serdes_type != PEX0) &&
		    ((serdes_map[idx].serdes_mode == PEX_ROOT_COMPLEX_X4) ||
		     (serdes_map[idx].serdes_mode == PEX_END_POINT_X4))) {
			/* for PEX by4 - relevant for the first port only */
			continue;
		}

		switch (serdes_type) {
		case PEX0:
			tmp |= 0x1 << PCIE0_ENABLE_OFFS;
			break;
		case PEX1:
			tmp |= 0x1 << PCIE1_ENABLE_OFFS;
			break;
		case PEX2:
			tmp |= 0x1 << PCIE2_ENABLE_OFFS;
			break;
		case PEX3:
			tmp |= 0x1 << PCIE3_ENABLE_OFFS;
			break;
		default:
			break;
		}
	}

	reg_write(SOC_CONTROL_REG1, tmp);

	board_pex_config();

	return MV_OK;
}
