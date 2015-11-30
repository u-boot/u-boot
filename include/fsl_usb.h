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
#ifdef CONFIG_SYS_FSL_SINGLE_SOURCE_CLK
#define CONFIG_SYS_FSL_USB_PLLPRG2_REF_DIV_INTERNAL_CLK (5 << 4)
#define CONFIG_SYS_FSL_USB_PLLPRG2_MFI_INTERNAL_CLK (6 << 16)
#define CONFIG_SYS_FSL_USB_INTERNAL_SOC_CLK_EN (1 << 20)
#endif
#define CONFIG_SYS_FSL_USB_PLLPRG2_REF_DIV (1 << 4)
#define CONFIG_SYS_FSL_USB_PLLPRG2_MFI (5 << 16)
#define CONFIG_SYS_FSL_USB_PLLPRG2_PLL_EN (1 << 21)
#define CONFIG_SYS_FSL_USB_SYS_CLK_VALID (1 << 0)
#define CONFIG_SYS_FSL_USB_XCVRPRG_HS_DCNT_PROG_EN (1 << 7)
#define CONFIG_SYS_FSL_USB_XCVRPRG_HS_DCNT_PROG_MASK (3 << 4)

#define INC_DCNT_THRESHOLD_25MV        (0 << 4)
#define INC_DCNT_THRESHOLD_50MV        (1 << 4)
#define DEC_DCNT_THRESHOLD_25MV        (2 << 4)
#define DEC_DCNT_THRESHOLD_50MV        (3 << 4)
#else
struct ccsr_usb_phy {
	u32     config1;
	u32     config2;
	u32     config3;
	u32     config4;
	u32     config5;
	u32     status1;
	u32	usb_enable_override;
	u8	res[0xe4];
};
#define CONFIG_SYS_FSL_USB_HS_DISCNCT_INC (3 << 22)
#define CONFIG_SYS_FSL_USB_RX_AUTO_CAL_RD_WR_SEL (1 << 20)
#define CONFIG_SYS_FSL_USB_SQUELCH_PROG_WR_0 13
#define CONFIG_SYS_FSL_USB_SQUELCH_PROG_WR_3 16
#define CONFIG_SYS_FSL_USB_SQUELCH_PROG_RD_0 0
#define CONFIG_SYS_FSL_USB_SQUELCH_PROG_RD_3 3
#define CONFIG_SYS_FSL_USB_ENABLE_OVERRIDE 1
#define CONFIG_SYS_FSL_USB_SQUELCH_PROG_MASK 0x07
#endif

/* USB Erratum Checking code */
#ifdef CONFIG_PPC
static inline bool has_dual_phy(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	switch (soc) {
	case SVR_T1023:
	case SVR_T1024:
	case SVR_T1013:
	case SVR_T1014:
		return IS_SVR_REV(svr, 1, 0);
	case SVR_T1040:
	case SVR_T1042:
	case SVR_T1020:
	case SVR_T1022:
	case SVR_T2080:
	case SVR_T2081:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 1, 1);
	case SVR_T4240:
	case SVR_T4160:
	case SVR_T4080:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	}

	return false;
}

static inline bool has_erratum_a006261(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	switch (soc) {
	case SVR_P1010:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_P2041:
	case SVR_P2040:
		return IS_SVR_REV(svr, 1, 0) ||
			IS_SVR_REV(svr, 1, 1) || IS_SVR_REV(svr, 2, 1);
	case SVR_P3041:
		return IS_SVR_REV(svr, 1, 0) ||
			IS_SVR_REV(svr, 1, 1) ||
			IS_SVR_REV(svr, 2, 0) || IS_SVR_REV(svr, 2, 1);
	case SVR_P5010:
	case SVR_P5020:
	case SVR_P5021:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_T4240:
	case SVR_T4160:
	case SVR_T4080:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_T1040:
		return IS_SVR_REV(svr, 1, 0);
	case SVR_T2080:
	case SVR_T2081:
		return IS_SVR_REV(svr, 1, 0);
	case SVR_P5040:
		return IS_SVR_REV(svr, 1, 0);
	}

	return false;
}

static inline bool has_erratum_a007075(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	switch (soc) {
	case SVR_B4860:
	case SVR_B4420:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_P1010:
		return IS_SVR_REV(svr, 1, 0);
	case SVR_P4080:
		return IS_SVR_REV(svr, 2, 0) || IS_SVR_REV(svr, 3, 0);
	}
	return false;
}

static inline bool has_erratum_a007798(void)
{
	return SVR_SOC_VER(get_svr()) == SVR_T4240 &&
		IS_SVR_REV(get_svr(), 2, 0);
}

static inline bool has_erratum_a007792(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	switch (soc) {
	case SVR_T4240:
	case SVR_T4160:
	case SVR_T4080:
		return IS_SVR_REV(svr, 2, 0);
	case SVR_T1024:
	case SVR_T1023:
		return IS_SVR_REV(svr, 1, 0);
	case SVR_T1040:
	case SVR_T1042:
	case SVR_T1020:
	case SVR_T1022:
	case SVR_T2080:
	case SVR_T2081:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 1, 1);
	}
	return false;
}

static inline bool has_erratum_a005697(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	switch (soc) {
	case SVR_9131:
	case SVR_9132:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 1, 1);
	}
	return false;
}

static inline bool has_erratum_a004477(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	switch (soc) {
	case SVR_P1010:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_P1022:
	case SVR_9131:
	case SVR_9132:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 1, 1);
	case SVR_P2020:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0) ||
			IS_SVR_REV(svr, 2, 1);
	case SVR_B4860:
	case SVR_B4420:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_P4080:
		return IS_SVR_REV(svr, 2, 0) || IS_SVR_REV(svr, 3, 0);
	}

	return false;
}
#else
static inline bool has_dual_phy(void)
{
	return false;
}

static inline bool has_erratum_a006261(void)
{
	return false;
}

static inline bool has_erratum_a007075(void)
{
	return false;
}

static inline bool has_erratum_a007798(void)
{
	return false;
}

static inline bool has_erratum_a007792(void)
{
	return false;
}

static inline bool has_erratum_a005697(void)
{
	return false;
}

static inline bool has_erratum_a004477(void)
{
	return false;
}
#endif
#endif /*_ASM_FSL_USB_H_ */
