// SPDX-License-Identifier: GPL-2.0+
/*
 * Microchip's PolarFire SoC (MPFS) System Controller Driver
 *
 * Copyright (C) 2024 Microchip Technology Inc. All rights reserved.
 *
 * Author: Jamie Gibbons <jamie.gibbons@microchip.com>
 *
 */

#include <asm/system.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <env.h>
#include <errno.h>
#include <linux/compat.h>
#include <linux/completion.h>
#include <linux/err.h>
#include <linux/mtd/mtd.h>
#include <log.h>
#include <mailbox.h>
#include <misc.h>
#include <mpfs-mailbox.h>

#define SYS_SPI_CMD						0x50
#define SYS_SPI_MAILBOX_DATA_LEN		17
#define SYS_SPI_MAILBOX_SRC_OFFSET		8
#define SYS_SPI_MAILBOX_LENGTH_OFFSET	12
#define SYS_SPI_MAILBOX_FREQ_OFFSET		16
#define SYS_SPI_MAILBOX_FREQ			3
#define SPI_FLASH_ADDR					0x400

/* Descriptor table */
#define START_OFFSET					4
#define END_OFFSET						8
#define SIZE_OFFSET						12
#define DESC_NEXT						12
#define DESC_RESERVED_SIZE				0
#define DESC_SIZE						16

#define DESIGN_MAGIC_0					0x4d /* 'M' */
#define DESIGN_MAGIC_1					0x43 /* 'C'*/
#define DESIGN_MAGIC_2					0x48 /* 'H'*/
#define DESIGN_MAGIC_3					0x50 /* 'P'*/

#define CMD_OPCODE						0x0u
#define CMD_DATA_SIZE					0U
#define CMD_DATA						NULL
#define MBOX_OFFSET						0x0
#define RESP_OFFSET						0x0
#define RESP_BYTES						16U

/**
 * struct mpfs_syscontroller_priv - Structure representing System Controller data.
 * @chan:	Mailbox channel
 * @c:	Completion signal
 */
struct mpfs_syscontroller_priv {
	struct mbox_chan chan;
	struct completion c;
};

/**
 * mpfs_syscontroller_run_service() - Run the MPFS system service
 * @sys_controller:	corresponding MPFS system service device
 * @msg:	Message to send
 *
 * Return: 0 if all goes good, else appropriate error message.
 */
int mpfs_syscontroller_run_service(struct mpfs_syscontroller_priv *sys_controller, struct mpfs_mss_msg *msg)
{
	int ret;

	reinit_completion(&sys_controller->c);

	/* Run the System Service Request */
	ret = mbox_send(&sys_controller->chan, msg);
	if (ret < 0)
		dev_warn(sys_controller->chan.dev, "MPFS sys controller service timeout\n");

	debug("%s: Service successful %s\n",
	      __func__, sys_controller->chan.dev->name);

	return ret;
}
EXPORT_SYMBOL_GPL(mpfs_syscontroller_run_service);

/**
 * mpfs_syscontroller_read_sernum() - Use system service to read the device serial number
 * @sys_serv_priv:	system service private data
 * @device_serial_number:	device serial number
 *
 * Return: 0 if all went ok, else return appropriate error
 */
int mpfs_syscontroller_read_sernum(struct mpfs_sys_serv *sys_serv_priv, u8 *device_serial_number)
{
	unsigned long timeoutsecs = 300;
	int ret;

	struct mpfs_mss_response response = {
		.resp_status = 0U,
		.resp_msg = (u32 *)device_serial_number,
		.resp_size = RESP_BYTES};
	struct mpfs_mss_msg msg = {
		.cmd_opcode = CMD_OPCODE,
		.cmd_data_size = CMD_DATA_SIZE,
		.response = &response,
		.cmd_data = CMD_DATA,
		.mbox_offset = MBOX_OFFSET,
		.resp_offset = RESP_OFFSET};

	ret = mpfs_syscontroller_run_service(sys_serv_priv->sys_controller, &msg);
	if (ret) {
		dev_err(sys_serv_priv->sys_controller->chan.dev, "Service failed: %d, abort\n", ret);
		return ret;
	}

	/* Receive the response */
	ret = mbox_recv(&sys_serv_priv->sys_controller->chan, &msg, timeoutsecs);
	if (ret) {
		dev_err(sys_serv_priv->sys_controller->chan.dev, "Service failed: %d, abort. Failure: %u\n", ret, msg.response->resp_status);
		return ret;
	}

	debug("%s: Read successful %s\n",
	      __func__, sys_serv_priv->sys_controller->chan.dev->name);

	return 0;
}
EXPORT_SYMBOL(mpfs_syscontroller_read_sernum);

static u16 mpfs_syscontroller_service_spi_copy(struct mpfs_sys_serv *sys_serv_priv, u64 dst_addr, u32 src_addr, u32 length)
{
	int ret;
	u32 mailbox_format[SYS_SPI_MAILBOX_DATA_LEN];

	*(u64 *)mailbox_format = dst_addr;
	mailbox_format[SYS_SPI_MAILBOX_SRC_OFFSET/4] = src_addr;
	mailbox_format[SYS_SPI_MAILBOX_LENGTH_OFFSET/4] = length;
	mailbox_format[SYS_SPI_MAILBOX_FREQ_OFFSET/4] = SYS_SPI_MAILBOX_FREQ;

	struct mpfs_mss_response response = {
		.resp_status = 0U,
		.resp_msg = mailbox_format,
		.resp_size = RESP_BYTES};
	struct mpfs_mss_msg msg = {
		.cmd_opcode = SYS_SPI_CMD,
		.cmd_data_size = SYS_SPI_MAILBOX_DATA_LEN,
		.response = &response,
		.cmd_data = (u8 *)mailbox_format,
		.mbox_offset = MBOX_OFFSET,
		.resp_offset = RESP_OFFSET};

	ret = mpfs_syscontroller_run_service(sys_serv_priv->sys_controller, &msg);
	if (ret) {
		dev_err(sys_serv_priv->sys_controller->chan.dev, "Service failed: %d, abort. Failure: %u\n", ret, msg.response->resp_status);
	}

	return ret;
}

static u16 mpfs_syscontroller_get_dtbo_desc_header(struct mpfs_sys_serv *sys_serv_priv, u8 *desc_data, u32 desc_addr)
{
	u32 length, no_of_descs;
	int ret = -ENOENT;

	/* Get first four bytes to calculate length */
	ret = mpfs_syscontroller_service_spi_copy(sys_serv_priv, (u64)desc_data, desc_addr, BYTES_4);
	if (!ret) {
		no_of_descs = *((u32 *)desc_data);
		if (no_of_descs) {
			length = DESC_SIZE + ((no_of_descs - 1) * DESC_SIZE);
			ret = mpfs_syscontroller_service_spi_copy(sys_serv_priv, (u64)desc_data, desc_addr,
						      length);
		}
	}

	return ret;
}

static u8 *mpfs_syscontroller_get_dtbo(struct mpfs_sys_serv *sys_serv_priv, u32 start_addr, u32 size)
{
	int ret;
	u8 *dtbo;

	/* Intentionally never freed, even on success so that u-boot "userspace" can access it. */
	dtbo = (u8 *)malloc(size);

	ret = mpfs_syscontroller_service_spi_copy(sys_serv_priv, (u64)dtbo, start_addr, size);
	if (ret) {
		free(dtbo);
		dtbo = NULL;
	}

	return dtbo;
}

static void mpfs_syscontroller_parse_desc_header(struct mpfs_sys_serv *sys_serv_priv, u8 *desc_header, u8 *no_of_dtbo, u32 *dtbos_size)
{
	u32 dtbo_desc_start_addr;
	u32 dtbo_desc_size;
	u32 no_of_descs;
	u16 i;
	u8 dtbo_name[16];
	u8 dtbo_addr[20];
	u8 *desc;
	u8 *dtbo;

	no_of_descs = *((u32 *)desc_header);

	for (i = 0; i < no_of_descs; i++) {
		desc = &desc_header[START_OFFSET + (DESC_NEXT * i)];
		/*
		 * The dtbo info structure contains addresses that are relative
		 * to the start of structure, so the offset of the structure in
		 * flash must be added to get the actual start address.
		 */
		dtbo_desc_start_addr = *((u32 *)desc) + SPI_FLASH_ADDR;

		desc = &desc_header[SIZE_OFFSET + (DESC_NEXT * i)];
		dtbo_desc_size = *((u32 *)desc);

		dtbo = mpfs_syscontroller_get_dtbo(sys_serv_priv, dtbo_desc_start_addr, dtbo_desc_size);
		if (dtbo) {
			sprintf(dtbo_name, "dtbo_image%d", *no_of_dtbo);
			sprintf(dtbo_addr, "0x%llx", (u64)dtbo);
			env_set(dtbo_name, dtbo_addr);
			++*no_of_dtbo;
			*dtbos_size += dtbo_desc_size;
		}
	}
}

void mpfs_syscontroller_process_dtbo(struct mpfs_sys_serv *sys_serv_priv)
{
	u32 desc_length;
	u32 dtbo_desc_addr;
	u32 dtbo_addr[5];
	u16 i, hart, no_of_harts;
	u8 design_info_desc[256];
	u8 dtbo_desc_data[256];
	u8 no_of_dtbos[8];
	u8 dtbo_size[8];
	u8 *desc;
	u8 no_of_dtbo = 0;
	u32 dtbos_size = 0;
	int ret;

	/* Read first 10 bytes to verify the descriptor is found or not */
	ret = mpfs_syscontroller_service_spi_copy(sys_serv_priv, (u64)design_info_desc, SPI_FLASH_ADDR, 10);
	if (ret) {
		sprintf(no_of_dtbos, "%d", no_of_dtbo);
		env_set("no_of_overlays", no_of_dtbos);
		sprintf(dtbo_size, "%d", dtbos_size);
		env_set("dtbo_size", dtbo_size);
		return;
	}

	if (design_info_desc[0] != DESIGN_MAGIC_0 ||
	    design_info_desc[1] != DESIGN_MAGIC_1 ||
	    design_info_desc[2] != DESIGN_MAGIC_2 ||
	    design_info_desc[3] != DESIGN_MAGIC_3) {
		dev_dbg(sys_serv_priv->dev, "magic not found in desc structure.\n");
		sprintf(no_of_dtbos, "%d", no_of_dtbo);
		env_set("no_of_overlays", no_of_dtbos);
		sprintf(dtbo_size, "%d", dtbos_size);
		env_set("dtbo_size", dtbo_size);
		return;
	}
	desc_length = *((u32 *)&design_info_desc[4]);
	/* Read Design descriptor */
	ret = mpfs_syscontroller_service_spi_copy(sys_serv_priv, (u64)design_info_desc,
						SPI_FLASH_ADDR, desc_length);
	if (ret)
		return;

	no_of_harts = *((u16 *)&design_info_desc[10]);

	for (hart = 0; hart < no_of_harts; hart++) {
		/* Start address of DTBO descriptor */
		desc = &design_info_desc[(0x4 * hart) + 0xc];

		dtbo_desc_addr = *((u32 *)desc);
		dtbo_addr[hart] = dtbo_desc_addr;

		if (!dtbo_addr[hart])
			continue;

		for (i = 0; i < hart; i++) {
			if (dtbo_addr[hart] == dtbo_addr[i])
				continue;
		}

		if (hart && hart == i)
			continue;

		dtbo_desc_addr += SPI_FLASH_ADDR;
		ret = mpfs_syscontroller_get_dtbo_desc_header(sys_serv_priv, dtbo_desc_data,
							dtbo_desc_addr);
		if (ret)
			continue;
		else
			mpfs_syscontroller_parse_desc_header(sys_serv_priv, dtbo_desc_data, &no_of_dtbo, &dtbos_size);
	}
	sprintf(no_of_dtbos, "%d", no_of_dtbo);
	env_set("no_of_overlays", no_of_dtbos);
	sprintf(dtbo_size, "%d", dtbos_size);
	env_set("dtbo_size", dtbo_size);
}
EXPORT_SYMBOL(mpfs_syscontroller_process_dtbo);

static int mpfs_syscontroller_probe(struct udevice *dev)
{
	struct mpfs_syscontroller_priv *sys_controller = dev_get_priv(dev);
	int ret;

	ret = mbox_get_by_index(dev, 0, &sys_controller->chan);
	if (ret) {
		dev_err(dev, "%s: Acquiring mailbox channel failed. ret = %d\n",
			__func__, ret);
		return ret;
	}

	init_completion(&sys_controller->c);
	dev_info(dev, "Registered MPFS system controller\n");

	return 0;
}

static const struct udevice_id mpfs_syscontroller_ids[] = {
	{ .compatible = "microchip,mpfs-sys-controller" },
	{ }
};

struct mpfs_syscontroller_priv *mpfs_syscontroller_get(struct udevice *dev)
{
	struct mpfs_syscontroller_priv *sys_controller;

	sys_controller = dev_get_priv(dev);
	if (!sys_controller) {
		debug("%s: MPFS system controller found but could not register as a sub device %p\n",
		      __func__, sys_controller);
		return ERR_PTR(-EPROBE_DEFER);
	}

	return sys_controller;
}
EXPORT_SYMBOL(mpfs_syscontroller_get);

U_BOOT_DRIVER(mpfs_syscontroller) = {
	.name           = "mpfs_syscontroller",
	.id             = UCLASS_MISC,
	.of_match       = mpfs_syscontroller_ids,
	.probe          = mpfs_syscontroller_probe,
	.priv_auto	= sizeof(struct mpfs_syscontroller_priv),
};
