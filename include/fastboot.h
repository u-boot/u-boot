/*
 * (C) Copyright 2008 - 2009
 * Windriver, <www.windriver.com>
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * Copyright 2011 Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *
 * Copyright 2014 Linaro, Ltd.
 * Rob Herring <robh@kernel.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _FASTBOOT_H_
#define _FASTBOOT_H_

/* The 64 defined bytes plus \0 */
#define FASTBOOT_RESPONSE_LEN	(64 + 1)

void fastboot_fail(const char *reason);
void fastboot_okay(const char *reason);

#endif /* _FASTBOOT_H_ */
