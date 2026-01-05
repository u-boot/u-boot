// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 * Swamil Jain <s-jain1@ti.com>
 *
 * based on the linux tidss_oldi.c, which is
 *
 * Copyright (C) 2024 - Texas Instruments Incorporated
 * Author: Aradhya Bhatia <a-bhatia1@ti.com>
 */

#include <dm.h>
#include <malloc.h>
#include <syscon.h>
#include <clk.h>
#include <regmap.h>

#include <dm/ofnode_graph.h>
#include <dm/ofnode.h>
#include <dm/device_compat.h>

#include <linux/bug.h>

#include "tidss_oldi.h"

enum tidss_oldi_pixels {
	OLDI_PIXELS_EVEN = BIT(0),
	OLDI_PIXELS_ODD = BIT(1),
};

/**
 * enum tidss_oldi_dual_link_pixels - Pixel order of an OLDI dual-link connection
 * @TIDSS_OLDI_DUAL_LINK_EVEN_ODD_PIXELS: Even pixels are expected to be generated
 *    from the first port, odd pixels from the second port
 * @TIDSS_OLDI_DUAL_LINK_ODD_EVEN_PIXELS: Odd pixels are expected to be generated
 *    from the first port, even pixels from the second port
 */
enum tidss_oldi_dual_link_pixels {
	TIDSS_OLDI_DUAL_LINK_EVEN_ODD_PIXELS = 0,
	TIDSS_OLDI_DUAL_LINK_ODD_EVEN_PIXELS = 1,
};

static int tidss_oldi_get_port_pixels_type(ofnode port_node)
{
	bool even_pixels =
		ofnode_has_property(port_node, "dual-lvds-even-pixels");
	bool odd_pixels =
		ofnode_has_property(port_node, "dual-lvds-odd-pixels");
	return (even_pixels ? OLDI_PIXELS_EVEN : 0) |
	       (odd_pixels ? OLDI_PIXELS_ODD : 0);
}

static int tidss_oldi_get_remote_pixels_type(ofnode port_node)
{
	ofnode endpoint = ofnode_null();
	int pixels_type = -EPIPE;

	ofnode_for_each_subnode(endpoint, port_node) {
		ofnode remote_port;
		int current_pt;

		if (!ofnode_name_eq(endpoint, "endpoint"))
			continue;

		remote_port = ofnode_graph_get_remote_port(endpoint);
		if (!ofnode_valid(remote_port))
			return -EPIPE;

		current_pt = tidss_oldi_get_port_pixels_type(remote_port);
		if (pixels_type < 0)
			pixels_type = current_pt;

		if (!current_pt || pixels_type != current_pt)
			return -EINVAL;
	}

	return pixels_type;
}

int tidss_oldi_get_dual_link_pixel_order(ofnode port1, ofnode port2)
{
	int remote_p1_pt, remote_p2_pt;

	if (!ofnode_valid(port1) || !ofnode_valid(port2))
		return -EINVAL;

	remote_p1_pt = tidss_oldi_get_remote_pixels_type(port1);
	if (remote_p1_pt < 0)
		return remote_p1_pt;

	remote_p2_pt = tidss_oldi_get_remote_pixels_type(port2);
	if (remote_p2_pt < 0)
		return remote_p2_pt;

	/*
	 * A valid dual-lVDS bus is found when one remote port is marked with
	 * "dual-lvds-even-pixels", and the other remote port is marked with
	 * "dual-lvds-odd-pixels", bail out if the markers are not right.
	 */
	if (remote_p1_pt + remote_p2_pt != OLDI_PIXELS_EVEN + OLDI_PIXELS_ODD)
		return -EINVAL;

	return remote_p1_pt == OLDI_PIXELS_EVEN ?
		TIDSS_OLDI_DUAL_LINK_EVEN_ODD_PIXELS :
		TIDSS_OLDI_DUAL_LINK_ODD_EVEN_PIXELS;
}

static int get_oldi_mode(ofnode oldi_tx, u32 *companion_instance)
{
	ofnode companion;
	ofnode port0, port1;
	u32 companion_reg;
	bool secondary_oldi = false;
	int pixel_order;

	/*
	 * Find if the OLDI is paired with another OLDI for combined OLDI
	 * operation (dual-lvds or clone).
	 */
	companion = ofnode_parse_phandle(oldi_tx, "ti,companion-oldi", 0);
	if (!ofnode_valid(companion))
		/*
		 * OLDI TXes in Single Link mode do not have companion
		 * OLDI TXes and, Secondary OLDI nodes don't need this
		 * information.
		 */
		return OLDI_MODE_SINGLE_LINK;

	if (ofnode_read_u32(companion, "reg", &companion_reg))
		return OLDI_MODE_UNSUPPORTED;

	if (companion_reg > (TIDSS_MAX_OLDI_TXES - 1))
		/* Invalid companion OLDI reg value. */
		return OLDI_MODE_UNSUPPORTED;

	*companion_instance = companion_reg;

	if (ofnode_read_bool(oldi_tx, "ti,secondary-oldi"))
		secondary_oldi = true;

	/*
	 * We need to work out if the sink is expecting us to function in
	 * dual-link mode. We do this by looking at the DT port nodes we are
	 * connected to, if they are marked as expecting even pixels and
	 * odd pixels than we need to enable vertical stripe output.
	 */
	port0 = ofnode_graph_get_port_by_id(oldi_tx, 1);
	port1 = ofnode_graph_get_port_by_id(companion, 1);
	pixel_order = tidss_oldi_get_dual_link_pixel_order(port0, port1);
	switch (pixel_order) {
	case -EINVAL:
		/*
		 * The dual link properties were not found in at least
		 * one of the sink nodes. Since 2 OLDI ports are present
		 * in the DT, it can be safely assumed that the required
		 * configuration is Clone Mode.
		 */
		return (secondary_oldi ? OLDI_MODE_SECONDARY_CLONE_SINGLE_LINK :
			OLDI_MODE_CLONE_SINGLE_LINK);

	case TIDSS_OLDI_DUAL_LINK_ODD_EVEN_PIXELS:
		/*
		 * Primary OLDI can only support "ODD" pixels. So, from its
		 * perspective, the pixel order has to be ODD-EVEN.
		 */
		return (secondary_oldi ? OLDI_MODE_UNSUPPORTED :
			OLDI_MODE_DUAL_LINK);

	/* Unsupported OLDI Modes */
	case TIDSS_OLDI_DUAL_LINK_EVEN_ODD_PIXELS:
		/*
		 * Secondary OLDI can only support "EVEN" pixels. So, from its
		 * perspective, the pixel order has to be EVEN-ODD.
		 */
		return (secondary_oldi ? OLDI_MODE_SECONDARY_DUAL_LINK :
			OLDI_MODE_UNSUPPORTED);

	default:
		return OLDI_MODE_UNSUPPORTED;
	}
}

static int get_parent_dss_vp(ofnode oldi_tx, u32 *parent_vp)
{
	ofnode ep, dss_port;
	int ret;

	ep = ofnode_graph_get_endpoint_by_regs(oldi_tx, 0, -1);
	if (ofnode_valid(ep)) {
		dss_port = ofnode_graph_get_remote_port(ep);
		if (!ofnode_valid(dss_port))
			ret = -ENODEV;

		ret = ofnode_read_u32(dss_port, "reg", parent_vp);
		if (ret)
			return -ENODEV;
		return 0;
	}

	return -ENODEV;
}

static int tidss_init_oldi_io_ctrl(struct udevice *dev, struct tidss_oldi *tidss_oldi)
{
	struct udevice *syscon;
	struct regmap *regmap = NULL;
	int ret = 0;

	ret = uclass_get_device_by_phandle(UCLASS_SYSCON, dev, "ti,am65x-oldi-io-ctrl",
					   &syscon);
	if (ret) {
		debug("unable to find ti,am65x-oldi-io-ctrl syscon device (%d)\n", ret);
		return ret;
	}

	/* get grf-reg base address */
	regmap = syscon_get_regmap(syscon);
	if (!regmap) {
		debug("unable to find rockchip grf regmap\n");
		return -ENODEV;
	}
	tidss_oldi->io_ctrl = regmap;
	return 0;
}

int tidss_oldi_init(struct udevice *dev)
{
	struct tidss_drv_priv *priv = dev_get_priv(dev);
	u32 parent_vp, oldi_instance, companion_instance;
	int ret, tidss_oldi_panel_count = 0;
	enum tidss_oldi_link_type link_type;
	struct tidss_oldi *tidss_oldi;
	struct clk *serial;
	ofnode child;
	ofnode oldi_parent = ofnode_find_subnode(dev_ofnode(dev), "oldi-transmitters");

	if (!ofnode_valid(oldi_parent))
		/* Return gracefully */
		return 0;

	ofnode_for_each_subnode(child, oldi_parent) {
		priv->oldis[tidss_oldi_panel_count] = NULL;

		ret = get_parent_dss_vp(child, &parent_vp);
		if (ret == -ENODEV) {
			/*
			 * ENODEV means that this particular OLDI node
			 * is not connected with the DSS, which is not
			 * a harmful case. There could be another OLDI
			 * which may still be connected.
			 * Continue to search for that.
			 */
			ret = 0;
			continue;
		}

		ret = ofnode_read_u32(child, "reg", &oldi_instance);
		if (ret) {
			ret = -ENODEV;
			break;
		}

		link_type = get_oldi_mode(child, &companion_instance);
		if (link_type == OLDI_MODE_UNSUPPORTED) {
			dev_warn(dev, "OLDI%u: Unsupported OLDI connection.\n", oldi_instance);
			ret = OLDI_MODE_UNSUPPORTED;
			/* Return gracefully, no supported OLDI panel found */
			goto return_err;
		} else if ((link_type == OLDI_MODE_SECONDARY_CLONE_SINGLE_LINK) ||
			(link_type == OLDI_MODE_CLONE_SINGLE_LINK)) {
		/*
		 * The OLDI driver cannot support OLDI clone mode
		 * properly at present.
		 * The clone mode requires 2 working encoder-bridge
		 * pipelines, generating from the same crtc. The DRM
		 * framework does not support this at present. If
		 * there were to be, say, 2 OLDI sink bridges each
		 * connected to an OLDI TXes, they couldn't both be
		 * supported simultaneously.
		 * This driver still has some code pertaining to OLDI
		 * clone mode configuration in DSS hardware for future,
		 * when there is a better infrastructure in the DRM
		 * framework to support 2 encoder-bridge pipelines
		 * simultaneously.
		 * Till that time, this driver shall error out if it
		 * detects a clone mode configuration.
		 */
			ret = -EOPNOTSUPP;
			goto return_err;
		} else if (link_type == OLDI_MODE_SECONDARY_DUAL_LINK) {
		/*
		 * This is the secondary OLDI node, which serves as a
		 * companion to the primary OLDI, when it is configured
		 * for the dual-link mode. Since the primary OLDI will
		 * be a part of bridge chain, no need to put this one
		 * too. Continue onto the next OLDI node.
		 */
			ret = 0;
			continue;
		}

		serial = malloc(sizeof(struct clk));
		ret = clk_get_by_name_nodev(child, "serial", serial);
		if (ret) {
			dev_err(dev, "video port %d clock enable error %d\n", parent_vp, ret);
			free(serial);
			goto return_err;
		}

		tidss_oldi = malloc(sizeof(struct tidss_oldi));
		ret = tidss_init_oldi_io_ctrl(dev, tidss_oldi);
		if (ret) {
			debug("tidss could not initialize oldi_io_ctrl\n");
			free(serial);
			free(tidss_oldi);
			goto return_err;
		}

		tidss_oldi->dev = dev;
		tidss_oldi->parent_vp = parent_vp;
		tidss_oldi->oldi_instance = oldi_instance;
		tidss_oldi->companion_instance = companion_instance;
		tidss_oldi->link_type = link_type;
		tidss_oldi->serial = serial;
		priv->oldis[tidss_oldi_panel_count] = tidss_oldi;
		priv->oldi_mode = link_type;
		tidss_oldi_panel_count++;
	}
	priv->num_oldis = tidss_oldi_panel_count;
	ret = 0;
return_err:
	return ret;
}

void dss_oldi_tx_power(struct tidss_drv_priv *priv, bool power)
{
	u32 val;

	if (WARN_ON(!priv->oldis[0]->io_ctrl))
		return;

	if (priv->feat->subrev == DSS_AM625) {
		if (power) {
			switch (priv->oldi_mode) {
			case OLDI_MODE_SINGLE_LINK:
				/* Power down OLDI TX 1 */
				val = OLDI1_PWRDN_TX;
				break;
			case OLDI_MODE_DUAL_LINK:
				/* No Power down */
				val = 0;
			break;
			default:
				/* Power down both the OLDI TXes */
				val = OLDI_BANDGAP_PWR | OLDI0_PWRDN_TX | OLDI1_PWRDN_TX;
				break;
			}
		} else {
			val = OLDI_BANDGAP_PWR | OLDI0_PWRDN_TX | OLDI1_PWRDN_TX;
		}
		regmap_update_bits(priv->oldis[0]->io_ctrl, OLDI_PD_CTRL,
				   OLDI_BANDGAP_PWR | OLDI0_PWRDN_TX | OLDI1_PWRDN_TX, val);
	}
}
