/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Emulation of enough SCSI commands to find and read from a unit
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 *
 * implementations of SCSI functions required so that CONFIG_SCSI can be enabled
 * for sandbox
 */

#ifndef __scsi_emul_h
#define __scsi_emul_h

/**
 * struct scsi_emul_info - information for emulating a SCSI device
 *
 * @vendor: Vendor name
 * @product: Product name
 *
 * @phase: Current SCSI phase
 * @buff_used: Number of bytes ready to transfer back to host
 * @read_len: Number of bytes of data left in the current read command
 * @alloc_len: Allocation length from the last incoming command
 * @transfer_len: Transfer length from CBW header
 * @buff: Data buffer for outgoing data
 */
struct scsi_emul_info {
	/* provided by the caller: */
	void *buff;
	const char *vendor;
	const char *product;

	/* state maintained by the emulator: */
	enum scsi_cmd_phase phase;
	int buff_used;
	int read_len;
	int alloc_len;
	uint transfer_len;
};

#endif
