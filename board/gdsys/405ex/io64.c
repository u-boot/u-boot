/*
 * (C) Copyright 2010
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * based on kilauea.c
 * by Stefan Roese, DENX Software Engineering, sr@denx.de.
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
#include <asm/ppc4xx.h>
#include <asm/ppc405.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/ppc4xx-gpio.h>
#include <flash.h>

#include <pca9698.h>

#include "405ex.h"
#include <gdsys_fpga.h>

#include <miiphy.h>
#include <i2c.h>
#include <dtt.h>

DECLARE_GLOBAL_DATA_PTR;

#define PHYREG_CONTROL				0
#define PHYREG_PAGE_ADDRESS			22
#define PHYREG_PG0_COPPER_SPECIFIC_CONTROL_1	16
#define PHYREG_PG2_COPPER_SPECIFIC_CONTROL_2	26
#define PHYREG_PG2_MAC_SPECIFIC_STATUS_1	17
#define PHYREG_PG2_MAC_SPECIFIC_CONTROL		21

#define LATCH0_BASE (CONFIG_SYS_LATCH_BASE)
#define LATCH1_BASE (CONFIG_SYS_LATCH_BASE + 0x100)
#define LATCH2_BASE (CONFIG_SYS_LATCH_BASE + 0x200)
#define LATCH3_BASE (CONFIG_SYS_LATCH_BASE + 0x300)

enum {
	UNITTYPE_CCD_SWITCH = 1,
};

enum {
	HWVER_100 = 0,
	HWVER_110 = 1,
};

static inline void blank_string(int size)
{
	int i;

	for (i = 0; i < size; i++)
		putc('\b');
	for (i = 0; i < size; i++)
		putc(' ');
	for (i = 0; i < size; i++)
		putc('\b');
}

/*
 * Board early initialization function
 */
int misc_init_r(void)
{
	/* startup fans */
	dtt_init();

#ifdef CONFIG_ENV_IS_IN_FLASH
	/* Monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      -CONFIG_SYS_MONITOR_LEN,
		      0xffffffff,
		      &flash_info[0]);
#endif

	return 0;
}

static void print_fpga_info(unsigned dev)
{
	ihs_fpga_t *fpga = (ihs_fpga_t *) CONFIG_SYS_FPGA_BASE(dev);
	u16 versions = in_le16(&fpga->versions);
	u16 fpga_version = in_le16(&fpga->fpga_version);
	u16 fpga_features = in_le16(&fpga->fpga_features);
	int fpga_state = get_fpga_state(dev);

	unsigned unit_type;
	unsigned hardware_version;
	unsigned feature_channels;
	unsigned feature_expansion;

	printf("FPGA%d: ", dev);
	if (fpga_state & FPGA_STATE_PLATFORM)
		printf("(legacy) ");

	if (fpga_state & FPGA_STATE_DONE_FAILED) {
		printf(" done timed out\n");
		return;
	}

	if (fpga_state & FPGA_STATE_REFLECTION_FAILED) {
		printf(" refelectione test failed\n");
		return;
	}

	unit_type = (versions & 0xf000) >> 12;
	hardware_version = versions & 0x000f;
	feature_channels = fpga_features & 0x007f;
	feature_expansion = fpga_features & (1<<15);

	switch (unit_type) {
	case UNITTYPE_CCD_SWITCH:
		printf("CCD-Switch");
		break;

	default:
		printf("UnitType %d(not supported)", unit_type);
		break;
	}

	switch (hardware_version) {
	case HWVER_100:
		printf(" HW-Ver 1.00\n");
		break;

	case HWVER_110:
		printf(" HW-Ver 1.10\n");
		break;

	default:
		printf(" HW-Ver %d(not supported)\n",
		       hardware_version);
		break;
	}

	printf("       FPGA V %d.%02d, features:",
		fpga_version / 100, fpga_version % 100);

	printf(" %d channel(s)", feature_channels);

	printf(", expansion %ssupported\n", feature_expansion ? "" : "un");
}

int checkboard(void)
{
	char *s = getenv("serial#");

	printf("Board: CATCenter Io64\n");

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}

	return 0;
}

int configure_gbit_phy(char *bus, unsigned char addr)
{
	unsigned short value;

	/* select page 0 */
	if (miiphy_write(bus, addr, PHYREG_PAGE_ADDRESS, 0x0000))
		goto err_out;
	/* switch to powerdown */
	if (miiphy_read(bus, addr, PHYREG_PG0_COPPER_SPECIFIC_CONTROL_1,
		&value))
		goto err_out;
	if (miiphy_write(bus, addr, PHYREG_PG0_COPPER_SPECIFIC_CONTROL_1,
		value | 0x0004))
		goto err_out;
	/* select page 2 */
	if (miiphy_write(bus, addr, PHYREG_PAGE_ADDRESS, 0x0002))
		goto err_out;
	/* disable SGMII autonegotiation */
	if (miiphy_write(bus, addr, PHYREG_PG2_MAC_SPECIFIC_CONTROL, 48))
		goto err_out;
	/* select page 0 */
	if (miiphy_write(bus, addr, PHYREG_PAGE_ADDRESS, 0x0000))
		goto err_out;
	/* switch from powerdown to normal operation */
	if (miiphy_read(bus, addr, PHYREG_PG0_COPPER_SPECIFIC_CONTROL_1,
		&value))
		goto err_out;
	if (miiphy_write(bus, addr, PHYREG_PG0_COPPER_SPECIFIC_CONTROL_1,
		value & ~0x0004))
		goto err_out;
	/* reset phy so settings take effect */
	if (miiphy_write(bus, addr, PHYREG_CONTROL, 0x9140))
		goto err_out;

	return 0;

err_out:
	printf("Error writing to the PHY addr=%02x\n", addr);
	return -1;
}

int verify_gbit_phy(char *bus, unsigned char addr)
{
	unsigned short value;

	/* select page 2 */
	if (miiphy_write(bus, addr, PHYREG_PAGE_ADDRESS, 0x0002))
		goto err_out;
	/* verify SGMII link status */
	if (miiphy_read(bus, addr, PHYREG_PG2_MAC_SPECIFIC_STATUS_1, &value))
		goto err_out;
	if (!(value & (1 << 10)))
		return -2;

	return 0;

err_out:
	printf("Error writing to the PHY addr=%02x\n", addr);
	return -1;
}

int last_stage_init(void)
{
	unsigned int k;
	unsigned int fpga;
	ihs_fpga_t *fpga0 = (ihs_fpga_t *) CONFIG_SYS_FPGA_BASE(0);
	ihs_fpga_t *fpga1 = (ihs_fpga_t *) CONFIG_SYS_FPGA_BASE(1);
	int failed = 0;
	char str_phys[] = "Setup PHYs -";
	char str_serdes[] = "Start SERDES blocks";
	char str_channels[] = "Start FPGA channels";
	char str_locks[] = "Verify SERDES locks";
	char str_hicb[] = "Verify HICB status";
	char str_status[] = "Verify PHY status -";
	char slash[] = "\\|/-\\|/-";

	print_fpga_info(0);
	print_fpga_info(1);

	/* setup Gbit PHYs */
	puts("TRANS: ");
	puts(str_phys);
	miiphy_register(CONFIG_SYS_GBIT_MII_BUSNAME,
		bb_miiphy_read, bb_miiphy_write);

	for (k = 0; k < 32; ++k) {
		configure_gbit_phy(CONFIG_SYS_GBIT_MII_BUSNAME, k);
		putc('\b');
		putc(slash[k % 8]);
	}

	miiphy_register(CONFIG_SYS_GBIT_MII1_BUSNAME,
		bb_miiphy_read, bb_miiphy_write);

	for (k = 0; k < 32; ++k) {
		configure_gbit_phy(CONFIG_SYS_GBIT_MII1_BUSNAME, k);
		putc('\b');
		putc(slash[k % 8]);
	}
	blank_string(strlen(str_phys));

	/* take fpga serdes blocks out of reset */
	puts(str_serdes);
	udelay(500000);
	out_le16(&fpga0->quad_serdes_reset, 0);
	out_le16(&fpga1->quad_serdes_reset, 0);
	blank_string(strlen(str_serdes));

	/* take channels out of reset */
	puts(str_channels);
	udelay(500000);
	for (fpga = 0; fpga < 2; ++fpga) {
		u16 *ch0_config_int = &(fpga ? fpga1 : fpga0)->ch0_config_int;
		for (k = 0; k < 32; ++k)
			out_le16(ch0_config_int + 4 * k, 0);
	}
	blank_string(strlen(str_channels));

	/* verify channels serdes lock */
	puts(str_locks);
	udelay(500000);
	for (fpga = 0; fpga < 2; ++fpga) {
		u16 *ch0_status_int = &(fpga ? fpga1 : fpga0)->ch0_status_int;
		for (k = 0; k < 32; ++k) {
			u16 status = in_le16(ch0_status_int + 4*k);
			if (!(status & (1 << 4))) {
				failed = 1;
				printf("fpga %d channel %d: no serdes lock\n",
					fpga, k);
			}
			/* reset events */
			out_le16(ch0_status_int + 4*k, status);
		}
	}
	blank_string(strlen(str_locks));

	/* verify hicb_status */
	puts(str_hicb);
	for (fpga = 0; fpga < 2; ++fpga) {
		u16 *ch0_hicb_status_int = &(fpga ? fpga1 : fpga0)->ch0_hicb_status_int;
		for (k = 0; k < 32; ++k) {
			u16 status = in_le16(ch0_hicb_status_int + 4*k);
			if (status)
				printf("fpga %d hicb %d: hicb status %04x\n",
					fpga, k, status);
			/* reset events */
			out_le16(ch0_hicb_status_int + 4*k, status);
		}
	}
	blank_string(strlen(str_hicb));

	/* verify phy status */
	puts(str_status);
	for (k = 0; k < 32; ++k) {
		if (verify_gbit_phy(CONFIG_SYS_GBIT_MII_BUSNAME, k)) {
			printf("verify baseboard phy %d failed\n", k);
			failed = 1;
		}
		putc('\b');
		putc(slash[k % 8]);
	}
	for (k = 0; k < 32; ++k) {
		if (verify_gbit_phy(CONFIG_SYS_GBIT_MII1_BUSNAME, k)) {
			printf("verify extensionboard phy %d failed\n", k);
			failed = 1;
		}
		putc('\b');
		putc(slash[k % 8]);
	}
	blank_string(strlen(str_status));

	printf("Starting 64 channels %s\n", failed ? "failed" : "ok");

	return 0;
}

void gd405ex_init(void)
{
	unsigned int k;

	if (i2c_probe(0x22)) { /* i2c_probe returns 0 on success */
		for (k = 0; k < CONFIG_SYS_FPGA_COUNT; ++k)
			gd->fpga_state[k] |= FPGA_STATE_PLATFORM;
	} else {
		pca9698_direction_output(0x22, 39, 1);
	}
}

void gd405ex_set_fpga_reset(unsigned state)
{
	int legacy = get_fpga_state(0) & FPGA_STATE_PLATFORM;

	if (legacy) {
		if (state) {
			out_le16((void *)LATCH0_BASE, CONFIG_SYS_LATCH0_RESET);
			out_le16((void *)LATCH1_BASE, CONFIG_SYS_LATCH1_RESET);
		} else {
			out_le16((void *)LATCH0_BASE, CONFIG_SYS_LATCH0_BOOT);
			out_le16((void *)LATCH1_BASE, CONFIG_SYS_LATCH1_BOOT);
		}
	} else {
		pca9698_set_value(0x22, 39, state ? 0 : 1);
	}
}

void gd405ex_setup_hw(void)
{
	gpio_write_bit(CONFIG_SYS_GPIO_STARTUP_FINISHED_N, 0);
	gpio_write_bit(CONFIG_SYS_GPIO_STARTUP_FINISHED, 1);
}

int gd405ex_get_fpga_done(unsigned fpga)
{
	int legacy = get_fpga_state(0) & FPGA_STATE_PLATFORM;

	if (legacy)
		return in_le16((void *)LATCH3_BASE)
			& CONFIG_SYS_FPGA_DONE(fpga);
	else
		return pca9698_get_value(0x22, fpga ? 9 : 8);
}
