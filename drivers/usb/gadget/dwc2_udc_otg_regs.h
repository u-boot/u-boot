/* SPDX-License-Identifier: GPL-2.0+ */
/* linux/arch/arm/plat-s3c/include/plat/regs-otg.h
 *
 * Copyright (C) 2004 Herbert Poetzl <herbert@13thfloor.at>
 *
 * Registers remapping:
 * Lukasz Majewski <l.majewski@samsumg.com>
 */

#ifndef __ASM_ARCH_REGS_USB_OTG_HS_H
#define __ASM_ARCH_REGS_USB_OTG_HS_H

#include "../common/dwc2_core.h"

struct dwc2_usbotg_phy {
	u32 phypwr;
	u32 phyclk;
	u32 rstcon;
};

#define FULL_SPEED_CONTROL_PKT_SIZE	8
#define FULL_SPEED_BULK_PKT_SIZE	64

#define HIGH_SPEED_CONTROL_PKT_SIZE	64
#define HIGH_SPEED_BULK_PKT_SIZE	512

#define RX_FIFO_SIZE			1024
#define NPTX_FIFO_SIZE			1024
#define PTX_FIFO_SIZE			384

#define USB_PHY_CTRL_EN0                BIT(0)

/* OPHYPWR */
#define PHY_0_SLEEP			BIT(5)
#define OTG_DISABLE_0			BIT(4)
#define ANALOG_PWRDOWN			BIT(3)
#define FORCE_SUSPEND_0			BIT(0)

/* URSTCON */
#define HOST_SW_RST			BIT(4)
#define PHY_SW_RST1			BIT(3)
#define PHYLNK_SW_RST			BIT(2)
#define LINK_SW_RST			BIT(1)
#define PHY_SW_RST0			BIT(0)

/* OPHYCLK */
#define COMMON_ON_N1			BIT(7)
#define COMMON_ON_N0			BIT(4)
#define ID_PULLUP0			BIT(2)
#define CLK_SEL_24MHZ			(0x3 << 0)
#define CLK_SEL_12MHZ			(0x2 << 0)
#define CLK_SEL_48MHZ			(0x0 << 0)

#define EXYNOS4X12_ID_PULLUP0		BIT(3)
#define EXYNOS4X12_COMMON_ON_N0		BIT(4)
#define EXYNOS4X12_CLK_SEL_12MHZ	(0x02 << 0)
#define EXYNOS4X12_CLK_SEL_24MHZ	(0x05 << 0)

/* Masks definitions */
#define GINTMSK_INIT	(GINTSTS_WKUPINT | GINTSTS_OEPINT | GINTSTS_IEPINT | GINTSTS_ENUMDONE | \
			 GINTSTS_USBRST | GINTSTS_USBSUSP | GINTSTS_OTGINT)
#define DOEPMSK_INIT	(DOEPMSK_SETUPMSK | DOEPMSK_AHBERRMSK | DOEPMSK_XFERCOMPLMSK)
#define DIEPMSK_INIT	(DIEPMSK_TIMEOUTMSK | DIEPMSK_AHBERRMSK | DIEPMSK_XFERCOMPLMSK)
#define GAHBCFG_INIT	(GAHBCFG_DMA_EN | \
			 FIELD_PREP(GAHBCFG_HBSTLEN_MASK, GAHBCFG_HBSTLEN_INCR4) | \
			 GAHBCFG_GLBL_INTR_EN)

#endif
