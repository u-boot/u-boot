/*
 * Copyright (c) 2012, Google Inc. All rights reserved.
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _X86_GPIO_H_
#define _X86_GPIO_H_

#include <asm-generic/gpio.h>

struct ich6_bank_platdata {
	uint16_t base_addr;
	const char *bank_name;
	int offset;
};

#endif /* _X86_GPIO_H_ */
