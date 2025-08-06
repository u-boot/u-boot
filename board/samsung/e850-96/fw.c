// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 Linaro Ltd.
 * Author: Sam Protsenko <semen.protsenko@linaro.org>
 *
 * Firmware loading code.
 */

#include <part.h>
#include <fs.h>
#include <linux/arm-smccc.h>
#include "fw.h"

#define LDFW_RAW_PART		"ldfw"
#define LDFW_FAT_PATH		"/EFI/firmware/ldfw.bin"

#define LDFW_MAGIC		0x10adab1e
#define SMC_CMD_LOAD_LDFW	-0x500
#define SDM_HW_RESET_STATUS	0x1230
#define SDM_SW_RESET_STATUS	0x1231
#define SB_ERROR_PREFIX		0xfdaa0000

struct ldfw_header {
	u32 magic;
	u32 size;
	u32 init_entry;
	u32 entry_point;
	u32 suspend_entry;
	u32 resume_entry;
	u32 start_smc_id;
	u32 version;
	u32 set_runtime_entry;
	u32 reserved[3];
	char fw_name[16];
};

/* Load LDFW binary as a file from FAT partition */
static int read_fw_from_fat(const char *ifname, int dev, int part,
			    const char *path, void *buf)
{
	struct blk_desc *blk_desc;
	loff_t len_read;
	int err;

	blk_desc = blk_get_dev(ifname, dev);
	if (!blk_desc) {
		debug("%s: Can't get block device\n", __func__);
		return -ENODEV;
	}

	err = fs_set_blk_dev_with_part(blk_desc, part);
	if (err) {
		debug("%s: Can't set partition\n", __func__);
		return -ENOENT;
	}

	err = fs_read(path, (ulong)buf, 0, 0, &len_read);
	if (err) {
		debug("%s: Can't read LDFW file\n", __func__);
		return -EIO;
	}

	return 0;
}

/* Load LDFW binary from raw partition on block device into RAM buffer */
static int read_fw_from_raw(const char *ifname, int dev, const char *part_name,
			    void *buf)
{
	struct blk_desc *blk_desc;
	struct disk_partition part;
	unsigned long cnt;
	int part_num;

	blk_desc = blk_get_dev(ifname, dev);
	if (!blk_desc) {
		debug("%s: Can't get block device\n", __func__);
		return -ENODEV;
	}

	part_num = part_get_info_by_name(blk_desc, part_name, &part);
	if (part_num < 0) {
		debug("%s: Can't get LDWF partition\n", __func__);
		return -ENOENT;
	}

	cnt = blk_dread(blk_desc, part.start, part.size, buf);
	if (cnt != part.size) {
		debug("%s: Can't read LDFW partition\n", __func__);
		return -EIO;
	}

	return 0;
}

/**
 * load_ldfw - Load the loadable firmware (LDFW)
 * @ifname: Interface name of the block device to load the firmware from
 * @dev: Device number
 * @part: Partition number
 * @addr: Temporary memory (Normal World) to use for loading the firmware
 *
 * Return: 0 on success or a negative value on error.
 */
int load_ldfw(const char *ifname, int dev, int part, phys_addr_t addr)
{
	struct ldfw_header *hdr;
	struct arm_smccc_res res;
	void *buf = (void *)addr;
	u64 size = 0;
	int err, i;

	/* First try to read LDFW from EFI partition, then from the raw one */
	err = read_fw_from_fat(ifname, dev, part, LDFW_FAT_PATH, buf);
	if (err) {
		err = read_fw_from_raw(ifname, dev, LDFW_RAW_PART, buf);
		if (err)
			return err;
	}

	/* Validate LDFW by magic number in its header */
	hdr = buf;
	if (hdr->magic != LDFW_MAGIC) {
		debug("%s: Wrong LDFW magic; is LDFW flashed?\n", __func__);
		return -EINVAL;
	}

	/* Calculate actual total size of all LDFW blobs */
	for (i = 0; hdr->magic == LDFW_MAGIC; ++i) {
#ifdef DEBUG
		char name[17] = { 0 };

		strncpy(name, hdr->fw_name, 16);
		debug("%s: ldfw #%d: version = 0x%x, name = %s\n", __func__, i,
		      hdr->version, name);
#endif

		size += (u64)hdr->size;
		hdr = (struct ldfw_header *)((u64)hdr + (u64)hdr->size);
	}
	debug("%s: The whole size of all LDFWs: 0x%llx\n", __func__, size);

	/* Load LDFW firmware to SWD (Secure World) memory via EL3 monitor */
	arm_smccc_smc(SMC_CMD_LOAD_LDFW, addr, size, 0, 0, 0, 0, 0, &res);
	err = (int)res.a0;
	if (err == -1 || err == SDM_HW_RESET_STATUS) {
		debug("%s: Can't load LDFW in dump_gpr state\n", __func__);
		return -EIO;
	} else if (err == SDM_SW_RESET_STATUS) {
		debug("%s: Can't load LDFW in kernel panic (SW RESET) state\n",
		      __func__);
		return -EIO;
	} else if (err < 0 && (err & 0xffff0000) == SB_ERROR_PREFIX) {
		debug("%s: LDFW signature is corrupted! ret=0x%x\n", __func__,
		      (u32)err);
		return -EIO;
	} else if (err == 0) {
		debug("%s: No LDFW is inited\n", __func__);
		return -EIO;
	}

#ifdef DEBUG
	u32 tried = res.a0 & 0xffff;
	u32 failed = (res.a0 >> 16) & 0xffff;

	debug("%s: %d/%d LDFWs have been loaded successfully\n", __func__,
	      tried - failed, tried);
#endif

	return 0;
}
