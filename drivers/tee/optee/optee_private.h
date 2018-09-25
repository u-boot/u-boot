/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2018 Linaro Limited
 */

#ifndef __OPTEE_PRIVATE_H
#define __OPTEE_PRIVATE_H

void *optee_alloc_and_init_page_list(void *buf, ulong len, u64 *phys_buf_ptr);
void optee_suppl_cmd(struct udevice *dev, void *shm, void **page_list);

#endif /* __OPTEE_PRIVATE_H */
