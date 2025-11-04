/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024 Kontron Electronics GmbH
 */

#ifndef _KONTRON_HW_UID_H
#define _KONTRON_HW_UID_H

#include <compiler.h>
#include <stddef.h>

enum {
	UID_OTP_FORMAT_DEC = 0,
	UID_OTP_FORMAT_HEX,
};

struct uid_otp_loc {
	u32 *addr;
	size_t len;
	unsigned int format;
	char *desc;
};

void get_serial_number(struct uid_otp_loc *locs, unsigned int num);

#endif /* _KONTRON_HW_UID_H */
