/*
 * Sunxi usb-phy code
 *
 * Copyright (C) 2015 Hans de Goede <hdegoede@redhat.com>
 * Copyright (C) 2014 Roman Byshko <rbyshko@gmail.com>
 *
 * Based on code from
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

int sunxi_usb_phy_probe(void);
int sunxi_usb_phy_remove(void);
void sunxi_usb_phy_init(int index);
void sunxi_usb_phy_exit(int index);
void sunxi_usb_phy_power_on(int index);
void sunxi_usb_phy_power_off(int index);
int sunxi_usb_phy_power_is_on(int index);
int sunxi_usb_phy_vbus_detect(int index);
int sunxi_usb_phy_id_detect(int index);
void sunxi_usb_phy_enable_squelch_detect(int index, int enable);

/* Not really phy related, but we have to declare this somewhere ... */
#if defined(CONFIG_USB_MUSB_HOST) || defined(CONFIG_USB_MUSB_GADGET)
void sunxi_musb_board_init(void);
#else
#define sunxi_musb_board_init()
#endif
