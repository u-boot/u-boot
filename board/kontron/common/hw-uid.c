// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Kontron Electronics GmbH
 */

#include <linux/errno.h>
#include <compiler.h>
#include <asm/io.h>
#include <env.h>
#include <string.h>
#include <vsprintf.h>

#include "hw-uid.h"

int get_serial_str_from_otp(struct uid_otp_loc loc, char *str, size_t str_len)
{
	u64 uid;
	int ret;

	if (loc.len < 1 || loc.len > 2) {
		printf("Invalid number of UID OTP registers set!\n");
		return -EINVAL;
	}

	uid = readl(loc.addr);

	if (loc.len == 2)
		uid |= (u64)readl(loc.addr + 0x4) << 32;

	if (!uid)
		return -ENOENT;

	if (uid) {
		switch (loc.format) {
		case UID_OTP_FORMAT_DEC:
			ret = snprintf(str, str_len, "%010llu", uid);
			break;
		case UID_OTP_FORMAT_HEX:
			ret = snprintf(str, str_len, "%016llX", uid);
			break;
		}
		if (ret < 0 || ret >= str_len) {
			printf("Failed to convert UID!\n");
			return -EFAULT;
		}
	}

	return 0;
}

void get_serial_number(struct uid_otp_loc *locs, unsigned int num)
{
	char serial_string[17];
	unsigned int i;
	int ret;

	for (i = 0; i < num; i++) {
		ret  = get_serial_str_from_otp(locs[i], serial_string, sizeof(serial_string));
		if (ret == 0)
			break;
	}

	/* No valid UID in the OTP fuses, skip. */
	if (ret) {
		printf("Serial Number: None\n");
		return;
	}

	printf("Serial Number: %s (%s)\n", serial_string, locs[i].desc);

	if (!env_get("serial#"))
		env_set("serial#", serial_string);
	else if (strcmp(env_get("serial#"), serial_string))
		printf("Warning: mismatch of UIDs in env and OTPs!\n");
}
