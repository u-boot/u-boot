/*
 * Copyright (C) 2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __UNIPHIER_SOC_INFO_H__
#define __UNIPHIER_SOC_INFO_H__

#define UNIPHIER_SLD3_ID	0x25
#define UNIPHIER_LD4_ID		0x26
#define UNIPHIER_PRO4_ID	0x28
#define UNIPHIER_SLD8_ID	0x29
#define UNIPHIER_PRO5_ID	0x2a
#define UNIPHIER_PXS2_ID	0x2e
#define UNIPHIER_LD6B_ID	0x2f
#define UNIPHIER_LD11_ID	0x31
#define UNIPHIER_LD20_ID	0x32

unsigned int uniphier_get_soc_id(void);
unsigned int uniphier_get_soc_model(void);
unsigned int uniphier_get_soc_revision(void);

#endif /* __UNIPHIER_SOC_INFO_H__ */
