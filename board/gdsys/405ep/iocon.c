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

enum {
	UNITTYPE_MAIN_SERVER = 0,
	UNITTYPE_MAIN_USER = 1,
	UNITTYPE_VIDEO_SERVER = 2,
	UNITTYPE_VIDEO_USER = 3,
};

enum {
	HWVER_100 = 0,
	HWVER_104 = 1,
	HWVER_110 = 2,
};

enum {
	COMPRESSION_NONE = 0,
	COMPRESSION_TYPE1_DELTA,
};

enum {
	AUDIO_NONE = 0,
	AUDIO_TX = 1,
	AUDIO_RX = 2,
	AUDIO_RXTX = 3,
};

enum {
	SYSCLK_147456 = 0,
};

enum {
	RAM_DDR2_32 = 0,
};

/*
 * Check Board Identity:
 */
int checkboard(void)
{
	char buf[64];
	int i = getenv_f("serial#", buf, sizeof(buf));
	ihs_fpga_t *fpga = (ihs_fpga_t *) CONFIG_SYS_FPGA_BASE(0);
	u16 versions = in_le16(&fpga->versions);
	u16 fpga_version = in_le16(&fpga->fpga_version);
	u16 fpga_features = in_le16(&fpga->fpga_features);
	unsigned unit_type;
	unsigned hardware_version;
	unsigned feature_compression;
	unsigned feature_osd;
	unsigned feature_audio;
	unsigned feature_sysclock;
	unsigned feature_ramconfig;
	unsigned feature_carriers;
	unsigned feature_video_channels;

	unit_type = (versions & 0xf000) >> 12;
	hardware_version = versions & 0x000f;
	feature_compression = (fpga_features & 0xe000) >> 13;
	feature_osd = fpga_features & (1<<11);
	feature_audio = (fpga_features & 0x0600) >> 9;
	feature_sysclock = (fpga_features & 0x0180) >> 7;
	feature_ramconfig = (fpga_features & 0x0060) >> 5;
	feature_carriers = (fpga_features & 0x000c) >> 2;
	feature_video_channels = fpga_features & 0x0003;

	printf("Board: ");

	printf("IoCon");

	if (i > 0) {
		puts(", serial# ");
		puts(buf);
	}
	puts("\n       ");

	switch (unit_type) {
	case UNITTYPE_MAIN_USER:
		printf("Mainchannel");
		break;

	case UNITTYPE_VIDEO_USER:
		printf("Videochannel");
		break;

	default:
		printf("UnitType %d(not supported)", unit_type);
		break;
	}

	switch (hardware_version) {
	case HWVER_100:
		printf(" HW-Ver 1.00\n");
		break;

	case HWVER_104:
		printf(" HW-Ver 1.04\n");
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


	switch (feature_compression) {
	case COMPRESSION_NONE:
		printf(" no compression");
		break;

	case COMPRESSION_TYPE1_DELTA:
		printf(" type1-deltacompression");
		break;

	default:
		printf(" compression %d(not supported)", feature_compression);
		break;
	}

	printf(", %sosd", feature_osd ? "" : "no ");

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

	puts(",\n       ");

	switch (feature_sysclock) {
	case SYSCLK_147456:
		printf("clock 147.456 MHz");
		break;

	default:
		printf("clock %d(not supported)", feature_sysclock);
		break;
	}

	switch (feature_ramconfig) {
	case RAM_DDR2_32:
		printf(", RAM 32 bit DDR2");
		break;

	default:
		printf(", RAM %d(not supported)", feature_ramconfig);
		break;
	}

	printf(", %d carrier(s)", feature_carriers);

	printf(", %d video channel(s)\n", feature_video_channels);

	return 0;
}

int last_stage_init(void)
{
	return osd_probe(0);
}

/*
 * provide access to fpga gpios (for I2C bitbang)
 */
void fpga_gpio_set(int pin)
{
	out_le16((void *)(CONFIG_SYS_FPGA0_BASE + 0x18), pin);
}

void fpga_gpio_clear(int pin)
{
	out_le16((void *)(CONFIG_SYS_FPGA0_BASE + 0x16), pin);
}

int fpga_gpio_get(int pin)
{
	return in_le16((void *)(CONFIG_SYS_FPGA0_BASE + 0x14)) & pin;
}
