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
#include <dm.h>
#include <fdt_support.h>
#include <lcd.h>
#include <mmc.h>
#include <asm/gpio.h>
#include <asm/arch/mbox.h>
#include <asm/arch/sdhci.h>
#include <asm/global_data.h>
#include <dm/platform_data/serial_pl01x.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct bcm2835_gpio_platdata gpio_platdata = {
	.base = BCM2835_GPIO_BASE,
};

U_BOOT_DEVICE(bcm2835_gpios) = {
	.name = "gpio_bcm2835",
	.platdata = &gpio_platdata,
};

static const struct pl01x_serial_platdata serial_platdata = {
	.base = 0x20201000,
	.type = TYPE_PL011,
	.clock = 3000000,
};

U_BOOT_DEVICE(bcm2835_serials) = {
	.name = "serial_pl01x",
	.platdata = &serial_platdata,
};

struct msg_get_arm_mem {
	struct bcm2835_mbox_hdr hdr;
	struct bcm2835_mbox_tag_get_arm_mem get_arm_mem;
	u32 end_tag;
};

struct msg_get_board_rev {
	struct bcm2835_mbox_hdr hdr;
	struct bcm2835_mbox_tag_get_board_rev get_board_rev;
	u32 end_tag;
};

struct msg_get_mac_address {
	struct bcm2835_mbox_hdr hdr;
	struct bcm2835_mbox_tag_get_mac_address get_mac_address;
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

/* See comments in mbox.h for data source */
static const struct {
	const char *name;
	const char *fdtfile;
	bool has_onboard_eth;
} models[] = {
	[0] = {
		"Unknown model",
		"bcm2835-rpi-other.dtb",
		false,
	},
	[BCM2835_BOARD_REV_B_I2C0_2] = {
		"Model B (no P5)",
		"bcm2835-rpi-b-i2c0.dtb",
		true,
	},
	[BCM2835_BOARD_REV_B_I2C0_3] = {
		"Model B (no P5)",
		"bcm2835-rpi-b-i2c0.dtb",
		true,
	},
	[BCM2835_BOARD_REV_B_I2C1_4] = {
		"Model B",
		"bcm2835-rpi-b.dtb",
		true,
	},
	[BCM2835_BOARD_REV_B_I2C1_5] = {
		"Model B",
		"bcm2835-rpi-b.dtb",
		true,
	},
	[BCM2835_BOARD_REV_B_I2C1_6] = {
		"Model B",
		"bcm2835-rpi-b.dtb",
		true,
	},
	[BCM2835_BOARD_REV_A_7] = {
		"Model A",
		"bcm2835-rpi-a.dtb",
		false,
	},
	[BCM2835_BOARD_REV_A_8] = {
		"Model A",
		"bcm2835-rpi-a.dtb",
		false,
	},
	[BCM2835_BOARD_REV_A_9] = {
		"Model A",
		"bcm2835-rpi-a.dtb",
		false,
	},
	[BCM2835_BOARD_REV_B_REV2_d] = {
		"Model B rev2",
		"bcm2835-rpi-b-rev2.dtb",
		true,
	},
	[BCM2835_BOARD_REV_B_REV2_e] = {
		"Model B rev2",
		"bcm2835-rpi-b-rev2.dtb",
		true,
	},
	[BCM2835_BOARD_REV_B_REV2_f] = {
		"Model B rev2",
		"bcm2835-rpi-b-rev2.dtb",
		true,
	},
	[BCM2835_BOARD_REV_B_PLUS] = {
		"Model B+",
		"bcm2835-rpi-b-plus.dtb",
		true,
	},
	[BCM2835_BOARD_REV_CM] = {
		"Compute Module",
		"bcm2835-rpi-cm.dtb",
		false,
	},
	[BCM2835_BOARD_REV_A_PLUS] = {
		"Model A+",
		"bcm2835-rpi-a-plus.dtb",
		false,
	},
};

u32 rpi_board_rev = 0;

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

static void set_fdtfile(void)
{
	const char *fdtfile;

	if (getenv("fdtfile"))
		return;

	fdtfile = models[rpi_board_rev].fdtfile;
	setenv("fdtfile", fdtfile);
}

static void set_usbethaddr(void)
{
	ALLOC_ALIGN_BUFFER(struct msg_get_mac_address, msg, 1, 16);
	int ret;

	if (!models[rpi_board_rev].has_onboard_eth)
		return;

	if (getenv("usbethaddr"))
		return;

	BCM2835_MBOX_INIT_HDR(msg);
	BCM2835_MBOX_INIT_TAG(&msg->get_mac_address, GET_MAC_ADDRESS);

	ret = bcm2835_mbox_call_prop(BCM2835_MBOX_PROP_CHAN, &msg->hdr);
	if (ret) {
		printf("bcm2835: Could not query MAC address\n");
		/* Ignore error; not critical */
		return;
	}

	eth_setenv_enetaddr("usbethaddr", msg->get_mac_address.body.resp.mac);

	return;
}

int misc_init_r(void)
{
	set_fdtfile();
	set_usbethaddr();
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

static void get_board_rev(void)
{
	ALLOC_ALIGN_BUFFER(struct msg_get_board_rev, msg, 1, 16);
	int ret;
	const char *name;

	BCM2835_MBOX_INIT_HDR(msg);
	BCM2835_MBOX_INIT_TAG(&msg->get_board_rev, GET_BOARD_REV);

	ret = bcm2835_mbox_call_prop(BCM2835_MBOX_PROP_CHAN, &msg->hdr);
	if (ret) {
		printf("bcm2835: Could not query board revision\n");
		/* Ignore error; not critical */
		return;
	}

	rpi_board_rev = msg->get_board_rev.body.resp.rev;
	if (rpi_board_rev >= ARRAY_SIZE(models)) {
		printf("RPI: Board rev %u outside known range\n",
		       rpi_board_rev);
		rpi_board_rev = 0;
	}
	if (!models[rpi_board_rev].name) {
		printf("RPI: Board rev %u unknown\n", rpi_board_rev);
		rpi_board_rev = 0;
	}

	name = models[rpi_board_rev].name;
	printf("RPI model: %s\n", name);
}

int board_init(void)
{
	get_board_rev();

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

int ft_board_setup(void *blob, bd_t *bd)
{
	/*
	 * For now, we simply always add the simplefb DT node. Later, we
	 * should be more intelligent, and e.g. only do this if no enabled DT
	 * node exists for the "real" graphics driver.
	 */
	lcd_dt_simplefb_add_node(blob);

	return 0;
}
