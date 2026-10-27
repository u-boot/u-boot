/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2026 Free Mobile - Vincent Jardin
 */

#ifndef __LX2160A_SERDES_SPLIT_H
#define __LX2160A_SERDES_SPLIT_H

/*
 * SD1 modes are for LX2160A only: LX2162A's SD 1 is four lanes with no
 * 100GE in its protocol table.
 */
#if defined(CONFIG_ARCH_LX2160A)

/*
 * Apply `hwconfig=serdes1:<mode>` if the board's environment asks for it.
 * Boards call this from reset_phy(), before mc_env_boot().
 */
void lx2160a_serdes_apply_hwconfig(void);

#endif /* CONFIG_ARCH_LX2160A */

#endif /* __LX2160A_SERDES_SPLIT_H */
