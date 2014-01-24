/*
 * (C) Copyright 2013
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _TEGRA124_USB_H_
#define _TEGRA124_USB_H_


/* USB Controller (USBx_CONTROLLER_) regs */
struct usb_ctlr {
	/* 0x000 */
	uint id;
	uint reserved0;
	uint host;
	uint device;

	/* 0x010 */
	uint txbuf;
	uint rxbuf;
	uint reserved1[2];

	/* 0x020 */
	uint reserved2[56];

	/* 0x100 */
	u16 cap_length;
	u16 hci_version;
	uint hcs_params;
	uint hcc_params;
	uint reserved3[5];

	/* 0x120 */
	uint dci_version;
	uint dcc_params;
	uint reserved4[2];

	/* 0x130 */
	uint usb_cmd;
	uint usb_sts;
	uint usb_intr;
	uint frindex;

	/* 0x140 */
	uint reserved5;
	uint periodic_list_base;
	uint async_list_addr;
	uint reserved5_1;

	/* 0x150 */
	uint burst_size;
	uint tx_fill_tuning;
	uint reserved6;
	uint icusb_ctrl;

	/* 0x160 */
	uint ulpi_viewport;
	uint reserved7;
	uint reserved7_0;
	uint reserved7_1;

	/* 0x170 */
	uint reserved;
	uint port_sc1;
	uint reserved8[6];

	/* 0x190 */
	uint reserved9[8];

	/* 0x1b0 */
	uint reserved10;
	uint hostpc1_devlc;
	uint reserved10_1[2];

	/* 0x1c0 */
	uint reserved10_2[4];

	/* 0x1d0 */
	uint reserved10_3[4];

	/* 0x1e0 */
	uint reserved10_4[4];

	/* 0x1f0 */
	uint reserved10_5;
	uint otgsc;
	uint usb_mode;
	uint reserved10_6;

	/* 0x200 */
	uint endpt_nak;
	uint endpt_nak_enable;
	uint endpt_setup_stat;
	uint reserved11_1[0x7D];

	/* 0x400 */
	uint susp_ctrl;
	uint phy_vbus_sensors;
	uint phy_vbus_wakeup_id;
	uint phy_alt_vbus_sys;

	/* 0x410 */
	uint usb1_legacy_ctrl;
	uint reserved12[3];

	/* 0x420 */
	uint reserved13[56];

	/* 0x500 */
	uint reserved14[64 * 3];

	/* 0x800 */
	uint utmip_pll_cfg0;
	uint utmip_pll_cfg1;
	uint utmip_xcvr_cfg0;
	uint utmip_bias_cfg0;

	/* 0x810 */
	uint utmip_hsrx_cfg0;
	uint utmip_hsrx_cfg1;
	uint utmip_fslsrx_cfg0;
	uint utmip_fslsrx_cfg1;

	/* 0x820 */
	uint utmip_tx_cfg0;
	uint utmip_misc_cfg0;
	uint utmip_misc_cfg1;
	uint utmip_debounce_cfg0;

	/* 0x830 */
	uint utmip_bat_chrg_cfg0;
	uint utmip_spare_cfg0;
	uint utmip_xcvr_cfg1;
	uint utmip_bias_cfg1;
};

/* USB1_LEGACY_CTRL */
#define USB1_NO_LEGACY_MODE		1

#define VBUS_SENSE_CTL_SHIFT			1
#define VBUS_SENSE_CTL_MASK			(3 << VBUS_SENSE_CTL_SHIFT)
#define VBUS_SENSE_CTL_VBUS_WAKEUP		0
#define VBUS_SENSE_CTL_AB_SESS_VLD_OR_VBUS_WAKEUP	1
#define VBUS_SENSE_CTL_AB_SESS_VLD		2
#define VBUS_SENSE_CTL_A_SESS_VLD		3

/* USBx_IF_USB_SUSP_CTRL_0 */
#define UTMIP_PHY_ENB			        (1 << 12)
#define UTMIP_RESET			        (1 << 11)
#define USB_PHY_CLK_VALID			(1 << 7)
#define USB_SUSP_CLR				(1 << 5)

/* USBx_UTMIP_MISC_CFG0 */
#define UTMIP_SUSPEND_EXIT_ON_EDGE		(1 << 22)

/* USBx_UTMIP_MISC_CFG1 */
#define UTMIP_PHY_XTAL_CLOCKEN			(1 << 30)

/* Moved to Clock and Reset register space */
#define UTMIP_PLLU_STABLE_COUNT_SHIFT		6
#define UTMIP_PLLU_STABLE_COUNT_MASK		\
				(0xfff << UTMIP_PLLU_STABLE_COUNT_SHIFT)
/* Moved to Clock and Reset register space */
#define UTMIP_PLL_ACTIVE_DLY_COUNT_SHIFT	18
#define UTMIP_PLL_ACTIVE_DLY_COUNT_MASK		\
				(0x1f << UTMIP_PLL_ACTIVE_DLY_COUNT_SHIFT)

/* USBx_UTMIP_PLL_CFG1_0 */
/* Moved to Clock and Reset register space */
#define UTMIP_PLLU_ENABLE_DLY_COUNT_SHIFT	27
#define UTMIP_PLLU_ENABLE_DLY_COUNT_MASK	\
				(0x1f << UTMIP_PLLU_ENABLE_DLY_COUNT_SHIFT)
#define UTMIP_XTAL_FREQ_COUNT_SHIFT		0
#define UTMIP_XTAL_FREQ_COUNT_MASK		0xfff

/* USBx_UTMIP_BIAS_CFG0_0 */
#define UTMIP_HSDISCON_LEVEL_MSB		(1 << 24)
#define UTMIP_OTGPD				(1 << 11)
#define UTMIP_BIASPD				(1 << 10)
#define UTMIP_HSDISCON_LEVEL_SHIFT		2
#define UTMIP_HSDISCON_LEVEL_MASK		\
				(0x3 << UTMIP_HSDISCON_LEVEL_SHIFT)
#define UTMIP_HSSQUELCH_LEVEL_SHIFT		0
#define UTMIP_HSSQUELCH_LEVEL_MASK		\
				(0x3 << UTMIP_HSSQUELCH_LEVEL_SHIFT)

/* USBx_UTMIP_BIAS_CFG1_0 */
#define UTMIP_FORCE_PDTRK_POWERDOWN		1
#define UTMIP_BIAS_PDTRK_COUNT_SHIFT		3
#define UTMIP_BIAS_PDTRK_COUNT_MASK		\
				(0x1f << UTMIP_BIAS_PDTRK_COUNT_SHIFT)

/* USBx_UTMIP_DEBOUNCE_CFG0_0 */
#define UTMIP_DEBOUNCE_CFG0_SHIFT		0
#define UTMIP_DEBOUNCE_CFG0_MASK		0xffff

/* USBx_UTMIP_TX_CFG0_0 */
#define UTMIP_FS_PREAMBLE_J			(1 << 19)

/* USBx_UTMIP_BAT_CHRG_CFG0_0 */
#define UTMIP_PD_CHRG				1

/* USBx_UTMIP_SPARE_CFG0_0 */
#define FUSE_SETUP_SEL				(1 << 3)

/* USBx_UTMIP_HSRX_CFG0_0 */
#define UTMIP_IDLE_WAIT_SHIFT			15
#define UTMIP_IDLE_WAIT_MASK			(0x1f << UTMIP_IDLE_WAIT_SHIFT)
#define UTMIP_ELASTIC_LIMIT_SHIFT		10
#define UTMIP_ELASTIC_LIMIT_MASK		\
				(0x1f << UTMIP_ELASTIC_LIMIT_SHIFT)

/* USBx_UTMIP_HSRX_CFG0_1 */
#define UTMIP_HS_SYNC_START_DLY_SHIFT		1
#define UTMIP_HS_SYNC_START_DLY_MASK		\
				(0x1f << UTMIP_HS_SYNC_START_DLY_SHIFT)

/* USBx_CONTROLLER_2_USB2D_ICUSB_CTRL_0 */
#define IC_ENB1					(1 << 3)

/* PORTSC1, USB1, defined for Tegra20 to avoid compiling error */
#define PTS1_SHIFT				31
#define PTS1_MASK				(1 << PTS1_SHIFT)
#define STS1					(1 << 30)

/* USB2D_HOSTPC1_DEVLC_0 */
#define PTS_SHIFT				29
#define PTS_MASK				(0x7U << PTS_SHIFT)
#define PTS_UTMI	0
#define PTS_RESERVED	1
#define PTS_ULPI	2
#define PTS_ICUSB_SER	3
#define PTS_HSIC	4

#define STS					(1 << 28)

/* SB2_CONTROLLER_2_USB2D_PORTSC1_0 */
#define WKOC				(1 << 22)
#define WKDS				(1 << 21)
#define WKCN				(1 << 20)

/* USBx_UTMIP_XCVR_CFG0_0 */
#define UTMIP_FORCE_PD_POWERDOWN		(1 << 14)
#define UTMIP_FORCE_PD2_POWERDOWN		(1 << 16)
#define UTMIP_FORCE_PDZI_POWERDOWN		(1 << 18)
#define UTMIP_XCVR_LSBIAS_SE			(1 << 21)
#define UTMIP_XCVR_HSSLEW_MSB_SHIFT		25
#define UTMIP_XCVR_HSSLEW_MSB_MASK		\
			(0x7f << UTMIP_XCVR_HSSLEW_MSB_SHIFT)
#define UTMIP_XCVR_SETUP_MSB_SHIFT	22
#define UTMIP_XCVR_SETUP_MSB_MASK	(0x7 << UTMIP_XCVR_SETUP_MSB_SHIFT)
#define UTMIP_XCVR_SETUP_SHIFT		0
#define UTMIP_XCVR_SETUP_MASK		(0xf << UTMIP_XCVR_SETUP_SHIFT)

/* USBx_UTMIP_XCVR_CFG1_0 */
#define UTMIP_XCVR_TERM_RANGE_ADJ_SHIFT		18
#define UTMIP_XCVR_TERM_RANGE_ADJ_MASK		\
			(0xf << UTMIP_XCVR_TERM_RANGE_ADJ_SHIFT)
#define UTMIP_FORCE_PDDISC_POWERDOWN		(1 << 0)
#define UTMIP_FORCE_PDCHRP_POWERDOWN		(1 << 2)
#define UTMIP_FORCE_PDDR_POWERDOWN		(1 << 4)

/* USB3_IF_USB_PHY_VBUS_SENSORS_0 */
#define VBUS_VLD_STS			(1 << 26)

#endif	/* _TEGRA124_USB_H_ */
