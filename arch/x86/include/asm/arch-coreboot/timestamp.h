/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 The ChromiumOS Authors.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA, 02110-1301 USA
 */

#ifndef __COREBOOT_TIMESTAMP_H__
#define __COREBOOT_TIMESTAMP_H__

enum timestamp_id {
	/* coreboot specific timestamp IDs */
	TS_START_ROMSTAGE = 1,
	TS_BEFORE_INITRAM = 2,
	TS_AFTER_INITRAM = 3,
	TS_END_ROMSTAGE = 4,
	TS_START_COPYRAM = 8,
	TS_END_COPYRAM = 9,
	TS_START_RAMSTAGE = 10,
	TS_DEVICE_ENUMERATE = 30,
	TS_DEVICE_CONFIGURE = 40,
	TS_DEVICE_ENABLE = 50,
	TS_DEVICE_INITIALIZE = 60,
	TS_DEVICE_DONE = 70,
	TS_CBMEM_POST = 75,
	TS_WRITE_TABLES = 80,
	TS_LOAD_PAYLOAD = 90,
	TS_ACPI_WAKE_JUMP = 98,
	TS_SELFBOOT_JUMP = 99,

	/* U-Boot entry IDs start at 1000 */
	TS_U_BOOT_INITTED = 1000, /* This is where u-boot starts */
	TS_U_BOOT_START_KERNEL = 1100, /* Right before jumping to kernel. */
};

void timestamp_init(void);
void timestamp_add(enum timestamp_id id, uint64_t ts_time);
void timestamp_add_now(enum timestamp_id id);

#endif
