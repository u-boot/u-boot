// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015
 * Lukasz Majewski <l.majewski@majess.pl>
 */

#include <common.h>
#include <log.h>
#include <malloc.h>
#include <errno.h>
#include <dfu.h>

/**
 * dfu_write_by_name() - write data to DFU medium
 * @dfu_entity_name:    Name of DFU entity to write
 * @addr:               Address of data buffer to write
 * @len:                Number of bytes
 * @interface:          Destination DFU medium (e.g. "mmc")
 * @devstring:          Instance number of destination DFU medium (e.g. "1")
 *
 * This function is storing data received on DFU supported medium which
 * is specified by @dfu_entity_name.
 *
 * Return:              0 - on success, error code - otherwise
 */
int dfu_write_by_name(char *dfu_entity_name, void *addr,
		      unsigned int len, char *interface, char *devstring)
{
	char *s, *sb;
	int alt_setting_num, ret;
	struct dfu_entity *dfu;

	debug("%s: name: %s addr: 0x%p len: %d device: %s:%s\n", __func__,
	      dfu_entity_name, addr, len, interface, devstring);

	ret = dfu_init_env_entities(interface, devstring);
	if (ret)
		goto done;

	/*
	 * We need to copy name pointed by *dfu_entity_name since this text
	 * is the integral part of the FDT image.
	 * Any implicit modification (i.e. done by strsep()) will corrupt
	 * the FDT image and prevent other images to be stored.
	 */
	s = strdup(dfu_entity_name);
	sb = s;
	if (!s) {
		ret = -ENOMEM;
		goto done;
	}

	strsep(&s, "@");
	debug("%s: image name: %s strlen: %zd\n", __func__, sb, strlen(sb));

	alt_setting_num = dfu_get_alt(sb);
	free(sb);
	if (alt_setting_num < 0) {
		pr_err("Alt setting [%d] to write not found!",
		       alt_setting_num);
		ret = -ENODEV;
		goto done;
	}

	dfu = dfu_get_entity(alt_setting_num);
	if (!dfu) {
		pr_err("DFU entity for alt: %d not found!", alt_setting_num);
		ret = -ENODEV;
		goto done;
	}

	ret = dfu_write_from_mem_addr(dfu, (void *)addr, len);

done:
	dfu_free_entities();

	return ret;
}
