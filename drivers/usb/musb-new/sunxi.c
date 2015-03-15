/*
 * Allwinner SUNXI "glue layer"
 *
 * Copyright © 2015 Hans de Goede <hdegoede@redhat.com>
 * Copyright © 2013 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * Based on the sw_usb "Allwinner OTG Dual Role Controller" code.
 *  Copyright 2007-2012 (C) Allwinner Technology Co., Ltd.
 *  javen <javen@allwinnertech.com>
 *
 * Based on the DA8xx "glue layer" code.
 *  Copyright (c) 2008-2009 MontaVista Software, Inc. <source@mvista.com>
 *  Copyright (C) 2005-2006 by Texas Instruments
 *
 * This file is part of the Inventra Controller Driver for Linux.
 *
 * The Inventra Controller Driver for Linux is free software; you
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License version 2 as published by the Free Software
 * Foundation.
 *
 */
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/usbc.h>
#include <asm-generic/gpio.h>
#include "linux-compat.h"
#include "musb_core.h"
#ifdef CONFIG_AXP152_POWER
#include <axp152.h>
#endif
#ifdef CONFIG_AXP209_POWER
#include <axp209.h>
#endif
#ifdef CONFIG_AXP221_POWER
#include <axp221.h>
#endif

/******************************************************************************
 ******************************************************************************
 * From the Allwinner driver
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 * From include/sunxi_usb_bsp.h
 ******************************************************************************/

/* reg offsets */
#define  USBC_REG_o_ISCR	0x0400
#define  USBC_REG_o_PHYCTL	0x0404
#define  USBC_REG_o_PHYBIST	0x0408
#define  USBC_REG_o_PHYTUNE	0x040c

#define  USBC_REG_o_VEND0	0x0043

/* Interface Status and Control */
#define  USBC_BP_ISCR_VBUS_VALID_FROM_DATA	30
#define  USBC_BP_ISCR_VBUS_VALID_FROM_VBUS	29
#define  USBC_BP_ISCR_EXT_ID_STATUS		28
#define  USBC_BP_ISCR_EXT_DM_STATUS		27
#define  USBC_BP_ISCR_EXT_DP_STATUS		26
#define  USBC_BP_ISCR_MERGED_VBUS_STATUS	25
#define  USBC_BP_ISCR_MERGED_ID_STATUS		24

#define  USBC_BP_ISCR_ID_PULLUP_EN		17
#define  USBC_BP_ISCR_DPDM_PULLUP_EN		16
#define  USBC_BP_ISCR_FORCE_ID			14
#define  USBC_BP_ISCR_FORCE_VBUS_VALID		12
#define  USBC_BP_ISCR_VBUS_VALID_SRC		10

#define  USBC_BP_ISCR_HOSC_EN			7
#define  USBC_BP_ISCR_VBUS_CHANGE_DETECT	6
#define  USBC_BP_ISCR_ID_CHANGE_DETECT		5
#define  USBC_BP_ISCR_DPDM_CHANGE_DETECT	4
#define  USBC_BP_ISCR_IRQ_ENABLE		3
#define  USBC_BP_ISCR_VBUS_CHANGE_DETECT_EN	2
#define  USBC_BP_ISCR_ID_CHANGE_DETECT_EN	1
#define  USBC_BP_ISCR_DPDM_CHANGE_DETECT_EN	0

/******************************************************************************
 * From usbc/usbc.c
 ******************************************************************************/

static u32 USBC_WakeUp_ClearChangeDetect(u32 reg_val)
{
	u32 temp = reg_val;

	temp &= ~(1 << USBC_BP_ISCR_VBUS_CHANGE_DETECT);
	temp &= ~(1 << USBC_BP_ISCR_ID_CHANGE_DETECT);
	temp &= ~(1 << USBC_BP_ISCR_DPDM_CHANGE_DETECT);

	return temp;
}

static void USBC_EnableIdPullUp(__iomem void *base)
{
	u32 reg_val;

	reg_val = musb_readl(base, USBC_REG_o_ISCR);
	reg_val |= (1 << USBC_BP_ISCR_ID_PULLUP_EN);
	reg_val = USBC_WakeUp_ClearChangeDetect(reg_val);
	musb_writel(base, USBC_REG_o_ISCR, reg_val);
}

static void USBC_DisableIdPullUp(__iomem void *base)
{
	u32 reg_val;

	reg_val = musb_readl(base, USBC_REG_o_ISCR);
	reg_val &= ~(1 << USBC_BP_ISCR_ID_PULLUP_EN);
	reg_val = USBC_WakeUp_ClearChangeDetect(reg_val);
	musb_writel(base, USBC_REG_o_ISCR, reg_val);
}

static void USBC_EnableDpDmPullUp(__iomem void *base)
{
	u32 reg_val;

	reg_val = musb_readl(base, USBC_REG_o_ISCR);
	reg_val |= (1 << USBC_BP_ISCR_DPDM_PULLUP_EN);
	reg_val = USBC_WakeUp_ClearChangeDetect(reg_val);
	musb_writel(base, USBC_REG_o_ISCR, reg_val);
}

static void USBC_DisableDpDmPullUp(__iomem void *base)
{
	u32 reg_val;

	reg_val = musb_readl(base, USBC_REG_o_ISCR);
	reg_val &= ~(1 << USBC_BP_ISCR_DPDM_PULLUP_EN);
	reg_val = USBC_WakeUp_ClearChangeDetect(reg_val);
	musb_writel(base, USBC_REG_o_ISCR, reg_val);
}

static void USBC_ForceIdToLow(__iomem void *base)
{
	u32 reg_val;

	reg_val = musb_readl(base, USBC_REG_o_ISCR);
	reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_ID);
	reg_val |= (0x02 << USBC_BP_ISCR_FORCE_ID);
	reg_val = USBC_WakeUp_ClearChangeDetect(reg_val);
	musb_writel(base, USBC_REG_o_ISCR, reg_val);
}

static void USBC_ForceIdToHigh(__iomem void *base)
{
	u32 reg_val;

	reg_val = musb_readl(base, USBC_REG_o_ISCR);
	reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_ID);
	reg_val |= (0x03 << USBC_BP_ISCR_FORCE_ID);
	reg_val = USBC_WakeUp_ClearChangeDetect(reg_val);
	musb_writel(base, USBC_REG_o_ISCR, reg_val);
}

static void USBC_ForceVbusValidToHigh(__iomem void *base)
{
	u32 reg_val;

	reg_val = musb_readl(base, USBC_REG_o_ISCR);
	reg_val &= ~(0x03 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val |= (0x03 << USBC_BP_ISCR_FORCE_VBUS_VALID);
	reg_val = USBC_WakeUp_ClearChangeDetect(reg_val);
	musb_writel(base, USBC_REG_o_ISCR, reg_val);
}

static void USBC_ConfigFIFO_Base(void)
{
	u32 reg_value;

	/* config usb fifo, 8kb mode */
	reg_value = readl(SUNXI_SRAMC_BASE + 0x04);
	reg_value &= ~(0x03 << 0);
	reg_value |= (1 << 0);
	writel(reg_value, SUNXI_SRAMC_BASE + 0x04);
}

/******************************************************************************
 * MUSB Glue code
 ******************************************************************************/

static irqreturn_t sunxi_musb_interrupt(int irq, void *__hci)
{
	struct musb		*musb = __hci;
	irqreturn_t		retval = IRQ_NONE;

	/* read and flush interrupts */
	musb->int_usb = musb_readb(musb->mregs, MUSB_INTRUSB);
	if (musb->int_usb)
		musb_writeb(musb->mregs, MUSB_INTRUSB, musb->int_usb);
	musb->int_tx = musb_readw(musb->mregs, MUSB_INTRTX);
	if (musb->int_tx)
		musb_writew(musb->mregs, MUSB_INTRTX, musb->int_tx);
	musb->int_rx = musb_readw(musb->mregs, MUSB_INTRRX);
	if (musb->int_rx)
		musb_writew(musb->mregs, MUSB_INTRRX, musb->int_rx);

	if (musb->int_usb || musb->int_tx || musb->int_rx)
		retval |= musb_interrupt(musb);

	return retval;
}

static void sunxi_musb_enable(struct musb *musb)
{
	pr_debug("%s():\n", __func__);

	/* select PIO mode */
	musb_writeb(musb->mregs, USBC_REG_o_VEND0, 0);

	if (is_host_enabled(musb)) {
		/* port power on */
		sunxi_usbc_vbus_enable(0);
	}
}

static void sunxi_musb_disable(struct musb *musb)
{
	pr_debug("%s():\n", __func__);

	/* Put the controller back in a pristane state for "usb reset" */
	if (musb->is_active) {
		sunxi_usbc_disable(0);
		sunxi_usbc_enable(0);
		musb->is_active = 0;
	}
}

static int sunxi_musb_init(struct musb *musb)
{
	int err;

	pr_debug("%s():\n", __func__);

	if (is_host_enabled(musb)) {
		int vbus_det = sunxi_name_to_gpio(CONFIG_USB0_VBUS_DET);

#ifdef AXP_VBUS_DETECT
		if (!strcmp(CONFIG_USB0_VBUS_DET, "axp_vbus_detect")) {
			err = axp_get_vbus();
			if (err < 0)
				return err;
		} else {
#endif
			if (vbus_det == -1) {
				eprintf("Error invalid Vusb-det pin\n");
				return -EINVAL;
			}

			err = gpio_request(vbus_det, "vbus0_det");
			if (err)
				return err;

			err = gpio_direction_input(vbus_det);
			if (err) {
				gpio_free(vbus_det);
				return err;
			}

			err = gpio_get_value(vbus_det);
			if (err < 0) {
				gpio_free(vbus_det);
				return -EIO;
			}

			gpio_free(vbus_det);
#ifdef AXP_VBUS_DETECT
		}
#endif

		if (err) {
			eprintf("Error: A charger is plugged into the OTG\n");
			return -EIO;
		}
	}

	err = sunxi_usbc_request_resources(0);
	if (err)
		return err;

	musb->isr = sunxi_musb_interrupt;
	sunxi_usbc_enable(0);

	USBC_ConfigFIFO_Base();
	USBC_EnableDpDmPullUp(musb->mregs);
	USBC_EnableIdPullUp(musb->mregs);

	if (is_host_enabled(musb)) {
		/* Host mode */
		USBC_ForceIdToLow(musb->mregs);
	} else {
		/* Peripheral mode */
		USBC_ForceIdToHigh(musb->mregs);
	}
	USBC_ForceVbusValidToHigh(musb->mregs);

	return 0;
}

static int sunxi_musb_exit(struct musb *musb)
{
	pr_debug("%s():\n", __func__);

	USBC_DisableDpDmPullUp(musb->mregs);
	USBC_DisableIdPullUp(musb->mregs);
	sunxi_usbc_vbus_disable(0);
	sunxi_usbc_disable(0);

	return sunxi_usbc_free_resources(0);
}

const struct musb_platform_ops sunxi_musb_ops = {
	.init		= sunxi_musb_init,
	.exit		= sunxi_musb_exit,

	.enable		= sunxi_musb_enable,
	.disable	= sunxi_musb_disable,
};
