/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Verified Boot for Embedded (VBE) vbe-simple common file
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ifndef __VBE_SIMPLE_H
#define __VBE_SIMPLE_H

/** struct simple_priv - information read from the device tree */
struct simple_priv {
	u32 area_start;
	u32 area_size;
	u32 skip_offset;
	u32 state_offset;
	u32 state_size;
	u32 version_offset;
	u32 version_size;
	const char *storage;
};

/**
 * vbe_simple_read_fw_bootflow() - Read a bootflow for firmware
 *
 * Locates and loads the firmware image (FIT) needed for the next phase. The FIT
 * should ideally use external data, to reduce the amount of it that needs to be
 * read.
 *
 * @bdev: bootdev device containing the firmwre
 * @blow: Place to put the created bootflow, on success
 * @return 0 if OK, -ve on error
 */
int vbe_simple_read_bootflow_fw(struct udevice *dev, struct bootflow *bflow);

#endif /* __VBE_SIMPLE_H */
