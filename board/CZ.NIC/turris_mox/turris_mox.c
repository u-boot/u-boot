// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marek Behun <marek.behun@nic.cz>
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <net.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <clk.h>
#include <dm.h>
#include <env.h>
#include <fdt_support.h>
#include <init.h>
#include <linux/delay.h>
#include <linux/libfdt.h>
#include <linux/string.h>
#include <miiphy.h>
#include <mvebu/comphy.h>
#include <spi.h>

#include "mox_sp.h"

#define MAX_MOX_MODULES		10

#define MOX_MODULE_SFP		0x1
#define MOX_MODULE_PCI		0x2
#define MOX_MODULE_TOPAZ	0x3
#define MOX_MODULE_PERIDOT	0x4
#define MOX_MODULE_USB3		0x5
#define MOX_MODULE_PASSPCI	0x6

#define ARMADA_37XX_NB_GPIO_SEL	(MVEBU_REGISTER(0x13830))
#define ARMADA_37XX_SPI_CTRL	(MVEBU_REGISTER(0x10600))
#define ARMADA_37XX_SPI_CFG	(MVEBU_REGISTER(0x10604))
#define ARMADA_37XX_SPI_DOUT	(MVEBU_REGISTER(0x10608))
#define ARMADA_37XX_SPI_DIN	(MVEBU_REGISTER(0x1060c))

#define ETH1_PATH	"/soc/internal-regs@d0000000/ethernet@40000"
#define MDIO_PATH	"/soc/internal-regs@d0000000/mdio@32004"
#define SFP_GPIO_PATH	"/soc/internal-regs@d0000000/spi@10600/moxtet@1/gpio@0"
#define PCIE_PATH	"/soc/pcie@d0070000"
#define SFP_PATH	"/sfp"

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_OF_BOARD_FIXUP)
int board_fix_fdt(void *blob)
{
	u8 topology[MAX_MOX_MODULES];
	int i, size, node;
	bool enable;

	/*
	 * SPI driver is not loaded in driver model yet, but we have to find out
	 * if pcie should be enabled in U-Boot's device tree. Therefore we have
	 * to read SPI by reading/writing SPI registers directly
	 */

	writel(0x10df, ARMADA_37XX_SPI_CFG);
	/* put pin from GPIO to SPI mode */
	clrbits_le32(ARMADA_37XX_NB_GPIO_SEL, BIT(12));
	/* enable SPI CS1 */
	setbits_le32(ARMADA_37XX_SPI_CTRL, BIT(17));

	while (!(readl(ARMADA_37XX_SPI_CTRL) & 0x2))
		udelay(1);

	for (i = 0; i < MAX_MOX_MODULES; ++i) {
		writel(0x0, ARMADA_37XX_SPI_DOUT);

		while (!(readl(ARMADA_37XX_SPI_CTRL) & 0x2))
			udelay(1);

		topology[i] = readl(ARMADA_37XX_SPI_DIN) & 0xff;
		if (topology[i] == 0xff)
			break;

		topology[i] &= 0xf;
	}

	size = i;

	/* disable SPI CS1 */
	clrbits_le32(ARMADA_37XX_SPI_CTRL, BIT(17));

	if (size > 1 && (topology[1] == MOX_MODULE_PCI ||
			 topology[1] == MOX_MODULE_USB3 ||
			 topology[1] == MOX_MODULE_PASSPCI))
		enable = true;
	else
		enable = false;

	node = fdt_path_offset(blob, PCIE_PATH);

	if (node < 0) {
		printf("Cannot find PCIe node in U-Boot's device tree!\n");
		return 0;
	}

	if (fdt_setprop_string(blob, node, "status",
			       enable ? "okay" : "disabled") < 0) {
		printf("Cannot %s PCIe in U-Boot's device tree!\n",
		       enable ? "enable" : "disable");
		return 0;
	}

	if (a3700_fdt_fix_pcie_regions(blob) < 0) {
		printf("Cannot fix PCIe regions in U-Boot's device tree!\n");
		return 0;
	}

	return 0;
}
#endif

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

static int mox_do_spi(u8 *in, u8 *out, size_t size)
{
	struct spi_slave *slave;
	struct udevice *dev;
	int ret;

	ret = spi_get_bus_and_cs(0, 1, 1000000, SPI_CPHA | SPI_CPOL,
				 "spi_generic_drv", "moxtet@1", &dev,
				 &slave);
	if (ret)
		goto fail;

	ret = spi_claim_bus(slave);
	if (ret)
		goto fail_free;

	ret = spi_xfer(slave, size * 8, out, in, SPI_XFER_ONCE);

	spi_release_bus(slave);
fail_free:
	spi_free_slave(slave);
fail:
	return ret;
}

static int mox_get_topology(const u8 **ptopology, int *psize, int *pis_sd)
{
	static int is_sd;
	static u8 topology[MAX_MOX_MODULES - 1];
	static int size;
	u8 din[MAX_MOX_MODULES], dout[MAX_MOX_MODULES];
	int ret, i;

	if (size) {
		if (ptopology)
			*ptopology = topology;
		if (psize)
			*psize = size;
		if (pis_sd)
			*pis_sd = is_sd;
		return 0;
	}

	memset(din, 0, MAX_MOX_MODULES);
	memset(dout, 0, MAX_MOX_MODULES);

	ret = mox_do_spi(din, dout, MAX_MOX_MODULES);
	if (ret)
		return ret;

	if (din[0] == 0x10)
		is_sd = 1;
	else if (din[0] == 0x00)
		is_sd = 0;
	else
		return -ENODEV;

	for (i = 1; i < MAX_MOX_MODULES && din[i] != 0xff; ++i)
		topology[i - 1] = din[i] & 0xf;
	size = i - 1;

	if (ptopology)
		*ptopology = topology;
	if (psize)
		*psize = size;
	if (pis_sd)
		*pis_sd = is_sd;

	return 0;
}

int comphy_update_map(struct comphy_map *serdes_map, int count)
{
	int ret, i, size, sfpindex = -1, swindex = -1;
	const u8 *topology;

	ret = mox_get_topology(&topology, &size, NULL);
	if (ret)
		return ret;

	for (i = 0; i < size; ++i) {
		if (topology[i] == MOX_MODULE_SFP && sfpindex == -1)
			sfpindex = i;
		else if ((topology[i] == MOX_MODULE_TOPAZ ||
			  topology[i] == MOX_MODULE_PERIDOT) &&
			 swindex == -1)
			swindex = i;
	}

	if (sfpindex >= 0 && swindex >= 0) {
		if (sfpindex < swindex)
			serdes_map[0].speed = PHY_SPEED_1_25G;
		else
			serdes_map[0].speed = PHY_SPEED_3_125G;
	} else if (sfpindex >= 0) {
		serdes_map[0].speed = PHY_SPEED_1_25G;
	} else if (swindex >= 0) {
		serdes_map[0].speed = PHY_SPEED_3_125G;
	}

	return 0;
}

#define SW_SMI_CMD_R(d, r)	(0x9800 | (((d) & 0x1f) << 5) | ((r) & 0x1f))
#define SW_SMI_CMD_W(d, r)	(0x9400 | (((d) & 0x1f) << 5) | ((r) & 0x1f))

static int sw_multi_read(struct mii_dev *bus, int sw, int dev, int reg)
{
	bus->write(bus, sw, 0, 0, SW_SMI_CMD_R(dev, reg));
	mdelay(5);
	return bus->read(bus, sw, 0, 1);
}

static void sw_multi_write(struct mii_dev *bus, int sw, int dev, int reg,
			   u16 val)
{
	bus->write(bus, sw, 0, 1, val);
	bus->write(bus, sw, 0, 0, SW_SMI_CMD_W(dev, reg));
	mdelay(5);
}

static int sw_scratch_read(struct mii_dev *bus, int sw, int reg)
{
	sw_multi_write(bus, sw, 0x1c, 0x1a, (reg & 0x7f) << 8);
	return sw_multi_read(bus, sw, 0x1c, 0x1a) & 0xff;
}

static void sw_led_write(struct mii_dev *bus, int sw, int port, int reg,
			 u16 val)
{
	sw_multi_write(bus, sw, port, 0x16, 0x8000 | ((reg & 7) << 12)
					    | (val & 0x7ff));
}

static void sw_blink_leds(struct mii_dev *bus, int peridot, int topaz)
{
	int i, p;
	struct {
		int port;
		u16 val;
		int wait;
	} regs[] = {
		{ 2, 0xef, 1 }, { 2, 0xfe, 1 }, { 2, 0x33, 0 },
		{ 4, 0xef, 1 }, { 4, 0xfe, 1 }, { 4, 0x33, 0 },
		{ 3, 0xfe, 1 }, { 3, 0xef, 1 }, { 3, 0x33, 0 },
		{ 1, 0xfe, 1 }, { 1, 0xef, 1 }, { 1, 0x33, 0 }
	};

	for (i = 0; i < 12; ++i) {
		for (p = 0; p < peridot; ++p) {
			sw_led_write(bus, 0x10 + p, regs[i].port, 0,
				     regs[i].val);
			sw_led_write(bus, 0x10 + p, regs[i].port + 4, 0,
				     regs[i].val);
		}
		if (topaz) {
			sw_led_write(bus, 0x2, 0x10 + regs[i].port, 0,
				     regs[i].val);
		}

		if (regs[i].wait)
			mdelay(75);
	}
}

static void check_switch_address(struct mii_dev *bus, int addr)
{
	if (sw_scratch_read(bus, addr, 0x70) >> 3 != addr)
		printf("Check of switch MDIO address failed for 0x%02x\n",
		       addr);
}

static int sfp, pci, topaz, peridot, usb, passpci;
static int sfp_pos, peridot_pos[3];
static int module_count;

static int configure_peridots(struct gpio_desc *reset_gpio)
{
	int i, ret;
	u8 dout[MAX_MOX_MODULES];

	memset(dout, 0, MAX_MOX_MODULES);

	/* set addresses of Peridot modules */
	for (i = 0; i < peridot; ++i)
		dout[module_count - peridot_pos[i]] = (~i) & 3;

	/*
	 * if there is a SFP module connected to the last Peridot module, set
	 * the P10_SMODE to 1 for the Peridot module
	 */
	if (sfp)
		dout[module_count - peridot_pos[i - 1]] |= 1 << 3;

	dm_gpio_set_value(reset_gpio, 1);
	mdelay(10);

	ret = mox_do_spi(NULL, dout, module_count + 1);

	mdelay(10);
	dm_gpio_set_value(reset_gpio, 0);

	mdelay(50);

	return ret;
}

static int get_reset_gpio(struct gpio_desc *reset_gpio)
{
	int node;

	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, "cznic,moxtet");
	if (node < 0) {
		printf("Cannot find Moxtet bus device node!\n");
		return -1;
	}

	gpio_request_by_name_nodev(offset_to_ofnode(node), "reset-gpios", 0,
				   reset_gpio, GPIOD_IS_OUT);

	if (!dm_gpio_is_valid(reset_gpio)) {
		printf("Cannot find reset GPIO for Moxtet bus!\n");
		return -1;
	}

	return 0;
}

int misc_init_r(void)
{
	int ret;
	u8 mac1[6], mac2[6];

	ret = mbox_sp_get_board_info(NULL, mac1, mac2, NULL, NULL);
	if (ret < 0) {
		printf("Cannot read data from OTP!\n");
		return 0;
	}

	if (is_valid_ethaddr(mac1) && !env_get("ethaddr"))
		eth_env_set_enetaddr("ethaddr", mac1);

	if (is_valid_ethaddr(mac2) && !env_get("eth1addr"))
		eth_env_set_enetaddr("eth1addr", mac2);

	return 0;
}

static void mox_print_info(void)
{
	int ret, board_version, ram_size;
	u64 serial_number;
	const char *pub_key;

	ret = mbox_sp_get_board_info(&serial_number, NULL, NULL, &board_version,
				     &ram_size);
	if (ret < 0)
		return;

	printf("Turris Mox:\n");
	printf("  Board version: %i\n", board_version);
	printf("  RAM size: %i MiB\n", ram_size);
	printf("  Serial Number: %016llX\n", serial_number);

	pub_key = mox_sp_get_ecdsa_public_key();
	if (pub_key)
		printf("  ECDSA Public Key: %s\n", pub_key);
	else
		printf("Cannot read ECDSA Public Key\n");
}

int last_stage_init(void)
{
	int ret, i;
	const u8 *topology;
	int is_sd;
	struct mii_dev *bus;
	struct gpio_desc reset_gpio = {};

	mox_print_info();

	ret = mox_get_topology(&topology, &module_count, &is_sd);
	if (ret) {
		printf("Cannot read module topology!\n");
		return 0;
	}

	printf("  SD/eMMC version: %s\n", is_sd ? "SD" : "eMMC");

	if (module_count)
		printf("Module Topology:\n");

	for (i = 0; i < module_count; ++i) {
		switch (topology[i]) {
		case MOX_MODULE_SFP:
			printf("% 4i: SFP Module\n", i + 1);
			break;
		case MOX_MODULE_PCI:
			printf("% 4i: Mini-PCIe Module\n", i + 1);
			break;
		case MOX_MODULE_TOPAZ:
			printf("% 4i: Topaz Switch Module (4-port)\n", i + 1);
			break;
		case MOX_MODULE_PERIDOT:
			printf("% 4i: Peridot Switch Module (8-port)\n", i + 1);
			break;
		case MOX_MODULE_USB3:
			printf("% 4i: USB 3.0 Module (4 ports)\n", i + 1);
			break;
		case MOX_MODULE_PASSPCI:
			printf("% 4i: Passthrough Mini-PCIe Module\n", i + 1);
			break;
		default:
			printf("% 4i: unknown (ID %i)\n", i + 1, topology[i]);
		}
	}

	/* now check if modules are connected in supported mode */

	for (i = 0; i < module_count; ++i) {
		switch (topology[i]) {
		case MOX_MODULE_SFP:
			if (sfp) {
				printf("Error: Only one SFP module is supported!\n");
			} else if (topaz) {
				printf("Error: SFP module cannot be connected after Topaz Switch module!\n");
			} else {
				sfp_pos = i;
				++sfp;
			}
			break;
		case MOX_MODULE_PCI:
			if (pci)
				printf("Error: Only one Mini-PCIe module is supported!\n");
			else if (usb)
				printf("Error: Mini-PCIe module cannot come after USB 3.0 module!\n");
			else if (i && (i != 1 || !passpci))
				printf("Error: Mini-PCIe module should be the first connected module or come right after Passthrough Mini-PCIe module!\n");
			else
				++pci;
			break;
		case MOX_MODULE_TOPAZ:
			if (topaz)
				printf("Error: Only one Topaz module is supported!\n");
			else if (peridot >= 3)
				printf("Error: At most two Peridot modules can come before Topaz module!\n");
			else
				++topaz;
			break;
		case MOX_MODULE_PERIDOT:
			if (sfp || topaz) {
				printf("Error: Peridot module must come before SFP or Topaz module!\n");
			} else if (peridot >= 3) {
				printf("Error: At most three Peridot modules are supported!\n");
			} else {
				peridot_pos[peridot] = i;
				++peridot;
			}
			break;
		case MOX_MODULE_USB3:
			if (pci)
				printf("Error: USB 3.0 module cannot come after Mini-PCIe module!\n");
			else if (usb)
				printf("Error: Only one USB 3.0 module is supported!\n");
			else if (i && (i != 1 || !passpci))
				printf("Error: USB 3.0 module should be the first connected module or come right after Passthrough Mini-PCIe module!\n");
			else
				++usb;
			break;
		case MOX_MODULE_PASSPCI:
			if (passpci)
				printf("Error: Only one Passthrough Mini-PCIe module is supported!\n");
			else if (i != 0)
				printf("Error: Passthrough Mini-PCIe module should be the first connected module!\n");
			else
				++passpci;
		}
	}

	/* now configure modules */

	if (get_reset_gpio(&reset_gpio) < 0)
		return 0;

	if (peridot > 0) {
		if (configure_peridots(&reset_gpio) < 0) {
			printf("Cannot configure Peridot modules!\n");
			peridot = 0;
		}
	} else {
		dm_gpio_set_value(&reset_gpio, 1);
		mdelay(50);
		dm_gpio_set_value(&reset_gpio, 0);
		mdelay(50);
	}

	if (peridot || topaz) {
		/*
		 * now check if the addresses are set by reading Scratch & Misc
		 * register 0x70 of Peridot (and potentially Topaz) modules
		 */

		bus = miiphy_get_dev_by_name("neta@30000");
		if (!bus) {
			printf("Cannot get MDIO bus device!\n");
		} else {
			for (i = 0; i < peridot; ++i)
				check_switch_address(bus, 0x10 + i);

			if (topaz)
				check_switch_address(bus, 0x2);

			sw_blink_leds(bus, peridot, topaz);
		}
	}

	printf("\n");

	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)

static int vnode_by_path(void *blob, const char *fmt, va_list ap)
{
	char path[128];

	vsnprintf(path, 128, fmt, ap);
	return fdt_path_offset(blob, path);
}

static int node_by_path(void *blob, const char *fmt, ...)
{
	va_list ap;
	int res;

	va_start(ap, fmt);
	res = vnode_by_path(blob, fmt, ap);
	va_end(ap);

	return res;
}

static int phandle_by_path(void *blob, const char *fmt, ...)
{
	va_list ap;
	int node, phandle, res;

	va_start(ap, fmt);
	node = vnode_by_path(blob, fmt, ap);
	va_end(ap);

	if (node < 0)
		return node;

	phandle = fdt_get_phandle(blob, node);
	if (phandle > 0)
		return phandle;

	phandle = fdt_get_max_phandle(blob);
	if (phandle < 0)
		return phandle;

	phandle += 1;

	res = fdt_setprop_u32(blob, node, "linux,phandle", phandle);
	if (res < 0)
		return res;

	res = fdt_setprop_u32(blob, node, "phandle", phandle);
	if (res < 0)
		return res;

	return phandle;
}

static int enable_by_path(void *blob, const char *fmt, ...)
{
	va_list ap;
	int node;

	va_start(ap, fmt);
	node = vnode_by_path(blob, fmt, ap);
	va_end(ap);

	if (node < 0)
		return node;

	return fdt_setprop_string(blob, node, "status", "okay");
}

static bool is_topaz(int id)
{
	return topaz && id == peridot + topaz - 1;
}

static int switch_addr(int id)
{
	return is_topaz(id) ? 0x2 : 0x10 + id;
}

static int setup_switch(void *blob, int id)
{
	int res, addr, i, node, phandle;

	addr = switch_addr(id);

	/* first enable the switch by setting status = "okay" */
	res = enable_by_path(blob, MDIO_PATH "/switch%i@%x", id, addr);
	if (res < 0)
		return res;

	/*
	 * now if there are more switches or a SFP module coming after,
	 * enable corresponding ports
	 */
	if (id < peridot + topaz - 1) {
		res = enable_by_path(blob,
				     MDIO_PATH "/switch%i@%x/ports/port@a",
				     id, addr);
	} else if (id == peridot - 1 && !topaz && sfp) {
		res = enable_by_path(blob,
				     MDIO_PATH "/switch%i@%x/ports/port-sfp@a",
				     id, addr);
	} else {
		res = 0;
	}
	if (res < 0)
		return res;

	if (id >= peridot + topaz - 1)
		return 0;

	/* finally change link property if needed */
	node = node_by_path(blob, MDIO_PATH "/switch%i@%x/ports/port@a", id,
			    addr);
	if (node < 0)
		return node;

	for (i = id + 1; i < peridot + topaz; ++i) {
		phandle = phandle_by_path(blob,
					  MDIO_PATH "/switch%i@%x/ports/port@%x",
					  i, switch_addr(i),
					  is_topaz(i) ? 5 : 9);
		if (phandle < 0)
			return phandle;

		if (i == id + 1)
			res = fdt_setprop_u32(blob, node, "link", phandle);
		else
			res = fdt_appendprop_u32(blob, node, "link", phandle);
		if (res < 0)
			return res;
	}

	return 0;
}

static int remove_disabled_nodes(void *blob)
{
	while (1) {
		int res, offset;

		offset = fdt_node_offset_by_prop_value(blob, -1, "status",
						       "disabled", 9);
		if (offset < 0)
			break;

		res = fdt_del_node(blob, offset);
		if (res < 0)
			return res;
	}

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	int node, phandle, res;

	/*
	 * If MOX B (PCI), MOX F (USB) or MOX G (Passthrough PCI) modules are
	 * connected, enable the PCIe node.
	 */
	if (pci || usb || passpci) {
		node = fdt_path_offset(blob, PCIE_PATH);
		if (node < 0)
			return node;

		res = fdt_setprop_string(blob, node, "status", "okay");
		if (res < 0)
			return res;

		/* Fix PCIe regions for devices with 4 GB RAM */
		res = a3700_fdt_fix_pcie_regions(blob);
		if (res < 0)
			return res;
	}

	/*
	 * If MOX C (Topaz switch) and/or MOX E (Peridot switch) are connected,
	 * enable the eth1 node and setup the switches.
	 */
	if (peridot || topaz) {
		int i;

		res = enable_by_path(blob, ETH1_PATH);
		if (res < 0)
			return res;

		for (i = 0; i < peridot + topaz; ++i) {
			res = setup_switch(blob, i);
			if (res < 0)
				return res;
		}
	}

	/*
	 * If MOX D (SFP cage module) is connected, enable the SFP node and eth1
	 * node. If there is no Peridot switch between MOX A and MOX D, add link
	 * to the SFP node to eth1 node.
	 * Also enable and configure SFP GPIO controller node.
	 */
	if (sfp) {
		res = enable_by_path(blob, SFP_PATH);
		if (res < 0)
			return res;

		res = enable_by_path(blob, ETH1_PATH);
		if (res < 0)
			return res;

		if (!peridot) {
			phandle = phandle_by_path(blob, SFP_PATH);
			if (phandle < 0)
				return res;

			node = node_by_path(blob, ETH1_PATH);
			if (node < 0)
				return node;

			res = fdt_setprop_u32(blob, node, "sfp", phandle);
			if (res < 0)
				return res;

			res = fdt_setprop_string(blob, node, "phy-mode",
						 "sgmii");
			if (res < 0)
				return res;
		}

		res = enable_by_path(blob, SFP_GPIO_PATH);
		if (res < 0)
			return res;

		if (sfp_pos) {
			char newname[16];

			/* moxtet-sfp is on non-zero position, change default */
			node = node_by_path(blob, SFP_GPIO_PATH);
			if (node < 0)
				return node;

			res = fdt_setprop_u32(blob, node, "reg", sfp_pos);
			if (res < 0)
				return res;

			sprintf(newname, "gpio@%x", sfp_pos);

			res = fdt_set_name(blob, node, newname);
			if (res < 0)
				return res;
		}
	}

	fdt_fixup_ethernet(blob);

	/* Finally remove disabled nodes, as per Rob Herring's request. */
	remove_disabled_nodes(blob);

	return 0;
}

#endif
