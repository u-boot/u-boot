/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2026 Free Mobile - Vincent Jardin
 */

#ifndef __NBXV3_CARRIER_H
#define __NBXV3_CARRIER_H

/*
 * Ensure ${carrier} is set before anything consumes it.
 * It is idempotent, and it never overrides a value already present
 * in the uboot's environment.
 * It shall run after initr_env and before ${mcinitcmd} is evaluated.
 */
void nbxv3_carrier_env_init(void);

#endif /* __NBXV3_CARRIER_H */
