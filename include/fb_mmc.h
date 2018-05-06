/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Broadcom Corporation.
 */

void fb_mmc_flash_write(const char *cmd, void *download_buffer,
			unsigned int download_bytes);
void fb_mmc_erase(const char *cmd);
