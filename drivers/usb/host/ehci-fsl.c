/*
 * (C) Copyright 2009, 2011 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2008, Excito Elektronik i Sk=E5ne AB
 *
 * Author: Tor Krill tor@excito.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <pci.h>
#include <usb.h>
#include <asm/io.h>
#include <usb/ehci-fsl.h>
#include <hwconfig.h>
#include <fsl_usb.h>
#include <fdt_support.h>

#include "ehci.h"

#ifndef CONFIG_USB_MAX_CONTROLLER_COUNT
#define CONFIG_USB_MAX_CONTROLLER_COUNT 1
#endif

static void set_txfifothresh(struct usb_ehci *, u32);

/* Check USB PHY clock valid */
static int usb_phy_clk_valid(struct usb_ehci *ehci)
{
	if (!((in_be32(&ehci->control) & PHY_CLK_VALID) ||
			in_be32(&ehci->prictrl))) {
		printf("USB PHY clock invalid!\n");
		return 0;
	} else {
		return 1;
	}
}

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 *
 * Excerpts from linux ehci fsl driver.
 */
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	struct usb_ehci *ehci = NULL;
	const char *phy_type = NULL;
	size_t len;
	char current_usb_controller[5];
#ifdef CONFIG_SYS_FSL_USB_INTERNAL_UTMI_PHY
	char usb_phy[5];

	usb_phy[0] = '\0';
#endif
	if (has_erratum_a007075()) {
		/*
		 * A 5ms delay is needed after applying soft-reset to the
		 * controller to let external ULPI phy come out of reset.
		 * This delay needs to be added before re-initializing
		 * the controller after soft-resetting completes
		 */
		mdelay(5);
	}
	memset(current_usb_controller, '\0', 5);
	snprintf(current_usb_controller, 4, "usb%d", index+1);

	switch (index) {
	case 0:
		ehci = (struct usb_ehci *)CONFIG_SYS_FSL_USB1_ADDR;
		break;
	case 1:
		ehci = (struct usb_ehci *)CONFIG_SYS_FSL_USB2_ADDR;
		break;
	default:
		printf("ERROR: wrong controller index!!\n");
		return -EINVAL;
	};

	*hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
	*hcor = (struct ehci_hcor *)((uint32_t) *hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	/* Set to Host mode */
	setbits_le32(&ehci->usbmode, CM_HOST);

	out_be32(&ehci->snoop1, SNOOP_SIZE_2GB);
	out_be32(&ehci->snoop2, 0x80000000 | SNOOP_SIZE_2GB);

	/* Init phy */
	if (hwconfig_sub(current_usb_controller, "phy_type"))
		phy_type = hwconfig_subarg(current_usb_controller,
				"phy_type", &len);
	else
		phy_type = getenv("usb_phy_type");

	if (!phy_type) {
#ifdef CONFIG_SYS_FSL_USB_INTERNAL_UTMI_PHY
		/* if none specified assume internal UTMI */
		strcpy(usb_phy, "utmi");
		phy_type = usb_phy;
#else
		printf("WARNING: USB phy type not defined !!\n");
		return -1;
#endif
	}

	if (!strncmp(phy_type, "utmi", 4)) {
#if defined(CONFIG_SYS_FSL_USB_INTERNAL_UTMI_PHY)
		clrsetbits_be32(&ehci->control, CONTROL_REGISTER_W1C_MASK,
				PHY_CLK_SEL_UTMI);
		clrsetbits_be32(&ehci->control, CONTROL_REGISTER_W1C_MASK,
				UTMI_PHY_EN);
		udelay(1000); /* delay required for PHY Clk to appear */
#endif
		out_le32(&(*hcor)->or_portsc[0], PORT_PTS_UTMI);
		clrsetbits_be32(&ehci->control, CONTROL_REGISTER_W1C_MASK,
				USB_EN);
	} else {
		clrsetbits_be32(&ehci->control, CONTROL_REGISTER_W1C_MASK,
				PHY_CLK_SEL_ULPI);
		clrsetbits_be32(&ehci->control, UTMI_PHY_EN |
				CONTROL_REGISTER_W1C_MASK, USB_EN);
		udelay(1000); /* delay required for PHY Clk to appear */
		if (!usb_phy_clk_valid(ehci))
			return -EINVAL;
		out_le32(&(*hcor)->or_portsc[0], PORT_PTS_ULPI);
	}

	out_be32(&ehci->prictrl, 0x0000000c);
	out_be32(&ehci->age_cnt_limit, 0x00000040);
	out_be32(&ehci->sictrl, 0x00000001);

	in_le32(&ehci->usbmode);

	if (has_erratum_a007798())
		set_txfifothresh(ehci, TXFIFOTHRESH);

	if (has_erratum_a004477()) {
		/*
		 * When reset is issued while any ULPI transaction is ongoing
		 * then it may result to corruption of ULPI Function Control
		 * Register which eventually causes phy clock to enter low
		 * power mode which stops the clock. Thus delay is required
		 * before reset to let ongoing ULPI transaction complete.
		 */
		udelay(1);
	}
	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
	return 0;
}

/*
 * Setting the value of TXFIFO_THRESH field in TXFILLTUNING register
 * to counter DDR latencies in writing data into Tx buffer.
 * This prevents Tx buffer from getting underrun
 */
static void set_txfifothresh(struct usb_ehci *ehci, u32 txfifo_thresh)
{
	u32 cmd;
	cmd = ehci_readl(&ehci->txfilltuning);
	cmd &= ~TXFIFO_THRESH_MASK;
	cmd |= TXFIFO_THRESH(txfifo_thresh);
	ehci_writel(&ehci->txfilltuning, cmd);
}

#if defined(CONFIG_HAS_FSL_DR_USB) || defined(CONFIG_HAS_FSL_MPH_USB)
static int fdt_fixup_usb_mode_phy_type(void *blob, const char *mode,
				       const char *phy_type, int start_offset)
{
	const char *compat_dr = "fsl-usb2-dr";
	const char *compat_mph = "fsl-usb2-mph";
	const char *prop_mode = "dr_mode";
	const char *prop_type = "phy_type";
	const char *node_type = NULL;
	int node_offset;
	int err;

	node_offset = fdt_node_offset_by_compatible(blob,
						    start_offset, compat_mph);
	if (node_offset < 0) {
		node_offset = fdt_node_offset_by_compatible(blob,
							    start_offset,
							    compat_dr);
		if (node_offset < 0) {
			printf("WARNING: could not find compatible node: %s",
			       fdt_strerror(node_offset));
			return -1;
		}
		node_type = compat_dr;
	} else {
		node_type = compat_mph;
	}

	if (mode) {
		err = fdt_setprop(blob, node_offset, prop_mode, mode,
				  strlen(mode) + 1);
		if (err < 0)
			printf("WARNING: could not set %s for %s: %s.\n",
			       prop_mode, node_type, fdt_strerror(err));
	}

	if (phy_type) {
		err = fdt_setprop(blob, node_offset, prop_type, phy_type,
				  strlen(phy_type) + 1);
		if (err < 0)
			printf("WARNING: could not set %s for %s: %s.\n",
			       prop_type, node_type, fdt_strerror(err));
	}

	return node_offset;
}

static const char *fdt_usb_get_node_type(void *blob, int start_offset,
					 int *node_offset)
{
	const char *compat_dr = "fsl-usb2-dr";
	const char *compat_mph = "fsl-usb2-mph";
	const char *node_type = NULL;

	*node_offset = fdt_node_offset_by_compatible(blob, start_offset,
						     compat_mph);
	if (*node_offset < 0) {
		*node_offset = fdt_node_offset_by_compatible(blob,
							     start_offset,
							     compat_dr);
		if (*node_offset < 0) {
			printf("ERROR: could not find compatible node: %s\n",
			       fdt_strerror(*node_offset));
		} else {
			node_type = compat_dr;
		}
	} else {
		node_type = compat_mph;
	}

	return node_type;
}

static int fdt_fixup_usb_erratum(void *blob, const char *prop_erratum,
				 int start_offset)
{
	int node_offset, err;
	const char *node_type = NULL;

	node_type = fdt_usb_get_node_type(blob, start_offset, &node_offset);
	if (!node_type)
		return -1;

	err = fdt_setprop(blob, node_offset, prop_erratum, NULL, 0);
	if (err < 0) {
		printf("ERROR: could not set %s for %s: %s.\n",
		       prop_erratum, node_type, fdt_strerror(err));
	}

	return node_offset;
}

void fdt_fixup_dr_usb(void *blob, bd_t *bd)
{
	static const char * const modes[] = { "host", "peripheral", "otg" };
	static const char * const phys[] = { "ulpi", "utmi", "utmi_dual" };
	int usb_erratum_a006261_off = -1;
	int usb_erratum_a007075_off = -1;
	int usb_erratum_a007792_off = -1;
	int usb_erratum_a005697_off = -1;
	int usb_mode_off = -1;
	int usb_phy_off = -1;
	char str[5];
	int i, j;

	for (i = 1; i <= CONFIG_USB_MAX_CONTROLLER_COUNT; i++) {
		const char *dr_mode_type = NULL;
		const char *dr_phy_type = NULL;
		int mode_idx = -1, phy_idx = -1;

		snprintf(str, 5, "%s%d", "usb", i);
		if (hwconfig(str)) {
			for (j = 0; j < ARRAY_SIZE(modes); j++) {
				if (hwconfig_subarg_cmp(str, "dr_mode",
							modes[j])) {
					mode_idx = j;
					break;
				}
			}

			for (j = 0; j < ARRAY_SIZE(phys); j++) {
				if (hwconfig_subarg_cmp(str, "phy_type",
							phys[j])) {
					phy_idx = j;
					break;
				}
			}

			if (mode_idx < 0 && phy_idx < 0) {
				printf("WARNING: invalid phy or mode\n");
				return;
			}

			if (mode_idx > -1)
				dr_mode_type = modes[mode_idx];

			if (phy_idx > -1)
				dr_phy_type = phys[phy_idx];
		}

		if (has_dual_phy())
			dr_phy_type = phys[2];

		usb_mode_off = fdt_fixup_usb_mode_phy_type(blob,
							   dr_mode_type, NULL,
							   usb_mode_off);

		if (usb_mode_off < 0)
			return;

		usb_phy_off = fdt_fixup_usb_mode_phy_type(blob,
							  NULL, dr_phy_type,
							  usb_phy_off);

		if (usb_phy_off < 0)
			return;

		if (has_erratum_a006261()) {
			usb_erratum_a006261_off =  fdt_fixup_usb_erratum
						   (blob,
						    "fsl,usb-erratum-a006261",
						    usb_erratum_a006261_off);
			if (usb_erratum_a006261_off < 0)
				return;
		}

		if (has_erratum_a007075()) {
			usb_erratum_a007075_off =  fdt_fixup_usb_erratum
						   (blob,
						    "fsl,usb-erratum-a007075",
						    usb_erratum_a007075_off);
			if (usb_erratum_a007075_off < 0)
				return;
		}

		if (has_erratum_a007792()) {
			usb_erratum_a007792_off =  fdt_fixup_usb_erratum
						   (blob,
						    "fsl,usb-erratum-a007792",
						    usb_erratum_a007792_off);
			if (usb_erratum_a007792_off < 0)
				return;
		}
		if (has_erratum_a005697()) {
			usb_erratum_a005697_off =  fdt_fixup_usb_erratum
						   (blob,
						    "fsl,usb-erratum-a005697",
						    usb_erratum_a005697_off);
			if (usb_erratum_a005697_off < 0)
				return;
		}
	}
}
#endif
