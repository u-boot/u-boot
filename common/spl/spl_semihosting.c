// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#include <common.h>
#include <image.h>
#include <log.h>
#include <semihosting.h>
#include <spl.h>

static ulong spl_smh_fit_read(struct spl_load_info *load, ulong sector,
			      ulong count, void *buf)
{
	int ret, fd = *(int *)load->priv;

	if (smh_seek(fd, sector))
		return 0;

	ret = smh_read(fd, buf, count);
	return ret < 0 ? 0 : ret;
}

static int spl_smh_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	const char *filename = CONFIG_SPL_FS_LOAD_PAYLOAD_NAME;
	int ret;
	long fd, len;
	struct spl_load_info load = {
		.bl_len = 1,
		.read = spl_smh_fit_read,
	};

	fd = smh_open(filename, MODE_READ | MODE_BINARY);
	if (fd < 0) {
		log_debug("could not open %s: %ld\n", filename, fd);
		return fd;
	}
	load.priv = &fd;

	ret = smh_flen(fd);
	if (ret < 0) {
		log_debug("could not get length of image: %d\n", ret);
		goto out;
	}
	len = ret;

	ret = spl_load(spl_image, bootdev, &load, len, 0);
	if (ret)
		log_debug("could not read %s: %d\n", filename, ret);
out:
	smh_close(fd);
	return ret;
}
SPL_LOAD_IMAGE_METHOD("SEMIHOSTING", 0, BOOT_DEVICE_SMH, spl_smh_load_image);
