/*
 * (C) Copyright 2014
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:     GPL-2.0
 */

#ifndef _TEGRA_COMMON_USB_GADGET_H_
#define _TEGRA_COMMON_USB_GADGET_H_

#ifndef CONFIG_SPL_BUILD
/* USB gadget mode support*/
#define CONFIG_CI_UDC_HAS_HOSTPC
/* USB mass storage protocol */
#define CONFIG_USB_FUNCTION_MASS_STORAGE
/* DFU protocol */
#define CONFIG_USB_FUNCTION_DFU
#define CONFIG_SYS_DFU_DATA_BUF_SIZE SZ_1M
#define CONFIG_SYS_DFU_MAX_FILE_SIZE SZ_32M
#ifdef CONFIG_MMC
#define CONFIG_DFU_MMC
#endif
#ifdef CONFIG_SPI_FLASH
#define CONFIG_DFU_SF
#endif
#define CONFIG_DFU_RAM
#endif

#endif /* _TEGRA_COMMON_USB_GADGET_H_ */
