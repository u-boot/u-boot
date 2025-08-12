/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __ASM_SANDBOX_SYSTEM_H
#define __ASM_SANDBOX_SYSTEM_H

/* Define this as nops for sandbox architecture */
#define local_irq_save(x)	do { (x) = 0; } while (0)
#define local_irq_enable()
#define local_irq_disable()
#define local_irq_restore(x)

#endif
