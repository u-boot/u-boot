/*
 * (C) Copyright 2010
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

#include <gdsys_fpga.h>

#include "../common/osd.h"

#define LATCH2_BASE (CONFIG_SYS_LATCH_BASE + 0x200)
#define LATCH2_MC2_PRESENT_N 0x0080

#define LATCH3_BASE (CONFIG_SYS_LATCH_BASE + 0x300)

enum {
	UNITTYPE_VIDEO_USER = 0,
	UNITTYPE_MAIN_USER = 1,
	UNITTYPE_VIDEO_SERVER = 2,
	UNITTYPE_MAIN_SERVER = 3,
};

enum {
	HWVER_101 = 0,
	HWVER_110 = 1,
};

enum {
	AUDIO_NONE = 0,
	AUDIO_TX = 1,
	AUDIO_RX = 2,
	AUDIO_RXTX = 3,
};

enum {
	SYSCLK_156250 = 2,
};

enum {
	RAM_NONE = 0,
	RAM_DDR2_32 = 1,
	RAM_DDR2_64 = 2,
};

static unsigned int get_hwver(void)
{
	u16 latch3 = in_le16((void *)LATCH3_BASE);

	return latch3 & 0x0003;
}

static unsigned int get_mc2_present(void)
{
	u16 latch2 = in_le16((void *)LATCH2_BASE);

	return !(latch2 & LATCH2_MC2_PRESENT_N);
}

static void print_fpga_info(unsigned dev)
{
	ihs_fpga_t *fpga = (ihs_fpga_t *) CONFIG_SYS_FPGA_BASE(dev);
	u16 versions = in_le16(&fpga->versions);
	u16 fpga_version = in_le16(&fpga->fpga_version);
	u16 fpga_features = in_le16(&fpga->fpga_features);
	unsigned unit_type;
	unsigned hardware_version;
	unsigned feature_rs232;
	unsigned feature_audio;
	unsigned feature_sysclock;
	unsigned feature_ramconfig;
	unsigned feature_carrier_speed;
	unsigned feature_carriers;
	unsigned feature_video_channels;
	int fpga_state = get_fpga_state(dev);

	printf("FPGA%d: ", dev);

	hardware_version = versions & 0x000f;

	if (fpga_state
	    && !((hardware_version == HWVER_101)
		 && (fpga_state == FPGA_STATE_DONE_FAILED))) {
		puts("not available\n");
		print_fpga_state(dev);
		return;
	}

	unit_type = (versions >> 4) & 0x000f;
	hardware_version = versions & 0x000f;
	feature_rs232 = fpga_features & (1<<11);
	feature_audio = (fpga_features >> 9) & 0x0003;
	feature_sysclock = (fpga_features >> 7) & 0x0003;
	feature_ramconfig = (fpga_features >> 5) & 0x0003;
	feature_carrier_speed = fpga_features & (1<<4);
	feature_carriers = (fpga_features >> 2) & 0x0003;
	feature_video_channels = fpga_features & 0x0003;

	switch (unit_type) {
	case UNITTYPE_VIDEO_USER:
		printf("Videochannel Userside");
		break;

	case UNITTYPE_MAIN_USER:
		printf("Mainchannel Userside");
		break;

	case UNITTYPE_VIDEO_SERVER:
		printf("Videochannel Serverside");
		break;

	case UNITTYPE_MAIN_SERVER:
		printf("Mainchannel Serverside");
		break;

	default:
		printf("UnitType %d(not supported)", unit_type);
		break;
	}

	switch (hardware_version) {
	case HWVER_101:
		printf(" HW-Ver 1.01\n");
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

	printf(" %sRS232", feature_rs232 ? "" : "no ");

	switch (feature_audio) {
	case AUDIO_NONE:
		printf(", no audio");
		break;

	case AUDIO_TX:
		printf(", audio tx");
		break;

	case AUDIO_RX:
		printf(", audio rx");
		break;

	case AUDIO_RXTX:
		printf(", audio rx+tx");
		break;

	default:
		printf(", audio %d(not supported)", feature_audio);
		break;
	}

	switch (feature_sysclock) {
	case SYSCLK_156250:
		printf(", clock 156.25 MHz");
		break;

	default:
		printf(", clock %d(not supported)", feature_sysclock);
		break;
	}

	puts(",\n       ");

	switch (feature_ramconfig) {
	case RAM_NONE:
		printf("no RAM");
		break;

	case RAM_DDR2_32:
		printf("RAM 32 bit DDR2");
		break;

	case RAM_DDR2_64:
		printf("RAM 64 bit DDR2");
		break;

	default:
		printf("RAM %d(not supported)", feature_ramconfig);
		break;
	}

	printf(", %d carrier(s) %s", feature_carriers,
		feature_carrier_speed ? "10 Gbit/s" : "of unknown speed");

	printf(", %d video channel(s)\n", feature_video_channels);
}

/*
 * Check Board Identity:
 */
int checkboard(void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));

	printf("Board: ");

	printf("DLVision 10G");

	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}

	puts("\n");

	print_fpga_info(0);
	if (get_mc2_present())
		print_fpga_info(1);

	return 0;
}

int last_stage_init(void)
{
	ihs_fpga_t *fpga = (ihs_fpga_t *) CONFIG_SYS_FPGA_BASE(0);
	u16 versions = in_le16(&fpga->versions);

	if (((versions >> 4) & 0x000f) != UNITTYPE_MAIN_USER)
		return 0;

	if (!get_fpga_state(0) || (get_hwver() == HWVER_101))
		osd_probe(0);

	if (get_mc2_present() &&
	    (!get_fpga_state(1) || (get_hwver() == HWVER_101)))
		osd_probe(1);

	return 0;
}
