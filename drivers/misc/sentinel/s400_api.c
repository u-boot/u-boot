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
#include <asm/mach-imx/s400_api.h>
#include <misc.h>

DECLARE_GLOBAL_DATA_PTR;

int ahab_release_rdc(u8 core_id, u8 xrdc, u32 *response)
{
	struct udevice *dev = gd->arch.s400_dev;
	int size = sizeof(struct sentinel_msg);
	struct sentinel_msg msg;
	int ret;

	if (!dev) {
		printf("s400 dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = AHAB_VERSION;
	msg.tag = AHAB_CMD_TAG;
	msg.size = 2;
	msg.command = AHAB_RELEASE_RDC_REQ_CID;
	switch (xrdc) {
	case 0:
		msg.data[0] = (0x74 << 8) | core_id;
		break;
	case 1:
		msg.data[0] = (0x78 << 8) | core_id;
		break;
	case 2:
		msg.data[0] = (0x82 << 8) | core_id;
		break;
	case 3:
		msg.data[0] = (0x86 << 8) | core_id;
		break;
	default:
		printf("Error: wrong xrdc index %u\n", xrdc);
		return -EINVAL;
	}

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
	int size = sizeof(struct sentinel_msg);
	struct sentinel_msg msg;
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
	int size = sizeof(struct sentinel_msg);
	struct sentinel_msg msg;
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
	int size = sizeof(struct sentinel_msg);
	struct sentinel_msg msg;
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
	int size = sizeof(struct sentinel_msg);
	struct sentinel_msg msg;
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
	int size = sizeof(struct sentinel_msg);
	struct sentinel_msg msg;
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
	int size = sizeof(struct sentinel_msg);
	struct sentinel_msg msg;
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

int ahab_release_caam(u32 core_did, u32 *response)
{
	struct udevice *dev = gd->arch.s400_dev;
	int size = sizeof(struct sentinel_msg);
	struct sentinel_msg msg;
	int ret;

	if (!dev) {
		printf("s400 dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = AHAB_VERSION;
	msg.tag = AHAB_CMD_TAG;
	msg.size = 2;
	msg.command = AHAB_CAAM_RELEASE_CID;
	msg.data[0] = core_did;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, response 0x%x\n",
		       __func__, ret, msg.data[0]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ahab_get_fw_version(u32 *fw_version, u32 *sha1, u32 *response)
{
	struct udevice *dev = gd->arch.s400_dev;
	int size = sizeof(struct sentinel_msg);
	struct sentinel_msg msg;
	int ret;

	if (!dev) {
		printf("s400 dev is not initialized\n");
		return -ENODEV;
	}

	if (!fw_version) {
		printf("Invalid parameters for f/w version read\n");
		return -EINVAL;
	}

	if (!sha1) {
		printf("Invalid parameters for commit sha1\n");
		return -EINVAL;
	}

	msg.version = AHAB_VERSION;
	msg.tag = AHAB_CMD_TAG;
	msg.size = 1;
	msg.command = AHAB_GET_FW_VERSION_CID;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, response 0x%x\n",
		       __func__, ret, msg.data[0]);

	if (response)
		*response = msg.data[0];

	*fw_version = msg.data[1];
	*sha1 = msg.data[2];

	return ret;
}

int ahab_dump_buffer(u32 *buffer, u32 buffer_length)
{
	struct udevice *dev = gd->arch.s400_dev;
	int size = sizeof(struct sentinel_msg);
	struct sentinel_msg msg;
	int ret, i = 0;

	if (!dev) {
		printf("s400 dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = AHAB_VERSION;
	msg.tag = AHAB_CMD_TAG;
	msg.size = 1;
	msg.command = AHAB_LOG_CID;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret) {
		printf("Error: %s: ret %d, response 0x%x\n",
		       __func__, ret, msg.data[0]);

		return ret;
	}

	if (buffer) {
		buffer[i++] = *(u32 *)&msg; /* Need dump the response header */
		for (; i < buffer_length && i < msg.size; i++)
			buffer[i] = msg.data[i - 1];
	}

	return i;
}

int ahab_get_info(struct sentinel_get_info_data *info, u32 *response)
{
	struct udevice *dev = gd->arch.s400_dev;
	int size = sizeof(struct sentinel_msg);
	struct sentinel_msg msg;
	int ret;

	if (!dev) {
		printf("s400 dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = AHAB_VERSION;
	msg.tag = AHAB_CMD_TAG;
	msg.size = 4;
	msg.command = AHAB_GET_INFO_CID;
	msg.data[0] = upper_32_bits((ulong)info);
	msg.data[1] = lower_32_bits((ulong)info);
	msg.data[2] = sizeof(struct sentinel_get_info_data);

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, response 0x%x\n",
		       __func__, ret, msg.data[0]);

	if (response)
		*response = msg.data[0];

	return ret;
}

int ahab_get_fw_status(u32 *status, u32 *response)
{
	struct udevice *dev = gd->arch.s400_dev;
	int size = sizeof(struct sentinel_msg);
	struct sentinel_msg msg;
	int ret;

	if (!dev) {
		printf("s400 dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = AHAB_VERSION;
	msg.tag = AHAB_CMD_TAG;
	msg.size = 1;
	msg.command = AHAB_GET_FW_STATUS_CID;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, response 0x%x\n",
		       __func__, ret, msg.data[0]);

	if (response)
		*response = msg.data[0];

	*status = msg.data[1] & 0xF;

	return ret;
}

int ahab_release_m33_trout(void)
{
	struct udevice *dev = gd->arch.s400_dev;
	int size = sizeof(struct sentinel_msg);
	struct sentinel_msg msg;
	int ret;

	if (!dev) {
		printf("s400 dev is not initialized\n");
		return -ENODEV;
	}

	msg.version = AHAB_VERSION;
	msg.tag = AHAB_CMD_TAG;
	msg.size = 1;
	msg.command = 0xd3;

	ret = misc_call(dev, false, &msg, size, &msg, size);
	if (ret)
		printf("Error: %s: ret %d, response 0x%x\n",
		       __func__, ret, msg.data[0]);

	return ret;
}
