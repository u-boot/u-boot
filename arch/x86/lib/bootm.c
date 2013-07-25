/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright (C) 2001  Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <u-boot/zlib.h>
#include <asm/bootparam.h>
#include <asm/byteorder.h>
#include <asm/zimage.h>

#define COMMAND_LINE_OFFSET 0x9000

/*cmd_boot.c*/
int do_bootm_linux(int flag, int argc, char * const argv[],
		bootm_headers_t *images)
{
	struct boot_params *base_ptr = NULL;
	ulong os_data, os_len;
	image_header_t *hdr;
	void *load_address;

#if defined(CONFIG_FIT)
	const void	*data;
	size_t		len;
#endif

	if (flag & BOOTM_STATE_OS_PREP)
		return 0;
	if ((flag != 0) && (flag != BOOTM_STATE_OS_GO))
		return 1;

	if (images->legacy_hdr_valid) {
		hdr = images->legacy_hdr_os;
		if (image_check_type(hdr, IH_TYPE_MULTI)) {
			/* if multi-part image, we need to get first subimage */
			image_multi_getimg(hdr, 0, &os_data, &os_len);
		} else {
			/* otherwise get image data */
			os_data = image_get_data(hdr);
			os_len = image_get_data_size(hdr);
		}
#if defined(CONFIG_FIT)
	} else if (images->fit_uname_os) {
		int ret;

		ret = fit_image_get_data(images->fit_hdr_os,
					images->fit_noffset_os, &data, &len);
		if (ret) {
			puts("Can't get image data/size!\n");
			goto error;
		}
		os_data = (ulong)data;
		os_len = (ulong)len;
#endif
	} else {
		puts("Could not find kernel image!\n");
		goto error;
	}

#ifdef CONFIG_CMD_ZBOOT
	base_ptr = load_zimage((void *)os_data, os_len, &load_address);
#endif

	if (NULL == base_ptr) {
		printf("## Kernel loading failed ...\n");
		goto error;
	}

	if (setup_zimage(base_ptr, (char *)base_ptr + COMMAND_LINE_OFFSET,
			0, images->rd_start,
			images->rd_end - images->rd_start)) {
		printf("## Setting up boot parameters failed ...\n");
		goto error;
	}

	boot_zimage(base_ptr, load_address);
	/* does not return */

error:
	return 1;
}
