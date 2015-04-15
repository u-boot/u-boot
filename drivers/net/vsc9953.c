/*
 *  Copyright 2014 Freescale Semiconductor, Inc.
 *
 *  SPDX-License-Identifier:      GPL-2.0+
 *
 *  Driver for the Vitesse VSC9953 L2 Switch
 */

#include <asm/io.h>
#include <asm/fsl_serdes.h>
#include <fm_eth.h>
#include <asm/fsl_memac.h>
#include <vsc9953.h>

static struct vsc9953_info vsc9953_l2sw = {
		.port[0] = VSC9953_PORT_INFO_INITIALIZER(0),
		.port[1] = VSC9953_PORT_INFO_INITIALIZER(1),
		.port[2] = VSC9953_PORT_INFO_INITIALIZER(2),
		.port[3] = VSC9953_PORT_INFO_INITIALIZER(3),
		.port[4] = VSC9953_PORT_INFO_INITIALIZER(4),
		.port[5] = VSC9953_PORT_INFO_INITIALIZER(5),
		.port[6] = VSC9953_PORT_INFO_INITIALIZER(6),
		.port[7] = VSC9953_PORT_INFO_INITIALIZER(7),
		.port[8] = VSC9953_PORT_INFO_INITIALIZER(8),
		.port[9] = VSC9953_PORT_INFO_INITIALIZER(9),
};

void vsc9953_port_info_set_mdio(int port, struct mii_dev *bus)
{
	if (!VSC9953_PORT_CHECK(port))
		return;

	vsc9953_l2sw.port[port].bus = bus;
}

void vsc9953_port_info_set_phy_address(int port, int address)
{
	if (!VSC9953_PORT_CHECK(port))
		return;

	vsc9953_l2sw.port[port].phyaddr = address;
}

void vsc9953_port_info_set_phy_int(int port, phy_interface_t phy_int)
{
	if (!VSC9953_PORT_CHECK(port))
		return;

	vsc9953_l2sw.port[port].enet_if = phy_int;
}

void vsc9953_port_enable(int port)
{
	if (!VSC9953_PORT_CHECK(port))
		return;

	vsc9953_l2sw.port[port].enabled = 1;
}

void vsc9953_port_disable(int port)
{
	if (!VSC9953_PORT_CHECK(port))
		return;

	vsc9953_l2sw.port[port].enabled = 0;
}

static void vsc9953_mdio_write(struct vsc9953_mii_mng *phyregs, int port_addr,
		int regnum, int value)
{
	int			timeout = 50000;

	out_le32(&phyregs->miimcmd, (0x1 << 31) | ((port_addr & 0x1f) << 25) |
			((regnum & 0x1f) << 20) | ((value & 0xffff) << 4) |
			(0x1 << 1));
	asm("sync");

	while ((in_le32(&phyregs->miimstatus) & 0x8) && --timeout)
		udelay(1);

	if (timeout == 0)
		debug("Timeout waiting for MDIO write\n");
}

static int vsc9953_mdio_read(struct vsc9953_mii_mng *phyregs, int port_addr,
		int regnum)
{
	int			value = 0xFFFF;
	int			timeout = 50000;

	while ((in_le32(&phyregs->miimstatus) & MIIMIND_OPR_PEND) && --timeout)
		udelay(1);
	if (timeout == 0) {
		debug("Timeout waiting for MDIO operation to finish\n");
		return value;
	}

	/* Put the address of the phy, and the register
	 * number into MIICMD
	 */
	out_le32(&phyregs->miimcmd, (0x1 << 31) | ((port_addr & 0x1f) << 25) |
			((regnum & 0x1f) << 20) | ((value & 0xffff) << 4) |
			(0x2 << 1));

	timeout = 50000;
	/* Wait for the the indication that the read is done */
	while ((in_le32(&phyregs->miimstatus) & 0x8) && --timeout)
		udelay(1);
	if (timeout == 0)
		debug("Timeout waiting for MDIO read\n");

	/* Grab the value read from the PHY */
	value = in_le32(&phyregs->miimdata);

	if ((value & 0x00030000) == 0)
		return value & 0x0000ffff;

	return value;
}

static int init_phy(struct eth_device *dev)
{
	struct vsc9953_port_info	*l2sw_port = dev->priv;
	struct phy_device		*phydev = NULL;

#ifdef CONFIG_PHYLIB
	if (!l2sw_port->bus)
		return 0;
	phydev = phy_connect(l2sw_port->bus, l2sw_port->phyaddr, dev,
			l2sw_port->enet_if);
	if (!phydev) {
		printf("Failed to connect\n");
		return -1;
	}

	phydev->supported &= SUPPORTED_10baseT_Half |
			SUPPORTED_10baseT_Full |
			SUPPORTED_100baseT_Half |
			SUPPORTED_100baseT_Full |
			SUPPORTED_1000baseT_Full;
	phydev->advertising = phydev->supported;

	l2sw_port->phydev = phydev;

	phy_config(phydev);
#endif

	return 0;
}

static int vsc9953_port_init(int port)
{
	struct eth_device		*dev;

	/* Internal ports never have a PHY */
	if (VSC9953_INTERNAL_PORT_CHECK(port))
		return 0;

	/* alloc eth device */
	dev = (struct eth_device *)calloc(1, sizeof(struct eth_device));
	if (!dev)
		return 1;

	sprintf(dev->name, "SW@PORT%d", port);
	dev->priv = &vsc9953_l2sw.port[port];
	dev->init = NULL;
	dev->halt = NULL;
	dev->send = NULL;
	dev->recv = NULL;

	if (init_phy(dev)) {
		free(dev);
		return 1;
	}

	return 0;
}

void vsc9953_init(bd_t *bis)
{
	u32				i, hdx_cfg = 0, phy_addr = 0;
	int				timeout;
	struct vsc9953_system_reg	*l2sys_reg;
	struct vsc9953_qsys_reg		*l2qsys_reg;
	struct vsc9953_dev_gmii		*l2dev_gmii_reg;
	struct vsc9953_analyzer		*l2ana_reg;
	struct vsc9953_devcpu_gcb	*l2dev_gcb;

	l2dev_gmii_reg = (struct vsc9953_dev_gmii *)(VSC9953_OFFSET +
			VSC9953_DEV_GMII_OFFSET);

	l2ana_reg = (struct vsc9953_analyzer *)(VSC9953_OFFSET +
			VSC9953_ANA_OFFSET);

	l2sys_reg = (struct vsc9953_system_reg *)(VSC9953_OFFSET +
			VSC9953_SYS_OFFSET);

	l2qsys_reg = (struct vsc9953_qsys_reg *)(VSC9953_OFFSET +
			VSC9953_QSYS_OFFSET);

	l2dev_gcb = (struct vsc9953_devcpu_gcb *)(VSC9953_OFFSET +
			VSC9953_DEVCPU_GCB);

	out_le32(&l2dev_gcb->chip_regs.soft_rst,
		 CONFIG_VSC9953_SOFT_SWC_RST_ENA);
	timeout = 50000;
	while ((in_le32(&l2dev_gcb->chip_regs.soft_rst) &
			CONFIG_VSC9953_SOFT_SWC_RST_ENA) && --timeout)
		udelay(1); /* busy wait for vsc9953 soft reset */
	if (timeout == 0)
		debug("Timeout waiting for VSC9953 to reset\n");

	out_le32(&l2sys_reg->sys.reset_cfg, CONFIG_VSC9953_MEM_ENABLE |
		 CONFIG_VSC9953_MEM_INIT);

	timeout = 50000;
	while ((in_le32(&l2sys_reg->sys.reset_cfg) &
		CONFIG_VSC9953_MEM_INIT) && --timeout)
		udelay(1); /* busy wait for vsc9953 memory init */
	if (timeout == 0)
		debug("Timeout waiting for VSC9953 memory to initialize\n");

	out_le32(&l2sys_reg->sys.reset_cfg, (in_le32(&l2sys_reg->sys.reset_cfg)
			| CONFIG_VSC9953_CORE_ENABLE));

	/* VSC9953 Setting to be done once only */
	out_le32(&l2qsys_reg->sys.ext_cpu_cfg, 0x00000b00);

	for (i = 0; i < VSC9953_MAX_PORTS; i++) {
		if (vsc9953_port_init(i))
			printf("Failed to initialize l2switch port %d\n", i);

		/* Enable VSC9953 GMII Ports Port ID 0 - 7 */
		if (VSC9953_INTERNAL_PORT_CHECK(i)) {
			out_le32(&l2ana_reg->pfc[i].pfc_cfg,
				 CONFIG_VSC9953_PFC_FC_QSGMII);
			out_le32(&l2sys_reg->pause_cfg.mac_fc_cfg[i],
				 CONFIG_VSC9953_MAC_FC_CFG_QSGMII);
		} else {
			out_le32(&l2ana_reg->pfc[i].pfc_cfg,
				 CONFIG_VSC9953_PFC_FC);
			out_le32(&l2sys_reg->pause_cfg.mac_fc_cfg[i],
				 CONFIG_VSC9953_MAC_FC_CFG);
		}
		out_le32(&l2dev_gmii_reg->port_mode.clock_cfg,
			 CONFIG_VSC9953_CLOCK_CFG);
		out_le32(&l2dev_gmii_reg->mac_cfg_status.mac_ena_cfg,
			 CONFIG_VSC9953_MAC_ENA_CFG);
		out_le32(&l2dev_gmii_reg->mac_cfg_status.mac_mode_cfg,
			 CONFIG_VSC9953_MAC_MODE_CFG);
		out_le32(&l2dev_gmii_reg->mac_cfg_status.mac_ifg_cfg,
			 CONFIG_VSC9953_MAC_IFG_CFG);
		/* mac_hdx_cfg varies with port id*/
		hdx_cfg = (CONFIG_VSC9953_MAC_HDX_CFG | (i << 16));
		out_le32(&l2dev_gmii_reg->mac_cfg_status.mac_hdx_cfg, hdx_cfg);
		out_le32(&l2sys_reg->sys.front_port_mode[i],
			 CONFIG_VSC9953_FRONT_PORT_MODE);
		out_le32(&l2qsys_reg->sys.switch_port_mode[i],
			 CONFIG_VSC9953_PORT_ENA);
		out_le32(&l2dev_gmii_reg->mac_cfg_status.mac_maxlen_cfg,
			 CONFIG_VSC9953_MAC_MAX_LEN);
		out_le32(&l2sys_reg->pause_cfg.pause_cfg[i],
			 CONFIG_VSC9953_PAUSE_CFG);
		/* WAIT FOR 2 us*/
		udelay(2);

		l2dev_gmii_reg = (struct vsc9953_dev_gmii *)(
				(char *)l2dev_gmii_reg
				+ T1040_SWITCH_GMII_DEV_OFFSET);

		/* Initialize Lynx PHY Wrappers */
		phy_addr = 0;
		if (vsc9953_l2sw.port[i].enet_if ==
				PHY_INTERFACE_MODE_QSGMII)
			phy_addr = (i + 0x4) & 0x1F;
		else if (vsc9953_l2sw.port[i].enet_if ==
				PHY_INTERFACE_MODE_SGMII)
			phy_addr = (i + 1) & 0x1F;

		if (phy_addr) {
			/* SGMII IF mode + AN enable */
			vsc9953_mdio_write(&l2dev_gcb->mii_mng[0], phy_addr,
					   0x14, PHY_SGMII_IF_MODE_AN |
					   PHY_SGMII_IF_MODE_SGMII);
			/* Dev ability according to SGMII specification */
			vsc9953_mdio_write(&l2dev_gcb->mii_mng[0], phy_addr,
					   0x4, PHY_SGMII_DEV_ABILITY_SGMII);
			/* Adjust link timer for SGMII
			 * 1.6 ms in units of 8 ns = 2 * 10^5 = 0x30d40
			 */
			vsc9953_mdio_write(&l2dev_gcb->mii_mng[0], phy_addr,
					   0x13, 0x0003);
			vsc9953_mdio_write(&l2dev_gcb->mii_mng[0], phy_addr,
					   0x12, 0x0d40);
			/* Restart AN */
			vsc9953_mdio_write(&l2dev_gcb->mii_mng[0], phy_addr,
					   0x0, PHY_SGMII_CR_DEF_VAL |
					   PHY_SGMII_CR_RESET_AN);

			timeout = 50000;
			while ((vsc9953_mdio_read(&l2dev_gcb->mii_mng[0],
					phy_addr, 0x01) & 0x0020) && --timeout)
				udelay(1); /* wait for AN to complete */
			if (timeout == 0)
				debug("Timeout waiting for AN to complete\n");
		}
	}

	printf("VSC9953 L2 switch initialized\n");
	return;
}

#ifdef CONFIG_VSC9953_CMD
/* Enable/disable status of a VSC9953 port */
static void vsc9953_port_status_set(int port_nr, u8 enabled)
{
	u32			val;
	struct vsc9953_qsys_reg	*l2qsys_reg;

	/* Administrative down */
	if (vsc9953_l2sw.port[port_nr].enabled == 0)
		return;

	l2qsys_reg = (struct vsc9953_qsys_reg *)(VSC9953_OFFSET +
			VSC9953_QSYS_OFFSET);

	val = in_le32(&l2qsys_reg->sys.switch_port_mode[port_nr]);
	if (enabled == 1)
		val |= (1 << 13);
	else
		val &= ~(1 << 13);

	out_le32(&l2qsys_reg->sys.switch_port_mode[port_nr], val);
}

/* Set all VSC9953 ports' status */
static void vsc9953_port_all_status_set(u8 enabled)
{
	int		i;

	for (i = 0; i < VSC9953_MAX_PORTS; i++)
		vsc9953_port_status_set(i, enabled);
}

/* Start autonegotiation for a VSC9953 PHY */
static void vsc9953_phy_autoneg(int port_nr)
{
	if (!vsc9953_l2sw.port[port_nr].phydev)
		return;

	if (vsc9953_l2sw.port[port_nr].phydev->drv->startup(
			vsc9953_l2sw.port[port_nr].phydev))
		printf("Failed to start PHY for port %d\n", port_nr);
}

/* Start autonegotiation for all VSC9953 PHYs */
static void vsc9953_phy_all_autoneg(void)
{
	int		i;

	for (i = 0; i < VSC9953_MAX_PORTS; i++)
		vsc9953_phy_autoneg(i);
}

/* Print a VSC9953 port's configuration */
static void vsc9953_port_config_show(int port)
{
	int			speed;
	int			duplex;
	int			link;
	u8			enabled;
	u32			val;
	struct vsc9953_qsys_reg	*l2qsys_reg;

	l2qsys_reg = (struct vsc9953_qsys_reg *)(VSC9953_OFFSET +
			VSC9953_QSYS_OFFSET);

	val = in_le32(&l2qsys_reg->sys.switch_port_mode[port]);
	enabled = vsc9953_l2sw.port[port].enabled &
			((val & 0x00002000) >> 13);

	/* internal ports (8 and 9) are fixed */
	if (VSC9953_INTERNAL_PORT_CHECK(port)) {
		link = 1;
		speed = SPEED_2500;
		duplex = DUPLEX_FULL;
	} else {
		if (vsc9953_l2sw.port[port].phydev) {
			link = vsc9953_l2sw.port[port].phydev->link;
			speed = vsc9953_l2sw.port[port].phydev->speed;
			duplex = vsc9953_l2sw.port[port].phydev->duplex;
		} else {
			link = -1;
			speed = -1;
			duplex = -1;
		}
	}

	printf("%8d ", port);
	printf("%8s ", enabled == 1 ? "enabled" : "disabled");
	printf("%8s ", link == 1 ? "up" : "down");

	switch (speed) {
	case SPEED_10:
		printf("%8d ", 10);
		break;
	case SPEED_100:
		printf("%8d ", 100);
		break;
	case SPEED_1000:
		printf("%8d ", 1000);
		break;
	case SPEED_2500:
		printf("%8d ", 2500);
		break;
	case SPEED_10000:
		printf("%8d ", 10000);
		break;
	default:
		printf("%8s ", "-");
	}

	printf("%8s\n", duplex == DUPLEX_FULL ? "full" : "half");
}

/* Print VSC9953 ports' configuration */
static void vsc9953_port_all_config_show(void)
{
	int		i;

	for (i = 0; i < VSC9953_MAX_PORTS; i++)
		vsc9953_port_config_show(i);
}

/* function to interpret commands starting with "ethsw " */
static int do_ethsw(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u8 enable;
	u32 port;

	if (argc < 4)
		return -1;

	if (strcmp(argv[1], "port"))
		return -1;

	if (!strcmp(argv[3], "show")) {
		if (!strcmp(argv[2], "all")) {
			vsc9953_phy_all_autoneg();
			printf("%8s %8s %8s %8s %8s\n",
			       "Port", "Status", "Link", "Speed",
			       "Duplex");
			vsc9953_port_all_config_show();
			return 0;
		} else {
			port = simple_strtoul(argv[2], NULL, 10);
			if (!VSC9953_PORT_CHECK(port))
				return -1;
			vsc9953_phy_autoneg(port);
			printf("%8s %8s %8s %8s %8s\n",
			       "Port", "Status", "Link", "Speed",
			       "Duplex");
			vsc9953_port_config_show(port);
			return 0;
		}
	} else if (!strcmp(argv[3], "enable")) {
		enable = 1;
	} else if (!strcmp(argv[3], "disable")) {
		enable = 0;
	} else {
		return -1;
	}

	if (!strcmp(argv[2], "all")) {
		vsc9953_port_all_status_set(enable);
		return 0;
	} else {
		port = simple_strtoul(argv[2], NULL, 10);
		if (!VSC9953_PORT_CHECK(port))
			return -1;
		vsc9953_port_status_set(port, enable);
		return 0;
	}

	return -1;
}

U_BOOT_CMD(ethsw, 5, 0, do_ethsw,
	   "vsc9953 l2 switch commands",
	   "port <port_nr> enable|disable\n"
	   "    - enable/disable an l2 switch port\n"
	   "      port_nr=0..9; use \"all\" for all ports\n"
	   "ethsw port <port_nr> show\n"
	   "    - show an l2 switch port's configuration\n"
	   "      port_nr=0..9; use \"all\" for all ports\n"
);
#endif /* CONFIG_VSC9953_CMD */
