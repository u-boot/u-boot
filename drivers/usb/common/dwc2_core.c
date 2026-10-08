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

static unsigned int dwc2_op_mode(struct dwc2_core_regs *regs)
{
	return FIELD_GET(GHWCFG2_OP_MODE_MASK, readl(&regs->global_regs.ghwcfg2));
}

/* Returns true if the controller is capable of DRD. */
static bool dwc2_hw_is_otg(struct dwc2_core_regs *regs)
{
	unsigned int op_mode = dwc2_op_mode(regs);

	return (op_mode == GHWCFG2_OP_MODE_HNP_SRP_CAPABLE) ||
		(op_mode == GHWCFG2_OP_MODE_SRP_ONLY_CAPABLE) ||
		(op_mode == GHWCFG2_OP_MODE_NO_HNP_SRP_CAPABLE);
}

/*
 * dwc2_iddig_filter_enabled() - Returns true if the IDDIG debounce
 * filter is enabled.
 */
bool dwc2_iddig_filter_enabled(struct dwc2_core_regs *regs)
{
	u32 snpsid;

	if (!dwc2_hw_is_otg(regs))
		return false;

	/* Check if core configuration includes the IDDIG filter. */
	if (!FIELD_GET(GHWCFG4_IDDIG_FILT_EN, readl(&regs->global_regs.ghwcfg4)))
		return false;

	/*
	 * Check if the IDDIG debounce filter is bypassed. Available
	 * in core version >= 3.10a.
	 */
	snpsid = readl(&regs->global_regs.gsnpsid);
	if (FIELD_GET(GSNPSID_VER_MASK, snpsid) >= 0x310a) {
		u32 gotgctl = readl(&regs->global_regs.gotgctl);

		if (gotgctl & GOTGCTL_DBNCE_FLTR_BYPASS)
			return false;
	}

	return true;
}
