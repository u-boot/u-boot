/*
 * Copyright (c) 2016 Google, Inc
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __asm_spl_h
#define __asm_spl_h

#define CONFIG_SPL_BOARD_LOAD_IMAGE

/**
 * Board-specific load method for boards that have a special way of loading
 * U-Boot, which does not fit with the existing SPL code.
 *
 * @return 0 on success, negative errno value on failure.
 */
int spl_board_load_image(void);

enum {
	BOOT_DEVICE_BOARD,
};

#endif
