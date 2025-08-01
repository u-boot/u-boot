/* SPDX-License-Identifier: GPL-2.0 */
/*
 *
 * Microchip PolarFire SoC (MPFS)
 *
 * Copyright (c) 2020 Microchip Corporation. All rights reserved.
 *
 *
 */

#ifndef _MPFS_MAILBOX_H__
#define _MPFS_MAILBOX_H__

#include <linux/types.h>

#define BYTES_4 4

struct udevice;

/**
 * struct mpfs_mss_msg - PolarFire SoC message structure
 * @cmd_opcode:	Command opcode
 * @cmd_data_size:	Size of the command data.
 * @response:	Pointer to the response data.
 * @cmd_data:	Pointer to the command data.
 * @mbox_offset:	Mailbox offset
 * @resp_offset:	Response offset
 *
 */
struct mpfs_mss_msg {
	u8 cmd_opcode;
	u16 cmd_data_size;
	struct mpfs_mss_response *response;
	u8 *cmd_data;
	u16 mbox_offset;
	u16 resp_offset;
};

/**
 * struct mpfs_mss_response - PolarFire SoC response structure
 * @resp_status:	Response status
 * @resp_msg:	Pointer to response message.
 * @resp_size:	Size of the response message.
 *
 */
struct mpfs_mss_response {
	u32 resp_status;
	u32 *resp_msg;
	u16 resp_size;
};

struct mpfs_syscontroller_priv;

struct mpfs_sys_serv {
	struct udevice *dev;
	struct mpfs_syscontroller_priv *sys_controller;
	struct mpfs_mss_msg *msg;
};

int mpfs_syscontroller_run_service(struct mpfs_syscontroller_priv *sys_controller, struct mpfs_mss_msg *msg);
int mpfs_syscontroller_read_sernum(struct mpfs_sys_serv *sys_serv_priv, u8 *device_serial_number);
void mpfs_syscontroller_process_dtbo(struct mpfs_sys_serv *sys_serv_priv);
struct mpfs_syscontroller_priv *mpfs_syscontroller_get(struct udevice *dev);

#endif /* __MPFS_MAILBOX_H__ */

