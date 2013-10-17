/*
 * Copyright (C) 2013 Boundary Devices Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __ASM_ARCH_MX6_PINS_H__
#define __ASM_ARCH_MX6_PINS_H__

#ifdef CONFIG_MX6Q
#include "mx6q_pins.h"
#else
#if defined(CONFIG_MX6DL) || defined(CONFIG_MX6S)
#include "mx6dl_pins.h"
#else
#if defined(CONFIG_MX6SL)
#include "mx6sl_pins.h"
#else
#error "Please select cpu"
#endif	/* CONFIG_MX6SL */
#endif	/* CONFIG_MX6DL or CONFIG_MX6S */
#endif	/* CONFIG_MX6Q */

#endif	/*__ASM_ARCH_MX6_PINS_H__ */
