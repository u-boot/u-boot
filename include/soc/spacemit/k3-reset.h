/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * SpacemiT K3 reset driver — header interface.
 */

#ifndef _SOC_SPACEMIT_K3_RESET_H
#define _SOC_SPACEMIT_K3_RESET_H

struct udevice;

enum spacemit_k3_reset_syscon {
	SPACEMIT_K3_RESET_MPMU,
	SPACEMIT_K3_RESET_APBC,
	SPACEMIT_K3_RESET_APMU,
	SPACEMIT_K3_RESET_DCIU,
};

int spacemit_k3_reset_bind(struct udevice *parent,
			   enum spacemit_k3_reset_syscon syscon);

#endif /* _SOC_SPACEMIT_K3_RESET_H */
