/*
 * (C) Copyright 2014
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <hwconfig.h>
#include <i2c.h>
#include <spi.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <pci.h>
#include <mpc83xx.h>
#include <fsl_esdhc.h>
#include <asm/io.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_mpc83xx_serdes.h>

#include "mpc8308.h"

#include <gdsys_fpga.h>

#include "../common/osd.h"
#include "../common/mclink.h"
#include "../common/phy.h"

#include <pca953x.h>
#include <pca9698.h>

#include <miiphy.h>

DECLARE_GLOBAL_DATA_PTR;

#define MAX_MUX_CHANNELS 2

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
	HWVER_100 = 0,
	HWVER_110 = 1,
};

enum {
	FPGA_HWVER_200 = 0,
	FPGA_HWVER_210 = 1,
};

enum {
	COMPRESSION_NONE = 0,
	COMPRESSION_TYPE1_DELTA = 1,
	COMPRESSION_TYPE1_TYPE2_DELTA = 3,
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
};

enum {
	CARRIER_SPEED_1G = 0,
	CARRIER_SPEED_2_5G = 1,
};

enum {
	MCFPGA_DONE = 1 << 0,
	MCFPGA_INIT_N = 1 << 1,
	MCFPGA_PROGRAM_N = 1 << 2,
	MCFPGA_UPDATE_ENABLE_N = 1 << 3,
	MCFPGA_RESET_N = 1 << 4,
};

enum {
	GPIO_MDC = 1 << 14,
	GPIO_MDIO = 1 << 15,
};

unsigned int mclink_fpgacount;
struct ihs_fpga *fpga_ptr[] = CONFIG_SYS_FPGA_PTR;

int fpga_set_reg(u32 fpga, u16 *reg, off_t regoff, u16 data)
{
	int res;

	switch (fpga) {
	case 0:
		out_le16(reg, data);
		break;
	default:
		res = mclink_send(fpga - 1, regoff, data);
		if (res < 0) {
			printf("mclink_send reg %02lx data %04x returned %d\n",
			       regoff, data, res);
			return res;
		}
		break;
	}

	return 0;
}

int fpga_get_reg(u32 fpga, u16 *reg, off_t regoff, u16 *data)
{
	int res;

	switch (fpga) {
	case 0:
		*data = in_le16(reg);
		break;
	default:
		if (fpga > mclink_fpgacount)
			return -EINVAL;
		res = mclink_receive(fpga - 1, regoff, data);
		if (res < 0) {
			printf("mclink_receive reg %02lx returned %d\n",
			       regoff, res);
			return res;
		}
	}

	return 0;
}

int checkboard(void)
{
	char *s = getenv("serial#");
	bool hw_type_cat = pca9698_get_value(0x20, 20);

	puts("Board: ");

	printf("HRCon %s", hw_type_cat ? "CAT" : "Fiber");

	if (s != NULL) {
		puts(", serial# ");
		puts(s);
	}

	puts("\n");

	return 0;
}

static void print_fpga_info(unsigned int fpga, bool rgmii2_present)
{
	u16 versions;
	u16 fpga_version;
	u16 fpga_features;
	unsigned unit_type;
	unsigned unit_type_pcb_video;
	unsigned hardware_version;
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

	if (unit_type == UNITTYPE_MAIN_USER) {
		hardware_version =
			  (!!pca9698_get_value(0x20, 24) << 0)
			| (!!pca9698_get_value(0x20, 25) << 1)
			| (!!pca9698_get_value(0x20, 26) << 2)
			| (!!pca9698_get_value(0x20, 27) << 3)
			| (!!pca9698_get_value(0x20, 28) << 4);
		switch (hardware_version) {
		case HWVER_100:
			printf(" HW-Ver 1.00,");
			break;

		case HWVER_110:
			printf(" HW-Ver 1.10,");
			break;

		default:
			printf(" HW-Ver %d(not supported),",
			       hardware_version);
			break;
		}
		if (rgmii2_present)
			printf(" RGMII2,");
	}

	if (unit_type == UNITTYPE_VIDEO_USER) {
		hardware_version = versions & 0x000f;
		switch (hardware_version) {
		case FPGA_HWVER_200:
			printf(" HW-Ver 2.00,");
			break;

		case FPGA_HWVER_210:
			printf(" HW-Ver 2.10,");
			break;

		default:
			printf(" HW-Ver %d(not supported),",
			       hardware_version);
			break;
		}
	}

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

	case COMPRESSION_TYPE1_DELTA:
		printf(" type1-deltacompression");
		break;

	case COMPRESSION_TYPE1_TYPE2_DELTA:
		printf(" type1-deltacompression, type2-inlinecompression");
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

	default:
		printf(", RAM %d(not supported)", feature_ramconfig);
		break;
	}

	printf(", %d carrier(s) %s", feature_carriers,
	       feature_carrier_speed ? "2.5Gbit/s" : "1Gbit/s");

	printf(", %d video channel(s)\n", feature_video_channels);
}

int last_stage_init(void)
{
	int slaves;
	unsigned int k;
	unsigned int mux_ch;
	unsigned char mclink_controllers[] = { 0x24, 0x25, 0x26 };
	u16 fpga_features;
	bool hw_type_cat = pca9698_get_value(0x20, 20);
	bool ch0_rgmii2_present = false;

	FPGA_GET_REG(0, fpga_features, &fpga_features);

	/* Turn on Parade DP501 */
	pca9698_direction_output(0x20, 10, 1);

	ch0_rgmii2_present = !pca9698_get_value(0x20, 30);

	/* wait for FPGA done */
	for (k = 0; k < ARRAY_SIZE(mclink_controllers); ++k) {
		unsigned int ctr = 0;

		if (i2c_probe(mclink_controllers[k]))
			continue;

		while (!(pca953x_get_val(mclink_controllers[k])
		       & MCFPGA_DONE)) {
			udelay(100000);
			if (ctr++ > 5) {
				printf("no done for mclink_controller %d\n", k);
				break;
			}
		}
	}

	if (hw_type_cat) {
		miiphy_register(bb_miiphy_buses[0].name, bb_miiphy_read,
				bb_miiphy_write);
		for (mux_ch = 0; mux_ch < MAX_MUX_CHANNELS; ++mux_ch) {
			if ((mux_ch == 1) && !ch0_rgmii2_present)
				continue;

			setup_88e1514(bb_miiphy_buses[0].name, mux_ch);
		}
	}

	/* give slave-PLLs and Parade DP501 some time to be up and running */
	udelay(500000);

	mclink_fpgacount = CONFIG_SYS_MCLINK_MAX;
	slaves = mclink_probe();
	mclink_fpgacount = 0;

	print_fpga_info(0, ch0_rgmii2_present);
	osd_probe(0);

	if (slaves <= 0)
		return 0;

	mclink_fpgacount = slaves;

	for (k = 1; k <= slaves; ++k) {
		FPGA_GET_REG(k, fpga_features, &fpga_features);

		print_fpga_info(k, false);
		osd_probe(k);
		if (hw_type_cat) {
			miiphy_register(bb_miiphy_buses[k].name,
					bb_miiphy_read, bb_miiphy_write);
			setup_88e1514(bb_miiphy_buses[k].name, 0);
		}
	}

	return 0;
}

/*
 * provide access to fpga gpios (for I2C bitbang)
 * (these may look all too simple but make iocon.h much more readable)
 */
void fpga_gpio_set(unsigned int bus, int pin)
{
	FPGA_SET_REG(bus, gpio.set, pin);
}

void fpga_gpio_clear(unsigned int bus, int pin)
{
	FPGA_SET_REG(bus, gpio.clear, pin);
}

int fpga_gpio_get(unsigned int bus, int pin)
{
	u16 val;

	FPGA_GET_REG(bus, gpio.read, &val);

	return val & pin;
}

void mpc8308_init(void)
{
	pca9698_direction_output(0x20, 4, 1);
}

void mpc8308_set_fpga_reset(unsigned state)
{
	pca9698_set_value(0x20, 4, state ? 0 : 1);
}

void mpc8308_setup_hw(void)
{
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;

	/*
	 * set "startup-finished"-gpios
	 */
	setbits_be32(&immr->gpio[0].dir, (1 << (31-11)) | (1 << (31-12)));
	setbits_be32(&immr->gpio[0].dat, 1 << (31-12));
}

int mpc8308_get_fpga_done(unsigned fpga)
{
	return pca9698_get_value(0x20, 19);
}

#ifdef CONFIG_FSL_ESDHC
int board_mmc_init(bd_t *bd)
{
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	sysconf83xx_t *sysconf = &immr->sysconf;

	/* Enable cache snooping in eSDHC system configuration register */
	out_be32(&sysconf->sdhccr, 0x02000000);

	return fsl_esdhc_mmc_init(bd);
}
#endif

static struct pci_region pcie_regions_0[] = {
	{
		.bus_start = CONFIG_SYS_PCIE1_MEM_BASE,
		.phys_start = CONFIG_SYS_PCIE1_MEM_PHYS,
		.size = CONFIG_SYS_PCIE1_MEM_SIZE,
		.flags = PCI_REGION_MEM,
	},
	{
		.bus_start = CONFIG_SYS_PCIE1_IO_BASE,
		.phys_start = CONFIG_SYS_PCIE1_IO_PHYS,
		.size = CONFIG_SYS_PCIE1_IO_SIZE,
		.flags = PCI_REGION_IO,
	},
};

void pci_init_board(void)
{
	immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	sysconf83xx_t *sysconf = &immr->sysconf;
	law83xx_t *pcie_law = sysconf->pcielaw;
	struct pci_region *pcie_reg[] = { pcie_regions_0 };

	fsl_setup_serdes(CONFIG_FSL_SERDES1, FSL_SERDES_PROTO_PEX,
			 FSL_SERDES_CLK_100, FSL_SERDES_VDD_1V);

	/* Deassert the resets in the control register */
	out_be32(&sysconf->pecr1, 0xE0008000);
	udelay(2000);

	/* Configure PCI Express Local Access Windows */
	out_be32(&pcie_law[0].bar, CONFIG_SYS_PCIE1_BASE & LAWBAR_BAR);
	out_be32(&pcie_law[0].ar, LBLAWAR_EN | LBLAWAR_512MB);

	mpc83xx_pcie_init(1, pcie_reg);
}

ulong board_flash_get_legacy(ulong base, int banknum, flash_info_t *info)
{
	info->portwidth = FLASH_CFI_16BIT;
	info->chipwidth = FLASH_CFI_BY16;
	info->interface = FLASH_CFI_X16;
	return 1;
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	fdt_fixup_dr_usb(blob, bd);
	fdt_fixup_esdhc(blob, bd);

	return 0;
}
#endif

/*
 * FPGA MII bitbang implementation
 */

struct fpga_mii {
	unsigned fpga;
	int mdio;
} fpga_mii[] = {
	{ 0, 1},
	{ 1, 1},
	{ 2, 1},
	{ 3, 1},
};

static int mii_dummy_init(struct bb_miiphy_bus *bus)
{
	return 0;
}

static int mii_mdio_active(struct bb_miiphy_bus *bus)
{
	struct fpga_mii *fpga_mii = bus->priv;

	if (fpga_mii->mdio)
		FPGA_SET_REG(fpga_mii->fpga, gpio.set, GPIO_MDIO);
	else
		FPGA_SET_REG(fpga_mii->fpga, gpio.clear, GPIO_MDIO);

	return 0;
}

static int mii_mdio_tristate(struct bb_miiphy_bus *bus)
{
	struct fpga_mii *fpga_mii = bus->priv;

	FPGA_SET_REG(fpga_mii->fpga, gpio.set, GPIO_MDIO);

	return 0;
}

static int mii_set_mdio(struct bb_miiphy_bus *bus, int v)
{
	struct fpga_mii *fpga_mii = bus->priv;

	if (v)
		FPGA_SET_REG(fpga_mii->fpga, gpio.set, GPIO_MDIO);
	else
		FPGA_SET_REG(fpga_mii->fpga, gpio.clear, GPIO_MDIO);

	fpga_mii->mdio = v;

	return 0;
}

static int mii_get_mdio(struct bb_miiphy_bus *bus, int *v)
{
	u16 gpio;
	struct fpga_mii *fpga_mii = bus->priv;

	FPGA_GET_REG(fpga_mii->fpga, gpio.read, &gpio);

	*v = ((gpio & GPIO_MDIO) != 0);

	return 0;
}

static int mii_set_mdc(struct bb_miiphy_bus *bus, int v)
{
	struct fpga_mii *fpga_mii = bus->priv;

	if (v)
		FPGA_SET_REG(fpga_mii->fpga, gpio.set, GPIO_MDC);
	else
		FPGA_SET_REG(fpga_mii->fpga, gpio.clear, GPIO_MDC);

	return 0;
}

static int mii_delay(struct bb_miiphy_bus *bus)
{
	udelay(1);

	return 0;
}

struct bb_miiphy_bus bb_miiphy_buses[] = {
	{
		.name = "board0",
		.init = mii_dummy_init,
		.mdio_active = mii_mdio_active,
		.mdio_tristate = mii_mdio_tristate,
		.set_mdio = mii_set_mdio,
		.get_mdio = mii_get_mdio,
		.set_mdc = mii_set_mdc,
		.delay = mii_delay,
		.priv = &fpga_mii[0],
	},
	{
		.name = "board1",
		.init = mii_dummy_init,
		.mdio_active = mii_mdio_active,
		.mdio_tristate = mii_mdio_tristate,
		.set_mdio = mii_set_mdio,
		.get_mdio = mii_get_mdio,
		.set_mdc = mii_set_mdc,
		.delay = mii_delay,
		.priv = &fpga_mii[1],
	},
	{
		.name = "board2",
		.init = mii_dummy_init,
		.mdio_active = mii_mdio_active,
		.mdio_tristate = mii_mdio_tristate,
		.set_mdio = mii_set_mdio,
		.get_mdio = mii_get_mdio,
		.set_mdc = mii_set_mdc,
		.delay = mii_delay,
		.priv = &fpga_mii[2],
	},
	{
		.name = "board3",
		.init = mii_dummy_init,
		.mdio_active = mii_mdio_active,
		.mdio_tristate = mii_mdio_tristate,
		.set_mdio = mii_set_mdio,
		.get_mdio = mii_get_mdio,
		.set_mdc = mii_set_mdc,
		.delay = mii_delay,
		.priv = &fpga_mii[3],
	},
};

int bb_miiphy_buses_num = sizeof(bb_miiphy_buses) /
			  sizeof(bb_miiphy_buses[0]);
