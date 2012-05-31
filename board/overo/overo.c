/*
 * Maintainer : Steve Sakoman <steve@sakoman.com>
 *
 * Derived from Beagle Board, 3430 SDP, and OMAP3EVM code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *	Sunil Kumar <sunilsaini05@gmail.com>
 *	Shashi Ranjan <shashiranjanmca05@gmail.com>
 *
 * (C) Copyright 2004-2008
 * Texas Instruments, <www.ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <netdev.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>
#include "overo.h"

DECLARE_GLOBAL_DATA_PTR;

#define TWL4030_I2C_BUS			0
#define EXPANSION_EEPROM_I2C_BUS	2
#define EXPANSION_EEPROM_I2C_ADDRESS	0x51

#define GUMSTIX_SUMMIT			0x01000200
#define GUMSTIX_TOBI			0x02000200
#define GUMSTIX_TOBI_DUO		0x03000200
#define GUMSTIX_PALO35			0x04000200
#define GUMSTIX_PALO43			0x05000200
#define GUMSTIX_CHESTNUT43		0x06000200
#define GUMSTIX_PINTO			0x07000200
#define GUMSTIX_GALLOP43		0x08000200

#define ETTUS_USRP_E			0x01000300

#define GUMSTIX_NO_EEPROM		0xffffffff

static struct {
	unsigned int device_vendor;
	unsigned char revision;
	unsigned char content;
	char fab_revision[8];
	char env_var[16];
	char env_setting[64];
} expansion_config;

#if defined(CONFIG_CMD_NET)
static void setup_net_chip(void);
#endif

/* GPMC definitions for LAN9221 chips on Tobi expansion boards */
static const u32 gpmc_lan_config[] = {
    NET_LAN9221_GPMC_CONFIG1,
    NET_LAN9221_GPMC_CONFIG2,
    NET_LAN9221_GPMC_CONFIG3,
    NET_LAN9221_GPMC_CONFIG4,
    NET_LAN9221_GPMC_CONFIG5,
    NET_LAN9221_GPMC_CONFIG6,
    /*CONFIG7- computed as params */
};

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OVERO;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

/*
 * Routine: get_board_revision
 * Description: Returns the board revision
 */
int get_board_revision(void)
{
	int revision;

	if (!omap_request_gpio(112) &&
	    !omap_request_gpio(113) &&
	    !omap_request_gpio(115)) {

		omap_set_gpio_direction(112, 1);
		omap_set_gpio_direction(113, 1);
		omap_set_gpio_direction(115, 1);

		revision = omap_get_gpio_datain(115) << 2 |
			   omap_get_gpio_datain(113) << 1 |
			   omap_get_gpio_datain(112);

		omap_free_gpio(112);
		omap_free_gpio(113);
		omap_free_gpio(115);
	} else {
		printf("Error: unable to acquire board revision GPIOs\n");
		revision = -1;
	}

	return revision;
}

/*
 * Routine: get_sdio2_config
 * Description: Return information about the wifi module connection
 *              Returns 0 if the module connects though a level translator
 *              Returns 1 if the module connects directly
 */
int get_sdio2_config(void)
{
	int sdio_direct;

	if (!omap_request_gpio(130) && !omap_request_gpio(139)) {

		omap_set_gpio_direction(130, 0);
		omap_set_gpio_direction(139, 1);

		sdio_direct = 1;
		omap_set_gpio_dataout(130, 0);
		if (omap_get_gpio_datain(139) == 0) {
			omap_set_gpio_dataout(130, 1);
			if (omap_get_gpio_datain(139) == 1)
				sdio_direct = 0;
		}

		omap_free_gpio(130);
		omap_free_gpio(139);
	} else {
		printf("Error: unable to acquire sdio2 clk GPIOs\n");
		sdio_direct = -1;
	}

	return sdio_direct;
}

/*
 * Routine: get_expansion_id
 * Description: This function checks for expansion board by checking I2C
 *		bus 2 for the availability of an AT24C01B serial EEPROM.
 *		returns the device_vendor field from the EEPROM
 */
unsigned int get_expansion_id(void)
{
	i2c_set_bus_num(EXPANSION_EEPROM_I2C_BUS);

	/* return GUMSTIX_NO_EEPROM if eeprom doesn't respond */
	if (i2c_probe(EXPANSION_EEPROM_I2C_ADDRESS) == 1) {
		i2c_set_bus_num(TWL4030_I2C_BUS);
		return GUMSTIX_NO_EEPROM;
	}

	/* read configuration data */
	i2c_read(EXPANSION_EEPROM_I2C_ADDRESS, 0, 1, (u8 *)&expansion_config,
		 sizeof(expansion_config));

	i2c_set_bus_num(TWL4030_I2C_BUS);

	return expansion_config.device_vendor;
}

/*
 * Routine: misc_init_r
 * Description: Configure board specific parts
 */
int misc_init_r(void)
{
	twl4030_power_init();
	twl4030_led_init(TWL4030_LED_LEDEN_LEDAON | TWL4030_LED_LEDEN_LEDBON);

#if defined(CONFIG_CMD_NET)
	setup_net_chip();
#endif

	printf("Board revision: %d\n", get_board_revision());

	switch (get_sdio2_config()) {
	case 0:
		printf("Tranceiver detected on mmc2\n");
		MUX_OVERO_SDIO2_TRANSCEIVER();
		break;
	case 1:
		printf("Direct connection on mmc2\n");
		MUX_OVERO_SDIO2_DIRECT();
		break;
	default:
		printf("Unable to detect mmc2 connection type\n");
	}

	switch (get_expansion_id()) {
	case GUMSTIX_SUMMIT:
		printf("Recognized Summit expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		setenv("defaultdisplay", "dvi");
		break;
	case GUMSTIX_TOBI:
		printf("Recognized Tobi expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		setenv("defaultdisplay", "dvi");
		break;
	case GUMSTIX_TOBI_DUO:
		printf("Recognized Tobi Duo expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		break;
	case GUMSTIX_PALO35:
		printf("Recognized Palo35 expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		setenv("defaultdisplay", "lcd35");
		break;
	case GUMSTIX_PALO43:
		printf("Recognized Palo43 expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		setenv("defaultdisplay", "lcd43");
		break;
	case GUMSTIX_CHESTNUT43:
		printf("Recognized Chestnut43 expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		setenv("defaultdisplay", "lcd43");
		break;
	case GUMSTIX_PINTO:
		printf("Recognized Pinto expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		break;
	case GUMSTIX_GALLOP43:
		printf("Recognized Gallop43 expansion board (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		setenv("defaultdisplay", "lcd43");
		break;
	case ETTUS_USRP_E:
		printf("Recognized Ettus Research USRP-E (rev %d %s)\n",
			expansion_config.revision,
			expansion_config.fab_revision);
		MUX_USRP_E();
		setenv("defaultdisplay", "dvi");
		break;
	case GUMSTIX_NO_EEPROM:
		printf("No EEPROM on expansion board\n");
		break;
	default:
		printf("Unrecognized expansion board\n");
	}

	if (expansion_config.content == 1)
		setenv(expansion_config.env_var, expansion_config.env_setting);

	dieid_num_r();

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
	MUX_OVERO();
}

#if defined(CONFIG_CMD_NET)
/*
 * Routine: setup_net_chip
 * Description: Setting up the configuration GPMC registers specific to the
 *	      Ethernet hardware.
 */
static void setup_net_chip(void)
{
	struct ctrl *ctrl_base = (struct ctrl *)OMAP34XX_CTRL_BASE;

	/* first lan chip */
	enable_gpmc_cs_config(gpmc_lan_config, &gpmc_cfg->cs[5], 0x2C000000,
			GPMC_SIZE_16M);

	/* second lan chip */
	enable_gpmc_cs_config(gpmc_lan_config, &gpmc_cfg->cs[4], 0x2B000000,
			GPMC_SIZE_16M);

	/* Enable off mode for NWE in PADCONF_GPMC_NWE register */
	writew(readw(&ctrl_base ->gpmc_nwe) | 0x0E00, &ctrl_base->gpmc_nwe);
	/* Enable off mode for NOE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_noe) | 0x0E00, &ctrl_base->gpmc_noe);
	/* Enable off mode for ALE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_nadv_ale) | 0x0E00,
		&ctrl_base->gpmc_nadv_ale);

	/* Make GPIO 64 as output pin and send a magic pulse through it */
	if (!omap_request_gpio(64)) {
		omap_set_gpio_direction(64, 0);
		omap_set_gpio_dataout(64, 1);
		udelay(1);
		omap_set_gpio_dataout(64, 0);
		udelay(1);
		omap_set_gpio_dataout(64, 1);
	}
}
#endif

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0);
	return 0;
}
#endif
