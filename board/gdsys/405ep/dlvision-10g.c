/*
 * (C) Copyright 2010
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/ppc4xx-gpio.h>
#include <dtt.h>

#include "405ep.h"
#include <gdsys_fpga.h>

#include "../common/osd.h"

#define LATCH0_BASE (CONFIG_SYS_LATCH_BASE)
#define LATCH1_BASE (CONFIG_SYS_LATCH_BASE + 0x100)
#define LATCH2_BASE (CONFIG_SYS_LATCH_BASE + 0x200)
#define LATCH3_BASE (CONFIG_SYS_LATCH_BASE + 0x300)

#define LATCH2_MC2_PRESENT_N 0x0080

enum {
	UNITTYPE_MAIN = 1<<0,
	UNITTYPE_SERVER = 1<<1,
	UNITTYPE_DISPLAYPORT = 1<<2,
};

enum {
	HWVER_101 = 0,
	HWVER_110 = 1,
	HWVER_130 = 2,
	HWVER_140 = 3,
	HWVER_150 = 4,
	HWVER_160 = 5,
	HWVER_170 = 6,
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

struct ihs_fpga *fpga_ptr[] = CONFIG_SYS_FPGA_PTR;

int misc_init_r(void)
{
	/* startup fans */
	dtt_init();

	return 0;
}

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
	u16 versions;
	u16 fpga_version;
	u16 fpga_features;
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

	FPGA_GET_REG(dev, versions, &versions);
	FPGA_GET_REG(dev, fpga_version, &fpga_version);
	FPGA_GET_REG(dev, fpga_features, &fpga_features);

	hardware_version = versions & 0x000f;

	if (fpga_state
	    && !((hardware_version == HWVER_101)
		 && (fpga_state == FPGA_STATE_DONE_FAILED))) {
		puts("not available\n");
		if (fpga_state & FPGA_STATE_DONE_FAILED)
			puts("       Waiting for FPGA-DONE timed out.\n");
		if (fpga_state & FPGA_STATE_REFLECTION_FAILED)
			puts("       FPGA reflection test failed.\n");
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

	if (unit_type & UNITTYPE_MAIN)
		printf("Mainchannel ");
	else
		printf("Videochannel ");

	if (unit_type & UNITTYPE_SERVER)
		printf("Serverside ");
	else
		printf("Userside ");

	if (unit_type & UNITTYPE_DISPLAYPORT)
		printf("DisplayPort");
	else
		printf("DVI-DL");

	switch (hardware_version) {
	case HWVER_101:
		printf(" HW-Ver 1.01\n");
		break;

	case HWVER_110:
		printf(" HW-Ver 1.10-1.20\n");
		break;

	case HWVER_130:
		printf(" HW-Ver 1.30\n");
		break;

	case HWVER_140:
		printf(" HW-Ver 1.40-1.43\n");
		break;

	case HWVER_150:
		printf(" HW-Ver 1.50\n");
		break;

	case HWVER_160:
		printf(" HW-Ver 1.60-1.61\n");
		break;

	case HWVER_170:
		printf(" HW-Ver 1.70\n");
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
	char *s = getenv("serial#");

	puts("Board: ");

	puts("DLVision 10G");

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}

	puts("\n");

	return 0;
}

int last_stage_init(void)
{
	u16 versions;

	FPGA_GET_REG(0, versions, &versions);

	print_fpga_info(0);
	if (get_mc2_present())
		print_fpga_info(1);

	if (((versions >> 4) & 0x000f) & UNITTYPE_SERVER)
		return 0;

	if (!get_fpga_state(0) || (get_hwver() == HWVER_101))
		osd_probe(0);

	if (get_mc2_present() &&
	    (!get_fpga_state(1) || (get_hwver() == HWVER_101)))
		osd_probe(1);

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
	return in_le16((void *)LATCH2_BASE) & CONFIG_SYS_FPGA_DONE(fpga);
}
