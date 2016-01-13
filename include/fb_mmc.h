/*
 * Copyright 2014 Broadcom Corporation.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

void fb_mmc_flash_write(const char *cmd, unsigned int session_id,
			void *download_buffer, unsigned int download_bytes,
			char *response);
void fb_mmc_erase(const char *cmd, char *response);
