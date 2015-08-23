/*
 * Copyright (C) 2011 Infineon Technologies
 *
 * Authors:
 * Peter Huewe <huewe.external@infineon.com>
 *
 * Version: 2.1.1
 *
 * Description:
 * Device driver for TCG/TCPA TPM (trusted platform module).
 * Specifications at www.trustedcomputinggroup.org
 *
 * It is based on the Linux kernel driver tpm.c from Leendert van
 * Dorn, Dave Safford, Reiner Sailer, and Kyleen Hall.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef _TPM_TIS_I2C_H
#define _TPM_TIS_I2C_H

#include <linux/compiler.h>
#include <linux/types.h>

enum tpm_timeout {
	TPM_TIMEOUT = 5,	/* msecs */
};

/* Size of external transmit buffer (used in tpm_transmit)*/
#define TPM_BUFSIZE 4096

/* Index of Count field in TPM response buffer */
#define TPM_RSP_SIZE_BYTE	2
#define TPM_RSP_RC_BYTE		6

struct tpm_chip;

struct tpm_chip {
	int is_open;
	u8 req_complete_mask;
	u8 req_complete_val;
	u8 req_canceled;
	int irq;
	int locality;
	unsigned long timeout_a, timeout_b, timeout_c, timeout_d;  /* msec */
	unsigned long duration[3];  /* msec */
};

struct tpm_input_header {
	__be16 tag;
	__be32 length;
	__be32 ordinal;
} __packed;

struct tpm_output_header {
	__be16 tag;
	__be32 length;
	__be32 return_code;
} __packed;

struct timeout_t {
	__be32 a;
	__be32 b;
	__be32 c;
	__be32 d;
} __packed;

struct duration_t {
	__be32 tpm_short;
	__be32 tpm_medium;
	__be32 tpm_long;
} __packed;

union cap_t {
	struct timeout_t timeout;
	struct duration_t duration;
};

struct tpm_getcap_params_in {
	__be32 cap;
	__be32 subcap_size;
	__be32 subcap;
} __packed;

struct tpm_getcap_params_out {
	__be32 cap_size;
	union cap_t cap;
} __packed;

union tpm_cmd_header {
	struct tpm_input_header in;
	struct tpm_output_header out;
};

union tpm_cmd_params {
	struct tpm_getcap_params_out getcap_out;
	struct tpm_getcap_params_in getcap_in;
};

struct tpm_cmd_t {
	union tpm_cmd_header header;
	union tpm_cmd_params params;
} __packed;

struct udevice;
int tpm_vendor_init(struct udevice *dev);

void tpm_vendor_cleanup(struct tpm_chip *chip);

#endif
