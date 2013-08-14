/*
 * Freescale USB Controller
 *
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ASM_FSL_USB_H_
#define _ASM_FSL_USB_H_

#ifdef CONFIG_SYS_FSL_USB_DUAL_PHY_ENABLE
struct ccsr_usb_port_ctrl {
	u32	ctrl;
	u32	drvvbuscfg;
	u32	pwrfltcfg;
	u32	sts;
	u8	res_14[0xc];
	u32	bistcfg;
	u32	biststs;
	u32	abistcfg;
	u32	abiststs;
	u8	res_30[0x10];
	u32	xcvrprg;
	u32	anaprg;
	u32	anadrv;
	u32	anasts;
};

struct ccsr_usb_phy {
	u32	id;
	struct ccsr_usb_port_ctrl port1;
	u8	res_50[0xc];
	u32	tvr;
	u32	pllprg[4];
	u8	res_70[0x4];
	u32	anaccfg;
	u32	dbg;
	u8	res_7c[0x4];
	struct ccsr_usb_port_ctrl port2;
	u8	res_dc[0x334];
};

#define CONFIG_SYS_FSL_USB_CTRL_PHY_EN (1 << 0)
#define CONFIG_SYS_FSL_USB_DRVVBUS_CR_EN (1 << 1)
#define CONFIG_SYS_FSL_USB_PWRFLT_CR_EN (1 << 1)
#define CONFIG_SYS_FSL_USB_PLLPRG1_PHY_DIV (1 << 0)
#define CONFIG_SYS_FSL_USB_PLLPRG2_PHY2_CLK_EN (1 << 0)
#define CONFIG_SYS_FSL_USB_PLLPRG2_PHY1_CLK_EN (1 << 1)
#define CONFIG_SYS_FSL_USB_PLLPRG2_FRAC_LPF_EN (1 << 13)
#define CONFIG_SYS_FSL_USB_PLLPRG2_REF_DIV (1 << 4)
#define CONFIG_SYS_FSL_USB_PLLPRG2_MFI (5 << 16)
#define CONFIG_SYS_FSL_USB_PLLPRG2_PLL_EN (1 << 21)
#define CONFIG_SYS_FSL_USB_SYS_CLK_VALID (1 << 0)
#else
struct ccsr_usb_phy {
	u8	res0[0x18];
	u32	usb_enable_override;
	u8	res[0xe4];
};
#define	CONFIG_SYS_FSL_USB_ENABLE_OVERRIDE	1
#endif

#endif /*_ASM_FSL_USB_H_ */
