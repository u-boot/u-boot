/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2010,2011 NVIDIA Corporation <www.nvidia.com>
 *
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

#include <common.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <asm/arch/tegra20.h>
#include <asm/arch/clk_rst.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/uart.h>
#include <asm/arch/usb.h>
#include <libfdt.h>
#include <fdtdec.h>

enum {
	USB_PORTS_MAX	= 4,			/* Maximum ports we allow */
};

/* Parameters we need for USB */
enum {
	PARAM_DIVN,                     /* PLL FEEDBACK DIVIDer */
	PARAM_DIVM,                     /* PLL INPUT DIVIDER */
	PARAM_DIVP,                     /* POST DIVIDER (2^N) */
	PARAM_CPCON,                    /* BASE PLLC CHARGE Pump setup ctrl */
	PARAM_LFCON,                    /* BASE PLLC LOOP FILter setup ctrl */
	PARAM_ENABLE_DELAY_COUNT,       /* PLL-U Enable Delay Count */
	PARAM_STABLE_COUNT,             /* PLL-U STABLE count */
	PARAM_ACTIVE_DELAY_COUNT,       /* PLL-U Active delay count */
	PARAM_XTAL_FREQ_COUNT,          /* PLL-U XTAL frequency count */
	PARAM_DEBOUNCE_A_TIME,          /* 10MS DELAY for BIAS_DEBOUNCE_A */
	PARAM_BIAS_TIME,                /* 20US DELAY AFter bias cell op */

	PARAM_COUNT
};

/* Possible port types (dual role mode) */
enum dr_mode {
	DR_MODE_NONE = 0,
	DR_MODE_HOST,		/* supports host operation */
	DR_MODE_DEVICE,		/* supports device operation */
	DR_MODE_OTG,		/* supports both */
};

/* Information about a USB port */
struct fdt_usb {
	struct usb_ctlr *reg;	/* address of registers in physical memory */
	unsigned utmi:1;	/* 1 if port has external tranceiver, else 0 */
	unsigned enabled:1;	/* 1 to enable, 0 to disable */
	unsigned has_legacy_mode:1; /* 1 if this port has legacy mode */
	enum dr_mode dr_mode;	/* dual role mode */
	enum periph_id periph_id;/* peripheral id */
	struct fdt_gpio_state vbus_gpio;	/* GPIO for vbus enable */
};

static struct fdt_usb port[USB_PORTS_MAX];	/* List of valid USB ports */
static unsigned port_count;			/* Number of available ports */
static int port_current;			/* Current port (-1 = none) */

/*
 * This table has USB timing parameters for each Oscillator frequency we
 * support. There are four sets of values:
 *
 * 1. PLLU configuration information (reference clock is osc/clk_m and
 * PLLU-FOs are fixed at 12MHz/60MHz/480MHz).
 *
 *  Reference frequency     13.0MHz      19.2MHz      12.0MHz      26.0MHz
 *  ----------------------------------------------------------------------
 *      DIVN                960 (0x3c0)  200 (0c8)    960 (3c0h)   960 (3c0)
 *      DIVM                13 (0d)      4 (04)       12 (0c)      26 (1a)
 * Filter frequency (MHz)   1            4.8          6            2
 * CPCON                    1100b        0011b        1100b        1100b
 * LFCON0                   0            0            0            0
 *
 * 2. PLL CONFIGURATION & PARAMETERS for different clock generators:
 *
 * Reference frequency     13.0MHz         19.2MHz         12.0MHz     26.0MHz
 * ---------------------------------------------------------------------------
 * PLLU_ENABLE_DLY_COUNT   02 (0x02)       03 (03)         02 (02)     04 (04)
 * PLLU_STABLE_COUNT       51 (33)         75 (4B)         47 (2F)    102 (66)
 * PLL_ACTIVE_DLY_COUNT    05 (05)         06 (06)         04 (04)     09 (09)
 * XTAL_FREQ_COUNT        127 (7F)        187 (BB)        118 (76)    254 (FE)
 *
 * 3. Debounce values IdDig, Avalid, Bvalid, VbusValid, VbusWakeUp, and
 * SessEnd. Each of these signals have their own debouncer and for each of
 * those one out of two debouncing times can be chosen (BIAS_DEBOUNCE_A or
 * BIAS_DEBOUNCE_B).
 *
 * The values of DEBOUNCE_A and DEBOUNCE_B are calculated as follows:
 *    0xffff -> No debouncing at all
 *    <n> ms = <n> *1000 / (1/19.2MHz) / 4
 *
 * So to program a 1 ms debounce for BIAS_DEBOUNCE_A, we have:
 * BIAS_DEBOUNCE_A[15:0] = 1000 * 19.2 / 4  = 4800 = 0x12c0
 *
 * We need to use only DebounceA for BOOTROM. We don't need the DebounceB
 * values, so we can keep those to default.
 *
 * 4. The 20 microsecond delay after bias cell operation.
 */
static const unsigned usb_pll[CLOCK_OSC_FREQ_COUNT][PARAM_COUNT] = {
	/* DivN, DivM, DivP, CPCON, LFCON, Delays             Debounce, Bias */
	{ 0x3C0, 0x0D, 0x00, 0xC,   0,  0x02, 0x33, 0x05, 0x7F, 0x7EF4, 5 },
	{ 0x0C8, 0x04, 0x00, 0x3,   0,  0x03, 0x4B, 0x06, 0xBB, 0xBB80, 7 },
	{ 0x3C0, 0x0C, 0x00, 0xC,   0,  0x02, 0x2F, 0x04, 0x76, 0x7530, 5 },
	{ 0x3C0, 0x1A, 0x00, 0xC,   0,  0x04, 0x66, 0x09, 0xFE, 0xFDE8, 9 }
};

/* UTMIP Idle Wait Delay */
static const u8 utmip_idle_wait_delay = 17;

/* UTMIP Elastic limit */
static const u8 utmip_elastic_limit = 16;

/* UTMIP High Speed Sync Start Delay */
static const u8 utmip_hs_sync_start_delay = 9;

/* Put the port into host mode */
static void set_host_mode(struct fdt_usb *config)
{
	/*
	 * If we are an OTG port, check if remote host is driving VBus and
	 * bail out in this case.
	 */
	if (config->dr_mode == DR_MODE_OTG &&
		(readl(&config->reg->phy_vbus_sensors) & VBUS_VLD_STS))
		return;

	/*
	 * If not driving, we set the GPIO to enable VBUS. We assume
	 * that the pinmux is set up correctly for this.
	 */
	if (fdt_gpio_isvalid(&config->vbus_gpio)) {
		fdtdec_setup_gpio(&config->vbus_gpio);
		gpio_direction_output(config->vbus_gpio.gpio,
			(config->vbus_gpio.flags & FDT_GPIO_ACTIVE_LOW) ?
				 0 : 1);
		debug("set_host_mode: GPIO %d %s\n", config->vbus_gpio.gpio,
			(config->vbus_gpio.flags & FDT_GPIO_ACTIVE_LOW) ?
				"low" : "high");
	}
}

void usbf_reset_controller(struct fdt_usb *config, struct usb_ctlr *usbctlr)
{
	/* Reset the USB controller with 2us delay */
	reset_periph(config->periph_id, 2);

	/*
	 * Set USB1_NO_LEGACY_MODE to 1, Registers are accessible under
	 * base address
	 */
	if (config->has_legacy_mode)
		setbits_le32(&usbctlr->usb1_legacy_ctrl, USB1_NO_LEGACY_MODE);

	/* Put UTMIP1/3 in reset */
	setbits_le32(&usbctlr->susp_ctrl, UTMIP_RESET);

	/* Enable the UTMIP PHY */
	if (config->utmi)
		setbits_le32(&usbctlr->susp_ctrl, UTMIP_PHY_ENB);

	/*
	 * TODO: where do we take the USB1 out of reset? The old code would
	 * take USB3 out of reset, but not USB1. This code doesn't do either.
	 */
}

/* set up the USB controller with the parameters provided */
static int init_usb_controller(struct fdt_usb *config,
				struct usb_ctlr *usbctlr, const u32 timing[])
{
	u32 val;
	int loop_count;

	clock_enable(config->periph_id);

	/* Reset the usb controller */
	usbf_reset_controller(config, usbctlr);

	/* Stop crystal clock by setting UTMIP_PHY_XTAL_CLOCKEN low */
	clrbits_le32(&usbctlr->utmip_misc_cfg1, UTMIP_PHY_XTAL_CLOCKEN);

	/* Follow the crystal clock disable by >100ns delay */
	udelay(1);

	/*
	 * To Use the A Session Valid for cable detection logic, VBUS_WAKEUP
	 * mux must be switched to actually use a_sess_vld threshold.
	 */
	if (fdt_gpio_isvalid(&config->vbus_gpio)) {
		clrsetbits_le32(&usbctlr->usb1_legacy_ctrl,
			VBUS_SENSE_CTL_MASK,
			VBUS_SENSE_CTL_A_SESS_VLD << VBUS_SENSE_CTL_SHIFT);
	}

	/*
	 * PLL Delay CONFIGURATION settings. The following parameters control
	 * the bring up of the plls.
	 */
	val = readl(&usbctlr->utmip_misc_cfg1);
	clrsetbits_le32(&val, UTMIP_PLLU_STABLE_COUNT_MASK,
		timing[PARAM_STABLE_COUNT] << UTMIP_PLLU_STABLE_COUNT_SHIFT);
	clrsetbits_le32(&val, UTMIP_PLL_ACTIVE_DLY_COUNT_MASK,
		timing[PARAM_ACTIVE_DELAY_COUNT] <<
			UTMIP_PLL_ACTIVE_DLY_COUNT_SHIFT);
	writel(val, &usbctlr->utmip_misc_cfg1);

	/* Set PLL enable delay count and crystal frequency count */
	val = readl(&usbctlr->utmip_pll_cfg1);
	clrsetbits_le32(&val, UTMIP_PLLU_ENABLE_DLY_COUNT_MASK,
		timing[PARAM_ENABLE_DELAY_COUNT] <<
			UTMIP_PLLU_ENABLE_DLY_COUNT_SHIFT);
	clrsetbits_le32(&val, UTMIP_XTAL_FREQ_COUNT_MASK,
		timing[PARAM_XTAL_FREQ_COUNT] <<
			UTMIP_XTAL_FREQ_COUNT_SHIFT);
	writel(val, &usbctlr->utmip_pll_cfg1);

	/* Setting the tracking length time */
	clrsetbits_le32(&usbctlr->utmip_bias_cfg1,
		UTMIP_BIAS_PDTRK_COUNT_MASK,
		timing[PARAM_BIAS_TIME] << UTMIP_BIAS_PDTRK_COUNT_SHIFT);

	/* Program debounce time for VBUS to become valid */
	clrsetbits_le32(&usbctlr->utmip_debounce_cfg0,
		UTMIP_DEBOUNCE_CFG0_MASK,
		timing[PARAM_DEBOUNCE_A_TIME] << UTMIP_DEBOUNCE_CFG0_SHIFT);

	setbits_le32(&usbctlr->utmip_tx_cfg0, UTMIP_FS_PREAMBLE_J);

	/* Disable battery charge enabling bit */
	setbits_le32(&usbctlr->utmip_bat_chrg_cfg0, UTMIP_PD_CHRG);

	clrbits_le32(&usbctlr->utmip_xcvr_cfg0, UTMIP_XCVR_LSBIAS_SE);
	setbits_le32(&usbctlr->utmip_spare_cfg0, FUSE_SETUP_SEL);

	/*
	 * Configure the UTMIP_IDLE_WAIT and UTMIP_ELASTIC_LIMIT
	 * Setting these fields, together with default values of the
	 * other fields, results in programming the registers below as
	 * follows:
	 *         UTMIP_HSRX_CFG0 = 0x9168c000
	 *         UTMIP_HSRX_CFG1 = 0x13
	 */

	/* Set PLL enable delay count and Crystal frequency count */
	val = readl(&usbctlr->utmip_hsrx_cfg0);
	clrsetbits_le32(&val, UTMIP_IDLE_WAIT_MASK,
		utmip_idle_wait_delay << UTMIP_IDLE_WAIT_SHIFT);
	clrsetbits_le32(&val, UTMIP_ELASTIC_LIMIT_MASK,
		utmip_elastic_limit << UTMIP_ELASTIC_LIMIT_SHIFT);
	writel(val, &usbctlr->utmip_hsrx_cfg0);

	/* Configure the UTMIP_HS_SYNC_START_DLY */
	clrsetbits_le32(&usbctlr->utmip_hsrx_cfg1,
		UTMIP_HS_SYNC_START_DLY_MASK,
		utmip_hs_sync_start_delay << UTMIP_HS_SYNC_START_DLY_SHIFT);

	/* Preceed the crystal clock disable by >100ns delay. */
	udelay(1);

	/* Resuscitate crystal clock by setting UTMIP_PHY_XTAL_CLOCKEN */
	setbits_le32(&usbctlr->utmip_misc_cfg1, UTMIP_PHY_XTAL_CLOCKEN);

	/* Finished the per-controller init. */

	/* De-assert UTMIP_RESET to bring out of reset. */
	clrbits_le32(&usbctlr->susp_ctrl, UTMIP_RESET);

	/* Wait for the phy clock to become valid in 100 ms */
	for (loop_count = 100000; loop_count != 0; loop_count--) {
		if (readl(&usbctlr->susp_ctrl) & USB_PHY_CLK_VALID)
			break;
		udelay(1);
	}
	if (!loop_count)
		return -1;

	return 0;
}

static void power_up_port(struct usb_ctlr *usbctlr)
{
	/* Deassert power down state */
	clrbits_le32(&usbctlr->utmip_xcvr_cfg0, UTMIP_FORCE_PD_POWERDOWN |
		UTMIP_FORCE_PD2_POWERDOWN | UTMIP_FORCE_PDZI_POWERDOWN);
	clrbits_le32(&usbctlr->utmip_xcvr_cfg1, UTMIP_FORCE_PDDISC_POWERDOWN |
		UTMIP_FORCE_PDCHRP_POWERDOWN | UTMIP_FORCE_PDDR_POWERDOWN);
}

static void config_clock(const u32 timing[])
{
	clock_start_pll(CLOCK_ID_USB,
		timing[PARAM_DIVM], timing[PARAM_DIVN], timing[PARAM_DIVP],
		timing[PARAM_CPCON], timing[PARAM_LFCON]);
}

/**
 * Add a new USB port to the list of available ports.
 *
 * @param config	USB port configuration
 * @return 0 if ok, -1 if error (too many ports)
 */
static int add_port(struct fdt_usb *config, const u32 timing[])
{
	struct usb_ctlr *usbctlr = config->reg;

	if (port_count == USB_PORTS_MAX) {
		debug("tegrausb: Cannot register more than %d ports\n",
		      USB_PORTS_MAX);
		return -1;
	}
	if (init_usb_controller(config, usbctlr, timing)) {
		debug("tegrausb: Cannot init port\n");
		return -1;
	}
	if (config->utmi) {
		/* Disable ICUSB FS/LS transceiver */
		clrbits_le32(&usbctlr->icusb_ctrl, IC_ENB1);

		/* Select UTMI parallel interface */
		clrsetbits_le32(&usbctlr->port_sc1, PTS_MASK,
				PTS_UTMI << PTS_SHIFT);
		clrbits_le32(&usbctlr->port_sc1, STS);
		power_up_port(usbctlr);
	}
	port[port_count++] = *config;

	return 0;
}

int tegrausb_start_port(unsigned portnum, u32 *hccr, u32 *hcor)
{
	struct usb_ctlr *usbctlr;

	if (portnum >= port_count)
		return -1;
	tegrausb_stop_port();
	set_host_mode(&port[portnum]);

	usbctlr = port[portnum].reg;
	*hccr = (u32)&usbctlr->cap_length;
	*hcor = (u32)&usbctlr->usb_cmd;
	port_current = portnum;
	return 0;
}

int tegrausb_stop_port(void)
{
	struct usb_ctlr *usbctlr;

	if (port_current == -1)
		return -1;

	usbctlr = port[port_current].reg;

	/* Stop controller */
	writel(0, &usbctlr->usb_cmd);
	udelay(1000);

	/* Initiate controller reset */
	writel(2, &usbctlr->usb_cmd);
	udelay(1000);
	port_current = -1;
	return 0;
}

int fdt_decode_usb(const void *blob, int node, unsigned osc_frequency_mhz,
		   struct fdt_usb *config)
{
	const char *phy, *mode;

	config->reg = (struct usb_ctlr *)fdtdec_get_addr(blob, node, "reg");
	mode = fdt_getprop(blob, node, "dr_mode", NULL);
	if (mode) {
		if (0 == strcmp(mode, "host"))
			config->dr_mode = DR_MODE_HOST;
		else if (0 == strcmp(mode, "peripheral"))
			config->dr_mode = DR_MODE_DEVICE;
		else if (0 == strcmp(mode, "otg"))
			config->dr_mode = DR_MODE_OTG;
		else {
			debug("%s: Cannot decode dr_mode '%s'\n", __func__,
			      mode);
			return -FDT_ERR_NOTFOUND;
		}
	} else {
		config->dr_mode = DR_MODE_HOST;
	}

	phy = fdt_getprop(blob, node, "phy_type", NULL);
	config->utmi = phy && 0 == strcmp("utmi", phy);
	config->enabled = fdtdec_get_is_enabled(blob, node);
	config->has_legacy_mode = fdtdec_get_bool(blob, node,
						  "nvidia,has-legacy-mode");
	config->periph_id = clock_decode_periph_id(blob, node);
	if (config->periph_id == PERIPH_ID_NONE) {
		debug("%s: Missing/invalid peripheral ID\n", __func__);
		return -FDT_ERR_NOTFOUND;
	}
	fdtdec_decode_gpio(blob, node, "nvidia,vbus-gpio", &config->vbus_gpio);
	debug("enabled=%d, legacy_mode=%d, utmi=%d, periph_id=%d, vbus=%d, "
	      "dr_mode=%d\n", config->enabled, config->has_legacy_mode,
	      config->utmi, config->periph_id, config->vbus_gpio.gpio,
	      config->dr_mode);

	return 0;
}

int board_usb_init(const void *blob)
{
	struct fdt_usb config;
	unsigned osc_freq = clock_get_rate(CLOCK_ID_OSC);
	enum clock_osc_freq freq;
	int node_list[USB_PORTS_MAX];
	int node, count, i;

	/* Set up the USB clocks correctly based on our oscillator frequency */
	freq = clock_get_osc_freq();
	config_clock(usb_pll[freq]);

	/* count may return <0 on error */
	count = fdtdec_find_aliases_for_id(blob, "usb",
			COMPAT_NVIDIA_TEGRA20_USB, node_list, USB_PORTS_MAX);
	for (i = 0; i < count; i++) {
		debug("USB %d: ", i);
		node = node_list[i];
		if (!node)
			continue;
		if (fdt_decode_usb(blob, node, osc_freq, &config)) {
			debug("Cannot decode USB node %s\n",
			      fdt_get_name(blob, node, NULL));
			return -1;
		}

		if (add_port(&config, usb_pll[freq]))
			return -1;
		set_host_mode(&config);
	}
	port_current = -1;

	return 0;
}
