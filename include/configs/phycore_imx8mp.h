/* SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2020 PHYTEC Messtechnik GmbH
 * Author: Teresa Remmet <t.remmet@phytec.de>
 */

#ifndef __PHYCORE_IMX8MP_H
#define __PHYCORE_IMX8MP_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_UBOOT_BASE \
		(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

/* Link Definitions */

#define CFG_SYS_INIT_RAM_ADDR	0x40000000
#define CFG_SYS_INIT_RAM_SIZE	SZ_512K

#define CFG_SYS_SDRAM_BASE		0x40000000

#define PHYS_SDRAM			0x40000000
#define PHYS_SDRAM_SIZE                 (SZ_2G + SZ_1G) /* 3GB */
#define PHYS_SDRAM_2                    0x100000000
#define PHYS_SDRAM_2_SIZE               (SZ_4G + SZ_1G) /* 5GB */

#endif /* __PHYCORE_IMX8MP_H */
