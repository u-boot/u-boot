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
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_VBUS_DRAW    2
#define CONFIG_CI_UDC
#define CONFIG_CI_UDC_HAS_HOSTPC
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_G_DNL_VENDOR_NUM 0x0955
#define CONFIG_G_DNL_PRODUCT_NUM 0x701A
#define CONFIG_G_DNL_MANUFACTURER "NVIDIA"
#define CONFIG_USBDOWNLOAD_GADGET
/* USB mass storage protocol */
#define CONFIG_USB_GADGET_MASS_STORAGE
#define CONFIG_CMD_USB_MASS_STORAGE
/* DFU protocol */
#define CONFIG_DFU_FUNCTION
#define CONFIG_SYS_DFU_DATA_BUF_SIZE (1 * 1024 * 1024)
#define CONFIG_CMD_DFU
#ifdef CONFIG_MMC
#define CONFIG_DFU_MMC
#endif
#ifdef CONFIG_SPI_FLASH
#define CONFIG_DFU_SF
#endif
#endif

#endif /* _TEGRA_COMMON_USB_GADGET_H_ */
