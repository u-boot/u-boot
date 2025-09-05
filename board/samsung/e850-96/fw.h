/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 */

#ifndef __E850_96_FW_H
#define __E850_96_FW_H

#include <asm/types.h>

int load_ldfw(const char *ifname, int dev, int part, phys_addr_t addr);

#endif /* __E850_96_FW_H */
