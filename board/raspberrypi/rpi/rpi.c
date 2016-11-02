/*
 * (C) Copyright 2012-2016 Stephen Warren
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <inttypes.h>
#include <config.h>
#include <dm.h>
#include <efi_loader.h>
#include <fdt_support.h>
#include <fdt_simplefb.h>
#include <lcd.h>
#include <memalign.h>
#include <mmc.h>
#include <asm/gpio.h>
#include <asm/arch/mbox.h>
#include <asm/arch/sdhci.h>
#include <asm/global_data.h>
#include <dm/platform_data/serial_bcm283x_mu.h>
#ifdef CONFIG_ARM64
#include <asm/armv8/mmu.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* From lowlevel_init.S */
extern unsigned long fw_dtb_pointer;


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

struct msg_get_board_serial {
	struct bcm2835_mbox_hdr hdr;
	struct bcm2835_mbox_tag_get_board_serial get_board_serial;
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

/*
 * http://raspberryalphaomega.org.uk/2013/02/06/automatic-raspberry-pi-board-revision-detection-model-a-b1-and-b2/
 * http://www.raspberrypi.org/forums/viewtopic.php?f=63&t=32733
 * http://git.drogon.net/?p=wiringPi;a=blob;f=wiringPi/wiringPi.c;h=503151f61014418b9c42f4476a6086f75cd4e64b;hb=refs/heads/master#l922
 *
 * In http://lists.denx.de/pipermail/u-boot/2016-January/243752.html
 * ("[U-Boot] [PATCH] rpi: fix up Model B entries") Dom Cobley at the RPi
 * Foundation stated that the following source was accurate:
 * https://github.com/AndrewFromMelbourne/raspberry_pi_revision
 */
struct rpi_model {
	const char *name;
	const char *fdtfile;
	bool has_onboard_eth;
};

static const struct rpi_model rpi_model_unknown = {
	"Unknown model",
	"bcm283x-rpi-other.dtb",
	false,
};

static const struct rpi_model rpi_models_new_scheme[] = {
	[0x4] = {
		"2 Model B",
		"bcm2836-rpi-2-b.dtb",
		true,
	},
	[0x8] = {
		"3 Model B",
		"bcm2837-rpi-3-b.dtb",
		true,
	},
	[0x9] = {
		"Zero",
		"bcm2835-rpi-zero.dtb",
		false,
	},
};

static const struct rpi_model rpi_models_old_scheme[] = {
	[0x2] = {
		"Model B",
		"bcm2835-rpi-b.dtb",
		true,
	},
	[0x3] = {
		"Model B",
		"bcm2835-rpi-b.dtb",
		true,
	},
	[0x4] = {
		"Model B rev2",
		"bcm2835-rpi-b-rev2.dtb",
		true,
	},
	[0x5] = {
		"Model B rev2",
		"bcm2835-rpi-b-rev2.dtb",
		true,
	},
	[0x6] = {
		"Model B rev2",
		"bcm2835-rpi-b-rev2.dtb",
		true,
	},
	[0x7] = {
		"Model A",
		"bcm2835-rpi-a.dtb",
		false,
	},
	[0x8] = {
		"Model A",
		"bcm2835-rpi-a.dtb",
		false,
	},
	[0x9] = {
		"Model A",
		"bcm2835-rpi-a.dtb",
		false,
	},
	[0xd] = {
		"Model B rev2",
		"bcm2835-rpi-b-rev2.dtb",
		true,
	},
	[0xe] = {
		"Model B rev2",
		"bcm2835-rpi-b-rev2.dtb",
		true,
	},
	[0xf] = {
		"Model B rev2",
		"bcm2835-rpi-b-rev2.dtb",
		true,
	},
	[0x10] = {
		"Model B+",
		"bcm2835-rpi-b-plus.dtb",
		true,
	},
	[0x11] = {
		"Compute Module",
		"bcm2835-rpi-cm.dtb",
		false,
	},
	[0x12] = {
		"Model A+",
		"bcm2835-rpi-a-plus.dtb",
		false,
	},
	[0x13] = {
		"Model B+",
		"bcm2835-rpi-b-plus.dtb",
		true,
	},
	[0x14] = {
		"Compute Module",
		"bcm2835-rpi-cm.dtb",
		false,
	},
	[0x15] = {
		"Model A+",
		"bcm2835-rpi-a-plus.dtb",
		false,
	},
};

static uint32_t revision;
static uint32_t rev_scheme;
static uint32_t rev_type;
static const struct rpi_model *model;

#ifdef CONFIG_ARM64
static struct mm_region bcm2837_mem_map[] = {
	{
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0x3f000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		.virt = 0x3f000000UL,
		.phys = 0x3f000000UL,
		.size = 0x01000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = bcm2837_mem_map;
#endif

int dram_init(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct msg_get_arm_mem, msg, 1);
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

	fdtfile = model->fdtfile;
	setenv("fdtfile", fdtfile);
}

/*
 * If the firmware provided a valid FDT at boot time, let's expose it in
 * ${fdt_addr} so it may be passed unmodified to the kernel.
 */
static void set_fdt_addr(void)
{
	if (getenv("fdt_addr"))
		return;

	if (fdt_magic(fw_dtb_pointer) != FDT_MAGIC)
		return;

	setenv_hex("fdt_addr", fw_dtb_pointer);
}

/*
 * Prevent relocation from stomping on a firmware provided FDT blob.
 */
unsigned long board_get_usable_ram_top(unsigned long total_size)
{
	if ((gd->ram_top - fw_dtb_pointer) > SZ_64M)
		return gd->ram_top;
	return fw_dtb_pointer & ~0xffff;
}

static void set_usbethaddr(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct msg_get_mac_address, msg, 1);
	int ret;

	if (!model->has_onboard_eth)
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

	if (!getenv("ethaddr"))
		setenv("ethaddr", getenv("usbethaddr"));

	return;
}

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
static void set_board_info(void)
{
	char s[11];

	snprintf(s, sizeof(s), "0x%X", revision);
	setenv("board_revision", s);
	snprintf(s, sizeof(s), "%d", rev_scheme);
	setenv("board_rev_scheme", s);
	/* Can't rename this to board_rev_type since it's an ABI for scripts */
	snprintf(s, sizeof(s), "0x%X", rev_type);
	setenv("board_rev", s);
	setenv("board_name", model->name);
}
#endif /* CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG */

static void set_serial_number(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct msg_get_board_serial, msg, 1);
	int ret;
	char serial_string[17] = { 0 };

	if (getenv("serial#"))
		return;

	BCM2835_MBOX_INIT_HDR(msg);
	BCM2835_MBOX_INIT_TAG_NO_REQ(&msg->get_board_serial, GET_BOARD_SERIAL);

	ret = bcm2835_mbox_call_prop(BCM2835_MBOX_PROP_CHAN, &msg->hdr);
	if (ret) {
		printf("bcm2835: Could not query board serial\n");
		/* Ignore error; not critical */
		return;
	}

	snprintf(serial_string, sizeof(serial_string), "%016" PRIx64,
		 msg->get_board_serial.body.resp.serial);
	setenv("serial#", serial_string);
}

int misc_init_r(void)
{
	set_fdt_addr();
	set_fdtfile();
	set_usbethaddr();
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	set_board_info();
#endif
	set_serial_number();

	return 0;
}

static int power_on_module(u32 module)
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
		return -1;
	}

	return 0;
}

static void get_board_rev(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct msg_get_board_rev, msg, 1);
	int ret;
	const struct rpi_model *models;
	uint32_t models_count;

	BCM2835_MBOX_INIT_HDR(msg);
	BCM2835_MBOX_INIT_TAG(&msg->get_board_rev, GET_BOARD_REV);

	ret = bcm2835_mbox_call_prop(BCM2835_MBOX_PROP_CHAN, &msg->hdr);
	if (ret) {
		printf("bcm2835: Could not query board revision\n");
		/* Ignore error; not critical */
		return;
	}

	/*
	 * For details of old-vs-new scheme, see:
	 * https://github.com/pimoroni/RPi.version/blob/master/RPi/version.py
	 * http://www.raspberrypi.org/forums/viewtopic.php?f=63&t=99293&p=690282
	 * (a few posts down)
	 *
	 * For the RPi 1, bit 24 is the "warranty bit", so we mask off just the
	 * lower byte to use as the board rev:
	 * http://www.raspberrypi.org/forums/viewtopic.php?f=63&t=98367&start=250
	 * http://www.raspberrypi.org/forums/viewtopic.php?f=31&t=20594
	 */
	revision = msg->get_board_rev.body.resp.rev;
	if (revision & 0x800000) {
		rev_scheme = 1;
		rev_type = (revision >> 4) & 0xff;
		models = rpi_models_new_scheme;
		models_count = ARRAY_SIZE(rpi_models_new_scheme);
	} else {
		rev_scheme = 0;
		rev_type = revision & 0xff;
		models = rpi_models_old_scheme;
		models_count = ARRAY_SIZE(rpi_models_old_scheme);
	}
	if (rev_type >= models_count) {
		printf("RPI: Board rev 0x%x outside known range\n", rev_type);
		model = &rpi_model_unknown;
	} else if (!models[rev_type].name) {
		printf("RPI: Board rev 0x%x unknown\n", rev_type);
		model = &rpi_model_unknown;
	} else {
		model = &models[rev_type];
	}

	printf("RPI %s (0x%x)\n", model->name, revision);
}

#ifndef CONFIG_PL01X_SERIAL
static bool rpi_is_serial_active(void)
{
	int serial_gpio = 15;
	struct udevice *dev;

	/*
	 * The RPi3 disables the mini uart by default. The easiest way to find
	 * out whether it is available is to check if the RX pin is muxed.
	 */

	if (uclass_first_device(UCLASS_GPIO, &dev) || !dev)
		return true;

	if (bcm2835_gpio_get_func_id(dev, serial_gpio) != BCM2835_GPIO_ALT5)
		return false;

	return true;
}

/* Disable mini-UART I/O if it's not pinmuxed to our pins.
 * The firmware only enables it if explicitly done in config.txt: enable_uart=1
 */
static void rpi_disable_inactive_uart(void)
{
	struct udevice *dev;
	struct bcm283x_mu_serial_platdata *plat;

	if (uclass_get_device_by_driver(UCLASS_SERIAL,
					DM_GET_DRIVER(serial_bcm283x_mu),
					&dev) || !dev)
		return;

	if (!rpi_is_serial_active()) {
		plat = dev_get_platdata(dev);
		plat->disabled = true;
	}
}
#endif

int board_init(void)
{
#ifndef CONFIG_PL01X_SERIAL
	rpi_disable_inactive_uart();
#endif

	get_board_rev();

	gd->bd->bi_boot_params = 0x100;

	return power_on_module(BCM2835_MBOX_POWER_DEVID_USB_HCD);
}

int board_mmc_init(bd_t *bis)
{
	ALLOC_CACHE_ALIGN_BUFFER(struct msg_get_clock_rate, msg_clk, 1);
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

#ifdef CONFIG_EFI_LOADER
	/* Reserve the spin table */
	efi_add_memory_map(0, 1, EFI_RESERVED_MEMORY_TYPE, 0);
#endif

	return 0;
}
