/*
 * (C) Copyright 2012-2013 Stephen Warren
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <config.h>
#include <fdt_support.h>
#include <lcd.h>
#include <mmc.h>
#include <asm/arch/mbox.h>
#include <asm/arch/sdhci.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

struct msg_get_arm_mem {
	struct bcm2835_mbox_hdr hdr;
	struct bcm2835_mbox_tag_get_arm_mem get_arm_mem;
	u32 end_tag;
};

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

int dram_init(void)
{
	ALLOC_ALIGN_BUFFER(struct msg_get_arm_mem, msg, 1, 16);
	int ret;

	BCM2835_MBOX_INIT_HDR(msg);
	BCM2835_MBOX_INIT_TAG(&msg->get_arm_mem, GET_ARM_MEMORY);

	ret = bcm2835_mbox_call_prop(BCM2835_MBOX_PROP_CHAN, &msg->hdr);
	if (ret) {
		printf("bcm2835: Could not query ARM memory size\n");
		return -1;
	}

	gd->ram_size = msg->get_arm_mem.body.resp.mem_size;

	return 0;
}

static int power_on_module(u32 module)
{
	ALLOC_ALIGN_BUFFER(struct msg_set_power_state, msg_pwr, 1, 16);
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
		return -1;
	}

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = 0x100;

	return power_on_module(BCM2835_MBOX_POWER_DEVID_USB_HCD);
}

int board_mmc_init(bd_t *bis)
{
	ALLOC_ALIGN_BUFFER(struct msg_get_clock_rate, msg_clk, 1, 16);
	int ret;

	power_on_module(BCM2835_MBOX_POWER_DEVID_SDHCI);

	BCM2835_MBOX_INIT_HDR(msg_clk);
	BCM2835_MBOX_INIT_TAG(&msg_clk->get_clock_rate, GET_CLOCK_RATE);
	msg_clk->get_clock_rate.body.req.clock_id = BCM2835_MBOX_CLOCK_ID_EMMC;

	ret = bcm2835_mbox_call_prop(BCM2835_MBOX_PROP_CHAN, &msg_clk->hdr);
	if (ret) {
		printf("bcm2835: Could not query eMMC clock rate\n");
		return -1;
	}

	return bcm2835_sdhci_init(BCM2835_SDHCI_BASE,
				  msg_clk->get_clock_rate.body.resp.rate_hz);
}

void ft_board_setup(void *blob, bd_t *bd)
{
	/*
	 * For now, we simply always add the simplefb DT node. Later, we
	 * should be more intelligent, and e.g. only do this if no enabled DT
	 * node exists for the "real" graphics driver.
	 */
	lcd_dt_simplefb_add_node(blob);
}
