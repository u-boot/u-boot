// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */

#include <spl.h>

char __data_start[0] __section(".__data_start");
char __data_save_start[0] __section(".__data_save_start");
char __data_save_end[0] __section(".__data_save_end");

u32 cold_reboot_flag = 1;

u32 __weak reset_flag(void)
{
	return 1;
}

void spl_save_restore_data(void)
{
	u32 data_size = __data_save_end - __data_save_start;
	cold_reboot_flag = reset_flag();

	if (cold_reboot_flag == 1) {
		/* Save data section to data_save section */
		memcpy(__data_save_start, __data_start, data_size);
	} else {
		/* Restore the data_save section to data section */
		memcpy(__data_start, __data_save_start, data_size);
	}

	cold_reboot_flag++;
}
