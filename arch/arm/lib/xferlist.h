/* SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause */
/*
 * Copyright (C) 2023 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */

#ifndef _XFERLIST_H_
#define _XFERLIST_H_

/*
 * Boot parameters saved from start.S
 * saved_args[0]: FDT base address
 * saved_args[1]: Bloblist signature
 * saved_args[2]: must be 0
 * saved_args[3]: Bloblist base address
 */
extern unsigned long saved_args[];

#endif /* _XFERLIST_H_ */
