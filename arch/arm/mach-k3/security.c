// SPDX-License-Identifier: GPL-2.0
/*
 * K3: Security functions
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Andrew F. Davis <afd@ti.com>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <hang.h>
#include <image.h>
#include <log.h>
#include <asm/cache.h>
#include <linux/soc/ti/ti_sci_protocol.h>
#include <mach/spl.h>
#include <spl.h>
#include <asm/arch/sys_proto.h>

void ti_secure_image_post_process(void **p_image, size_t *p_size)
{
	struct ti_sci_handle *ti_sci = get_ti_sci_handle();
	struct ti_sci_proc_ops *proc_ops = &ti_sci->ops.proc_ops;
	u64 image_addr;
	u32 image_size;
	int ret;

	image_addr = (uintptr_t)*p_image;
	image_size = *p_size;

	debug("Authenticating image at address 0x%016llx\n", image_addr);
	debug("Authenticating image of size %d bytes\n", image_size);

	flush_dcache_range((unsigned long)image_addr,
			   ALIGN((unsigned long)image_addr + image_size,
				 ARCH_DMA_MINALIGN));

	/* Authenticate image */
	ret = proc_ops->proc_auth_boot_image(ti_sci, &image_addr, &image_size);
	if (ret) {
		printf("Authentication failed!\n");
		hang();
	}

	if (image_size)
		invalidate_dcache_range((unsigned long)image_addr,
					ALIGN((unsigned long)image_addr +
					      image_size, ARCH_DMA_MINALIGN));

	/*
	 * The image_size returned may be 0 when the authentication process has
	 * moved the image. When this happens no further processing on the
	 * image is needed or often even possible as it may have also been
	 * placed behind a firewall when moved.
	 */
	*p_size = image_size;

	/*
	 * Output notification of successful authentication to re-assure the
	 * user that the secure code is being processed as expected. However
	 * suppress any such log output in case of building for SPL and booting
	 * via YMODEM. This is done to avoid disturbing the YMODEM serial
	 * protocol transactions.
	 */
	if (!(IS_ENABLED(CONFIG_SPL_BUILD) &&
	      IS_ENABLED(CONFIG_SPL_YMODEM_SUPPORT) &&
	      spl_boot_device() == BOOT_DEVICE_UART))
		printf("Authentication passed\n");
}
