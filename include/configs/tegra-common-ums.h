/*
 * (C) Copyright 2014
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:     GPL-2.0
 */

#ifndef _TEGRA_COMMON_UMS_H_
#define _TEGRA_COMMON_UMS_H_

#ifndef CONFIG_SPL_BUILD
/* USB gadget, and mass storage protocol */
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_VBUS_DRAW    2
#define CONFIG_CI_UDC
#define CONFIG_CI_UDC_HAS_HOSTPC
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_G_DNL_VENDOR_NUM 0x0955
#define CONFIG_G_DNL_PRODUCT_NUM 0x701A
#define CONFIG_G_DNL_MANUFACTURER "NVIDIA"
#define CONFIG_USBDOWNLOAD_GADGET
#define CONFIG_USB_GADGET_MASS_STORAGE
#define CONFIG_CMD_USB_MASS_STORAGE
#endif

#endif /* _TEGRA_COMMON_UMS_H */
