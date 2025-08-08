/* SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause */
/*
 * Copyright (C) 2023, STMicroelectronics - All Rights Reserved
 */

#ifndef MACH_RIF_H
#define MACH_RIF_H

#include <linux/types.h>

#if IS_ENABLED(CONFIG_STM32MP21X) || IS_ENABLED(CONFIG_STM32MP23X) || IS_ENABLED(CONFIG_STM32MP25X)
/**
 * stm32_rifsc_grant_access_by_id - Grant RIFSC access for a given peripheral using its ID
 *
 * @device_node Node of the peripheral
 * @id ID of the peripheral of which access should be granted
 */
int stm32_rifsc_grant_access_by_id(ofnode device_node, u32 id);

/**
 * stm32_rifsc_grant_access_by_id - Grant RIFSC access for a given peripheral using its node
 *
 * @id node of the peripheral of which access should be granted
 */
int stm32_rifsc_grant_access(ofnode device_node);

/**
 * stm32_rifsc_release_access_by_id - Release RIFSC access for a given peripheral using its ID
 *
 * @device_node Node of the peripheral
 * @id ID of the peripheral of which access should be released
 */
void stm32_rifsc_release_access_by_id(ofnode device_node, u32 id);

/**
 * stm32_rifsc_release_access_by_id - Release RIFSC access for a given peripheral using its node
 *
 * @id node of the peripheral of which access should be released
 */
void stm32_rifsc_release_access(ofnode device_node);
#else
static inline int stm32_rifsc_grant_access_by_id(ofnode device_node, u32 id)
{
	return -EACCES;
}

static inline int stm32_rifsc_grant_access(ofnode device_node)
{
	return -EACCES;
}

static inline void stm32_rifsc_release_access_by_id(ofnode device_node, u32 id)
{
}

static inline void stm32_rifsc_release_access(ofnode device_node)
{
}
#endif
#endif /* MACH_RIF_H*/
