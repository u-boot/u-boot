/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _TEGRA_USB_H_
#define _TEGRA_USB_H_


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
	uint reserved4[6];

	/* 0x140 */
	uint usb_cmd;
	uint usb_sts;
	uint usb_intr;
	uint frindex;

	/* 0x150 */
	uint reserved5;
	uint periodic_list_base;
	uint async_list_addr;
	uint async_tt_sts;

	/* 0x160 */
	uint burst_size;
	uint tx_fill_tuning;
	uint reserved6;   /* is this port_sc1 on some controllers? */
	uint icusb_ctrl;

	/* 0x170 */
	uint ulpi_viewport;
	uint reserved7;
	uint endpt_nak;
	uint endpt_nak_enable;

	/* 0x180 */
	uint reserved;
	uint port_sc1;
	uint reserved8[6];

	/* 0x1a0 */
	uint reserved9;
	uint otgsc;
	uint usb_mode;
	uint endpt_setup_stat;

	/* 0x1b0 */
	uint reserved10[20];

	/* 0x200 */
	uint reserved11[0x80];

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

/* USBx_UTMIP_MISC_CFG1 */
#define UTMIP_PLLU_STABLE_COUNT_SHIFT		6
#define UTMIP_PLLU_STABLE_COUNT_MASK		\
				(0xfff << UTMIP_PLLU_STABLE_COUNT_SHIFT)
#define UTMIP_PLL_ACTIVE_DLY_COUNT_SHIFT	18
#define UTMIP_PLL_ACTIVE_DLY_COUNT_MASK		\
				(0x1f << UTMIP_PLL_ACTIVE_DLY_COUNT_SHIFT)
#define UTMIP_PHY_XTAL_CLOCKEN			(1 << 30)

/* USBx_UTMIP_PLL_CFG1_0 */
#define UTMIP_PLLU_ENABLE_DLY_COUNT_SHIFT	27
#define UTMIP_PLLU_ENABLE_DLY_COUNT_MASK	\
				(0xf << UTMIP_PLLU_ENABLE_DLY_COUNT_SHIFT)
#define UTMIP_XTAL_FREQ_COUNT_SHIFT		0
#define UTMIP_XTAL_FREQ_COUNT_MASK		0xfff

/* USBx_UTMIP_BIAS_CFG1_0 */
#define UTMIP_BIAS_PDTRK_COUNT_SHIFT		3
#define UTMIP_BIAS_PDTRK_COUNT_MASK		\
				(0x1f << UTMIP_BIAS_PDTRK_COUNT_SHIFT)

#define UTMIP_DEBOUNCE_CFG0_SHIFT		0
#define UTMIP_DEBOUNCE_CFG0_MASK		0xffff

/* USBx_UTMIP_TX_CFG0_0 */
#define UTMIP_FS_PREAMBLE_J			(1 << 19)

/* USBx_UTMIP_BAT_CHRG_CFG0_0 */
#define UTMIP_PD_CHRG				1

/* USBx_UTMIP_XCVR_CFG0_0 */
#define UTMIP_XCVR_LSBIAS_SE			(1 << 21)

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
				(0xf << UTMIP_HS_SYNC_START_DLY_SHIFT)

/* USBx_CONTROLLER_2_USB2D_ICUSB_CTRL_0 */
#define IC_ENB1					(1 << 3)

/* SB2_CONTROLLER_2_USB2D_PORTSC1_0 */
#define PTS_SHIFT				30
#define PTS_MASK				(3U << PTS_SHIFT)
#define PTS_UTMI	0
#define PTS_RESERVED	1
#define PTS_ULP		2
#define PTS_ICUSB_SER	3

#define STS					(1 << 29)

/* USBx_UTMIP_XCVR_CFG0_0 */
#define UTMIP_FORCE_PD_POWERDOWN		(1 << 14)
#define UTMIP_FORCE_PD2_POWERDOWN		(1 << 16)
#define UTMIP_FORCE_PDZI_POWERDOWN		(1 << 18)

/* USBx_UTMIP_XCVR_CFG1_0 */
#define UTMIP_FORCE_PDDISC_POWERDOWN		(1 << 0)
#define UTMIP_FORCE_PDCHRP_POWERDOWN		(1 << 2)
#define UTMIP_FORCE_PDDR_POWERDOWN		(1 << 4)

/* USB3_IF_USB_PHY_VBUS_SENSORS_0 */
#define VBUS_VLD_STS			(1 << 26)


/* Change the USB host port into host mode */
void usb_set_host_mode(void);

/* Setup USB on the board */
int board_usb_init(const void *blob);

/**
 * Start up the given port number (ports are numbered from 0 on each board).
 * This returns values for the appropriate hccr and hcor addresses to use for
 * USB EHCI operations.
 *
 * @param portnum	port number to start
 * @param hccr		returns start address of EHCI HCCR registers
 * @param hcor		returns start address of EHCI HCOR registers
 * @return 0 if ok, -1 on error (generally invalid port number)
 */
int tegrausb_start_port(unsigned portnum, u32 *hccr, u32 *hcor);

/**
 * Stop the current port
 *
 * @return 0 if ok, -1 if no port was active
 */
int tegrausb_stop_port(void);

#endif	/* _TEGRA_USB_H_ */
