// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#include <common.h>

#include <gdsys_fpga.h>

enum {
	UNITTYPE_MAIN_SERVER = 0,
	UNITTYPE_MAIN_USER = 1,
	UNITTYPE_VIDEO_SERVER = 2,
	UNITTYPE_VIDEO_USER = 3,
};

enum {
	UNITTYPEPCB_DVI = 0,
	UNITTYPEPCB_DP_165 = 1,
	UNITTYPEPCB_DP_300 = 2,
	UNITTYPEPCB_HDMI = 3,
};

enum {
	COMPRESSION_NONE = 0,
	COMPRESSION_TYPE_1 = 1,
	COMPRESSION_TYPE_1_2 = 3,
	COMPRESSION_TYPE_1_2_3 = 7,
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
	RAM_DDR3_32 = 1,
	RAM_DDR3_48 = 2,
};

enum {
	CARRIER_SPEED_1G = 0,
	CARRIER_SPEED_2_5G = 1,
};

bool ioep_fpga_has_osd(unsigned int fpga)
{
	u16 fpga_features;
	unsigned feature_osd;

	FPGA_GET_REG(0, fpga_features, &fpga_features);
	feature_osd = fpga_features & (1<<11);

	return feature_osd;
}

void ioep_fpga_print_info(unsigned int fpga)
{
	u16 versions;
	u16 fpga_version;
	u16 fpga_features;
	unsigned unit_type;
	unsigned unit_type_pcb_video;
	unsigned feature_compression;
	unsigned feature_osd;
	unsigned feature_audio;
	unsigned feature_sysclock;
	unsigned feature_ramconfig;
	unsigned feature_carrier_speed;
	unsigned feature_carriers;
	unsigned feature_video_channels;

	FPGA_GET_REG(fpga, versions, &versions);
	FPGA_GET_REG(fpga, fpga_version, &fpga_version);
	FPGA_GET_REG(fpga, fpga_features, &fpga_features);

	unit_type = (versions & 0xf000) >> 12;
	unit_type_pcb_video = (versions & 0x01c0) >> 6;
	feature_compression = (fpga_features & 0xe000) >> 13;
	feature_osd = fpga_features & (1<<11);
	feature_audio = (fpga_features & 0x0600) >> 9;
	feature_sysclock = (fpga_features & 0x0180) >> 7;
	feature_ramconfig = (fpga_features & 0x0060) >> 5;
	feature_carrier_speed = fpga_features & (1<<4);
	feature_carriers = (fpga_features & 0x000c) >> 2;
	feature_video_channels = fpga_features & 0x0003;

	switch (unit_type) {
	case UNITTYPE_MAIN_SERVER:
	case UNITTYPE_MAIN_USER:
		printf("Mainchannel");
		break;

	case UNITTYPE_VIDEO_SERVER:
	case UNITTYPE_VIDEO_USER:
		printf("Videochannel");
		break;

	default:
		printf("UnitType %d(not supported)", unit_type);
		break;
	}

	switch (unit_type) {
	case UNITTYPE_MAIN_SERVER:
	case UNITTYPE_VIDEO_SERVER:
		printf(" Server");
		if (versions & (1<<4))
			printf(" UC");
		break;

	case UNITTYPE_MAIN_USER:
	case UNITTYPE_VIDEO_USER:
		printf(" User");
		break;

	default:
		break;
	}

	if (versions & (1<<5))
		printf(" Fiber");
	else
		printf(" CAT");

	switch (unit_type_pcb_video) {
	case UNITTYPEPCB_DVI:
		printf(" DVI,");
		break;

	case UNITTYPEPCB_DP_165:
		printf(" DP 165MPix/s,");
		break;

	case UNITTYPEPCB_DP_300:
		printf(" DP 300MPix/s,");
		break;

	case UNITTYPEPCB_HDMI:
		printf(" HDMI,");
		break;
	}

	printf(" FPGA V %d.%02d\n       features:",
	       fpga_version / 100, fpga_version % 100);


	switch (feature_compression) {
	case COMPRESSION_NONE:
		printf(" no compression");
		break;

	case COMPRESSION_TYPE_1:
		printf(" compression type1(delta)");
		break;

	case COMPRESSION_TYPE_1_2:
		printf(" compression type1(delta), type2(inline)");
		break;

	case COMPRESSION_TYPE_1_2_3:
		printf(" compression type1(delta), type2(inline), type3(intempo)");
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

	case RAM_DDR3_32:
		printf(", RAM 32 bit DDR3");
		break;

	case RAM_DDR3_48:
		printf(", RAM 48 bit DDR3");
		break;

	default:
		printf(", RAM %d(not supported)", feature_ramconfig);
		break;
	}

	printf(", %d carrier(s) %s", feature_carriers,
	       feature_carrier_speed ? "2.5Gbit/s" : "1Gbit/s");

	printf(", %d video channel(s)\n", feature_video_channels);
}
