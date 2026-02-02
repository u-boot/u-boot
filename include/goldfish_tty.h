/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2025, Kuan-Wei Chiu <visitorckw@gmail.com>
 */

#ifndef _GOLDFISH_TTY_H_
#define _GOLDFISH_TTY_H_

#include <linux/types.h>

/* Platform data for the Goldfish TTY driver
 * Used to pass hardware base address from Board to Driver
 */
struct goldfish_tty_plat {
	phys_addr_t reg;
};

#endif /* _GOLDFISH_TTY_H_ */
