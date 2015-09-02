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
#include "omap3logic.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * two dimensional array of strucures containining board name and Linux
 * machine IDs; row it selected based on CPU column is slected based
 * on hsusb0_data5 pin having a pulldown resistor
 */

static const struct ns16550_platdata omap3logic_serial = {
	OMAP34XX_UART1,
	2,
	V_NS16550_CLK
};

U_BOOT_DEVICE(omap3logic_uart) = {
	"serial_omap",
	&omap3logic_serial
};

static struct board_id {
	char *name;
	int machine_id;
} boards[2][2] = {
	{
		{
			.name		= "OMAP35xx SOM LV",
			.machine_id	= MACH_TYPE_OMAP3530_LV_SOM,
		},
		{
			.name		= "OMAP35xx Torpedo",
			.machine_id	= MACH_TYPE_OMAP3_TORPEDO,
		},
	},
	{
		{
			.name		= "DM37xx SOM LV",
			.machine_id	= MACH_TYPE_DM3730_SOM_LV,
		},
		{
			.name		= "DM37xx Torpedo",
			.machine_id	= MACH_TYPE_DM3730_TORPEDO,
		},
	},
};

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
	struct board_id *board;
	unsigned int val;

	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */

	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

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
		gd->bd->bi_arch_number = board->machine_id;
	}

	/* restore hsusb0_data5 pin as hsusb0_data5 */
	MUX_VAL(CP(HSUSB0_DATA5),	(IEN  | PTD | DIS | M0));

	return 0;
}

#if defined(CONFIG_GENERIC_MMC) && !defined(CONFIG_SPL_BUILD)
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}
#endif

#if defined(CONFIG_GENERIC_MMC)
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

/*
 * IEN  - Input Enable
 * IDIS - Input Disable
 * PTD  - Pull type Down
 * PTU  - Pull type Up
 * DIS  - Pull type selection is inactive
 * EN   - Pull type selection is active
 * M0   - Mode 0
 * The commented string gives the final mux configuration for that pin
 */

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	/*GPMC*/
	MUX_VAL(CP(GPMC_A1),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A2),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A3),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A4),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A5),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A6),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A7),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A8),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A9),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_A10),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D0),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D1),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D2),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D3),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D4),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D5),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D6),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D7),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D8),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D9),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D10),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D11),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D12),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D13),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D14),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_D15),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_NCS0),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_NCS1),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_NCS2),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_NCS3),		(IDIS | PTD | DIS | M0));
	MUX_VAL(CP(GPMC_NCS5),          (IDIS | PTU | DIS | M4));
	MUX_VAL(CP(GPMC_NCS7),		(IDIS | PTD | DIS | M1)); /*GPMC_IO_DIR*/
	MUX_VAL(CP(GPMC_NBE0_CLE),	(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(GPMC_WAIT1),		(IEN  | PTU | EN  | M0));

	/*Expansion card  */
	MUX_VAL(CP(MMC1_CLK),		(IDIS | PTU | EN  | M0));
	MUX_VAL(CP(MMC1_CMD),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(MMC1_DAT0),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(MMC1_DAT1),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(MMC1_DAT2),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(MMC1_DAT3),		(IEN  | PTU | EN  | M0));

	/* Serial Console */
	MUX_VAL(CP(UART1_TX),		(IDIS | PTD | DIS | M0));
	MUX_VAL(CP(UART1_RTS),		(IDIS | PTD | DIS | M0));
	MUX_VAL(CP(UART1_CTS),		(IEN  | PTU | DIS | M0));
	MUX_VAL(CP(UART1_RX),		(IEN  | PTD | DIS | M0));

	/* I2C */
	MUX_VAL(CP(I2C2_SCL),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(I2C2_SDA),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(I2C3_SCL),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(I2C3_SDA),		(IEN  | PTU | EN  | M0));

	MUX_VAL(CP(HDQ_SIO),		(IEN  | PTU | EN  | M0));

	/*Control and debug */
	MUX_VAL(CP(SYS_NIRQ),		(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(SYS_OFF_MODE),	(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SYS_CLKOUT1),	(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SYS_CLKOUT2),	(IEN  | PTU | EN  | M0));
	MUX_VAL(CP(JTAG_NTRST),		(IEN  | PTD | DIS | M0));
	MUX_VAL(CP(SDRC_CKE0),		(IDIS | PTU | EN  | M0));
}
