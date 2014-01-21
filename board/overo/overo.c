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
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <netdev.h>
#include <twl4030.h>
#include <linux/mtd/nand.h>
#include <asm/io.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <asm/omap_gpmc.h>
#include <asm/gpio.h>
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

#ifdef CONFIG_SYS_I2C_OMAP34XX
	unsigned char data;

	/* board revisions <= R2410 connect 4030 irq_1 to gpio112             */
	/* these boards should return a revision number of 0                  */
	/* the code below forces a 4030 RTC irq to ensure that gpio112 is low */
	i2c_set_bus_num(TWL4030_I2C_BUS);
	data = 0x01;
	i2c_write(0x4B, 0x29, 1, &data, 1);
	data = 0x0c;
	i2c_write(0x4B, 0x2b, 1, &data, 1);
	i2c_read(0x4B, 0x2a, 1, &data, 1);
#endif

	if (!gpio_request(112, "") &&
	    !gpio_request(113, "") &&
	    !gpio_request(115, "")) {

		gpio_direction_input(112);
		gpio_direction_input(113);
		gpio_direction_input(115);

		revision = gpio_get_value(115) << 2 |
			   gpio_get_value(113) << 1 |
			   gpio_get_value(112);
	} else {
		puts("Error: unable to acquire board revision GPIOs\n");
		revision = -1;
	}

	return revision;
}

#ifdef CONFIG_SPL_BUILD
/*
 * Routine: get_board_mem_timings
 * Description: If we use SPL then there is no x-loader nor config header
 * so we have to setup the DDR timings ourself on both banks.
 */
void get_board_mem_timings(struct board_sdrc_timings *timings)
{
	timings->mr = MICRON_V_MR_165;
	switch (get_board_revision()) {
	case REVISION_0: /* Micron 1286MB/256MB, 1/2 banks of 128MB */
		timings->mcfg = MICRON_V_MCFG_165(128 << 20);
		timings->ctrla = MICRON_V_ACTIMA_165;
		timings->ctrlb = MICRON_V_ACTIMB_165;
		timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
		break;
	case REVISION_1: /* Micron 256MB/512MB, 1/2 banks of 256MB */
		timings->mcfg = MICRON_V_MCFG_200(256 << 20);
		timings->ctrla = MICRON_V_ACTIMA_200;
		timings->ctrlb = MICRON_V_ACTIMB_200;
		timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_200MHz;
		break;
	case REVISION_2: /* Hynix 256MB/512MB, 1/2 banks of 256MB */
		timings->mcfg = HYNIX_V_MCFG_200(256 << 20);
		timings->ctrla = HYNIX_V_ACTIMA_200;
		timings->ctrlb = HYNIX_V_ACTIMB_200;
		timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_200MHz;
		break;
	case REVISION_3: /* Micron 512MB/1024MB, 1/2 banks of 512MB */
		timings->mcfg = MCFG(512 << 20, 15);
		timings->ctrla = MICRON_V_ACTIMA_200;
		timings->ctrlb = MICRON_V_ACTIMB_200;
		timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_200MHz;
		break;
	default:
		timings->mcfg = MICRON_V_MCFG_165(128 << 20);
		timings->ctrla = MICRON_V_ACTIMA_165;
		timings->ctrlb = MICRON_V_ACTIMB_165;
		timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_165MHz;
	}
}
#endif

/*
 * Routine: get_sdio2_config
 * Description: Return information about the wifi module connection
 *              Returns 0 if the module connects though a level translator
 *              Returns 1 if the module connects directly
 */
int get_sdio2_config(void)
{
	int sdio_direct;

	if (!gpio_request(130, "") && !gpio_request(139, "")) {

		gpio_direction_output(130, 0);
		gpio_direction_input(139);

		sdio_direct = 1;
		gpio_set_value(130, 0);
		if (gpio_get_value(139) == 0) {
			gpio_set_value(130, 1);
			if (gpio_get_value(139) == 1)
				sdio_direct = 0;
		}

		gpio_direction_input(130);
	} else {
		puts("Error: unable to acquire sdio2 clk GPIOs\n");
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
		puts("Tranceiver detected on mmc2\n");
		MUX_OVERO_SDIO2_TRANSCEIVER();
		break;
	case 1:
		puts("Direct connection on mmc2\n");
		MUX_OVERO_SDIO2_DIRECT();
		break;
	default:
		puts("Unable to detect mmc2 connection type\n");
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
		/* second lan chip */
		enable_gpmc_cs_config(gpmc_lan_config, &gpmc_cfg->cs[4],
		    0x2B000000, GPMC_SIZE_16M);
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
		puts("No EEPROM on expansion board\n");
		break;
	default:
		puts("Unrecognized expansion board\n");
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

	/* Enable off mode for NWE in PADCONF_GPMC_NWE register */
	writew(readw(&ctrl_base ->gpmc_nwe) | 0x0E00, &ctrl_base->gpmc_nwe);
	/* Enable off mode for NOE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_noe) | 0x0E00, &ctrl_base->gpmc_noe);
	/* Enable off mode for ALE in PADCONF_GPMC_NADV_ALE register */
	writew(readw(&ctrl_base->gpmc_nadv_ale) | 0x0E00,
		&ctrl_base->gpmc_nadv_ale);

	/* Make GPIO 64 as output pin and send a magic pulse through it */
	if (!gpio_request(64, "")) {
		gpio_direction_output(64, 0);
		gpio_set_value(64, 1);
		udelay(1);
		gpio_set_value(64, 0);
		udelay(1);
		gpio_set_value(64, 1);
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

#if defined(CONFIG_GENERIC_MMC) && !defined(CONFIG_SPL_BUILD)
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}
#endif
