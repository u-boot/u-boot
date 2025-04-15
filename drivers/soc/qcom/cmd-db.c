/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016-2018, 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#define pr_fmt(fmt) "cmd-db: " fmt

#include <asm/system.h>
#include <dm.h>
#include <dm/ofnode.h>
#include <dm/device_compat.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/ioport.h>
#include <linux/byteorder/generic.h>

#include <soc/qcom/cmd-db.h>

#define NUM_PRIORITY		2
#define MAX_SLV_ID		8
#define SLAVE_ID_MASK		0x7
#define SLAVE_ID_SHIFT		16
#define SLAVE_ID(addr)		FIELD_GET(GENMASK(19, 16), addr)
#define VRM_ADDR(addr)		FIELD_GET(GENMASK(19, 4), addr)

/**
 * struct entry_header: header for each entry in cmddb
 *
 * @id: resource's identifier
 * @priority: unused
 * @addr: the address of the resource
 * @len: length of the data
 * @offset: offset from :@data_offset, start of the data
 */
struct entry_header {
	u8 id[8];
	__le32 priority[NUM_PRIORITY];
	__le32 addr;
	__le16 len;
	__le16 offset;
};

/**
 * struct rsc_hdr: resource header information
 *
 * @slv_id: id for the resource
 * @header_offset: entry's header at offset from the end of the cmd_db_header
 * @data_offset: entry's data at offset from the end of the cmd_db_header
 * @cnt: number of entries for HW type
 * @version: MSB is major, LSB is minor
 * @reserved: reserved for future use.
 */
struct rsc_hdr {
	__le16 slv_id;
	__le16 header_offset;
	__le16 data_offset;
	__le16 cnt;
	__le16 version;
	__le16 reserved[3];
};

/**
 * struct cmd_db_header: The DB header information
 *
 * @version: The cmd db version
 * @magic: constant expected in the database
 * @header: array of resources
 * @checksum: checksum for the header. Unused.
 * @reserved: reserved memory
 * @data: driver specific data
 */
struct cmd_db_header {
	__le32 version;
	u8 magic[4];
	struct rsc_hdr header[MAX_SLV_ID];
	__le32 checksum;
	__le32 reserved;
	u8 data[];
};

/**
 * DOC: Description of the Command DB database.
 *
 * At the start of the command DB memory is the cmd_db_header structure.
 * The cmd_db_header holds the version, checksum, magic key as well as an
 * array for header for each slave (depicted by the rsc_header). Each h/w
 * based accelerator is a 'slave' (shared resource) and has slave id indicating
 * the type of accelerator. The rsc_header is the header for such individual
 * slaves of a given type. The entries for each of these slaves begin at the
 * rsc_hdr.header_offset. In addition each slave could have auxiliary data
 * that may be needed by the driver. The data for the slave starts at the
 * entry_header.offset to the location pointed to by the rsc_hdr.data_offset.
 *
 * Drivers have a stringified key to a slave/resource. They can query the slave
 * information and get the slave id and the auxiliary data and the length of the
 * data. Using this information, they can format the request to be sent to the
 * h/w accelerator and request a resource state.
 */

static const u8 CMD_DB_MAGIC[] = { 0xdb, 0x30, 0x03, 0x0c };

static bool cmd_db_magic_matches(const struct cmd_db_header *header)
{
	const u8 *magic = header->magic;

	return memcmp(magic, CMD_DB_MAGIC, ARRAY_SIZE(CMD_DB_MAGIC)) == 0;
}

static struct cmd_db_header *cmd_db_header __section(".data") = NULL;

static inline const void *rsc_to_entry_header(const struct rsc_hdr *hdr)
{
	u16 offset = le16_to_cpu(hdr->header_offset);

	return cmd_db_header->data + offset;
}

static inline void *
rsc_offset(const struct rsc_hdr *hdr, const struct entry_header *ent)
{
	u16 offset = le16_to_cpu(hdr->data_offset);
	u16 loffset = le16_to_cpu(ent->offset);

	return cmd_db_header->data + offset + loffset;
}

static int cmd_db_get_header(const char *id, const struct entry_header **eh,
			     const struct rsc_hdr **rh)
{
	const struct rsc_hdr *rsc_hdr;
	const struct entry_header *ent;
	int i, j;
	u8 query[sizeof(ent->id)] __nonstring;

	strncpy(query, id, sizeof(query));

	for (i = 0; i < MAX_SLV_ID; i++) {
		rsc_hdr = &cmd_db_header->header[i];
		if (!rsc_hdr->slv_id)
			break;

		ent = rsc_to_entry_header(rsc_hdr);
		for (j = 0; j < le16_to_cpu(rsc_hdr->cnt); j++, ent++) {
			if (strncmp(ent->id, query, sizeof(ent->id)) == 0) {
				if (eh)
					*eh = ent;
				if (rh)
					*rh = rsc_hdr;
				return 0;
			}
		}
	}

	return -ENODEV;
}

/**
 * cmd_db_read_addr() - Query command db for resource id address.
 *
 * @id: resource id to query for address
 *
 * Return: resource address on success, 0 on error
 *
 * This is used to retrieve resource address based on resource
 * id.
 */
u32 cmd_db_read_addr(const char *id)
{
	int ret;
	const struct entry_header *ent;

	debug("%s(%s)\n", __func__, id);

	if (!cmd_db_header) {
		log_err("%s: Command DB not initialized\n", __func__);
		return 0;
	}

	ret = cmd_db_get_header(id, &ent, NULL);

	return ret < 0 ? 0 : le32_to_cpu(ent->addr);
}
EXPORT_SYMBOL_GPL(cmd_db_read_addr);

static int cmd_db_bind(struct udevice *dev)
{
	void __iomem *base;
	fdt_size_t size;
	ofnode node;

	if (cmd_db_header)
		return 0;

	node = dev_ofnode(dev);

	debug("%s(%s)\n", __func__, ofnode_get_name(node));

	base = (void __iomem *)ofnode_get_addr_size(node, "reg", &size);
	if ((fdt_addr_t)base == FDT_ADDR_T_NONE) {
		log_err("%s: Failed to read base address\n", __func__);
		return -ENOENT;
	}

	/* On SM8550/SM8650 and newer SoCs cmd-db might not be mapped */
	mmu_map_region((phys_addr_t)base, (phys_size_t)size, false);

	cmd_db_header = base;
	if (!cmd_db_magic_matches(cmd_db_header)) {
		log_err("%s: Invalid Command DB Magic\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static const struct udevice_id cmd_db_ids[] = {
	{ .compatible = "qcom,cmd-db" },
	{ }
};

U_BOOT_DRIVER(qcom_cmd_db) = {
	.name		= "qcom_cmd_db",
	.id		= UCLASS_MISC,
	.bind		= cmd_db_bind,
	.of_match	= cmd_db_ids,
};

MODULE_DESCRIPTION("Qualcomm Technologies, Inc. Command DB Driver");
MODULE_LICENSE("GPL v2");
