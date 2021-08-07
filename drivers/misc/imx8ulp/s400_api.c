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

int ahab_release_rdc(u8 core_id, bool xrdc, u32 *response)
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
	if (xrdc)
		msg.data[0] = (0x78 << 8) | core_id;
	else
		msg.data[0] = (0x74 << 8) | core_id;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, core id %u, response 0x%x\n",
		       __func__, ret, core_id, msg.data[0]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ahab_auth_oem_ctnr(ulong ctnr_addr, u32 *response)
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
	msg.size = 3;
	msg.command = AHAB_AUTH_OEM_CTNR_CID;
	msg.data[0] = upper_32_bits(ctnr_addr);
	msg.data[1] = lower_32_bits(ctnr_addr);

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, cntr_addr 0x%lx, response 0x%x\n",
		       __func__, ret, ctnr_addr, msg.data[0]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ahab_release_container(u32 *response)
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
	msg.size = 1;
	msg.command = AHAB_RELEASE_CTNR_CID;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, response 0x%x\n",
		       __func__, ret, msg.data[0]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ahab_verify_image(u32 img_id, u32 *response)
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
	msg.command = AHAB_VERIFY_IMG_CID;
	msg.data[0] = 1 << img_id;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, img_id %u, response 0x%x\n",
		       __func__, ret, img_id, msg.data[0]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ahab_forward_lifecycle(u16 life_cycle, u32 *response)
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
	msg.command = AHAB_FWD_LIFECYCLE_UP_REQ_CID;
	msg.data[0] = life_cycle;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, life_cycle 0x%x, response 0x%x\n",
		       __func__, ret, life_cycle, msg.data[0]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ahab_read_common_fuse(u16 fuse_id, u32 *fuse_words, u32 fuse_num, u32 *response)
{
	struct udevice *dev = gd->arch.s400_dev;
	int size = sizeof(struct imx8ulp_s400_msg);
	struct imx8ulp_s400_msg msg;
	int ret;

	if (!dev) {
		printf("s400 dev is not initialized\n");
		return -ENODEV;
	}

	if (!fuse_words) {
		printf("Invalid parameters for fuse read\n");
		return -EINVAL;
	}

	if ((fuse_id != 1 && fuse_num != 1) ||
	    (fuse_id == 1 && fuse_num != 4)) {
		printf("Invalid fuse number parameter\n");
		return -EINVAL;
	}

	msg.version = AHAB_VERSION;
	msg.tag = AHAB_CMD_TAG;
	msg.size = 2;
	msg.command = AHAB_READ_FUSE_REQ_CID;
	msg.data[0] = fuse_id;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, fuse_id 0x%x, response 0x%x\n",
		       __func__, ret, fuse_id, msg.data[0]);

	if (response)
		*response = msg.data[0];

	fuse_words[0] = msg.data[1];
	if (fuse_id == 1) {
		/* OTP_UNIQ_ID */
		fuse_words[1] = msg.data[2];
		fuse_words[2] = msg.data[3];
		fuse_words[3] = msg.data[4];
	}

	return ret;
}

int ahab_write_fuse(u16 fuse_id, u32 fuse_val, bool lock, u32 *response)
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
	msg.size = 3;
	msg.command = AHAB_WRITE_FUSE_REQ_CID;
	msg.data[0] = (32 << 16) | (fuse_id << 5);
	if (lock)
		msg.data[0] |= (1 << 31);

	msg.data[1] = fuse_val;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, fuse_id 0x%x, response 0x%x\n",
		       __func__, ret, fuse_id, msg.data[0]);

	if (response)
		*response = msg.data[0];

	return ret;
}
