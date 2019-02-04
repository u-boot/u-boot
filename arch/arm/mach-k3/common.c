// SPDX-License-Identifier: GPL-2.0+
/*
 * K3: Common Architecture initialization
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <common.h>
#include <spl.h>
#include "common.h"
#include <dm.h>
#include <remoteproc.h>

#ifdef CONFIG_SYS_K3_SPL_ATF
void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	int ret;

	/*
	 * It is assumed that remoteproc device 1 is the corresponding
	 * Cortex-A core which runs ATF. Make sure DT reflects the same.
	 */
	ret = rproc_dev_init(1);
	if (ret)
		panic("%s: ATF failed to initialize on rproc (%d)\n", __func__,
		      ret);

	ret = rproc_load(1, spl_image->entry_point, 0x200);
	if (ret)
		panic("%s: ATF failed to load on rproc (%d)\n", __func__, ret);

	/* Add an extra newline to differentiate the ATF logs from SPL */
	printf("Starting ATF on ARM64 core...\n\n");

	ret = rproc_start(1);
	if (ret)
		panic("%s: ATF failed to start on rproc (%d)\n", __func__, ret);

	debug("ATF started. Waiting indefinitely...\n");
	while (1)
		asm volatile("wfe");
}
#endif
