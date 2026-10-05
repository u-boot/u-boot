// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Nodebox v3 CPU Module Ethernet wiring.
 */

#include <dm.h>
#include <dm/device.h>
#include <dm/ofnode.h>
#include <dm/uclass.h>
#include <event.h>
#include <netdev.h>
#include <exports.h>
#include <fsl-mc/fsl_mc.h>

#include "carrier.h"

DECLARE_GLOBAL_DATA_PTR;

int board_eth_init(struct bd_info *bis)
{
	return 0;
}

#if defined(CONFIG_RESET_PHY_R)
void reset_phy(void)
{
	/*
	 * MUST come first: mc_env_boot() evaluates ${mcinitcmd}, which
	 * imxtracts dpc-${carrier} / dpl-${carrier} out of the FIT. If
	 * ${carrier} is still unset a subimage named "dpc-" and the MC
	 * firmware never starts.
	 */
	nbxv3_carrier_env_init();

	if (IS_ENABLED(CONFIG_FSL_MC_ENET))
		mc_env_boot();
}
#endif /* CONFIG_RESET_PHY_R */

/*
 * Enforce the per DPMAC u-boot,nickname DT property as the U-Boot device
 * name, so `dm tree` and `setenv ethact ...` show role based names
 * (lanconsole0, ptp0, ptp1) instead of the auto generated
 * DPMAC<n>@<phy-mode> string
 */
static int nbxv3_eth_apply_nickname(void *ctx, struct event *event)
{
	struct udevice *dev = event->data.dm.dev;
	const char *nick;

	if (device_get_uclass_id(dev) != UCLASS_ETH)
		return 0;

	nick = ofnode_read_string(dev_ofnode(dev), "u-boot,nickname");
	if (!nick)
		return 0;

	return device_set_name(dev, nick);
}

EVENT_SPY_FULL(EVT_DM_PRE_PROBE, nbxv3_eth_apply_nickname);
