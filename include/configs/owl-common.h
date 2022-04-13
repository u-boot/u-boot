/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Board configuration file for Actions Semi Owl SoCs.
 *
 * Copyright (C) 2015 Actions Semi Co., Ltd.
 * Copyright (C) 2018 Manivannan Sadhasivam <manivannan.sadhasivam@linaro.org>
 *
 */

#ifndef _OWL_COMMON_CONFIG_H_
#define _OWL_COMMON_CONFIG_H_

/* SDRAM Definitions */
#define CONFIG_SYS_SDRAM_BASE		0x0

/* Some commands use this as the default load address */

/*
 * This is the initial SP which is used only briefly for relocating the u-boot
 * image to the top of SDRAM. After relocation u-boot moves the stack to the
 * proper place.
 */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE + 0x7ff00)

/* Console configuration */
#define CONFIG_SYS_CBSIZE		1024	/* Console buffer size */
#define CONFIG_SYS_MAXARGS		64
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#endif
