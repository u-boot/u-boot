/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SpacemiT K1 reset driver — header interface.
 *
 * Copyright (C) 2026 RISCstar Ltd.
 */

#ifndef _SOC_SPACEMIT_K1_RESET_H
#define _SOC_SPACEMIT_K1_RESET_H

struct udevice;

enum spacemit_k1_reset_syscon {
	SPACEMIT_K1_RESET_MPMU,
	SPACEMIT_K1_RESET_APMU,
	SPACEMIT_K1_RESET_APBC,
	SPACEMIT_K1_RESET_APBC2,
};

int spacemit_k1_reset_bind(struct udevice *parent,
			   enum spacemit_k1_reset_syscon syscon);

#endif /* _SOC_SPACEMIT_K1_RESET_H */
