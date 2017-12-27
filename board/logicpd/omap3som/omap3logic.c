/*
 * (C) Copyright 2011
 * Logic Product Development <www.logicpd.com>
 *
 * Author :
 *	Peter Barada <peter.barada@logicpd.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <ns16550.h>
#include <netdev.h>
#include <flash.h>
#include <nand.h>
#include <i2c.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>
#include <linux/mtd/rawnand.h>
#include <asm/omap_musb.h>
#include <linux/errno.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/musb.h>
#include "omap3logic.h"
#ifdef CONFIG_USB_EHCI_HCD
#include <usb.h>
#include <asm/ehci-omap.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/*
 * two dimensional array of strucures containining board name and Linux
 * machine IDs; row it selected based on CPU column is slected based
 * on hsusb0_data5 pin having a pulldown resistor
 */
static struct board_id {
	char *name;
	int machine_id;
	char *fdtfile;
} boards[2][2] = {
	{
		{
			.name		= "OMAP35xx SOM LV",
			.machine_id	= MACH_TYPE_OMAP3530_LV_SOM,
			.fdtfile	= "logicpd-som-lv-35xx-devkit.dtb",
		},
		{
			.name		= "OMAP35xx Torpedo",
			.machine_id	= MACH_TYPE_OMAP3_TORPEDO,
			.fdtfile	= "logicpd-torpedo-35xx-devkit.dtb",
		},
	},
	{
		{
			.name		= "DM37xx SOM LV",
			.fdtfile	= "logicpd-som-lv-37xx-devkit.dtb",
		},
		{
			.name		= "DM37xx Torpedo",
			.fdtfile	= "logicpd-torpedo-37xx-devkit.dtb",
		},
	},
};

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	/* break into full u-boot on 'c' */
	return serial_tstc() && serial_getc() == 'c';
}
#endif

#if defined(CONFIG_SPL_BUILD)
/*
 * Routine: get_board_mem_timings
 * Description: If we use SPL then there is no x-loader nor config header
 * so we have to setup the DDR timings ourself on the first bank.  This
 * provides the timing values back to the function that configures
 * the memory.
 */
void get_board_mem_timings(struct board_sdrc_timings *timings)
{
	timings->mr = MICRON_V_MR_165;
	/* 256MB DDR */
	timings->mcfg = MICRON_V_MCFG_200(256 << 20);
	timings->ctrla = MICRON_V_ACTIMA_200;
	timings->ctrlb = MICRON_V_ACTIMB_200;
	timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_200MHz;
}

#define GPMC_NAND_COMMAND_0 (OMAP34XX_GPMC_BASE + 0x7c)
#define GPMC_NAND_DATA_0 (OMAP34XX_GPMC_BASE + 0x84)
#define GPMC_NAND_ADDRESS_0 (OMAP34XX_GPMC_BASE + 0x80)

void spl_board_prepare_for_linux(void)
{
	/* The Micron NAND starts locked which
	 * prohibits mounting the NAND as RW
	 * The following commands are what unlocks
	 * the NAND to become RW Falcon Mode does not
	 * have as many smarts as U-Boot, but Logic PD
	 * only makes NAND with 512MB so these hard coded
	 * values should work for all current models
	 */

	writeb(0x70, GPMC_NAND_COMMAND_0);
	writeb(-1, GPMC_NAND_DATA_0);
	writeb(0x7a, GPMC_NAND_COMMAND_0);
	writeb(0x00, GPMC_NAND_ADDRESS_0);
	writeb(0x00, GPMC_NAND_ADDRESS_0);
	writeb(0x00, GPMC_NAND_ADDRESS_0);
	writeb(-1, GPMC_NAND_COMMAND_0);

	/* Begin address 0 */
	writeb(NAND_CMD_UNLOCK1, 0x6e00007c);
	writeb(0x00, GPMC_NAND_ADDRESS_0);
	writeb(0x00, GPMC_NAND_ADDRESS_0);
	writeb(0x00, GPMC_NAND_ADDRESS_0);
	writeb(-1, GPMC_NAND_DATA_0);

	/* Ending address at the end of Flash */
	writeb(NAND_CMD_UNLOCK2, GPMC_NAND_COMMAND_0);
	writeb(0xc0, GPMC_NAND_ADDRESS_0);
	writeb(0xff, GPMC_NAND_ADDRESS_0);
	writeb(0x03, GPMC_NAND_ADDRESS_0);
	writeb(-1, GPMC_NAND_DATA_0);
	writeb(0x79, GPMC_NAND_COMMAND_0);
	writeb(-1, GPMC_NAND_DATA_0);
	writeb(-1, GPMC_NAND_DATA_0);
}
#endif

#ifdef CONFIG_USB_MUSB_OMAP2PLUS
static struct musb_hdrc_config musb_config = {
	.multipoint     = 1,
	.dyn_fifo       = 1,
	.num_eps        = 16,
	.ram_bits       = 12,
};

static struct omap_musb_board_data musb_board_data = {
	.interface_type	= MUSB_INTERFACE_ULPI,
};

static struct musb_hdrc_platform_data musb_plat = {
#if defined(CONFIG_USB_MUSB_HOST)
	.mode           = MUSB_HOST,
#elif defined(CONFIG_USB_MUSB_GADGET)
	.mode		= MUSB_PERIPHERAL,
#else
#error "Please define either CONFIG_USB_MUSB_HOST or CONFIG_USB_MUSB_GADGET"
#endif
	.config         = &musb_config,
	.power          = 100,
	.platform_ops	= &omap2430_ops,
	.board_data	= &musb_board_data,
};
#endif

#if defined(CONFIG_USB_EHCI_HCD) && !defined(CONFIG_SPL_BUILD)
/* Call usb_stop() before starting the kernel */
void show_boot_progress(int val)
{
	if (val == BOOTSTAGE_ID_RUN_OS)
		usb_stop();
}

static struct omap_usbhs_board_data usbhs_bdata = {
	.port_mode[0] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[1] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED
};

int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	return omap_ehci_hcd_init(index, &usbhs_bdata, hccr, hcor);
}

int ehci_hcd_stop(int index)
{
	return omap_ehci_hcd_stop();
}

#endif /* CONFIG_USB_EHCI_HCD */


/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	twl4030_power_init();
	omap_die_id_display();

#ifdef CONFIG_USB_MUSB_OMAP2PLUS
	musb_register(&musb_plat, &musb_board_data, (void *)MUSB_BASE);
#endif

	return 0;
}

/*
 * BOARD_ID_GPIO - GPIO of pin with optional pulldown resistor on SOM LV
 */
#define BOARD_ID_GPIO	189 /* hsusb0_data5 pin */

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */

	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT

static void unlock_nand(void)
{
	int dev = nand_curr_device;
	struct mtd_info *mtd;

	mtd = get_nand_dev_by_index(dev);
	nand_unlock(mtd, 0, mtd->size, 0);
}

int board_late_init(void)
{
	struct board_id *board;
	unsigned int val;

	/*
	 * To identify between a SOM LV and Torpedo module,
	 * a pulldown resistor is on hsusb0_data5 for the SOM LV module.
	 * Drive the pin (and let it soak), then read it back.
	 * If the pin is still high its a Torpedo.  If low its a SOM LV
	 */

	/* Mux hsusb0_data5 as a GPIO */
	MUX_VAL(CP(HSUSB0_DATA5),	(IEN  | PTD | DIS | M4));

	if (gpio_request(BOARD_ID_GPIO, "husb0_data5.gpio_189") == 0) {

		/*
		 * Drive BOARD_ID_GPIO - the pulldown resistor on the SOM LV
		 * will drain the voltage.
		 */
		gpio_direction_output(BOARD_ID_GPIO, 0);
		gpio_set_value(BOARD_ID_GPIO, 1);

		/* Let it soak for a bit */
		sdelay(0x100);

		/*
		 * Read state of BOARD_ID_GPIO as an input and if its set.
		 * If so the board is a Torpedo
		 */
		gpio_direction_input(BOARD_ID_GPIO);
		val = gpio_get_value(BOARD_ID_GPIO);
		gpio_free(BOARD_ID_GPIO);

		board = &boards[!!(get_cpu_family() == CPU_OMAP36XX)][!!val];
		printf("Board: %s\n", board->name);

		/* Set the machine_id passed to Linux */
		if (board->machine_id)
			gd->bd->bi_arch_number = board->machine_id;

		/* If the user has not set fdtimage, set the default */
		if (!env_get("fdtimage"))
			env_set("fdtimage", board->fdtfile);
	}

	/* restore hsusb0_data5 pin as hsusb0_data5 */
	MUX_VAL(CP(HSUSB0_DATA5),	(IEN  | PTD | DIS | M0));

#ifdef CONFIG_CMD_NAND_LOCK_UNLOCK
	unlock_nand();
#endif
	return 0;
}
#endif

#if defined(CONFIG_MMC)
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}
#endif

#if defined(CONFIG_MMC)
void board_mmc_power_init(void)
{
	twl4030_power_mmc_init(0);
}
#endif

#ifdef CONFIG_SMC911X
/* GPMC CS1 settings for Logic SOM LV/Torpedo LAN92xx Ethernet chip */
static const u32 gpmc_lan92xx_config[] = {
	NET_LAN92XX_GPMC_CONFIG1,
	NET_LAN92XX_GPMC_CONFIG2,
	NET_LAN92XX_GPMC_CONFIG3,
	NET_LAN92XX_GPMC_CONFIG4,
	NET_LAN92XX_GPMC_CONFIG5,
	NET_LAN92XX_GPMC_CONFIG6,
};

int board_eth_init(bd_t *bis)
{
	enable_gpmc_cs_config(gpmc_lan92xx_config, &gpmc_cfg->cs[1],
			CONFIG_SMC911X_BASE, GPMC_SIZE_16M);

	return smc911x_initialize(0, CONFIG_SMC911X_BASE);
}
#endif


