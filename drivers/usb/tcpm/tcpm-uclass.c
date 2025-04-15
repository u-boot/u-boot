// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Collabora Ltd.
 *
 * USB Power Delivery protocol stack.
 */

#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/uclass.h>
#include <linux/err.h>
#include <usb/tcpm.h>
#include "tcpm-internal.h"

int tcpm_get_voltage(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	return port->supply_voltage;
}

int tcpm_get_current(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	return port->current_limit;
}

enum typec_orientation tcpm_get_orientation(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	switch (port->polarity) {
	case TYPEC_POLARITY_CC1:
		return TYPEC_ORIENTATION_NORMAL;
	case TYPEC_POLARITY_CC2:
		return TYPEC_ORIENTATION_REVERSE;
	default:
		return TYPEC_ORIENTATION_NONE;
	}
}

const char *tcpm_get_state(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	return tcpm_states[port->state];
}

int tcpm_get_pd_rev(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	return port->negotiated_rev;
}

enum typec_role tcpm_get_pwr_role(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	return port->pwr_role;
}

enum typec_data_role tcpm_get_data_role(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	return port->data_role;
}

bool tcpm_is_connected(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	return port->connected;
}

int tcpm_get(int index, struct udevice **devp)
{
	return uclass_get_device(UCLASS_TCPM, index, devp);
}

static int tcpm_post_bind(struct udevice *dev)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	const char *cap_str;
	ofnode node;
	int ret;

	/*
	 * USB Power Delivery (USB PD) specification requires, that communication
	 * with a sink happens within roughly 5 seconds. Otherwise the source
	 * might assume that the sink does not support USB PD. Starting to do
	 * USB PD communication after that results in a hard reset, which briefly
	 * removes any power from the USB-C port.
	 *
	 * On systems with alternative power supplies this is not an issue, but
	 * systems, which get soleley powered through their USB-C port will end
	 * up losing their power supply and doing a board level reset. The hard
	 * reset will also restart the 5 second timeout. That means a operating
	 * system initializing USB PD will put the system into a boot loop when
	 * it takes more than 5 seconds from cold boot to the operating system
	 * starting to transmit USB PD messages.
	 *
	 * The issue can be avoided by doing the initial USB PD communication
	 * in U-Boot. The operating system can then re-negotiate by doing a
	 * soft reset, which does not trigger removal of the supply voltage.
	 *
	 * Since the TCPM state machine is quite complex and depending on the
	 * remote side can take quite some time to finish, this tries to limit
	 * the automatic probing to systems probably relying on power being
	 * provided by the USB-C port(s):
	 *
	 * 1. self-powered devices won't reset when the USB-C port looses power
	 * 2. if the power is allowed to go into anything else than sink mode
	 *    it is not the only power source
	 */
	ret = drvops->get_connector_node(dev, &node);
	if (ret)
		return ret;

	if (ofnode_read_bool(node, "self-powered"))
		return 0;

	cap_str = ofnode_read_string(node, "power-role");
	if (!cap_str)
		return -EINVAL;

	if (strcmp("sink", cap_str))
		return 0;

	/* Do not auto-probe PD controller when PD is disabled */
	if (ofnode_read_bool(node, "pd-disable"))
		return 0;

	dev_info(dev, "probing Type-C port manager...");

	dev_or_flags(dev, DM_FLAG_PROBE_AFTER_BIND);

	return 0;
}

UCLASS_DRIVER(tcpm) = {
	.id		= UCLASS_TCPM,
	.name		= "tcpm",
	.per_device_plat_auto	= sizeof(struct tcpm_port),
	.post_bind	= tcpm_post_bind,
	.post_probe	= tcpm_post_probe,
};
