/*
 * (C) Copyright 2011
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
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
#include <command.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/ppc4xx-gpio.h>
#include <dtt.h>

#include "405ep.h"
#include <gdsys_fpga.h>

#define LATCH0_BASE (CONFIG_SYS_LATCH_BASE)
#define LATCH1_BASE (CONFIG_SYS_LATCH_BASE + 0x100)
#define LATCH2_BASE (CONFIG_SYS_LATCH_BASE + 0x200)

enum {
	UNITTYPE_CCX16 = 1,
	UNITTYPE_CCIP216 = 2,
};

enum {
	HWVER_300 = 3,
};

int misc_init_r(void)
{
	/* startup fans */
	dtt_init();

	return 0;
}

int checkboard(void)
{
	char *s = getenv("serial#");

	puts("Board: CATCenter Neo");

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}

	puts("\n");

	return 0;
}

static void print_fpga_info(void)
{
	struct ihs_fpga *fpga = (struct ihs_fpga *) CONFIG_SYS_FPGA_BASE(0);
	u16 versions = in_le16(&fpga->versions);
	u16 fpga_version = in_le16(&fpga->fpga_version);
	u16 fpga_features = in_le16(&fpga->fpga_features);
	int fpga_state = get_fpga_state(0);
	unsigned unit_type;
	unsigned hardware_version;
	unsigned feature_channels;

	puts("FPGA:  ");
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

	switch (unit_type) {
	case UNITTYPE_CCX16:
		printf("CCX-Switch");
		break;

	default:
		printf("UnitType %d(not supported)", unit_type);
		break;
	}

	switch (hardware_version) {
	case HWVER_300:
		printf(" HW-Ver 3.00-3.12\n");
		break;

	default:
		printf(" HW-Ver %d(not supported)\n",
		       hardware_version);
		break;
	}

	printf("       FPGA V %d.%02d, features:",
		fpga_version / 100, fpga_version % 100);

	printf(" %d channel(s)\n", feature_channels);
}

int last_stage_init(void)
{
	print_fpga_info();

	return 0;
}

void gd405ep_init(void)
{
}

void gd405ep_set_fpga_reset(unsigned state)
{
	if (state) {
		out_le16((void *)LATCH0_BASE, CONFIG_SYS_LATCH0_RESET);
		out_le16((void *)LATCH1_BASE, CONFIG_SYS_LATCH1_RESET);
	} else {
		out_le16((void *)LATCH0_BASE, CONFIG_SYS_LATCH0_BOOT);
		out_le16((void *)LATCH1_BASE, CONFIG_SYS_LATCH1_BOOT);
	}
}

void gd405ep_setup_hw(void)
{
	/*
	 * set "startup-finished"-gpios
	 */
	gpio_write_bit(21, 0);
	gpio_write_bit(22, 1);
}

int gd405ep_get_fpga_done(unsigned fpga)
{
	/*
	 * Neo hardware has no FPGA-DONE GPIO
	 */
	return 1;
}
