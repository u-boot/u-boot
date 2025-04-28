/* SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause */
/*
 * Copyright (C) 2023, STMicroelectronics - All Rights Reserved
 */

#ifndef MACH_RIF_H
#define MACH_RIF_H

#include <linux/types.h>

/**
 * stm32_rifsc_check_access - Check RIF accesses for given device node
 *
 * @device_node		Node of the device for which the accesses are checked
 */
int stm32_rifsc_check_access(ofnode device_node);

/**
 * stm32_rifsc_check_access - Check RIF accesses for given id
 *
 * @device_node		Node of the device to get a reference on RIFSC
 * @id			ID of the resource to check
 */
int stm32_rifsc_check_access_by_id(ofnode device_node, u32 id);

#endif /* MACH_RIF_H*/
