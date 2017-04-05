/*
 * (C) Copyright 2012 Stephen Warren
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <memalign.h>
#include <phys2bus.h>
#include <asm/arch/mbox.h>

struct msg_set_power_state {
	struct bcm2835_mbox_hdr hdr;
	struct bcm2835_mbox_tag_set_power_state set_power_state;
	u32 end_tag;
};

struct msg_get_clock_rate {
	struct bcm2835_mbox_hdr hdr;
	struct bcm2835_mbox_tag_get_clock_rate get_clock_rate;
	u32 end_tag;
};

struct msg_query {
	struct bcm2835_mbox_hdr hdr;
	struct bcm2835_mbox_tag_physical_w_h physical_w_h;
	u32 end_tag;
};

int bcm2835_power_on_module(u32 module)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct msg_set_power_state, msg_pwr, 1);
	int ret;

	BCM2835_MBOX_INIT_HDR(msg_pwr);
	BCM2835_MBOX_INIT_TAG(&msg_pwr->set_power_state,
			      SET_POWER_STATE);
	msg_pwr->set_power_state.body.req.device_id = module;
	msg_pwr->set_power_state.body.req.state =
		BCM2835_MBOX_SET_POWER_STATE_REQ_ON |
		BCM2835_MBOX_SET_POWER_STATE_REQ_WAIT;

	ret = bcm2835_mbox_call_prop(BCM2835_MBOX_PROP_CHAN,
				     &msg_pwr->hdr);
	if (ret) {
		printf("bcm2835: Could not set module %u power state\n",
		       module);
		return -EIO;
	}

	return 0;
}

int bcm2835_get_mmc_clock(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct msg_get_clock_rate, msg_clk, 1);
	int ret;

	ret = bcm2835_power_on_module(BCM2835_MBOX_POWER_DEVID_SDHCI);
	if (ret)
		return ret;

	BCM2835_MBOX_INIT_HDR(msg_clk);
	BCM2835_MBOX_INIT_TAG(&msg_clk->get_clock_rate, GET_CLOCK_RATE);
	msg_clk->get_clock_rate.body.req.clock_id = BCM2835_MBOX_CLOCK_ID_EMMC;

	ret = bcm2835_mbox_call_prop(BCM2835_MBOX_PROP_CHAN, &msg_clk->hdr);
	if (ret) {
		printf("bcm2835: Could not query eMMC clock rate\n");
		return -EIO;
	}

	return msg_clk->get_clock_rate.body.resp.rate_hz;
}

int bcm2835_get_video_size(int *widthp, int *heightp)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct msg_query, msg_query, 1);
	int ret;

	BCM2835_MBOX_INIT_HDR(msg_query);
	BCM2835_MBOX_INIT_TAG_NO_REQ(&msg_query->physical_w_h,
				     GET_PHYSICAL_W_H);
	ret = bcm2835_mbox_call_prop(BCM2835_MBOX_PROP_CHAN, &msg_query->hdr);
	if (ret) {
		printf("bcm2835: Could not query display resolution\n");
		return ret;
	}
	*widthp = msg_query->physical_w_h.body.resp.width;
	*heightp = msg_query->physical_w_h.body.resp.height;

	return 0;
}
