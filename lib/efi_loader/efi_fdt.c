// SPDX-License-Identifier: GPL-2.0+
/*
 * Bootmethod for distro boot via EFI
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <efi_loader.h>
#include <env.h>
#include <errno.h>
#include <log.h>
#include <string.h>
#include <vsprintf.h>

/**
 * distro_efi_get_fdt_name() - get the filename for reading the .dtb file
 *
 * @fname:	buffer for filename
 * @size:	buffer size
 * @seq:	sequence number, to cycle through options (0=first)
 *
 * Returns:
 * 0 on success,
 * -ENOENT if the "fdtfile" env var does not exist,
 * -EINVAL if there are no more options,
 * -EALREADY if the control FDT should be used
 */
int efi_get_distro_fdt_name(char *fname, int size, int seq)
{
	const char *fdt_fname;
	const char *prefix;

	/* select the prefix */
	switch (seq) {
	case 0:
		/* this is the default */
		prefix = "/dtb";
		break;
	case 1:
		prefix = "";
		break;
	case 2:
		prefix = "/dtb/current";
		break;
	default:
		return log_msg_ret("pref", -EINVAL);
	}

	fdt_fname = env_get("fdtfile");
	if (fdt_fname) {
		snprintf(fname, size, "%s/%s", prefix, fdt_fname);
		log_debug("Using device tree: %s\n", fname);
	} else if (IS_ENABLED(CONFIG_OF_HAS_PRIOR_STAGE)) {
		strcpy(fname, "<prior>");
		return log_msg_ret("pref", -EALREADY);
	/* Use this fallback only for 32-bit ARM */
	} else if (IS_ENABLED(CONFIG_ARM) && !IS_ENABLED(CONFIG_ARM64)) {
		const char *soc = env_get("soc");
		const char *board = env_get("board");
		const char *boardver = env_get("boardver");

		/* cf the code in label_boot() which seems very complex */
		snprintf(fname, size, "%s/%s%s%s%s.dtb", prefix,
			 soc ? soc : "", soc ? "-" : "", board ? board : "",
			 boardver ? boardver : "");
		log_debug("Using default device tree: %s\n", fname);
	} else {
		return log_msg_ret("env", -ENOENT);
	}

	return 0;
}

/**
 * efi_load_distro_fdt() - load distro device-tree
 *
 * @handle:	handle of loaded image
 * @fdt:	on return device-tree, must be freed via efi_free_pages()
 * @fdt_size:	buffer size
 */
void efi_load_distro_fdt(efi_handle_t handle, void **fdt, efi_uintn_t *fdt_size)
{
	struct efi_device_path *dp;
	efi_status_t  ret;
	struct efi_handler *handler;
	struct efi_loaded_image *loaded_image;
	efi_handle_t device;

	*fdt = NULL;

	/* Get boot device from loaded image protocol */
	ret = efi_search_protocol(handle, &efi_guid_loaded_image, &handler);
	if (ret != EFI_SUCCESS)
		return;
	loaded_image = handler->protocol_interface;
	device = loaded_image->device_handle;

	/* Get device path of boot device */
	ret = efi_search_protocol(device, &efi_guid_device_path, &handler);
	if (ret != EFI_SUCCESS)
		return;
	dp = handler->protocol_interface;

	/* Try the various available names */
	for (int seq = 0; ; ++seq) {
		struct efi_device_path *file;
		char buf[255];

		if (efi_get_distro_fdt_name(buf, sizeof(buf), seq))
			break;
		file = efi_dp_from_file(dp, buf);
		if (!file)
			break;
		ret = efi_load_image_from_path(true, file, fdt, fdt_size);
		efi_free_pool(file);
		if (ret == EFI_SUCCESS) {
			log_debug("Fdt %pD loaded\n", file);
			break;
		}
	}
}
