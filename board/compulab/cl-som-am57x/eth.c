// SPDX-License-Identifier: GPL-2.0+
/*
 * Ethernet specific code for CompuLab CL-SOM-AM57x module
 *
 * (C) Copyright 2016 CompuLab, Ltd. http://compulab.co.il/
 *
 * Author: Uri Mashiach <uri.mashiach@compulab.co.il>
 */

#include <common.h>
#include <cpsw.h>
#include <environment.h>
#include <miiphy.h>
#include <asm/gpio.h>
#include <asm/arch/sys_proto.h>
#include "../common/eeprom.h"

static void cpsw_control(int enabled)
{
	/* VTP can be added here */
}

static struct cpsw_slave_data cl_som_am57x_cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 0,
		.phy_if         = PHY_INTERFACE_MODE_RMII,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_addr	= 1,
		.phy_if         = PHY_INTERFACE_MODE_RMII,

	},
};

static struct cpsw_platform_data cl_som_am57_cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 2,
	.slave_data		= cl_som_am57x_cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};

/*
 * cl_som_am57x_efuse_read_mac_addr() - read Ethernet port MAC address.
 *       The information is retrieved from the SOC's registers.
 * @buff: read buffer.
 * @port_num: port number.
 */
static void cl_som_am57x_efuse_read_mac_addr(uchar *buff, uint port_num)
{
	uint32_t mac_hi, mac_lo;

	if (port_num) {
		mac_lo = readl((*ctrl)->control_core_mac_id_1_lo);
		mac_hi = readl((*ctrl)->control_core_mac_id_1_hi);
	} else {
		mac_lo = readl((*ctrl)->control_core_mac_id_0_lo);
		mac_hi = readl((*ctrl)->control_core_mac_id_0_hi);
	}

	buff[0] = (mac_hi & 0xFF0000) >> 16;
	buff[1] = (mac_hi & 0xFF00) >> 8;
	buff[2] = mac_hi & 0xFF;
	buff[3] = (mac_lo & 0xFF0000) >> 16;
	buff[4] = (mac_lo & 0xFF00) >> 8;
	buff[5] = mac_lo & 0xFF;
}

/*
 * cl_som_am57x_handle_mac_address() - set MAC address in the U-Boot
 *	environment.
 *      The address is retrieved retrieved from an EEPROM field or from the
 *	SOC's registers.
 * @env_name: U-Boot environment name.
 * @field_name: EEPROM field name.
 * @port_num: SOC's port number.
 */
static int cl_som_am57x_handle_mac_address(char *env_name, uint port_num)
{
	int ret;
	uint8_t enetaddr[6];

	ret = eth_env_get_enetaddr(env_name, enetaddr);
	if (ret)
		return 0;

	ret = cl_eeprom_read_mac_addr(enetaddr, CONFIG_SYS_I2C_EEPROM_BUS);

	if (ret || !is_valid_ethaddr(enetaddr))
		cl_som_am57x_efuse_read_mac_addr(enetaddr, port_num);

	if (!is_valid_ethaddr(enetaddr))
		return -1;

	ret = eth_env_set_enetaddr(env_name, enetaddr);
	if (ret)
		printf("cl-som-am57x: Failed to set Eth port %d MAC address\n",
		       port_num);

	return ret;
}

#define CL_SOM_AM57X_PHY_ADDR2			0x01
#define AR8033_PHY_DEBUG_ADDR_REG		0x1d
#define AR8033_PHY_DEBUG_DATA_REG		0x1e
#define AR8033_DEBUG_RGMII_RX_CLK_DLY_REG	0x00
#define AR8033_DEBUG_RGMII_TX_CLK_DLY_REG	0x05
#define AR8033_DEBUG_RGMII_RX_CLK_DLY_MASK	(1 << 15)
#define AR8033_DEBUG_RGMII_TX_CLK_DLY_MASK	(1 << 8)

/*
 * cl_som_am57x_rgmii_clk_delay() - Set RGMII clock delay.
 *	Enable RX delay, disable TX delay.
 */
static void cl_som_am57x_rgmii_clk_delay(void)
{
	uint16_t mii_reg_val;
	const char *devname;

	devname = miiphy_get_current_dev();
	/* PHY 2 */
	miiphy_write(devname, CL_SOM_AM57X_PHY_ADDR2, AR8033_PHY_DEBUG_ADDR_REG,
		     AR8033_DEBUG_RGMII_RX_CLK_DLY_REG);
	miiphy_read(devname, CL_SOM_AM57X_PHY_ADDR2, AR8033_PHY_DEBUG_DATA_REG,
		    &mii_reg_val);
	mii_reg_val |= AR8033_DEBUG_RGMII_RX_CLK_DLY_MASK;
	miiphy_write(devname, CL_SOM_AM57X_PHY_ADDR2, AR8033_PHY_DEBUG_DATA_REG,
		     mii_reg_val);

	miiphy_write(devname, CL_SOM_AM57X_PHY_ADDR2, AR8033_PHY_DEBUG_ADDR_REG,
		     AR8033_DEBUG_RGMII_TX_CLK_DLY_REG);
	miiphy_read(devname, CL_SOM_AM57X_PHY_ADDR2, AR8033_PHY_DEBUG_DATA_REG,
		    &mii_reg_val);
	mii_reg_val &= ~AR8033_DEBUG_RGMII_TX_CLK_DLY_MASK;
	miiphy_write(devname, CL_SOM_AM57X_PHY_ADDR2, AR8033_PHY_DEBUG_DATA_REG,
		     mii_reg_val);
}

#define CL_SOM_AM57X_GPIO_PHY1_RST 92 /* GPIO3_28 */
#define CL_SOM_AM57X_RGMII_PORT1 1

int board_eth_init(bd_t *bis)
{
	int ret;
	uint32_t ctrl_val;
	char *cpsw_phy_envval;
	int cpsw_act_phy = 1;

	/* SB-SOM-AM57x primary Eth (P21) is routed to RGMII1 */
	ret = cl_som_am57x_handle_mac_address("ethaddr",
					      CL_SOM_AM57X_RGMII_PORT1);

	if (ret)
		return -1;

	/* Select RGMII for GMII1_SEL */
	ctrl_val = readl((*ctrl)->control_core_control_io1) & (~0x33);
	ctrl_val |= 0x22;
	writel(ctrl_val, (*ctrl)->control_core_control_io1);
	mdelay(10);

	gpio_request(CL_SOM_AM57X_GPIO_PHY1_RST, "phy1_rst");
	gpio_direction_output(CL_SOM_AM57X_GPIO_PHY1_RST, 0);
	mdelay(20);

	gpio_set_value(CL_SOM_AM57X_GPIO_PHY1_RST, 1);
	mdelay(20);

	cpsw_phy_envval = env_get("cpsw_phy");
	if (cpsw_phy_envval != NULL)
		cpsw_act_phy = simple_strtoul(cpsw_phy_envval, NULL, 0);

	cl_som_am57_cpsw_data.active_slave = cpsw_act_phy;

	ret = cpsw_register(&cl_som_am57_cpsw_data);
	if (ret < 0)
		printf("Error %d registering CPSW switch\n", ret);

	/* Set RGMII clock delay */
	cl_som_am57x_rgmii_clk_delay();

	return ret;
}
