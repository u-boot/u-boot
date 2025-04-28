/* SPDX-License-Identifier: GPL-2.0-or-later OR BSD-3-Clause */
/*
 * Copyright (C) 2023, STMicroelectronics - All Rights Reserved
 */

#ifndef MACH_ETZPC_H
#define MACH_ETZPC_H

#include <linux/types.h>

/**
 * stm32_etzpc_check_access - Check ETZPC accesses for given device node
 *
 * @device_node		Node of the device for which the accesses are checked
 *
 * @returns 0 on success (if access is granted), -EINVAL if access is denied.
 *          Else, returns an appropriate negative ERRNO value
 */
int stm32_etzpc_check_access(ofnode device_node);

/**
 * stm32_etzpc_check_access_by_id - Check ETZPC accesses for given id
 *
 * @device_node		Node of the device to get a reference on ETZPC
 * @id			ID of the resource to check
 *
 * @returns 0 on success (if access is granted), -EINVAL if access is denied.
 *          Else, returns an appropriate negative ERRNO value
 */
int stm32_etzpc_check_access_by_id(ofnode device_node, u32 id);

#endif /* MACH_ETZPC_H*/
