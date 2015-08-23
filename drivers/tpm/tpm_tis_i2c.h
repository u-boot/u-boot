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
	TPM_TIMEOUT_MS			= 5,
	TIS_SHORT_TIMEOUT_MS		= 750,
	TIS_LONG_TIMEOUT_MS		= 2000,
	SLEEP_DURATION_US		= 60,
	SLEEP_DURATION_LONG_US		= 210,
};

/* Size of external transmit buffer (used in tpm_transmit)*/
#define TPM_BUFSIZE 4096

/* Index of Count field in TPM response buffer */
#define TPM_RSP_SIZE_BYTE	2
#define TPM_RSP_RC_BYTE		6

/* Max buffer size supported by our tpm */
#define TPM_DEV_BUFSIZE		1260

enum i2c_chip_type {
	SLB9635,
	SLB9645,
	UNKNOWN,
};

struct tpm_chip {
	bool inited;
	int is_open;
	u8 req_complete_mask;
	u8 req_complete_val;
	u8 req_canceled;
	int irq;
	int locality;
	unsigned long timeout_a, timeout_b, timeout_c, timeout_d;  /* msec */
	unsigned long duration[3];  /* msec */
	struct udevice *dev;
	u8 buf[TPM_DEV_BUFSIZE + sizeof(u8)];  /* Max buffer size + addr */
	enum i2c_chip_type chip_type;
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

/* Max number of iterations after i2c NAK */
#define MAX_COUNT		3

/*
 * Max number of iterations after i2c NAK for 'long' commands
 *
 * We need this especially for sending TPM_READY, since the cleanup after the
 * transtion to the ready state may take some time, but it is unpredictable
 * how long it will take.
 */
#define MAX_COUNT_LONG		50

#define TPM_HEADER_SIZE		10

enum tis_access {
	TPM_ACCESS_VALID		= 0x80,
	TPM_ACCESS_ACTIVE_LOCALITY	= 0x20,
	TPM_ACCESS_REQUEST_PENDING	= 0x04,
	TPM_ACCESS_REQUEST_USE		= 0x02,
};

enum tis_status {
	TPM_STS_VALID			= 0x80,
	TPM_STS_COMMAND_READY		= 0x40,
	TPM_STS_GO			= 0x20,
	TPM_STS_DATA_AVAIL		= 0x10,
	TPM_STS_DATA_EXPECT		= 0x08,
};

/* expected value for DIDVID register */
#define TPM_TIS_I2C_DID_VID_9635 0x000b15d1L
#define TPM_TIS_I2C_DID_VID_9645 0x001a15d1L

#define	TPM_ACCESS(l)			(0x0000 | ((l) << 4))
#define	TPM_STS(l)			(0x0001 | ((l) << 4))
#define	TPM_DATA_FIFO(l)		(0x0005 | ((l) << 4))
#define	TPM_DID_VID(l)			(0x0006 | ((l) << 4))

enum tpm_duration {
	TPM_SHORT = 0,
	TPM_MEDIUM = 1,
	TPM_LONG = 2,
	TPM_UNDEFINED,
};

/* Extended error numbers from linux (see errno.h) */
#define ECANCELED	125	/* Operation Canceled */

/* Timer frequency. Corresponds to msec timer resolution */
#define HZ		1000

#define TPM_MAX_ORDINAL			243
#define TPM_MAX_PROTECTED_ORDINAL	12
#define TPM_PROTECTED_ORDINAL_MASK	0xFF

#define TPM_CMD_COUNT_BYTE	2
#define TPM_CMD_ORDINAL_BYTE	6

/*
 * Array with one entry per ordinal defining the maximum amount
 * of time the chip could take to return the result.  The ordinal
 * designation of short, medium or long is defined in a table in
 * TCG Specification TPM Main Part 2 TPM Structures Section 17. The
 * values of the SHORT, MEDIUM, and LONG durations are retrieved
 * from the chip during initialization with a call to tpm_get_timeouts.
 */
static const u8 tpm_protected_ordinal_duration[TPM_MAX_PROTECTED_ORDINAL] = {
	TPM_UNDEFINED,		/* 0 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 5 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 10 */
	TPM_SHORT,
};

static const u8 tpm_ordinal_duration[TPM_MAX_ORDINAL] = {
	TPM_UNDEFINED,		/* 0 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 5 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 10 */
	TPM_SHORT,
	TPM_MEDIUM,
	TPM_LONG,
	TPM_LONG,
	TPM_MEDIUM,		/* 15 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_MEDIUM,
	TPM_LONG,
	TPM_SHORT,		/* 20 */
	TPM_SHORT,
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_SHORT,		/* 25 */
	TPM_SHORT,
	TPM_MEDIUM,
	TPM_SHORT,
	TPM_SHORT,
	TPM_MEDIUM,		/* 30 */
	TPM_LONG,
	TPM_MEDIUM,
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,		/* 35 */
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_MEDIUM,		/* 40 */
	TPM_LONG,
	TPM_MEDIUM,
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,		/* 45 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_LONG,
	TPM_MEDIUM,		/* 50 */
	TPM_MEDIUM,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 55 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_MEDIUM,		/* 60 */
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_SHORT,
	TPM_SHORT,
	TPM_MEDIUM,		/* 65 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 70 */
	TPM_SHORT,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 75 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_LONG,		/* 80 */
	TPM_UNDEFINED,
	TPM_MEDIUM,
	TPM_LONG,
	TPM_SHORT,
	TPM_UNDEFINED,		/* 85 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 90 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_UNDEFINED,		/* 95 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_MEDIUM,		/* 100 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 105 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 110 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,		/* 115 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_LONG,		/* 120 */
	TPM_LONG,
	TPM_MEDIUM,
	TPM_UNDEFINED,
	TPM_SHORT,
	TPM_SHORT,		/* 125 */
	TPM_SHORT,
	TPM_LONG,
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,		/* 130 */
	TPM_MEDIUM,
	TPM_UNDEFINED,
	TPM_SHORT,
	TPM_MEDIUM,
	TPM_UNDEFINED,		/* 135 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 140 */
	TPM_SHORT,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 145 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 150 */
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_SHORT,
	TPM_SHORT,
	TPM_UNDEFINED,		/* 155 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 160 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 165 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_LONG,		/* 170 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 175 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_MEDIUM,		/* 180 */
	TPM_SHORT,
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_MEDIUM,		/* 185 */
	TPM_SHORT,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 190 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 195 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 200 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,
	TPM_SHORT,		/* 205 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_MEDIUM,		/* 210 */
	TPM_UNDEFINED,
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_MEDIUM,
	TPM_UNDEFINED,		/* 215 */
	TPM_MEDIUM,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,
	TPM_SHORT,		/* 220 */
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_SHORT,
	TPM_UNDEFINED,		/* 225 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 230 */
	TPM_LONG,
	TPM_MEDIUM,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,		/* 235 */
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_UNDEFINED,
	TPM_SHORT,		/* 240 */
	TPM_UNDEFINED,
	TPM_MEDIUM,
};

#endif
