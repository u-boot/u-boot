// SPDX-License-Identifier: GPL-2.0+
/*
 * ENETC ethernet controller driver
 * Copyright 2017-2019 NXP
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <memalign.h>
#include <asm/io.h>
#include <pci.h>
#include <miiphy.h>

#include "fsl_enetc.h"

/*
 * Bind the device:
 * - set a more explicit name on the interface
 */
static int enetc_bind(struct udevice *dev)
{
	char name[16];
	static int eth_num_devices;

	/*
	 * prefer using PCI function numbers to number interfaces, but these
	 * are only available if dts nodes are present.  For PCI they are
	 * optional, handle that case too.  Just in case some nodes are present
	 * and some are not, use different naming scheme - enetc-N based on
	 * PCI function # and enetc#N based on interface count
	 */
	if (ofnode_valid(dev->node))
		sprintf(name, "enetc-%u", PCI_FUNC(pci_get_devfn(dev)));
	else
		sprintf(name, "enetc#%u", eth_num_devices++);
	device_set_name(dev, name);

	return 0;
}

/* MDIO wrappers, we're using these to drive internal MDIO to get to serdes */
static int enetc_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct enetc_mdio_priv priv;

	priv.regs_base = bus->priv;
	return enetc_mdio_read_priv(&priv, addr, devad, reg);
}

static int enetc_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			    u16 val)
{
	struct enetc_mdio_priv priv;

	priv.regs_base = bus->priv;
	return enetc_mdio_write_priv(&priv, addr, devad, reg, val);
}

/* only interfaces that can pin out through serdes have internal MDIO */
static bool enetc_has_imdio(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);

	return !!(priv->imdio.priv);
}

/* set up serdes for SGMII */
static int enetc_init_sgmii(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);

	if (!enetc_has_imdio(dev))
		return 0;

	/* Set to SGMII mode, use AN */
	enetc_mdio_write(&priv->imdio, ENETC_PCS_PHY_ADDR, MDIO_DEVAD_NONE,
			 ENETC_PCS_IF_MODE, ENETC_PCS_IF_MODE_SGMII_AN);

	/* Dev ability - SGMII */
	enetc_mdio_write(&priv->imdio, ENETC_PCS_PHY_ADDR, MDIO_DEVAD_NONE,
			 ENETC_PCS_DEV_ABILITY, ENETC_PCS_DEV_ABILITY_SGMII);

	/* Adjust link timer for SGMII */
	enetc_mdio_write(&priv->imdio, ENETC_PCS_PHY_ADDR, MDIO_DEVAD_NONE,
			 ENETC_PCS_LINK_TIMER1, ENETC_PCS_LINK_TIMER1_VAL);
	enetc_mdio_write(&priv->imdio, ENETC_PCS_PHY_ADDR, MDIO_DEVAD_NONE,
			 ENETC_PCS_LINK_TIMER2, ENETC_PCS_LINK_TIMER2_VAL);

	/* restart PCS AN */
	enetc_mdio_write(&priv->imdio, ENETC_PCS_PHY_ADDR, MDIO_DEVAD_NONE,
			 ENETC_PCS_CR,
			 ENETC_PCS_CR_RESET_AN | ENETC_PCS_CR_DEF_VAL);

	return 0;
}

/* set up MAC for RGMII */
static int enetc_init_rgmii(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	u32 if_mode;

	/* enable RGMII AN */
	if_mode = enetc_read_port(priv, ENETC_PM_IF_MODE);
	if_mode |= ENETC_PM_IF_MODE_AN_ENA;
	enetc_write_port(priv, ENETC_PM_IF_MODE, if_mode);

	return 0;
}

/* set up MAC and serdes for SXGMII */
static int enetc_init_sxgmii(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	u32 if_mode;

	/* set ifmode to (US)XGMII */
	if_mode = enetc_read_port(priv, ENETC_PM_IF_MODE);
	if_mode &= ~ENETC_PM_IF_IFMODE_MASK;
	enetc_write_port(priv, ENETC_PM_IF_MODE, if_mode);

	if (!enetc_has_imdio(dev))
		return 0;

	/* Dev ability - SXGMII */
	enetc_mdio_write(&priv->imdio, ENETC_PCS_PHY_ADDR, ENETC_PCS_DEVAD_REPL,
			 ENETC_PCS_DEV_ABILITY, ENETC_PCS_DEV_ABILITY_SXGMII);

	/* Restart PCS AN */
	enetc_mdio_write(&priv->imdio, ENETC_PCS_PHY_ADDR, ENETC_PCS_DEVAD_REPL,
			 ENETC_PCS_CR,
			 ENETC_PCS_CR_LANE_RESET | ENETC_PCS_CR_RESET_AN);

	return 0;
}

/* Apply protocol specific configuration to MAC, serdes as needed */
static void enetc_start_pcs(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	const char *if_str;

	priv->if_type = PHY_INTERFACE_MODE_NONE;

	/* check internal mdio capability, not all ports need it */
	if (enetc_read_port(priv, ENETC_PCAPR0) & ENETC_PCAPRO_MDIO) {
		/*
		 * set up internal MDIO, this is part of ETH PCI function and is
		 * used to access serdes / internal SoC PHYs.
		 * We don't currently register it as a MDIO bus as it goes away
		 * when the interface is removed, so it can't practically be
		 * used in the console.
		 */
		priv->imdio.read = enetc_mdio_read;
		priv->imdio.write = enetc_mdio_write;
		priv->imdio.priv = priv->port_regs + ENETC_PM_IMDIO_BASE;
		strncpy(priv->imdio.name, dev->name, MDIO_NAME_LEN);
	}

	if (!ofnode_valid(dev->node)) {
		enetc_dbg(dev, "no enetc ofnode found, skipping PCS set-up\n");
		return;
	}

	if_str = ofnode_read_string(dev->node, "phy-mode");
	if (if_str)
		priv->if_type = phy_get_interface_by_name(if_str);
	else
		enetc_dbg(dev,
			  "phy-mode property not found, defaulting to SGMII\n");
	if (priv->if_type < 0)
		priv->if_type = PHY_INTERFACE_MODE_NONE;

	switch (priv->if_type) {
	case PHY_INTERFACE_MODE_SGMII:
		enetc_init_sgmii(dev);
		break;
	case PHY_INTERFACE_MODE_RGMII:
		enetc_init_rgmii(dev);
		break;
	case PHY_INTERFACE_MODE_XGMII:
		enetc_init_sxgmii(dev);
		break;
	};
}

/* Configure the actual/external ethernet PHY, if one is found */
static void enetc_start_phy(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	struct udevice *miidev;
	struct phy_device *phy;
	u32 phandle, phy_id;
	ofnode phy_node;
	int supported;

	if (!ofnode_valid(dev->node)) {
		enetc_dbg(dev, "no enetc ofnode found, skipping PHY set-up\n");
		return;
	}

	if (ofnode_read_u32(dev->node, "phy-handle", &phandle)) {
		enetc_dbg(dev, "phy-handle not found, skipping PHY set-up\n");
		return;
	}

	phy_node = ofnode_get_by_phandle(phandle);
	if (!ofnode_valid(phy_node)) {
		enetc_dbg(dev, "invalid phy node, skipping PHY set-up\n");
		return;
	}
	enetc_dbg(dev, "phy node: %s\n", ofnode_get_name(phy_node));

	if (ofnode_read_u32(phy_node, "reg", &phy_id)) {
		enetc_dbg(dev,
			  "missing reg in PHY node, skipping PHY set-up\n");
		return;
	}

	if (uclass_get_device_by_ofnode(UCLASS_MDIO,
					ofnode_get_parent(phy_node),
					&miidev)) {
		enetc_dbg(dev, "can't find MDIO bus for node %s\n",
			  ofnode_get_name(ofnode_get_parent(phy_node)));
		return;
	}

	phy = dm_mdio_phy_connect(miidev, phy_id, dev, priv->if_type);
	if (!phy) {
		enetc_dbg(dev, "dm_mdio_phy_connect returned null\n");
		return;
	}

	supported = GENMASK(6, 0); /* speeds up to 1G & AN */
	phy->advertising = phy->supported & supported;
	phy_config(phy);
	phy_startup(phy);
}

/*
 * Probe ENETC driver:
 * - initialize port and station interface BARs
 */
static int enetc_probe(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);

	if (ofnode_valid(dev->node) && !ofnode_is_available(dev->node)) {
		enetc_dbg(dev, "interface disabled\n");
		return -ENODEV;
	}

	priv->enetc_txbd = memalign(ENETC_BD_ALIGN,
				    sizeof(struct enetc_tx_bd) * ENETC_BD_CNT);
	priv->enetc_rxbd = memalign(ENETC_BD_ALIGN,
				    sizeof(union enetc_rx_bd) * ENETC_BD_CNT);

	if (!priv->enetc_txbd || !priv->enetc_rxbd) {
		/* free should be able to handle NULL, just free all pointers */
		free(priv->enetc_txbd);
		free(priv->enetc_rxbd);

		return -ENOMEM;
	}

	/* initialize register */
	priv->regs_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0);
	if (!priv->regs_base) {
		enetc_dbg(dev, "failed to map BAR0\n");
		return -EINVAL;
	}
	priv->port_regs = priv->regs_base + ENETC_PORT_REGS_OFF;

	dm_pci_clrset_config16(dev, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);

	return 0;
}

/*
 * Remove the driver from an interface:
 * - free up allocated memory
 */
static int enetc_remove(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);

	free(priv->enetc_txbd);
	free(priv->enetc_rxbd);

	return 0;
}

/* ENETC Port MAC address registers, accepts big-endian format */
static void enetc_set_primary_mac_addr(struct enetc_priv *priv, const u8 *addr)
{
	u16 lower = *(const u16 *)(addr + 4);
	u32 upper = *(const u32 *)addr;

	enetc_write_port(priv, ENETC_PSIPMAR0, upper);
	enetc_write_port(priv, ENETC_PSIPMAR1, lower);
}

/* Configure port parameters (# of rings, frame size, enable port) */
static void enetc_enable_si_port(struct enetc_priv *priv)
{
	u32 val;

	/* set Rx/Tx BDR count */
	val = ENETC_PSICFGR_SET_TXBDR(ENETC_TX_BDR_CNT);
	val |= ENETC_PSICFGR_SET_RXBDR(ENETC_RX_BDR_CNT);
	enetc_write_port(priv, ENETC_PSICFGR(0), val);
	/* set Rx max frame size */
	enetc_write_port(priv, ENETC_PM_MAXFRM, ENETC_RX_MAXFRM_SIZE);
	/* enable MAC port */
	enetc_write_port(priv, ENETC_PM_CC, ENETC_PM_CC_RX_TX_EN);
	/* enable port */
	enetc_write_port(priv, ENETC_PMR, ENETC_PMR_SI0_EN);
	/* set SI cache policy */
	enetc_write(priv, ENETC_SICAR0,
		    ENETC_SICAR_RD_CFG | ENETC_SICAR_WR_CFG);
	/* enable SI */
	enetc_write(priv, ENETC_SIMR, ENETC_SIMR_EN);
}

/* returns DMA address for a given buffer index */
static inline u64 enetc_rxb_address(struct udevice *dev, int i)
{
	return cpu_to_le64(dm_pci_virt_to_mem(dev, net_rx_packets[i]));
}

/*
 * Setup a single Tx BD Ring (ID = 0):
 * - set Tx buffer descriptor address
 * - set the BD count
 * - initialize the producer and consumer index
 */
static void enetc_setup_tx_bdr(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	struct bd_ring *tx_bdr = &priv->tx_bdr;
	u64 tx_bd_add = (u64)priv->enetc_txbd;

	/* used later to advance to the next Tx BD */
	tx_bdr->bd_count = ENETC_BD_CNT;
	tx_bdr->next_prod_idx = 0;
	tx_bdr->next_cons_idx = 0;
	tx_bdr->cons_idx = priv->regs_base +
				ENETC_BDR(TX, ENETC_TX_BDR_ID, ENETC_TBCIR);
	tx_bdr->prod_idx = priv->regs_base +
				ENETC_BDR(TX, ENETC_TX_BDR_ID, ENETC_TBPIR);

	/* set Tx BD address */
	enetc_bdr_write(priv, TX, ENETC_TX_BDR_ID, ENETC_TBBAR0,
			lower_32_bits(tx_bd_add));
	enetc_bdr_write(priv, TX, ENETC_TX_BDR_ID, ENETC_TBBAR1,
			upper_32_bits(tx_bd_add));
	/* set Tx 8 BD count */
	enetc_bdr_write(priv, TX, ENETC_TX_BDR_ID, ENETC_TBLENR,
			tx_bdr->bd_count);

	/* reset both producer/consumer indexes */
	enetc_write_reg(tx_bdr->cons_idx, tx_bdr->next_cons_idx);
	enetc_write_reg(tx_bdr->prod_idx, tx_bdr->next_prod_idx);

	/* enable TX ring */
	enetc_bdr_write(priv, TX, ENETC_TX_BDR_ID, ENETC_TBMR, ENETC_TBMR_EN);
}

/*
 * Setup a single Rx BD Ring (ID = 0):
 * - set Rx buffer descriptors address (one descriptor per buffer)
 * - set buffer size as max frame size
 * - enable Rx ring
 * - reset consumer and producer indexes
 * - set buffer for each descriptor
 */
static void enetc_setup_rx_bdr(struct udevice *dev)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	struct bd_ring *rx_bdr = &priv->rx_bdr;
	u64 rx_bd_add = (u64)priv->enetc_rxbd;
	int i;

	/* used later to advance to the next BD produced by ENETC HW */
	rx_bdr->bd_count = ENETC_BD_CNT;
	rx_bdr->next_prod_idx = 0;
	rx_bdr->next_cons_idx = 0;
	rx_bdr->cons_idx = priv->regs_base +
				ENETC_BDR(RX, ENETC_RX_BDR_ID, ENETC_RBCIR);
	rx_bdr->prod_idx = priv->regs_base +
				ENETC_BDR(RX, ENETC_RX_BDR_ID, ENETC_RBPIR);

	/* set Rx BD address */
	enetc_bdr_write(priv, RX, ENETC_RX_BDR_ID, ENETC_RBBAR0,
			lower_32_bits(rx_bd_add));
	enetc_bdr_write(priv, RX, ENETC_RX_BDR_ID, ENETC_RBBAR1,
			upper_32_bits(rx_bd_add));
	/* set Rx BD count (multiple of 8) */
	enetc_bdr_write(priv, RX, ENETC_RX_BDR_ID, ENETC_RBLENR,
			rx_bdr->bd_count);
	/* set Rx buffer  size */
	enetc_bdr_write(priv, RX, ENETC_RX_BDR_ID, ENETC_RBBSR, PKTSIZE_ALIGN);

	/* fill Rx BD */
	memset(priv->enetc_rxbd, 0,
	       rx_bdr->bd_count * sizeof(union enetc_rx_bd));
	for (i = 0; i < rx_bdr->bd_count; i++) {
		priv->enetc_rxbd[i].w.addr = enetc_rxb_address(dev, i);
		/* each RX buffer must be aligned to 64B */
		WARN_ON(priv->enetc_rxbd[i].w.addr & (ARCH_DMA_MINALIGN - 1));
	}

	/* reset producer (ENETC owned) and consumer (SW owned) index */
	enetc_write_reg(rx_bdr->cons_idx, rx_bdr->next_cons_idx);
	enetc_write_reg(rx_bdr->prod_idx, rx_bdr->next_prod_idx);

	/* enable Rx ring */
	enetc_bdr_write(priv, RX, ENETC_RX_BDR_ID, ENETC_RBMR, ENETC_RBMR_EN);
}

/*
 * Start ENETC interface:
 * - perform FLR
 * - enable access to port and SI registers
 * - set mac address
 * - setup TX/RX buffer descriptors
 * - enable Tx/Rx rings
 */
static int enetc_start(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_platdata(dev);
	struct enetc_priv *priv = dev_get_priv(dev);

	/* reset and enable the PCI device */
	dm_pci_flr(dev);
	dm_pci_clrset_config16(dev, PCI_COMMAND, 0,
			       PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);

	if (!is_valid_ethaddr(plat->enetaddr)) {
		enetc_dbg(dev, "invalid MAC address, generate random ...\n");
		net_random_ethaddr(plat->enetaddr);
	}
	enetc_set_primary_mac_addr(priv, plat->enetaddr);

	enetc_enable_si_port(priv);

	/* setup Tx/Rx buffer descriptors */
	enetc_setup_tx_bdr(dev);
	enetc_setup_rx_bdr(dev);

	enetc_start_pcs(dev);
	enetc_start_phy(dev);

	return 0;
}

/*
 * Stop the network interface:
 * - just quiesce it, we can wipe all configuration as _start starts from
 * scratch each time
 */
static void enetc_stop(struct udevice *dev)
{
	/* FLR is sufficient to quiesce the device */
	dm_pci_flr(dev);
}

/*
 * ENETC transmit packet:
 * - check if Tx BD ring is full
 * - set buffer/packet address (dma address)
 * - set final fragment flag
 * - try while producer index equals consumer index or timeout
 */
static int enetc_send(struct udevice *dev, void *packet, int length)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	struct bd_ring *txr = &priv->tx_bdr;
	void *nv_packet = (void *)packet;
	int tries = ENETC_POLL_TRIES;
	u32 pi, ci;

	pi = txr->next_prod_idx;
	ci = enetc_read_reg(txr->cons_idx) & ENETC_BDR_IDX_MASK;
	/* Tx ring is full when */
	if (((pi + 1) % txr->bd_count) == ci) {
		enetc_dbg(dev, "Tx BDR full\n");
		return -ETIMEDOUT;
	}
	enetc_dbg(dev, "TxBD[%d]send: pkt_len=%d, buff @0x%x%08x\n", pi, length,
		  upper_32_bits((u64)nv_packet), lower_32_bits((u64)nv_packet));

	/* prepare Tx BD */
	memset(&priv->enetc_txbd[pi], 0x0, sizeof(struct enetc_tx_bd));
	priv->enetc_txbd[pi].addr =
		cpu_to_le64(dm_pci_virt_to_mem(dev, nv_packet));
	priv->enetc_txbd[pi].buf_len = cpu_to_le16(length);
	priv->enetc_txbd[pi].frm_len = cpu_to_le16(length);
	priv->enetc_txbd[pi].flags = cpu_to_le16(ENETC_TXBD_FLAGS_F);
	dmb();
	/* send frame: increment producer index */
	pi = (pi + 1) % txr->bd_count;
	txr->next_prod_idx = pi;
	enetc_write_reg(txr->prod_idx, pi);
	while ((--tries >= 0) &&
	       (pi != (enetc_read_reg(txr->cons_idx) & ENETC_BDR_IDX_MASK)))
		udelay(10);

	return tries > 0 ? 0 : -ETIMEDOUT;
}

/*
 * Receive frame:
 * - wait for the next BD to get ready bit set
 * - clean up the descriptor
 * - move on and indicate to HW that the cleaned BD is available for Rx
 */
static int enetc_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct enetc_priv *priv = dev_get_priv(dev);
	struct bd_ring *rxr = &priv->rx_bdr;
	int tries = ENETC_POLL_TRIES;
	int pi = rxr->next_prod_idx;
	int ci = rxr->next_cons_idx;
	u32 status;
	int len;
	u8 rdy;

	do {
		dmb();
		status = le32_to_cpu(priv->enetc_rxbd[pi].r.lstatus);
		/* check if current BD is ready to be consumed */
		rdy = ENETC_RXBD_STATUS_R(status);
	} while (--tries >= 0 && !rdy);

	if (!rdy)
		return -EAGAIN;

	dmb();
	len = le16_to_cpu(priv->enetc_rxbd[pi].r.buf_len);
	*packetp = (uchar *)enetc_rxb_address(dev, pi);
	enetc_dbg(dev, "RxBD[%d]: len=%d err=%d pkt=0x%x%08x\n", pi, len,
		  ENETC_RXBD_STATUS_ERRORS(status),
		  upper_32_bits((u64)*packetp), lower_32_bits((u64)*packetp));

	/* BD clean up and advance to next in ring */
	memset(&priv->enetc_rxbd[pi], 0, sizeof(union enetc_rx_bd));
	priv->enetc_rxbd[pi].w.addr = enetc_rxb_address(dev, pi);
	rxr->next_prod_idx = (pi + 1) % rxr->bd_count;
	ci = (ci + 1) % rxr->bd_count;
	rxr->next_cons_idx = ci;
	dmb();
	/* free up the slot in the ring for HW */
	enetc_write_reg(rxr->cons_idx, ci);

	return len;
}

static const struct eth_ops enetc_ops = {
	.start	= enetc_start,
	.send	= enetc_send,
	.recv	= enetc_recv,
	.stop	= enetc_stop,
};

U_BOOT_DRIVER(eth_enetc) = {
	.name	= "enetc_eth",
	.id	= UCLASS_ETH,
	.bind	= enetc_bind,
	.probe	= enetc_probe,
	.remove = enetc_remove,
	.ops	= &enetc_ops,
	.priv_auto_alloc_size = sizeof(struct enetc_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};

static struct pci_device_id enetc_ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_FREESCALE, PCI_DEVICE_ID_ENETC_ETH) },
	{}
};

U_BOOT_PCI_DEVICE(eth_enetc, enetc_ids);
