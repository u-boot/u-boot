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

#include <linux/bitops.h>

struct dwc2_usbotg_phy {
	u32 phypwr;
	u32 phyclk;
	u32 rstcon;
};

/*===================================================================== */
/*definitions related to CSR setting */

/* DWC2_UDC_OTG_GOTGCTL */
#define B_SESSION_VALID			BIT(19)
#define A_SESSION_VALID			BIT(18)
#define B_VALOVAL			BIT(7)
#define B_VALOEN			BIT(6)
#define A_VALOVAL			BIT(5)
#define A_VALOEN			BIT(4)
#define VB_VALOVAL			BIT(3)
#define VB_VALOEN			BIT(2)

/* DWC2_UDC_OTG_GOTINT */
#define GOTGINT_SES_END_DET		BIT(2)

/* DWC2_UDC_OTG_GAHBCFG */
#define PTXFE_HALF			(0 << 8)
#define PTXFE_ZERO			(1 << 8)
#define NPTXFE_HALF			(0 << 7)
#define NPTXFE_ZERO			(1 << 7)
#define MODE_SLAVE			(0 << 5)
#define MODE_DMA			(1 << 5)
#define BURST_SINGLE			(0 << 1)
#define BURST_INCR			(1 << 1)
#define BURST_INCR4			(3 << 1)
#define BURST_INCR8			(5 << 1)
#define BURST_INCR16			(7 << 1)
#define GBL_INT_UNMASK			(1 << 0)
#define GBL_INT_MASK			(0 << 0)

/* DWC2_UDC_OTG_GRSTCTL */
#define AHB_MASTER_IDLE			BIT(31)
#define CORE_SOFT_RESET			BIT(0)

/* DWC2_UDC_OTG_GINTSTS/DWC2_UDC_OTG_GINTMSK core interrupt register */
#define INT_RESUME			BIT(31)
#define INT_DISCONN			BIT(29)
#define INT_CONN_ID_STS_CNG		BIT(28)
#define INT_OUT_EP			BIT(19)
#define INT_IN_EP			BIT(18)
#define INT_ENUMDONE			BIT(13)
#define INT_RESET			BIT(12)
#define INT_SUSPEND			BIT(11)
#define INT_EARLY_SUSPEND		BIT(10)
#define INT_GOUTNakEff			BIT(7)
#define INT_GINNakEff			BIT(6)
#define INT_NP_TX_FIFO_EMPTY		BIT(5)
#define INT_RX_FIFO_NOT_EMPTY		BIT(4)
#define INT_SOF				BIT(3)
#define INT_OTG				BIT(2)
#define INT_HOST_MODE			BIT(1)

#define FULL_SPEED_CONTROL_PKT_SIZE	8
#define FULL_SPEED_BULK_PKT_SIZE	64

#define HIGH_SPEED_CONTROL_PKT_SIZE	64
#define HIGH_SPEED_BULK_PKT_SIZE	512

#define RX_FIFO_SIZE			1024
#define NPTX_FIFO_SIZE			1024
#define PTX_FIFO_SIZE			384

#define DEPCTL_TXFNUM_0			(0x0 << 22)
#define DEPCTL_TXFNUM_1			(0x1 << 22)
#define DEPCTL_TXFNUM_2			(0x2 << 22)
#define DEPCTL_TXFNUM_3			(0x3 << 22)
#define DEPCTL_TXFNUM_4			(0x4 << 22)

/* Enumeration speed */
#define USB_HIGH_30_60MHZ		(0x0 << 1)
#define USB_FULL_30_60MHZ		(0x1 << 1)
#define USB_LOW_6MHZ			(0x2 << 1)
#define USB_FULL_48MHZ			(0x3 << 1)

/* DWC2_UDC_OTG_GRXSTSP STATUS */
#define OUT_PKT_RECEIVED		(0x2 << 17)
#define OUT_TRANSFER_COMPLELTED		(0x3 << 17)
#define SETUP_TRANSACTION_COMPLETED	(0x4 << 17)
#define SETUP_PKT_RECEIVED		(0x6 << 17)
#define GLOBAL_OUT_NAK			(0x1 << 17)

/* DWC2_UDC_OTG_DCTL device control register */
#define NORMAL_OPERATION		BIT(0)
#define SOFT_DISCONNECT			BIT(1)

/* DWC2_UDC_OTG_DAINT device all endpoint interrupt register */
#define DAINT_OUTEP_MASK		GENMASK(31, 16)
#define DAINT_INEP_MASK			GENMASK(15, 0)

/* DWC2_UDC_OTG_DIEPCTL0/DOEPCTL0 device
   control IN/OUT endpoint 0 control register */
#define DEPCTL_EPENA			BIT(31)
#define DEPCTL_EPDIS			BIT(30)
#define DEPCTL_SETD1PID			BIT(29)
#define DEPCTL_SETD0PID			BIT(28)
#define DEPCTL_SNAK			BIT(27)
#define DEPCTL_CNAK			BIT(26)
#define DEPCTL_STALL			BIT(21)
#define DEPCTL_TYPE_MASK		GENMASK(19, 18)
#define DEPCTL_CTRL_TYPE		(0x0 << 18)
#define DEPCTL_ISO_TYPE			(0x1 << 18)
#define DEPCTL_BULK_TYPE		(0x2 << 18)
#define DEPCTL_INTR_TYPE		(0x3 << 18)
#define DEPCTL_USBACTEP			BIT(15)
#define DEPCTL_NEXT_EP_MASK		GENMASK(14, 11)
#define DEPCTL_MPS_MASK			GENMASK(10, 0)

#define DEPCTL0_MPS_64			(0x0 << 0)
#define DEPCTL0_MPS_32			(0x1 << 0)
#define DEPCTL0_MPS_16			(0x2 << 0)
#define DEPCTL0_MPS_8			(0x3 << 0)
#define DEPCTL_MPS_BULK_512		(512 << 0)
#define DEPCTL_MPS_INT_MPS_16		(16 << 0)

#define DIEPCTL0_NEXT_EP_BIT		(11)

/* DWC2_UDC_OTG_DIEPMSK/DOEPMSK device IN/OUT endpoint
   common interrupt mask register */
/* DWC2_UDC_OTG_DIEPINTn/DOEPINTn device IN/OUT endpoint interrupt register */
#define BACK2BACK_SETUP_RECEIVED	BIT(6)
#define INTKNEPMIS			BIT(5)
#define INTKN_TXFEMP			BIT(4)
#define NON_ISO_IN_EP_TIMEOUT		BIT(3)
#define CTRL_OUT_EP_SETUP_PHASE_DONE	BIT(3)
#define AHB_ERROR			BIT(2)
#define EPDISBLD			BIT(1)
#define TRANSFER_DONE			BIT(0)

#define USB_PHY_CTRL_EN0		BIT(0)

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

/* Device Configuration Register DCFG */
#define DEV_SPEED_HIGH_SPEED_20		(0x0 << 0)
#define DEV_SPEED_FULL_SPEED_20		(0x1 << 0)
#define DEV_SPEED_LOW_SPEED_11		(0x2 << 0)
#define DEV_SPEED_FULL_SPEED_11		(0x3 << 0)
#define EP_MISS_CNT(x)			((x) << 18)
#define DEVICE_ADDRESS(x)		((x) << 4)

/* Core Reset Register (GRSTCTL) */
#define TX_FIFO_FLUSH			BIT(5)
#define RX_FIFO_FLUSH			BIT(4)
#define TX_FIFO_NUMBER(x)		((x) << 6)
#define TX_FIFO_FLUSH_ALL		TX_FIFO_NUMBER(0x10)

/* Masks definitions */
#define GINTMSK_INIT	(INT_OUT_EP | INT_IN_EP | INT_RESUME | INT_ENUMDONE\
			| INT_RESET | INT_SUSPEND | INT_OTG)
#define DOEPMSK_INIT	(CTRL_OUT_EP_SETUP_PHASE_DONE | AHB_ERROR|TRANSFER_DONE)
#define DIEPMSK_INIT	(NON_ISO_IN_EP_TIMEOUT|AHB_ERROR|TRANSFER_DONE)
#define GAHBCFG_INIT	(PTXFE_HALF | NPTXFE_HALF | MODE_DMA | BURST_INCR4\
			| GBL_INT_UNMASK)

/* Device Endpoint X Transfer Size Register (DIEPTSIZX) */
#define DIEPT_SIZ_PKT_CNT(x)		((x) << 19)
#define DIEPT_SIZ_XFER_SIZE(x)		((x) << 0)

/* Device OUT Endpoint X Transfer Size Register (DOEPTSIZX) */
#define DOEPT_SIZ_PKT_CNT(x)		((x) << 19)
#define DOEPT_SIZ_XFER_SIZE(x)		((x) << 0)
#define DOEPT_SIZ_XFER_SIZE_MAX_EP0	(0x7F << 0)
#define DOEPT_SIZ_XFER_SIZE_MAX_EP	(0x7FFF << 0)

/* Device Endpoint-N Control Register (DIEPCTLn/DOEPCTLn) */
#define DIEPCTL_TX_FIFO_NUM_MASK	GENMASK(25, 22)

/* Device ALL Endpoints Interrupt Register (DAINT) */
#define DAINT_IN_EP_INT(x)		((x) << 0)
#define DAINT_OUT_EP_INT(x)		((x) << 16)

/* User HW Config4 */
#define GHWCFG4_NUM_IN_EPS_MASK		(0xf << 26)
#define GHWCFG4_NUM_IN_EPS_SHIFT	26

/* OTG general core configuration register (OTG_GCCFG:0x38) for STM32MP1 */
#define GGPIO_STM32_OTG_GCCFG_VBDEN	BIT(21)
#define GGPIO_STM32_OTG_GCCFG_IDEN	BIT(22)

#endif
