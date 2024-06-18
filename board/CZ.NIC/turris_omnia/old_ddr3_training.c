// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Marek Behún <kabel@kernel.org>
 */

#include <asm/arch/soc.h>
#include <asm/io.h>

#include "../drivers/ddr/marvell/a38x/old/ddr3_init.h"

static struct hws_topology_map board_topology_map_1g = {
	0x1, /* active interfaces */
	/* cs_mask, mirror, dqs_swap, ck_swap X PUPs */
	{ { { {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0} },
	    SPEED_BIN_DDR_1600K,	/* speed_bin */
	    BUS_WIDTH_16,		/* memory_width */
	    MEM_4G,			/* mem_size */
	    DDR_FREQ_800,		/* frequency */
	    0, 0,			/* cas_l cas_wl */
	    HWS_TEMP_NORMAL,		/* temperature */
	    HWS_TIM_2T} },              /* timing (force 2t) */
	5,				/* Num Of Bus Per Interface*/
	BUS_MASK_32BIT			/* Busses mask */
};

static struct hws_topology_map board_topology_map_2g = {
	0x1, /* active interfaces */
	/* cs_mask, mirror, dqs_swap, ck_swap X PUPs */
	{ { { {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0},
	      {0x1, 0, 0, 0} },
	    SPEED_BIN_DDR_1600K,	/* speed_bin */
	    BUS_WIDTH_16,		/* memory_width */
	    MEM_8G,			/* mem_size */
	    DDR_FREQ_800,		/* frequency */
	    0, 0,			/* cas_l cas_wl */
	    HWS_TEMP_NORMAL,		/* temperature */
	    HWS_TIM_2T} },              /* timing (force 2t) */
	5,				/* Num Of Bus Per Interface*/
	BUS_MASK_32BIT			/* Busses mask */
};

/* defined in turris_omnia.c */
extern int omnia_get_ram_size_gb(void);

struct hws_topology_map *ddr3_get_topology_map(void)
{
	if (omnia_get_ram_size_gb() == 2)
		return &board_topology_map_2g;
	else
		return &board_topology_map_1g;
}

__weak u32 sys_env_get_topology_update_info(struct topology_update_info *tui)
{
	return MV_OK;
}
