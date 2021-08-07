// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2020 NXP
 *
 */

#include <common.h>
#include <hang.h>
#include <malloc.h>
#include <asm/io.h>
#include <dm.h>
#include <asm/arch/s400_api.h>
#include <misc.h>

DECLARE_GLOBAL_DATA_PTR;

int ahab_release_rdc(u8 core_id)
{
	struct udevice *dev = gd->arch.s400_dev;
	int size = sizeof(struct imx8ulp_s400_msg);
	struct imx8ulp_s400_msg msg;
	int ret;

	if (!dev) {
		printf("s400 dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = AHAB_VERSION;
	msg.tag = AHAB_CMD_TAG;
	msg.size = 2;
	msg.command = AHAB_RELEASE_RDC_REQ_CID;
	msg.data[0] = core_id;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, core id %u, response 0x%x\n",
		       __func__, ret, core_id, msg.data[0]);

	return ret;
}
