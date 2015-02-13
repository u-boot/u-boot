/*
 * Xilinx xps_ll_temac ethernet driver for u-boot
 *
 * supports SDMA or FIFO access and MDIO bus communication
 *
 * Copyright (C) 2011 - 2012 Stephan Linz <linz@li-pro.net>
 * Copyright (C) 2008 - 2011 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2008 - 2011 PetaLogix
 *
 * Based on Yoshio Kashiwagi kashiwagi@co-nss.co.jp driver
 * Copyright (C) 2008 Nissin Systems Co.,Ltd.
 * March 2008 created
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * [0]: http://www.xilinx.com/support/documentation
 *
 * [S]:	[0]/ip_documentation/xps_ll_temac.pdf
 * [A]:	[0]/application_notes/xapp1041.pdf
 */

#include <config.h>
#include <common.h>
#include <net.h>
#include <netdev.h>
#include <malloc.h>
#include <asm/io.h>
#include <miiphy.h>

#include "xilinx_ll_temac.h"
#include "xilinx_ll_temac_fifo.h"
#include "xilinx_ll_temac_sdma.h"
#include "xilinx_ll_temac_mdio.h"

#if !defined(CONFIG_MII)
# error "LL_TEMAC requires MII -- missing CONFIG_MII"
#endif

#if !defined(CONFIG_PHYLIB)
# error "LL_TEMAC requires PHYLIB -- missing CONFIG_PHYLIB"
#endif

struct ll_temac_info {
	int			flags;
	unsigned long		base_addr;
	unsigned long		ctrl_addr;
	char			*devname;
	unsigned int		phyaddr;
	char			*mdio_busname;
};

/* Ethernet interface ready status */
int ll_temac_check_status(struct temac_reg *regs, u32 mask)
{
	unsigned timeout = 50;	/* 1usec * 50 = 50usec */

	/*
	 * Quote from LL TEMAC documentation: The bits in the RDY
	 * register are asserted when there is no access in progress.
	 * When an access is in progress, a bit corresponding to the
	 * type of access is automatically de-asserted. The bit is
	 * automatically re-asserted when the access is complete.
	 */
	while (timeout && (!(in_be32(&regs->rdy) & mask))) {
		timeout--;
		udelay(1);
	}

	if (!timeout) {
		printf("%s: Timeout on 0x%08x @%p\n", __func__,
				mask, &regs->rdy);
		return 1;
	}

	return 0;
}

/*
 * Indirect write to ll_temac.
 *
 * http://www.xilinx.com/support/documentation/ip_documentation/xps_ll_temac.pdf
 * page 23, second paragraph, The use of CTL0 register or CTL1 register
 */
int ll_temac_indirect_set(struct temac_reg *regs, u16 regn, u32 reg_data)
{
	out_be32(&regs->lsw, (reg_data & MLSW_MASK));
	out_be32(&regs->ctl, CTL_WEN | (regn & CTL_ADDR_MASK));

	if (ll_temac_check_status(regs, RSE_CFG_WR))
		return 0;

	return 1;
}

/*
 * Indirect read from ll_temac.
 *
 * http://www.xilinx.com/support/documentation/ip_documentation/xps_ll_temac.pdf
 * page 23, second paragraph, The use of CTL0 register or CTL1 register
 */
int ll_temac_indirect_get(struct temac_reg *regs, u16 regn, u32* reg_data)
{
	out_be32(&regs->ctl, (regn & CTL_ADDR_MASK));

	if (ll_temac_check_status(regs, RSE_CFG_RR))
		return 0;

	*reg_data = in_be32(&regs->lsw) & MLSW_MASK;
	return 1;
}

/* setting sub-controller and ll_temac to proper setting */
static int ll_temac_setup_ctrl(struct eth_device *dev)
{
	struct ll_temac *ll_temac = dev->priv;
	struct temac_reg *regs = (struct temac_reg *)dev->iobase;

	if (ll_temac->ctrlreset && ll_temac->ctrlreset(dev))
		return 0;

	if (ll_temac->ctrlinit && ll_temac->ctrlinit(dev))
		return 0;

	/* Promiscuous mode disable */
	if (!ll_temac_indirect_set(regs, TEMAC_AFM, 0))
		return 0;

	/* Enable Receiver - RX bit */
	if (!ll_temac_indirect_set(regs, TEMAC_RCW1, RCW1_RX))
		return 0;

	/* Enable Transmitter - TX bit */
	if (!ll_temac_indirect_set(regs, TEMAC_TC, TC_TX))
		return 0;

	return 1;
}

/*
 * Configure ll_temac based on negotiated speed and duplex
 * reported by PHY handling code
 */
static int ll_temac_adjust_link(struct eth_device *dev)
{
	unsigned int speed, emmc_reg;
	struct temac_reg *regs = (struct temac_reg *)dev->iobase;
	struct ll_temac *ll_temac = dev->priv;
	struct phy_device *phydev = ll_temac->phydev;

	if (!phydev->link) {
		printf("%s: No link.\n", phydev->dev->name);
		return 0;
	}

	switch (phydev->speed) {
	case 1000:
		speed = EMMC_LSPD_1000;
		break;
	case 100:
		speed = EMMC_LSPD_100;
		break;
	case 10:
		speed = EMMC_LSPD_10;
		break;
	default:
		return 0;
	}

	if (!ll_temac_indirect_get(regs, TEMAC_EMMC, &emmc_reg))
		return 0;

	emmc_reg &= ~EMMC_LSPD_MASK;
	emmc_reg |= speed;

	if (!ll_temac_indirect_set(regs, TEMAC_EMMC, emmc_reg))
		return 0;

	printf("%s: PHY is %s with %dbase%s, %s%s\n",
			dev->name, phydev->drv->name,
			phydev->speed, (phydev->port == PORT_TP) ? "T" : "X",
			(phydev->duplex) ? "FDX" : "HDX",
			(phydev->port == PORT_OTHER) ? ", unkown mode" : "");

	return 1;
}

/* setup mac addr */
static int ll_temac_setup_mac_addr(struct eth_device *dev)
{
	struct temac_reg *regs = (struct temac_reg *)dev->iobase;
	u32 val;

	/* set up unicast MAC address filter */
	val = ((dev->enetaddr[3] << 24) | (dev->enetaddr[2] << 16) |
			(dev->enetaddr[1] << 8) | (dev->enetaddr[0]));
	val &= UAW0_UADDR_MASK;

	if (!ll_temac_indirect_set(regs, TEMAC_UAW0, val))
		return 1;

	val = ((dev->enetaddr[5] << 8) | dev->enetaddr[4]);
	val &= UAW1_UADDR_MASK;

	if (!ll_temac_indirect_set(regs, TEMAC_UAW1, val))
		return 1;

	return 0;
}

/* halt device */
static void ll_temac_halt(struct eth_device *dev)
{
	struct ll_temac *ll_temac = dev->priv;
	struct temac_reg *regs = (struct temac_reg *)dev->iobase;

	/* Disable Receiver */
	ll_temac_indirect_set(regs, TEMAC_RCW0, 0);

	/* Disable Transmitter */
	ll_temac_indirect_set(regs, TEMAC_TC, 0);

	if (ll_temac->ctrlhalt)
		ll_temac->ctrlhalt(dev);

	/* Shut down the PHY, as needed */
	phy_shutdown(ll_temac->phydev);
}

static int ll_temac_init(struct eth_device *dev, bd_t *bis)
{
	struct ll_temac *ll_temac = dev->priv;
	int ret;

	printf("%s: Xilinx XPS LocalLink Tri-Mode Ether MAC #%d at 0x%08lx.\n",
		dev->name, dev->index, dev->iobase);

	if (!ll_temac_setup_ctrl(dev))
		return -1;

	/* Start up the PHY */
	ret = phy_startup(ll_temac->phydev);
	if (ret) {
		printf("%s: Could not initialize PHY %s\n",
		       dev->name, ll_temac->phydev->dev->name);
		return ret;
	}

	if (!ll_temac_adjust_link(dev)) {
		ll_temac_halt(dev);
		return -1;
	}

	/* If there's no link, fail */
	return ll_temac->phydev->link ? 0 : -1;
}

/*
 * Discover which PHY is attached to the device, and configure it
 * properly.  If the PHY is not recognized, then return 0
 * (failure).  Otherwise, return 1
 */
static int ll_temac_phy_init(struct eth_device *dev)
{
	struct ll_temac *ll_temac = dev->priv;
	struct phy_device *phydev;
	unsigned int supported = PHY_GBIT_FEATURES;

	/* interface - look at driver/net/tsec.c */
	phydev = phy_connect(ll_temac->bus, ll_temac->phyaddr,
			dev, PHY_INTERFACE_MODE_NONE);

	phydev->supported &= supported;
	phydev->advertising = phydev->supported;

	ll_temac->phydev = phydev;

	phy_config(phydev);

	return 1;
}

/*
 * Initialize a single ll_temac devices
 *
 * Returns the result of ll_temac phy interface that were initialized
 */
int xilinx_ll_temac_initialize(bd_t *bis, struct ll_temac_info *devinf)
{
	struct eth_device *dev;
	struct ll_temac *ll_temac;

	dev = calloc(1, sizeof(*dev));
	if (dev == NULL)
		return 0;

	ll_temac = calloc(1, sizeof(struct ll_temac));
	if (ll_temac == NULL) {
		free(dev);
		return 0;
	}

	/* use given name or generate its own unique name */
	if (devinf->devname) {
		strncpy(dev->name, devinf->devname, sizeof(dev->name));
	} else {
		snprintf(dev->name, sizeof(dev->name), "lltemac.%lx", devinf->base_addr);
		devinf->devname = dev->name;
	}

	dev->iobase = devinf->base_addr;

	dev->priv = ll_temac;
	dev->init = ll_temac_init;
	dev->halt = ll_temac_halt;
	dev->write_hwaddr = ll_temac_setup_mac_addr;

	ll_temac->ctrladdr = devinf->ctrl_addr;
	if (devinf->flags & XILINX_LL_TEMAC_M_SDMA_PLB) {
#if defined(CONFIG_XILINX_440) || defined(CONFIG_XILINX_405)
		if (devinf->flags & XILINX_LL_TEMAC_M_SDMA_DCR) {
			ll_temac_collect_xldcr_sdma_reg_addr(dev);
			ll_temac->in32 = ll_temac_xldcr_in32;
			ll_temac->out32 = ll_temac_xldcr_out32;
		} else
#endif
		{
			ll_temac_collect_xlplb_sdma_reg_addr(dev);
			ll_temac->in32 = ll_temac_xlplb_in32;
			ll_temac->out32 = ll_temac_xlplb_out32;
		}
		ll_temac->ctrlinit = ll_temac_init_sdma;
		ll_temac->ctrlhalt = ll_temac_halt_sdma;
		ll_temac->ctrlreset = ll_temac_reset_sdma;
		dev->recv = ll_temac_recv_sdma;
		dev->send = ll_temac_send_sdma;
	} else {
		ll_temac->in32 = NULL;
		ll_temac->out32 = NULL;
		ll_temac->ctrlinit = NULL;
		ll_temac->ctrlhalt = NULL;
		ll_temac->ctrlreset = ll_temac_reset_fifo;
		dev->recv = ll_temac_recv_fifo;
		dev->send = ll_temac_send_fifo;
	}

	/* Link to specified MDIO bus */
	strncpy(ll_temac->mdio_busname, devinf->mdio_busname, MDIO_NAME_LEN);
	ll_temac->bus = miiphy_get_dev_by_name(ll_temac->mdio_busname);

	/* Looking for a valid PHY address if it is not yet set */
	if (devinf->phyaddr == -1)
		ll_temac->phyaddr = ll_temac_phy_addr(ll_temac->bus);
	else
		ll_temac->phyaddr = devinf->phyaddr;

	eth_register(dev);

	/* Try to initialize PHY here, and return */
	return ll_temac_phy_init(dev);
}

/*
 * Initialize a single ll_temac device with its mdio bus behind ll_temac
 *
 * Returns 1 if the ll_temac device and the mdio bus were initialized
 * otherwise returns 0
 */
int xilinx_ll_temac_eth_init(bd_t *bis, unsigned long base_addr, int flags,
							unsigned long ctrl_addr)
{
	struct ll_temac_info devinf;
	struct ll_temac_mdio_info mdioinf;
	int ret;

	/* prepare the internal driver informations */
	devinf.flags = flags;
	devinf.base_addr = base_addr;
	devinf.ctrl_addr = ctrl_addr;
	devinf.devname = NULL;
	devinf.phyaddr = -1;

	mdioinf.name = devinf.mdio_busname = NULL;
	mdioinf.regs = (struct temac_reg *)devinf.base_addr;

	ret = xilinx_ll_temac_mdio_initialize(bis, &mdioinf);
	if (ret >= 0) {

		/*
		 * If there was no MDIO bus name then take over the
		 * new automaticaly generated by the MDIO init code.
		 */
		if (mdioinf.name != devinf.mdio_busname)
			devinf.mdio_busname = mdioinf.name;

		ret = xilinx_ll_temac_initialize(bis, &devinf);
		if (ret > 0)
			return 1;

	}

	return 0;
}
