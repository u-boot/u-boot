/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Based on include/sbi/{fw_dynamic.h,sbi_scratch.h} from the OpenSBI project.
 */
#ifndef OPENSBI_H
#define OPENSBI_H

/** Expected value of info magic ('OSBI' ascii string in hex) */
#define FW_DYNAMIC_INFO_MAGIC_VALUE		0x4942534f

/** Maximum supported info version */
#define FW_DYNAMIC_INFO_VERSION			0x1

/** Possible next mode values */
#define FW_DYNAMIC_INFO_NEXT_MODE_U		0x0
#define FW_DYNAMIC_INFO_NEXT_MODE_S		0x1
#define FW_DYNAMIC_INFO_NEXT_MODE_M		0x3

enum sbi_scratch_options {
	/** Disable prints during boot */
	SBI_SCRATCH_NO_BOOT_PRINTS = (1 << 0),
};

/** Representation dynamic info passed by previous booting stage */
struct fw_dynamic_info {
	/** Info magic */
	unsigned long magic;
	/** Info version */
	unsigned long version;
	/** Next booting stage address */
	unsigned long next_addr;
	/** Next booting stage mode */
	unsigned long next_mode;
	/** Options for OpenSBI library */
	unsigned long options;
} __packed;

#endif
