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
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _TPM_PRIVATE_H_
#define _TPM_PRIVATE_H_

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

struct tpm_vendor_specific {
	const u8 req_complete_mask;
	const u8 req_complete_val;
	const u8 req_canceled;
	int irq;
	int (*recv) (struct tpm_chip *, u8 *, size_t);
	int (*send) (struct tpm_chip *, u8 *, size_t);
	void (*cancel) (struct tpm_chip *);
	u8(*status) (struct tpm_chip *);
	int locality;
	unsigned long timeout_a, timeout_b, timeout_c, timeout_d;  /* msec */
	unsigned long duration[3];  /* msec */
};

struct tpm_chip {
	int is_open;
	struct tpm_vendor_specific vendor;
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

struct tpm_chip *tpm_register_hardware(const struct tpm_vendor_specific *);

int tpm_vendor_init(uint32_t dev_addr);

void tpm_vendor_cleanup(struct tpm_chip *chip);


#endif
