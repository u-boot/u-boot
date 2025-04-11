// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Amar <amarendra.xt@samsung.com>
 */

#include <log.h>
#include <mmc.h>
#include "mmc_private.h"

static int mmc_resize_boot_micron(struct mmc *mmc, unsigned long bootsize,
				  unsigned long rpmbsize)
{
	int err;

	/* Micron eMMC doesn't support resizing RPMB partition */
	(void)rpmbsize;

	/* BOOT partition size is multiple of 128KB */
	bootsize = (bootsize * 1024) / 128;

	if (bootsize > 0xff)
		bootsize = 0xff;

	/* Set EXT_CSD[175] ERASE_GROUP_DEF to 0x01 */
	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
			 EXT_CSD_ERASE_GROUP_DEF, 0x01);
	if (err)
		goto error;

	/* Set EXT_CSD[127:125] for BOOT partition size, [125] is low byte */
	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
			 EXT_CSD_BOOT_SIZE_MULT_MICRON, bootsize);
	if (err)
		goto error;

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
			 EXT_CSD_BOOT_SIZE_MULT_MICRON + 1, 0x00);
	if (err)
		goto error;

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
			 EXT_CSD_BOOT_SIZE_MULT_MICRON + 2, 0x00);
	if (err)
		goto error;

	/* Set EXT_CSD[155] PARTITION_SETTING_COMPLETE to 0x01 */
	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
			 EXT_CSD_PARTITION_SETTING, 0x01);
	if (err)
		goto error;

	return 0;

error:
	debug("%s: Error = %d\n", __func__, err);
	return err;
}

static int mmc_resize_boot_sandisk(struct mmc *mmc, unsigned long bootsize,
				   unsigned long rpmbsize)
{
	int err;
	struct mmc_cmd cmd;

	/* BOOT/RPMB partition size is multiple of 128KB */
	bootsize = (bootsize * 1024) / 128;
	rpmbsize = (rpmbsize * 1024) / 128;

	if (bootsize > 0xff)
		bootsize = 0xff;

	if (rpmbsize > 0xff)
		rpmbsize = 0xff;

	/* Send BOOT/RPMB resize op code */
	cmd.cmdidx = MMC_CMD_RES_MAN;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = MMC_CMD62_ARG_SANDISK;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto error;

	/* Arg: BOOT partition size */
	cmd.cmdidx = MMC_CMD_RES_MAN;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = bootsize;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto error;

	/* Arg: RPMB partition size */
	cmd.cmdidx = MMC_CMD_RES_MAN;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = rpmbsize;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto error;

	return 0;

error:
	debug("%s: Error = %d\n", __func__, err);
	return err;
}

static int mmc_resize_boot_samsung(struct mmc *mmc, unsigned long bootsize,
				   unsigned long rpmbsize)
{
	int err;
	struct mmc_cmd cmd;

	/* Only use this command for raw EMMC moviNAND. Enter backdoor mode */
	cmd.cmdidx = MMC_CMD_RES_MAN;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = MMC_CMD62_ARG1;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto error;

	/* Boot partition changing mode */
	cmd.cmdidx = MMC_CMD_RES_MAN;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = MMC_CMD62_ARG2;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto error;

	/* boot partition size is multiple of 128KB */
	bootsize = (bootsize * 1024) / 128;

	/* Arg: boot partition size */
	cmd.cmdidx = MMC_CMD_RES_MAN;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = bootsize;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto error;

	/* RPMB partition size is multiple of 128KB */
	rpmbsize = (rpmbsize * 1024) / 128;
	/* Arg: RPMB partition size */
	cmd.cmdidx = MMC_CMD_RES_MAN;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = rpmbsize;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		goto error;

	return 0;

error:
	debug("%s: Error = %d\n", __func__, err);
	return err;
}

/*
 * This function changes the size of BOOT partition and the size of RPMB
 * partition present on eMMC devices.
 *
 * Input Parameters:
 * struct *mmc: pointer for the mmc device strcuture
 * bootsize: size of BOOT partition
 * rpmbsize: size of RPMB partition
 *
 * Returns 0 on success.
 */

int mmc_boot_partition_size_change(struct mmc *mmc, unsigned long bootsize,
				   unsigned long rpmbsize)
{
	switch (mmc->cid[0] >> 24) {
	case CID_MANFID_MICRON:
		return mmc_resize_boot_micron(mmc, bootsize, rpmbsize);
	case CID_MANFID_SAMSUNG:
		return mmc_resize_boot_samsung(mmc, bootsize, rpmbsize);
	case CID_MANFID_SANDISK:
		return mmc_resize_boot_sandisk(mmc, bootsize, rpmbsize);
	default:
		printf("Unsupported manufacturer id 0x%02x\n",
		       mmc->cid[0] >> 24);
		return -EPERM;
	}
}

/*
 * Modify EXT_CSD[177] which is BOOT_BUS_WIDTH
 * based on the passed in values for BOOT_BUS_WIDTH, RESET_BOOT_BUS_WIDTH
 * and BOOT_MODE.
 *
 * Returns 0 on success.
 */
int mmc_set_boot_bus_width(struct mmc *mmc, u8 width, u8 reset, u8 mode)
{
	return mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BOOT_BUS_WIDTH,
			  EXT_CSD_BOOT_BUS_WIDTH_MODE(mode) |
			  EXT_CSD_BOOT_BUS_WIDTH_RESET(reset) |
			  EXT_CSD_BOOT_BUS_WIDTH_WIDTH(width));
}

/*
 * Modify EXT_CSD[179] which is PARTITION_CONFIG (formerly BOOT_CONFIG)
 * based on the passed in values for BOOT_ACK, BOOT_PARTITION_ENABLE and
 * PARTITION_ACCESS.
 *
 * Returns 0 on success.
 */
int mmc_set_part_conf(struct mmc *mmc, u8 ack, u8 part_num, u8 access)
{
	int ret;
	u8 part_conf;

	part_conf = EXT_CSD_BOOT_ACK(ack) |
		    EXT_CSD_BOOT_PART_NUM(part_num) |
		    EXT_CSD_PARTITION_ACCESS(access);

	ret = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PART_CONF,
			 part_conf);
	if (!ret)
		mmc->part_config = part_conf;

	return ret;
}

/*
 * Modify EXT_CSD[162] which is RST_n_FUNCTION based on the given value
 * for enable.  Note that this is a write-once field for non-zero values.
 *
 * Returns 0 on success.
 */
int mmc_set_rst_n_function(struct mmc *mmc, u8 enable)
{
	return mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_RST_N_FUNCTION,
			  enable);
}
