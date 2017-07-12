/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <dm/of_addr.h>
#include <dm/read.h>
#include <linux/ioport.h>

int dev_read_resource(struct udevice *dev, uint index, struct resource *res)
{
	ofnode node = dev_ofnode(dev);

#ifdef CONFIG_OF_LIVE
	if (ofnode_is_np(node)) {
		return of_address_to_resource(ofnode_to_np(node), index, res);
	} else
#endif
		{
		struct fdt_resource fres;
		int ret;

		ret = fdt_get_resource(gd->fdt_blob, ofnode_to_offset(node),
				       "reg", index, &fres);
		if (ret < 0)
			return -EINVAL;
		memset(res, '\0', sizeof(*res));
		res->start = fres.start;
		res->end = fres.end;

		return 0;
	}
}
