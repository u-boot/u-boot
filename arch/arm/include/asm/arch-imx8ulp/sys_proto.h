/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 NXP
 */

#ifndef __ARCH_IMX8ULP_SYS_PROTO_H
#define __ARCH_NMX8ULP_SYS_PROTO_H

#include <asm/mach-imx/sys_proto.h>

enum bt_mode get_boot_mode(void);
int xrdc_config_pdac(u32 bridge, u32 index, u32 dom, u32 perm);
int xrdc_config_pdac_openacc(u32 bridge, u32 index);
void set_lpav_qos(void);
void load_lposc_fuse(void);
bool m33_image_booted(void);
int m33_image_handshake(ulong timeout_ms);
#endif
