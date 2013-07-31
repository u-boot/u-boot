/*
 * (C) Copyright 2004-2009
 * Texas Instruments Incorporated, <www.ti.com>
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <netdev.h>
#include <twl4030.h>
#include <asm/io.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/mux.h>
#include <asm/arch/mem.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-types.h>
#include "sdp.h"

DECLARE_GLOBAL_DATA_PTR;

const omap3_sysinfo sysinfo = {
	DDR_DISCRETE,
	"OMAP3 SDP3430 board",
#if defined(CONFIG_ENV_IS_IN_ONENAND)
	"OneNAND",
#elif defined(CONFIG_ENV_IS_IN_NAND)
	"NAND",
#else
	"NOR",
#endif
};

/* Timing definitions for GPMC controller for Sibley NOR */
static const u32 gpmc_sdp_nor[] = {
    SDP3430_NOR_GPMC_CONF1,
    SDP3430_NOR_GPMC_CONF2,
    SDP3430_NOR_GPMC_CONF3,
    SDP3430_NOR_GPMC_CONF4,
    SDP3430_NOR_GPMC_CONF5,
    SDP3430_NOR_GPMC_CONF6,
    /*CONF7- computed as params */
};

/*
 * Timing definitions for GPMC controller for Debug Board
 * Debug board contains access to ethernet and DIP Switch setting
 * information etc.
 */
static const u32 gpmc_sdp_debug[] = {
    SDP3430_DEBUG_GPMC_CONF1,
    SDP3430_DEBUG_GPMC_CONF2,
    SDP3430_DEBUG_GPMC_CONF3,
    SDP3430_DEBUG_GPMC_CONF4,
    SDP3430_DEBUG_GPMC_CONF5,
    SDP3430_DEBUG_GPMC_CONF6,
    /*CONF7- computed as params */
};

/* Timing defintions for GPMC OneNAND */
static const u32 gpmc_sdp_onenand[] = {
    SDP3430_ONENAND_GPMC_CONF1,
    SDP3430_ONENAND_GPMC_CONF2,
    SDP3430_ONENAND_GPMC_CONF3,
    SDP3430_ONENAND_GPMC_CONF4,
    SDP3430_ONENAND_GPMC_CONF5,
    SDP3430_ONENAND_GPMC_CONF6,
    /*CONF7- computed as params */
};

/* GPMC definitions for GPMC NAND */
static const u32 gpmc_sdp_nand[] = {
    SDP3430_NAND_GPMC_CONF1,
    SDP3430_NAND_GPMC_CONF2,
    SDP3430_NAND_GPMC_CONF3,
    SDP3430_NAND_GPMC_CONF4,
    SDP3430_NAND_GPMC_CONF5,
    SDP3430_NAND_GPMC_CONF6,
    /*CONF7- computed as params */
};

/* gpmc_cfg is initialized by gpmc_init and we use it here */
extern struct gpmc *gpmc_cfg;

/**
 * @brief board_init - gpmc and basic setup as phase1 of boot sequence
 *
 * @return 0
 */
int board_init(void)
{
	gpmc_init(); /* in SRAM or SDRAM, finish GPMC */
	/* TODO: Dynamically pop out CS mapping and program accordingly */
	/* Configure devices for default ON ON ON settings */
	enable_gpmc_cs_config(gpmc_sdp_nor, &gpmc_cfg->cs[0],
			CONFIG_SYS_FLASH_BASE, GPMC_SIZE_128M);
	enable_gpmc_cs_config(gpmc_sdp_nand, &gpmc_cfg->cs[1], 0x28000000,
			GPMC_SIZE_16M);
	enable_gpmc_cs_config(gpmc_sdp_onenand, &gpmc_cfg->cs[2], 0x20000000,
			GPMC_SIZE_16M);
	enable_gpmc_cs_config(gpmc_sdp_debug, &gpmc_cfg->cs[3], DEBUG_BASE,
			GPMC_SIZE_16M);
	/* board id for Linux */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP_3430SDP;
	/* boot param addr */
	gd->bd->bi_boot_params = (OMAP34XX_SDRC_CS0 + 0x100);

	return 0;
}

#define LAN_RESET_REGISTER	(CONFIG_LAN91C96_BASE + 0x01c)
#define ETH_CONTROL_REG		(CONFIG_LAN91C96_BASE + 0x30b)

/**
 * @brief board_eth_init Take the Ethernet controller out of reset and wait
 * for the EEPROM load to complete.
 */
int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_LAN91C96
	int cnt = 20;

	writew(0x0, LAN_RESET_REGISTER);
	do {
		writew(0x1, LAN_RESET_REGISTER);
		udelay(100);
		if (cnt == 0)
			goto reset_err_out;
		--cnt;
	} while (readw(LAN_RESET_REGISTER) != 0x1);

	cnt = 20;

	do {
		writew(0x0, LAN_RESET_REGISTER);
		udelay(100);
		if (cnt == 0)
			goto reset_err_out;
		--cnt;
	} while (readw(LAN_RESET_REGISTER) != 0x0000);
	udelay(1000);

	writeb(readb(ETH_CONTROL_REG) & ~0x1, ETH_CONTROL_REG);
	udelay(1000);
	rc = lan91c96_initialize(0, CONFIG_LAN91C96_BASE);
reset_err_out:

#endif
	return rc;
}

/**
 * @brief misc_init_r - Configure SDP board specific configurations
 * such as power configurations, ethernet initialization as phase2 of
 * boot sequence
 *
 * @return 0
 */
int misc_init_r(void)
{
	/* Partial setup:
	 *   VAUX3 - 2.8V for DVI
	 *   VPLL1 - 1.8V
	 *   VDAC  - 1.8V
	 * and turns on LEDA/LEDB (not needed ... NOP?)
	 */
	twl4030_power_init();

	/* FIXME finish setup:
	 *   VAUX1 - 2.8V for mainboard I/O
	 *   VAUX2 - 2.8V for camera
	 *   VAUX4 - 1.8V for OMAP3 CSI
	 *   VMMC1 - 3.15V (init, variable) for MMC1
	 *   VMMC2 - 1.85V for MMC2
	 *   VSIM  - off (init, variable) for MMC1.DAT[3..7], SIM
	 *   VPLL2 - 1.8V
	 */

	return 0;
}

/**
 * @brief set_muxconf_regs Setting up the configuration Mux registers
 * specific to the hardware. Many pins need to be moved from protect
 * to primary mode.
 */
void set_muxconf_regs(void)
{
	/* platform specific muxes */
	MUX_SDP3430();
}

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(0, 0, 0, -1, -1);
}
#endif
