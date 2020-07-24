// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011 Ilya Yanok, Emcraft Systems
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * Derived from Beagle Board code by
 *	Sunil Kumar <sunilsaini05@gmail.com>
 *	Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 */

#include <common.h>
#include <log.h>
#include <usb.h>
#include <linux/delay.h>
#include <usb/ulpi.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/ehci.h>
#include <asm/ehci-omap.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <power/regulator.h>

#include "ehci.h"

static struct omap_uhh *const uhh = (struct omap_uhh *)OMAP_UHH_BASE;
static struct omap_usbtll *const usbtll = (struct omap_usbtll *)OMAP_USBTLL_BASE;
static struct omap_ehci *const ehci = (struct omap_ehci *)OMAP_EHCI_BASE;

static int omap_uhh_reset(void)
{
	int timeout = 0;
	u32 rev;

	rev = readl(&uhh->rev);

	/* Soft RESET */
	writel(OMAP_UHH_SYSCONFIG_SOFTRESET, &uhh->sysc);

	switch (rev) {
	case OMAP_USBHS_REV1:
		/* Wait for soft RESET to complete */
		while (!(readl(&uhh->syss) & 0x1)) {
			if (timeout > 100) {
				printf("%s: RESET timeout\n", __func__);
				return -1;
			}
			udelay(10);
			timeout++;
		}

		/* Set No-Idle, No-Standby */
		writel(OMAP_UHH_SYSCONFIG_VAL, &uhh->sysc);
		break;

	default:	/* Rev. 2 onwards */

		udelay(2); /* Need to wait before accessing SYSCONFIG back */

		/* Wait for soft RESET to complete */
		while ((readl(&uhh->sysc) & 0x1)) {
			if (timeout > 100) {
				printf("%s: RESET timeout\n", __func__);
				return -1;
			}
			udelay(10);
			timeout++;
		}

		writel(OMAP_UHH_SYSCONFIG_VAL, &uhh->sysc);
		break;
	}

	return 0;
}

static int omap_ehci_tll_reset(void)
{
	unsigned long init = get_timer(0);

	/* perform TLL soft reset, and wait until reset is complete */
	writel(OMAP_USBTLL_SYSCONFIG_SOFTRESET, &usbtll->sysc);

	/* Wait for TLL reset to complete */
	while (!(readl(&usbtll->syss) & OMAP_USBTLL_SYSSTATUS_RESETDONE))
		if (get_timer(init) > CONFIG_SYS_HZ) {
			debug("OMAP EHCI error: timeout resetting TLL\n");
			return -EL3RST;
	}

	return 0;
}

static void omap_usbhs_hsic_init(int port)
{
	unsigned int reg;

	/* Enable channels now */
	reg = readl(&usbtll->channel_conf + port);

	setbits_le32(&reg, (OMAP_TLL_CHANNEL_CONF_CHANMODE_TRANSPARENT_UTMI
		| OMAP_TLL_CHANNEL_CONF_ULPINOBITSTUFF
		| OMAP_TLL_CHANNEL_CONF_DRVVBUS
		| OMAP_TLL_CHANNEL_CONF_CHRGVBUS
		| OMAP_TLL_CHANNEL_CONF_CHANEN));

	writel(reg, &usbtll->channel_conf + port);
}

#ifdef CONFIG_USB_ULPI
static void omap_ehci_soft_phy_reset(int port)
{
	struct ulpi_viewport ulpi_vp;

	ulpi_vp.viewport_addr = (u32)&ehci->insreg05_utmi_ulpi;
	ulpi_vp.port_num = port;

	ulpi_reset(&ulpi_vp);
}
#else
static void omap_ehci_soft_phy_reset(int port)
{
	return;
}
#endif

#if defined(CONFIG_OMAP_EHCI_PHY1_RESET_GPIO) || \
	defined(CONFIG_OMAP_EHCI_PHY2_RESET_GPIO) || \
	defined(CONFIG_OMAP_EHCI_PHY3_RESET_GPIO)
/* controls PHY(s) reset signal(s) */
static inline void omap_ehci_phy_reset(int on, int delay)
{
	/*
	 * Refer ISSUE1:
	 * Hold the PHY in RESET for enough time till
	 * PHY is settled and ready
	 */
	if (delay && !on)
		udelay(delay);
#ifdef CONFIG_OMAP_EHCI_PHY1_RESET_GPIO
	gpio_request(CONFIG_OMAP_EHCI_PHY1_RESET_GPIO, "USB PHY1 reset");
	gpio_direction_output(CONFIG_OMAP_EHCI_PHY1_RESET_GPIO, !on);
#endif
#ifdef CONFIG_OMAP_EHCI_PHY2_RESET_GPIO
	gpio_request(CONFIG_OMAP_EHCI_PHY2_RESET_GPIO, "USB PHY2 reset");
	gpio_direction_output(CONFIG_OMAP_EHCI_PHY2_RESET_GPIO, !on);
#endif
#ifdef CONFIG_OMAP_EHCI_PHY3_RESET_GPIO
	gpio_request(CONFIG_OMAP_EHCI_PHY3_RESET_GPIO, "USB PHY3 reset");
	gpio_direction_output(CONFIG_OMAP_EHCI_PHY3_RESET_GPIO, !on);
#endif

	/* Hold the PHY in RESET for enough time till DIR is high */
	/* Refer: ISSUE1 */
	if (delay && on)
		udelay(delay);
}
#else
#define omap_ehci_phy_reset(on, delay)	do {} while (0)
#endif

/* Reset is needed otherwise the kernel-driver will throw an error. */
int omap_ehci_hcd_stop(void)
{
	debug("Resetting OMAP EHCI\n");
	omap_ehci_phy_reset(1, 0);

	if (omap_uhh_reset() < 0)
		return -1;

	if (omap_ehci_tll_reset() < 0)
		return -1;

	return 0;
}

/*
 * Initialize the OMAP EHCI controller and PHY.
 * Based on "drivers/usb/host/ehci-omap.c" from Linux 3.1
 * See there for additional Copyrights.
 */
#if !CONFIG_IS_ENABLED(DM_USB) || !CONFIG_IS_ENABLED(OF_CONTROL)

int omap_ehci_hcd_init(int index, struct omap_usbhs_board_data *usbhs_pdata,
		       struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	*hccr = (struct ehci_hccr *)(OMAP_EHCI_BASE);
	*hcor = (struct ehci_hcor *)(OMAP_EHCI_BASE + 0x10);
#else
int omap_ehci_hcd_init(int index, struct omap_usbhs_board_data *usbhs_pdata)
{
#endif
	int ret;
	unsigned int i, reg = 0, rev = 0;

	debug("Initializing OMAP EHCI\n");

	ret = board_usb_init(index, USB_INIT_HOST);
	if (ret < 0)
		return ret;

	/* Put the PHY in RESET */
	omap_ehci_phy_reset(1, 10);

	ret = omap_uhh_reset();
	if (ret < 0)
		return ret;

	ret = omap_ehci_tll_reset();
	if (ret)
		return ret;

	writel(OMAP_USBTLL_SYSCONFIG_ENAWAKEUP |
		OMAP_USBTLL_SYSCONFIG_SIDLEMODE |
		OMAP_USBTLL_SYSCONFIG_CACTIVITY, &usbtll->sysc);

	/* Put UHH in NoIdle/NoStandby mode */
	writel(OMAP_UHH_SYSCONFIG_VAL, &uhh->sysc);

	/* setup ULPI bypass and burst configurations */
	clrsetbits_le32(&reg, OMAP_UHH_HOSTCONFIG_INCRX_ALIGN_EN,
		(OMAP_UHH_HOSTCONFIG_INCR4_BURST_EN |
		OMAP_UHH_HOSTCONFIG_INCR8_BURST_EN |
		OMAP_UHH_HOSTCONFIG_INCR16_BURST_EN));

	rev = readl(&uhh->rev);
	if (rev == OMAP_USBHS_REV1) {
		if (is_ehci_phy_mode(usbhs_pdata->port_mode[0]))
			clrbits_le32(&reg, OMAP_UHH_HOSTCONFIG_ULPI_P1_BYPASS);
		else
			setbits_le32(&reg, OMAP_UHH_HOSTCONFIG_ULPI_P1_BYPASS);

		if (is_ehci_phy_mode(usbhs_pdata->port_mode[1]))
			clrbits_le32(&reg, OMAP_UHH_HOSTCONFIG_ULPI_P2_BYPASS);
		else
			setbits_le32(&reg, OMAP_UHH_HOSTCONFIG_ULPI_P2_BYPASS);

		if (is_ehci_phy_mode(usbhs_pdata->port_mode[2]))
			clrbits_le32(&reg, OMAP_UHH_HOSTCONFIG_ULPI_P3_BYPASS);
		else
			setbits_le32(&reg, OMAP_UHH_HOSTCONFIG_ULPI_P3_BYPASS);
	} else if (rev == OMAP_USBHS_REV2) {

		clrsetbits_le32(&reg, (OMAP_P1_MODE_CLEAR | OMAP_P2_MODE_CLEAR),
					OMAP4_UHH_HOSTCONFIG_APP_START_CLK);

		/* Clear port mode fields for PHY mode */

		if (is_ehci_hsic_mode(usbhs_pdata->port_mode[0]))
			setbits_le32(&reg, OMAP_P1_MODE_HSIC);

		if (is_ehci_hsic_mode(usbhs_pdata->port_mode[1]))
			setbits_le32(&reg, OMAP_P2_MODE_HSIC);

	} else if (rev == OMAP_USBHS_REV2_1) {

		clrsetbits_le32(&reg,
				(OMAP_P1_MODE_CLEAR |
				 OMAP_P2_MODE_CLEAR |
				 OMAP_P3_MODE_CLEAR),
				OMAP4_UHH_HOSTCONFIG_APP_START_CLK);

		/* Clear port mode fields for PHY mode */

		if (is_ehci_hsic_mode(usbhs_pdata->port_mode[0]))
			setbits_le32(&reg, OMAP_P1_MODE_HSIC);

		if (is_ehci_hsic_mode(usbhs_pdata->port_mode[1]))
			setbits_le32(&reg, OMAP_P2_MODE_HSIC);

		if (is_ehci_hsic_mode(usbhs_pdata->port_mode[2]))
			setbits_le32(&reg, OMAP_P3_MODE_HSIC);
	}

	debug("OMAP UHH_REVISION 0x%x\n", rev);
	writel(reg, &uhh->hostconfig);

	for (i = 0; i < OMAP_HS_USB_PORTS; i++)
		if (is_ehci_hsic_mode(usbhs_pdata->port_mode[i]))
			omap_usbhs_hsic_init(i);

	omap_ehci_phy_reset(0, 10);

	/*
	 * An undocumented "feature" in the OMAP3 EHCI controller,
	 * causes suspended ports to be taken out of suspend when
	 * the USBCMD.Run/Stop bit is cleared (for example when
	 * we do ehci_bus_suspend).
	 * This breaks suspend-resume if the root-hub is allowed
	 * to suspend. Writing 1 to this undocumented register bit
	 * disables this feature and restores normal behavior.
	 */
	writel(EHCI_INSNREG04_DISABLE_UNSUSPEND, &ehci->insreg04);

	for (i = 0; i < OMAP_HS_USB_PORTS; i++)
		if (is_ehci_phy_mode(usbhs_pdata->port_mode[i]))
			omap_ehci_soft_phy_reset(i);

	debug("OMAP EHCI init done\n");
	return 0;
}

#if CONFIG_IS_ENABLED(DM_USB)

static struct omap_usbhs_board_data usbhs_bdata = {
	.port_mode[0] = OMAP_USBHS_PORT_MODE_UNUSED,
	.port_mode[1] = OMAP_USBHS_PORT_MODE_UNUSED,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED,
};

static void omap_usbhs_set_mode(u8 index, const char *mode)
{
	if (!strcmp(mode, "ehci-phy"))
		usbhs_bdata.port_mode[index] = OMAP_EHCI_PORT_MODE_PHY;
	else if (!strcmp(mode, "ehci-tll"))
		usbhs_bdata.port_mode[index] = OMAP_EHCI_PORT_MODE_TLL;
	else if (!strcmp(mode, "ehci-hsic"))
		usbhs_bdata.port_mode[index] = OMAP_EHCI_PORT_MODE_HSIC;
}

static int omap_usbhs_probe(struct udevice *dev)
{
	u8 i;
	const char *mode;
	char prop[11];

	/* Go through each port portX-mode to determing phy mode */
	for (i = 0; i < OMAP_HS_USB_PORTS; i++) {
		snprintf(prop, sizeof(prop), "port%d-mode", i + 1);
		mode = dev_read_string(dev, prop);

		/* If the portX-mode exists, set the mode */
		if (mode)
			omap_usbhs_set_mode(i, mode);
	}

	return omap_ehci_hcd_init(0, &usbhs_bdata);
}

static const struct udevice_id omap_usbhs_dt_ids[] = {
	{ .compatible = "ti,usbhs-host" },
	{ }
};

U_BOOT_DRIVER(usb_omaphs_host) = {
	.name	= "usbhs-host",
	.id	= UCLASS_SIMPLE_BUS,
	.of_match = omap_usbhs_dt_ids,
	.probe	= omap_usbhs_probe,
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

struct ehci_omap_priv_data {
	struct ehci_ctrl ctrl;
	struct omap_ehci *ehci;
#ifdef CONFIG_DM_REGULATOR
	struct udevice *vbus_supply;
#endif
	enum usb_init_type init_type;
	int portnr;
	struct phy phy[OMAP_HS_USB_PORTS];
	int nports;
};

static int ehci_usb_ofdata_to_platdata(struct udevice *dev)
{
	struct usb_platdata *plat = dev_get_platdata(dev);

	plat->init_type = USB_INIT_HOST;

	return 0;
}

static int omap_ehci_probe(struct udevice *dev)
{
	struct usb_platdata *plat = dev_get_platdata(dev);
	struct ehci_omap_priv_data *priv = dev_get_priv(dev);
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;

	priv->ehci = (struct omap_ehci *)devfdt_get_addr(dev);
	priv->portnr = dev->seq;
	priv->init_type = plat->init_type;

	hccr = (struct ehci_hccr *)&priv->ehci->hccapbase;
	hcor = (struct ehci_hcor *)&priv->ehci->usbcmd;

	return ehci_register(dev, hccr, hcor, NULL, 0, USB_INIT_HOST);
}

static const struct udevice_id omap_ehci_dt_ids[] = {
	{ .compatible = "ti,ehci-omap" },
	{ }
};

U_BOOT_DRIVER(usb_omap_ehci) = {
	.name	= "omap-ehci",
	.id	= UCLASS_USB,
	.of_match = omap_ehci_dt_ids,
	.probe = omap_ehci_probe,
	.ofdata_to_platdata = ehci_usb_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct usb_platdata),
	.priv_auto_alloc_size = sizeof(struct ehci_omap_priv_data),
	.remove = ehci_deregister,
	.ops	= &ehci_usb_ops,
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

#endif
