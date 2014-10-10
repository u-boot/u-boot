/*
 * Copyright (c) 2014, Google Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _X86_ARCH_GPIO_H_
#define _X86_ARCH_GPIO_H_

struct ich6_bank_platdata {
	uint32_t base_addr;
	const char *bank_name;
};

#endif /* _X86_ARCH_GPIO_H_ */
