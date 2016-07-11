/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_BOOT_DEVICE_H_
#define _ASM_BOOT_DEVICE_H_

struct boot_device_info {
	u32 type;
	char *info;
};

u32 uniphier_sld3_boot_device(void);
u32 uniphier_ld4_boot_device(void);
u32 uniphier_pro5_boot_device(void);
u32 uniphier_pxs2_boot_device(void);
u32 uniphier_ld20_boot_device(void);

void uniphier_sld3_boot_mode_show(void);
void uniphier_ld4_boot_mode_show(void);
void uniphier_pro5_boot_mode_show(void);
void uniphier_pxs2_boot_mode_show(void);
void uniphier_ld20_boot_mode_show(void);

u32 spl_boot_device_raw(void);

#endif /* _ASM_BOOT_DEVICE_H_ */
