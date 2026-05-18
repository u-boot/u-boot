/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2013-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#ifndef __TQ_BB_H
#define __TQ_BB_H

struct mmc;
struct bd_info;
struct node_info;

int tq_bb_board_mmc_getwp(struct mmc *mmc);
int tq_bb_board_mmc_getcd(struct mmc *mmc);
int tq_bb_board_mmc_init(struct bd_info *bis);

int tq_bb_board_early_init_f(void);
int tq_bb_board_init(void);
int tq_bb_board_late_init(void);
int tq_bb_checkboard(void);
void tq_bb_board_quiesce_devices(void);

const char *tq_bb_get_boardname(void);

#if IS_ENABLED(CONFIG_SPL_BUILD)
void tq_bb_board_init_f(ulong dummy);
void tq_bb_spl_board_init(void);
#endif

/*
 * Device Tree Support
 */
#if IS_ENABLED(CONFIG_OF_BOARD_SETUP) && IS_ENABLED(CONFIG_OF_LIBFDT)
int tq_bb_ft_board_setup(void *blob, struct bd_info *bis);
#endif /* IS_ENABLED(CONFIG_OF_BOARD_SETUP) && IS_ENABLED(CONFIG_OF_LIBFDT) */

#endif /* __TQ_BB_H */
