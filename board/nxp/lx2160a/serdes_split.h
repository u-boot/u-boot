/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2026 Free Mobile - Vincent Jardin
 */

#ifndef __LX2160A_SERDES_SPLIT_H
#define __LX2160A_SERDES_SPLIT_H

/*
 * The split is LX2160A only: LX2162A's SerDes 1 is four lanes with no
 * 100GE in its protocol table.
 */
#if defined(CONFIG_ARCH_LX2160A)

/* Is SerDes 1 already 1x 100G + 4x 25G? */
bool lx2160a_serdes1_is_split(void);

/*
 * Split SerDes 1's 100GE.1 (lanes LNE-LNH) into 4x 25G.
 *
 * Refuses unless SD1 is in the unsplit state, and must be called before
 * the Management Complex starts: MC latches the live protocol-converter
 * configuration when it boots.
 */
int lx2160a_serdes1_split(void);

/*
 * Apply `hwconfig=serdes1:split` if the board's environment asks for it.
 * Boards call this from reset_phy(), before mc_env_boot().
 */
void lx2160a_serdes_apply_hwconfig(void);

#endif /* CONFIG_ARCH_LX2160A */

#endif /* __LX2160A_SERDES_SPLIT_H */
