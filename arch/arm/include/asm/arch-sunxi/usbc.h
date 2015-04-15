/*
 * Sunxi usb-controller code shared between the ehci and musb controllers
 *
 * Copyright (C) 2014 Roman Byshko
 *
 * Roman Byshko <rbyshko@gmail.com>
 *
 * Based on code from
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

extern const struct musb_platform_ops sunxi_musb_ops;

void *sunxi_usbc_get_io_base(int index);
int sunxi_usbc_request_resources(int index);
int sunxi_usbc_free_resources(int index);
void sunxi_usbc_enable(int index);
void sunxi_usbc_disable(int index);
void sunxi_usbc_vbus_enable(int index);
void sunxi_usbc_vbus_disable(int index);
void sunxi_usbc_enable_squelch_detect(int index, int enable);
