/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2017-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
 * D-82229 Seefeld, Germany.
 * Author: Markus Niebel
 */

#ifndef __TQMA6_EMMC_H__
#define __TQMA6_EMMC_H__

#define TQMA6_EMMC_DSR		0x0100

struct mmc;

int tqma6_emmc_need_dsr(const struct mmc *mmc);
void tqma6_ft_fixup_emmc_dsr(void *blob, const char *path, u32 value);
void tqma6_mmc_detect_card_type(struct mmc *mmc);

#endif /* __TQMA6_EMMC_H__ */
