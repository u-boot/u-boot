/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 NXP
 */

#ifndef __ARCH_IMX8ULP_SYS_PROTO_H
#define __ARCH_NMX8ULP_SYS_PROTO_H

#include <asm/mach-imx/sys_proto.h>

extern unsigned long rom_pointer[];

ulong spl_romapi_raw_seekable_read(u32 offset, u32 size, void *buf);
ulong spl_romapi_get_uboot_base(u32 image_offset, u32 rom_bt_dev);
enum bt_mode get_boot_mode(void);
int xrdc_config_pdac(u32 bridge, u32 index, u32 dom, u32 perm);
int xrdc_config_pdac_openacc(u32 bridge, u32 index);
enum boot_device get_boot_device(void);
#endif
