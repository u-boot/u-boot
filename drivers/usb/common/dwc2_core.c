// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024-2025, Kongyang Liu <seashell11234455@gmail.com>
 */

#include <linux/bitfield.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <wait_bit.h>

#include "dwc2_core.h"

int dwc2_core_reset(struct dwc2_core_regs *regs)
{
	u32 snpsid;
	int ret;
	bool host_mode = false;

	if (!(readl(&regs->global_regs.gotgctl) & GOTGCTL_CONID_B) ||
	    (readl(&regs->global_regs.gusbcfg) & GUSBCFG_FORCEHOSTMODE))
		host_mode = true;

	/* Core Soft Reset */
	snpsid = readl(&regs->global_regs.gsnpsid);
	writel(GRSTCTL_CSFTRST, &regs->global_regs.grstctl);
	if (FIELD_GET(GSNPSID_VER_MASK, snpsid) < 0x420a) {
		ret = wait_for_bit_le32(&regs->global_regs.grstctl, GRSTCTL_CSFTRST,
					false, 1000, false);
		if (ret) {
			log_warning("%s: Waiting for GRSTCTL_CSFTRST timeout\n", __func__);
			return ret;
		}
	} else {
		ret = wait_for_bit_le32(&regs->global_regs.grstctl, GRSTCTL_CSFTRST_DONE,
					true, 1000, false);
		if (ret) {
			log_warning("%s: Waiting for GRSTCTL_CSFTRST_DONE timeout\n", __func__);
			return ret;
		}
		clrsetbits_le32(&regs->global_regs.grstctl, GRSTCTL_CSFTRST, GRSTCTL_CSFTRST_DONE);
	}

	/* Wait for AHB master IDLE state. */
	ret = wait_for_bit_le32(&regs->global_regs.grstctl, GRSTCTL_AHBIDLE,
				true, 1000, false);
	if (ret) {
		log_warning("%s: Waiting for GRSTCTL_AHBIDLE timeout\n", __func__);
		return ret;
	}

	if (host_mode) {
		ret = wait_for_bit_le32(&regs->global_regs.gintsts, GINTSTS_CURMODE_HOST,
					host_mode, 1000, false);
		if (ret) {
			log_warning("%s: Waiting for GINTSTS_CURMODE_HOST timeout\n", __func__);
			return ret;
		}
	}

	return 0;
}

int dwc2_flush_tx_fifo(struct dwc2_core_regs *regs, const int num)
{
	int ret;

	log_debug("Flush Tx FIFO %d\n", num);

	/* Wait for AHB master IDLE state */
	ret = wait_for_bit_le32(&regs->global_regs.grstctl, GRSTCTL_AHBIDLE, true, 1000, false);
	if (ret) {
		log_warning("%s: Waiting for GRSTCTL_AHBIDLE timeout\n", __func__);
		return ret;
	}

	writel(GRSTCTL_TXFFLSH | FIELD_PREP(GRSTCTL_TXFNUM_MASK, num), &regs->global_regs.grstctl);

	ret = wait_for_bit_le32(&regs->global_regs.grstctl, GRSTCTL_TXFFLSH, false, 1000, false);
	if (ret) {
		log_warning("%s: Waiting for GRSTCTL_TXFFLSH timeout\n", __func__);
		return ret;
	}

	/*
	 * Wait for at least 3 PHY clocks.
	 *
	 * The PHY clock frequency can be configured to 6/30/48/60 MHz
	 * based on the speed mode. A fixed delay of 1us ensures that the
	 * wait time is sufficient even at the lowest PHY clock frequency
	 * (6 MHz), where 1us corresponds to twice the duration of 3 PHY
	 * clocks.
	 */
	udelay(1);

	return 0;
}

int dwc2_flush_rx_fifo(struct dwc2_core_regs *regs)
{
	int ret;

	log_debug("Flush Rx FIFO\n");

	/* Wait for AHB master IDLE state */
	ret = wait_for_bit_le32(&regs->global_regs.grstctl, GRSTCTL_AHBIDLE, true, 1000, false);
	if (ret) {
		log_warning("%s: Waiting for GRSTCTL_AHBIDLE timeout\n", __func__);
		return ret;
	}

	writel(GRSTCTL_RXFFLSH, &regs->global_regs.grstctl);

	ret = wait_for_bit_le32(&regs->global_regs.grstctl, GRSTCTL_RXFFLSH, false, 1000, false);
	if (ret) {
		log_warning("%s: Waiting for GRSTCTL_RXFFLSH timeout\n", __func__);
		return ret;
	}

	/*
	 * Wait for at least 3 PHY clocks.
	 *
	 * The PHY clock frequency can be configured to 6/30/48/60 MHz
	 * based on the speed mode. A fixed delay of 1us ensures that the
	 * wait time is sufficient even at the lowest PHY clock frequency
	 * (6 MHz), where 1us corresponds to twice the duration of 3 PHY
	 * clocks.
	 */
	udelay(1);

	return 0;
}
