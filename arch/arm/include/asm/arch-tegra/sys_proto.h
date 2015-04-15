/*
 * (C) Copyright 2010,2011
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

void invalidate_dcache(void);

/**
 * tegra_board_id() - Get the board iD
 *
 * @return a board ID, or -ve on error
 */
int tegra_board_id(void);

#endif
