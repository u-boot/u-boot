/*
 * CPSW Ethernet Switch Driver
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <command.h>
#include <net.h>
#include <miiphy.h>
#include <malloc.h>
#include <net.h>
#include <netdev.h>
#include <cpsw.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <phy.h>
#include <asm/arch/cpu.h>

#define BITMASK(bits)		(BIT(bits) - 1)
#define PHY_REG_MASK		0x1f
#define PHY_ID_MASK		0x1f
#define NUM_DESCS		(PKTBUFSRX * 2)
#define PKT_MIN			60
#define PKT_MAX			(1500 + 14 + 4 + 4)
#define CLEAR_BIT		1
#define GIGABITEN		BIT(7)
#define FULLDUPLEXEN		BIT(0)
#define MIIEN			BIT(15)

/* DMA Registers */
#define CPDMA_TXCONTROL		0x004
#define CPDMA_RXCONTROL		0x014
#define CPDMA_SOFTRESET		0x01c
#define CPDMA_RXFREE		0x0e0
#define CPDMA_TXHDP_VER1	0x100
#define CPDMA_TXHDP_VER2	0x200
#define CPDMA_RXHDP_VER1	0x120
#define CPDMA_RXHDP_VER2	0x220
#define CPDMA_TXCP_VER1		0x140
#define CPDMA_TXCP_VER2		0x240
#define CPDMA_RXCP_VER1		0x160
#define CPDMA_RXCP_VER2		0x260

/* Descriptor mode bits */
#define CPDMA_DESC_SOP		BIT(31)
#define CPDMA_DESC_EOP		BIT(30)
#define CPDMA_DESC_OWNER	BIT(29)
#define CPDMA_DESC_EOQ		BIT(28)

/*
 * This timeout definition is a worst-case ultra defensive measure against
 * unexpected controller lock ups.  Ideally, we should never ever hit this
 * scenario in practice.
 */
#define MDIO_TIMEOUT            100 /* msecs */
#define CPDMA_TIMEOUT		100 /* msecs */

struct cpsw_mdio_regs {
	u32	version;
	u32	control;
#define CONTROL_IDLE		BIT(31)
#define CONTROL_ENABLE		BIT(30)

	u32	alive;
	u32	link;
	u32	linkintraw;
	u32	linkintmasked;
	u32	__reserved_0[2];
	u32	userintraw;
	u32	userintmasked;
	u32	userintmaskset;
	u32	userintmaskclr;
	u32	__reserved_1[20];

	struct {
		u32		access;
		u32		physel;
#define USERACCESS_GO		BIT(31)
#define USERACCESS_WRITE	BIT(30)
#define USERACCESS_ACK		BIT(29)
#define USERACCESS_READ		(0)
#define USERACCESS_DATA		(0xffff)
	} user[0];
};

struct cpsw_regs {
	u32	id_ver;
	u32	control;
	u32	soft_reset;
	u32	stat_port_en;
	u32	ptype;
};

struct cpsw_slave_regs {
	u32	max_blks;
	u32	blk_cnt;
	u32	flow_thresh;
	u32	port_vlan;
	u32	tx_pri_map;
#ifdef CONFIG_AM33XX
	u32	gap_thresh;
#elif defined(CONFIG_TI814X)
	u32	ts_ctl;
	u32	ts_seq_ltype;
	u32	ts_vlan;
#endif
	u32	sa_lo;
	u32	sa_hi;
};

struct cpsw_host_regs {
	u32	max_blks;
	u32	blk_cnt;
	u32	flow_thresh;
	u32	port_vlan;
	u32	tx_pri_map;
	u32	cpdma_tx_pri_map;
	u32	cpdma_rx_chan_map;
};

struct cpsw_sliver_regs {
	u32	id_ver;
	u32	mac_control;
	u32	mac_status;
	u32	soft_reset;
	u32	rx_maxlen;
	u32	__reserved_0;
	u32	rx_pause;
	u32	tx_pause;
	u32	__reserved_1;
	u32	rx_pri_map;
};

#define ALE_ENTRY_BITS		68
#define ALE_ENTRY_WORDS		DIV_ROUND_UP(ALE_ENTRY_BITS, 32)

/* ALE Registers */
#define ALE_CONTROL		0x08
#define ALE_UNKNOWNVLAN		0x18
#define ALE_TABLE_CONTROL	0x20
#define ALE_TABLE		0x34
#define ALE_PORTCTL		0x40

#define ALE_TABLE_WRITE		BIT(31)

#define ALE_TYPE_FREE			0
#define ALE_TYPE_ADDR			1
#define ALE_TYPE_VLAN			2
#define ALE_TYPE_VLAN_ADDR		3

#define ALE_UCAST_PERSISTANT		0
#define ALE_UCAST_UNTOUCHED		1
#define ALE_UCAST_OUI			2
#define ALE_UCAST_TOUCHED		3

#define ALE_MCAST_FWD			0
#define ALE_MCAST_BLOCK_LEARN_FWD	1
#define ALE_MCAST_FWD_LEARN		2
#define ALE_MCAST_FWD_2			3

enum cpsw_ale_port_state {
	ALE_PORT_STATE_DISABLE	= 0x00,
	ALE_PORT_STATE_BLOCK	= 0x01,
	ALE_PORT_STATE_LEARN	= 0x02,
	ALE_PORT_STATE_FORWARD	= 0x03,
};

/* ALE unicast entry flags - passed into cpsw_ale_add_ucast() */
#define ALE_SECURE	1
#define ALE_BLOCKED	2

struct cpsw_slave {
	struct cpsw_slave_regs		*regs;
	struct cpsw_sliver_regs		*sliver;
	int				slave_num;
	u32				mac_control;
	struct cpsw_slave_data		*data;
};

struct cpdma_desc {
	/* hardware fields */
	u32			hw_next;
	u32			hw_buffer;
	u32			hw_len;
	u32			hw_mode;
	/* software fields */
	u32			sw_buffer;
	u32			sw_len;
};

struct cpdma_chan {
	struct cpdma_desc	*head, *tail;
	void			*hdp, *cp, *rxfree;
};

#define desc_write(desc, fld, val)	__raw_writel((u32)(val), &(desc)->fld)
#define desc_read(desc, fld)		__raw_readl(&(desc)->fld)
#define desc_read_ptr(desc, fld)	((void *)__raw_readl(&(desc)->fld))

#define chan_write(chan, fld, val)	__raw_writel((u32)(val), (chan)->fld)
#define chan_read(chan, fld)		__raw_readl((chan)->fld)
#define chan_read_ptr(chan, fld)	((void *)__raw_readl((chan)->fld))

#define for_each_slave(slave, priv) \
	for (slave = (priv)->slaves; slave != (priv)->slaves + \
				(priv)->data.slaves; slave++)

struct cpsw_priv {
	struct eth_device		*dev;
	struct cpsw_platform_data	data;
	int				host_port;

	struct cpsw_regs		*regs;
	void				*dma_regs;
	struct cpsw_host_regs		*host_port_regs;
	void				*ale_regs;

	struct cpdma_desc		*descs;
	struct cpdma_desc		*desc_free;
	struct cpdma_chan		rx_chan, tx_chan;

	struct cpsw_slave		*slaves;
	struct phy_device		*phydev;
	struct mii_dev			*bus;

	u32				mdio_link;
	u32				phy_mask;
};

static inline int cpsw_ale_get_field(u32 *ale_entry, u32 start, u32 bits)
{
	int idx;

	idx    = start / 32;
	start -= idx * 32;
	idx    = 2 - idx; /* flip */
	return (ale_entry[idx] >> start) & BITMASK(bits);
}

static inline void cpsw_ale_set_field(u32 *ale_entry, u32 start, u32 bits,
				      u32 value)
{
	int idx;

	value &= BITMASK(bits);
	idx    = start / 32;
	start -= idx * 32;
	idx    = 2 - idx; /* flip */
	ale_entry[idx] &= ~(BITMASK(bits) << start);
	ale_entry[idx] |=  (value << start);
}

#define DEFINE_ALE_FIELD(name, start, bits)				\
static inline int cpsw_ale_get_##name(u32 *ale_entry)			\
{									\
	return cpsw_ale_get_field(ale_entry, start, bits);		\
}									\
static inline void cpsw_ale_set_##name(u32 *ale_entry, u32 value)	\
{									\
	cpsw_ale_set_field(ale_entry, start, bits, value);		\
}

DEFINE_ALE_FIELD(entry_type,		60,	2)
DEFINE_ALE_FIELD(mcast_state,		62,	2)
DEFINE_ALE_FIELD(port_mask,		66,	3)
DEFINE_ALE_FIELD(ucast_type,		62,	2)
DEFINE_ALE_FIELD(port_num,		66,	2)
DEFINE_ALE_FIELD(blocked,		65,	1)
DEFINE_ALE_FIELD(secure,		64,	1)
DEFINE_ALE_FIELD(mcast,			40,	1)

/* The MAC address field in the ALE entry cannot be macroized as above */
static inline void cpsw_ale_get_addr(u32 *ale_entry, u8 *addr)
{
	int i;

	for (i = 0; i < 6; i++)
		addr[i] = cpsw_ale_get_field(ale_entry, 40 - 8*i, 8);
}

static inline void cpsw_ale_set_addr(u32 *ale_entry, u8 *addr)
{
	int i;

	for (i = 0; i < 6; i++)
		cpsw_ale_set_field(ale_entry, 40 - 8*i, 8, addr[i]);
}

static int cpsw_ale_read(struct cpsw_priv *priv, int idx, u32 *ale_entry)
{
	int i;

	__raw_writel(idx, priv->ale_regs + ALE_TABLE_CONTROL);

	for (i = 0; i < ALE_ENTRY_WORDS; i++)
		ale_entry[i] = __raw_readl(priv->ale_regs + ALE_TABLE + 4 * i);

	return idx;
}

static int cpsw_ale_write(struct cpsw_priv *priv, int idx, u32 *ale_entry)
{
	int i;

	for (i = 0; i < ALE_ENTRY_WORDS; i++)
		__raw_writel(ale_entry[i], priv->ale_regs + ALE_TABLE + 4 * i);

	__raw_writel(idx | ALE_TABLE_WRITE, priv->ale_regs + ALE_TABLE_CONTROL);

	return idx;
}

static int cpsw_ale_match_addr(struct cpsw_priv *priv, u8* addr)
{
	u32 ale_entry[ALE_ENTRY_WORDS];
	int type, idx;

	for (idx = 0; idx < priv->data.ale_entries; idx++) {
		u8 entry_addr[6];

		cpsw_ale_read(priv, idx, ale_entry);
		type = cpsw_ale_get_entry_type(ale_entry);
		if (type != ALE_TYPE_ADDR && type != ALE_TYPE_VLAN_ADDR)
			continue;
		cpsw_ale_get_addr(ale_entry, entry_addr);
		if (memcmp(entry_addr, addr, 6) == 0)
			return idx;
	}
	return -ENOENT;
}

static int cpsw_ale_match_free(struct cpsw_priv *priv)
{
	u32 ale_entry[ALE_ENTRY_WORDS];
	int type, idx;

	for (idx = 0; idx < priv->data.ale_entries; idx++) {
		cpsw_ale_read(priv, idx, ale_entry);
		type = cpsw_ale_get_entry_type(ale_entry);
		if (type == ALE_TYPE_FREE)
			return idx;
	}
	return -ENOENT;
}

static int cpsw_ale_find_ageable(struct cpsw_priv *priv)
{
	u32 ale_entry[ALE_ENTRY_WORDS];
	int type, idx;

	for (idx = 0; idx < priv->data.ale_entries; idx++) {
		cpsw_ale_read(priv, idx, ale_entry);
		type = cpsw_ale_get_entry_type(ale_entry);
		if (type != ALE_TYPE_ADDR && type != ALE_TYPE_VLAN_ADDR)
			continue;
		if (cpsw_ale_get_mcast(ale_entry))
			continue;
		type = cpsw_ale_get_ucast_type(ale_entry);
		if (type != ALE_UCAST_PERSISTANT &&
		    type != ALE_UCAST_OUI)
			return idx;
	}
	return -ENOENT;
}

static int cpsw_ale_add_ucast(struct cpsw_priv *priv, u8 *addr,
			      int port, int flags)
{
	u32 ale_entry[ALE_ENTRY_WORDS] = {0, 0, 0};
	int idx;

	cpsw_ale_set_entry_type(ale_entry, ALE_TYPE_ADDR);
	cpsw_ale_set_addr(ale_entry, addr);
	cpsw_ale_set_ucast_type(ale_entry, ALE_UCAST_PERSISTANT);
	cpsw_ale_set_secure(ale_entry, (flags & ALE_SECURE) ? 1 : 0);
	cpsw_ale_set_blocked(ale_entry, (flags & ALE_BLOCKED) ? 1 : 0);
	cpsw_ale_set_port_num(ale_entry, port);

	idx = cpsw_ale_match_addr(priv, addr);
	if (idx < 0)
		idx = cpsw_ale_match_free(priv);
	if (idx < 0)
		idx = cpsw_ale_find_ageable(priv);
	if (idx < 0)
		return -ENOMEM;

	cpsw_ale_write(priv, idx, ale_entry);
	return 0;
}

static int cpsw_ale_add_mcast(struct cpsw_priv *priv, u8 *addr, int port_mask)
{
	u32 ale_entry[ALE_ENTRY_WORDS] = {0, 0, 0};
	int idx, mask;

	idx = cpsw_ale_match_addr(priv, addr);
	if (idx >= 0)
		cpsw_ale_read(priv, idx, ale_entry);

	cpsw_ale_set_entry_type(ale_entry, ALE_TYPE_ADDR);
	cpsw_ale_set_addr(ale_entry, addr);
	cpsw_ale_set_mcast_state(ale_entry, ALE_MCAST_FWD_2);

	mask = cpsw_ale_get_port_mask(ale_entry);
	port_mask |= mask;
	cpsw_ale_set_port_mask(ale_entry, port_mask);

	if (idx < 0)
		idx = cpsw_ale_match_free(priv);
	if (idx < 0)
		idx = cpsw_ale_find_ageable(priv);
	if (idx < 0)
		return -ENOMEM;

	cpsw_ale_write(priv, idx, ale_entry);
	return 0;
}

static inline void cpsw_ale_control(struct cpsw_priv *priv, int bit, int val)
{
	u32 tmp, mask = BIT(bit);

	tmp  = __raw_readl(priv->ale_regs + ALE_CONTROL);
	tmp &= ~mask;
	tmp |= val ? mask : 0;
	__raw_writel(tmp, priv->ale_regs + ALE_CONTROL);
}

#define cpsw_ale_enable(priv, val)	cpsw_ale_control(priv, 31, val)
#define cpsw_ale_clear(priv, val)	cpsw_ale_control(priv, 30, val)
#define cpsw_ale_vlan_aware(priv, val)	cpsw_ale_control(priv,  2, val)

static inline void cpsw_ale_port_state(struct cpsw_priv *priv, int port,
				       int val)
{
	int offset = ALE_PORTCTL + 4 * port;
	u32 tmp, mask = 0x3;

	tmp  = __raw_readl(priv->ale_regs + offset);
	tmp &= ~mask;
	tmp |= val & mask;
	__raw_writel(tmp, priv->ale_regs + offset);
}

static struct cpsw_mdio_regs *mdio_regs;

/* wait until hardware is ready for another user access */
static inline u32 wait_for_user_access(void)
{
	u32 reg = 0;
	int timeout = MDIO_TIMEOUT;

	while (timeout-- &&
	((reg = __raw_readl(&mdio_regs->user[0].access)) & USERACCESS_GO))
		udelay(10);

	if (timeout == -1) {
		printf("wait_for_user_access Timeout\n");
		return -ETIMEDOUT;
	}
	return reg;
}

/* wait until hardware state machine is idle */
static inline void wait_for_idle(void)
{
	int timeout = MDIO_TIMEOUT;

	while (timeout-- &&
		((__raw_readl(&mdio_regs->control) & CONTROL_IDLE) == 0))
		udelay(10);

	if (timeout == -1)
		printf("wait_for_idle Timeout\n");
}

static int cpsw_mdio_read(struct mii_dev *bus, int phy_id,
				int dev_addr, int phy_reg)
{
	int data;
	u32 reg;

	if (phy_reg & ~PHY_REG_MASK || phy_id & ~PHY_ID_MASK)
		return -EINVAL;

	wait_for_user_access();
	reg = (USERACCESS_GO | USERACCESS_READ | (phy_reg << 21) |
	       (phy_id << 16));
	__raw_writel(reg, &mdio_regs->user[0].access);
	reg = wait_for_user_access();

	data = (reg & USERACCESS_ACK) ? (reg & USERACCESS_DATA) : -1;
	return data;
}

static int cpsw_mdio_write(struct mii_dev *bus, int phy_id, int dev_addr,
				int phy_reg, u16 data)
{
	u32 reg;

	if (phy_reg & ~PHY_REG_MASK || phy_id & ~PHY_ID_MASK)
		return -EINVAL;

	wait_for_user_access();
	reg = (USERACCESS_GO | USERACCESS_WRITE | (phy_reg << 21) |
		   (phy_id << 16) | (data & USERACCESS_DATA));
	__raw_writel(reg, &mdio_regs->user[0].access);
	wait_for_user_access();

	return 0;
}

static void cpsw_mdio_init(char *name, u32 mdio_base, u32 div)
{
	struct mii_dev *bus = mdio_alloc();

	mdio_regs = (struct cpsw_mdio_regs *)mdio_base;

	/* set enable and clock divider */
	__raw_writel(div | CONTROL_ENABLE, &mdio_regs->control);

	/*
	 * wait for scan logic to settle:
	 * the scan time consists of (a) a large fixed component, and (b) a
	 * small component that varies with the mii bus frequency.  These
	 * were estimated using measurements at 1.1 and 2.2 MHz on tnetv107x
	 * silicon.  Since the effect of (b) was found to be largely
	 * negligible, we keep things simple here.
	 */
	udelay(1000);

	bus->read = cpsw_mdio_read;
	bus->write = cpsw_mdio_write;
	sprintf(bus->name, name);

	mdio_register(bus);
}

/* Set a self-clearing bit in a register, and wait for it to clear */
static inline void setbit_and_wait_for_clear32(void *addr)
{
	__raw_writel(CLEAR_BIT, addr);
	while (__raw_readl(addr) & CLEAR_BIT)
		;
}

#define mac_hi(mac)	(((mac)[0] << 0) | ((mac)[1] << 8) |	\
			 ((mac)[2] << 16) | ((mac)[3] << 24))
#define mac_lo(mac)	(((mac)[4] << 0) | ((mac)[5] << 8))

static void cpsw_set_slave_mac(struct cpsw_slave *slave,
			       struct cpsw_priv *priv)
{
	__raw_writel(mac_hi(priv->dev->enetaddr), &slave->regs->sa_hi);
	__raw_writel(mac_lo(priv->dev->enetaddr), &slave->regs->sa_lo);
}

static void cpsw_slave_update_link(struct cpsw_slave *slave,
				   struct cpsw_priv *priv, int *link)
{
	struct phy_device *phy;
	u32 mac_control = 0;

	phy = priv->phydev;

	if (!phy)
		return;

	phy_startup(phy);
	*link = phy->link;

	if (*link) { /* link up */
		mac_control = priv->data.mac_control;
		if (phy->speed == 1000)
			mac_control |= GIGABITEN;
		if (phy->duplex == DUPLEX_FULL)
			mac_control |= FULLDUPLEXEN;
		if (phy->speed == 100)
			mac_control |= MIIEN;
	}

	if (mac_control == slave->mac_control)
		return;

	if (mac_control) {
		printf("link up on port %d, speed %d, %s duplex\n",
				slave->slave_num, phy->speed,
				(phy->duplex == DUPLEX_FULL) ? "full" : "half");
	} else {
		printf("link down on port %d\n", slave->slave_num);
	}

	__raw_writel(mac_control, &slave->sliver->mac_control);
	slave->mac_control = mac_control;
}

static int cpsw_update_link(struct cpsw_priv *priv)
{
	int link = 0;
	struct cpsw_slave *slave;

	for_each_slave(slave, priv)
		cpsw_slave_update_link(slave, priv, &link);
	priv->mdio_link = readl(&mdio_regs->link);
	return link;
}

static int cpsw_check_link(struct cpsw_priv *priv)
{
	u32 link = 0;

	link = __raw_readl(&mdio_regs->link) & priv->phy_mask;
	if ((link) && (link == priv->mdio_link))
		return 1;

	return cpsw_update_link(priv);
}

static inline u32  cpsw_get_slave_port(struct cpsw_priv *priv, u32 slave_num)
{
	if (priv->host_port == 0)
		return slave_num + 1;
	else
		return slave_num;
}

static void cpsw_slave_init(struct cpsw_slave *slave, struct cpsw_priv *priv)
{
	u32     slave_port;

	setbit_and_wait_for_clear32(&slave->sliver->soft_reset);

	/* setup priority mapping */
	__raw_writel(0x76543210, &slave->sliver->rx_pri_map);
	__raw_writel(0x33221100, &slave->regs->tx_pri_map);

	/* setup max packet size, and mac address */
	__raw_writel(PKT_MAX, &slave->sliver->rx_maxlen);
	cpsw_set_slave_mac(slave, priv);

	slave->mac_control = 0;	/* no link yet */

	/* enable forwarding */
	slave_port = cpsw_get_slave_port(priv, slave->slave_num);
	cpsw_ale_port_state(priv, slave_port, ALE_PORT_STATE_FORWARD);

	cpsw_ale_add_mcast(priv, NetBcastAddr, 1 << slave_port);

	priv->phy_mask |= 1 << slave->data->phy_id;
}

static struct cpdma_desc *cpdma_desc_alloc(struct cpsw_priv *priv)
{
	struct cpdma_desc *desc = priv->desc_free;

	if (desc)
		priv->desc_free = desc_read_ptr(desc, hw_next);
	return desc;
}

static void cpdma_desc_free(struct cpsw_priv *priv, struct cpdma_desc *desc)
{
	if (desc) {
		desc_write(desc, hw_next, priv->desc_free);
		priv->desc_free = desc;
	}
}

static int cpdma_submit(struct cpsw_priv *priv, struct cpdma_chan *chan,
			void *buffer, int len)
{
	struct cpdma_desc *desc, *prev;
	u32 mode;

	desc = cpdma_desc_alloc(priv);
	if (!desc)
		return -ENOMEM;

	if (len < PKT_MIN)
		len = PKT_MIN;

	mode = CPDMA_DESC_OWNER | CPDMA_DESC_SOP | CPDMA_DESC_EOP;

	desc_write(desc, hw_next,   0);
	desc_write(desc, hw_buffer, buffer);
	desc_write(desc, hw_len,    len);
	desc_write(desc, hw_mode,   mode | len);
	desc_write(desc, sw_buffer, buffer);
	desc_write(desc, sw_len,    len);

	if (!chan->head) {
		/* simple case - first packet enqueued */
		chan->head = desc;
		chan->tail = desc;
		chan_write(chan, hdp, desc);
		goto done;
	}

	/* not the first packet - enqueue at the tail */
	prev = chan->tail;
	desc_write(prev, hw_next, desc);
	chan->tail = desc;

	/* next check if EOQ has been triggered already */
	if (desc_read(prev, hw_mode) & CPDMA_DESC_EOQ)
		chan_write(chan, hdp, desc);

done:
	if (chan->rxfree)
		chan_write(chan, rxfree, 1);
	return 0;
}

static int cpdma_process(struct cpsw_priv *priv, struct cpdma_chan *chan,
			 void **buffer, int *len)
{
	struct cpdma_desc *desc = chan->head;
	u32 status;

	if (!desc)
		return -ENOENT;

	status = desc_read(desc, hw_mode);

	if (len)
		*len = status & 0x7ff;

	if (buffer)
		*buffer = desc_read_ptr(desc, sw_buffer);

	if (status & CPDMA_DESC_OWNER) {
		if (chan_read(chan, hdp) == 0) {
			if (desc_read(desc, hw_mode) & CPDMA_DESC_OWNER)
				chan_write(chan, hdp, desc);
		}

		return -EBUSY;
	}

	chan->head = desc_read_ptr(desc, hw_next);
	chan_write(chan, cp, desc);

	cpdma_desc_free(priv, desc);
	return 0;
}

static int cpsw_init(struct eth_device *dev, bd_t *bis)
{
	struct cpsw_priv	*priv = dev->priv;
	struct cpsw_slave	*slave;
	int i, ret;

	/* soft reset the controller and initialize priv */
	setbit_and_wait_for_clear32(&priv->regs->soft_reset);

	/* initialize and reset the address lookup engine */
	cpsw_ale_enable(priv, 1);
	cpsw_ale_clear(priv, 1);
	cpsw_ale_vlan_aware(priv, 0); /* vlan unaware mode */

	/* setup host port priority mapping */
	__raw_writel(0x76543210, &priv->host_port_regs->cpdma_tx_pri_map);
	__raw_writel(0, &priv->host_port_regs->cpdma_rx_chan_map);

	/* disable priority elevation and enable statistics on all ports */
	__raw_writel(0, &priv->regs->ptype);

	/* enable statistics collection only on the host port */
	__raw_writel(BIT(priv->host_port), &priv->regs->stat_port_en);
	__raw_writel(0x7, &priv->regs->stat_port_en);

	cpsw_ale_port_state(priv, priv->host_port, ALE_PORT_STATE_FORWARD);

	cpsw_ale_add_ucast(priv, priv->dev->enetaddr, priv->host_port,
			   ALE_SECURE);
	cpsw_ale_add_mcast(priv, NetBcastAddr, 1 << priv->host_port);

	for_each_slave(slave, priv)
		cpsw_slave_init(slave, priv);

	cpsw_update_link(priv);

	/* init descriptor pool */
	for (i = 0; i < NUM_DESCS; i++) {
		desc_write(&priv->descs[i], hw_next,
			   (i == (NUM_DESCS - 1)) ? 0 : &priv->descs[i+1]);
	}
	priv->desc_free = &priv->descs[0];

	/* initialize channels */
	if (priv->data.version == CPSW_CTRL_VERSION_2) {
		memset(&priv->rx_chan, 0, sizeof(struct cpdma_chan));
		priv->rx_chan.hdp       = priv->dma_regs + CPDMA_RXHDP_VER2;
		priv->rx_chan.cp        = priv->dma_regs + CPDMA_RXCP_VER2;
		priv->rx_chan.rxfree    = priv->dma_regs + CPDMA_RXFREE;

		memset(&priv->tx_chan, 0, sizeof(struct cpdma_chan));
		priv->tx_chan.hdp       = priv->dma_regs + CPDMA_TXHDP_VER2;
		priv->tx_chan.cp        = priv->dma_regs + CPDMA_TXCP_VER2;
	} else {
		memset(&priv->rx_chan, 0, sizeof(struct cpdma_chan));
		priv->rx_chan.hdp       = priv->dma_regs + CPDMA_RXHDP_VER1;
		priv->rx_chan.cp        = priv->dma_regs + CPDMA_RXCP_VER1;
		priv->rx_chan.rxfree    = priv->dma_regs + CPDMA_RXFREE;

		memset(&priv->tx_chan, 0, sizeof(struct cpdma_chan));
		priv->tx_chan.hdp       = priv->dma_regs + CPDMA_TXHDP_VER1;
		priv->tx_chan.cp        = priv->dma_regs + CPDMA_TXCP_VER1;
	}

	/* clear dma state */
	setbit_and_wait_for_clear32(priv->dma_regs + CPDMA_SOFTRESET);

	if (priv->data.version == CPSW_CTRL_VERSION_2) {
		for (i = 0; i < priv->data.channels; i++) {
			__raw_writel(0, priv->dma_regs + CPDMA_RXHDP_VER2 + 4
					* i);
			__raw_writel(0, priv->dma_regs + CPDMA_RXFREE + 4
					* i);
			__raw_writel(0, priv->dma_regs + CPDMA_RXCP_VER2 + 4
					* i);
			__raw_writel(0, priv->dma_regs + CPDMA_TXHDP_VER2 + 4
					* i);
			__raw_writel(0, priv->dma_regs + CPDMA_TXCP_VER2 + 4
					* i);
		}
	} else {
		for (i = 0; i < priv->data.channels; i++) {
			__raw_writel(0, priv->dma_regs + CPDMA_RXHDP_VER1 + 4
					* i);
			__raw_writel(0, priv->dma_regs + CPDMA_RXFREE + 4
					* i);
			__raw_writel(0, priv->dma_regs + CPDMA_RXCP_VER1 + 4
					* i);
			__raw_writel(0, priv->dma_regs + CPDMA_TXHDP_VER1 + 4
					* i);
			__raw_writel(0, priv->dma_regs + CPDMA_TXCP_VER1 + 4
					* i);

		}
	}

	__raw_writel(1, priv->dma_regs + CPDMA_TXCONTROL);
	__raw_writel(1, priv->dma_regs + CPDMA_RXCONTROL);

	/* submit rx descs */
	for (i = 0; i < PKTBUFSRX; i++) {
		ret = cpdma_submit(priv, &priv->rx_chan, NetRxPackets[i],
				   PKTSIZE);
		if (ret < 0) {
			printf("error %d submitting rx desc\n", ret);
			break;
		}
	}

	return 0;
}

static void cpsw_halt(struct eth_device *dev)
{
	struct cpsw_priv	*priv = dev->priv;

	writel(0, priv->dma_regs + CPDMA_TXCONTROL);
	writel(0, priv->dma_regs + CPDMA_RXCONTROL);

	/* soft reset the controller and initialize priv */
	setbit_and_wait_for_clear32(&priv->regs->soft_reset);

	/* clear dma state */
	setbit_and_wait_for_clear32(priv->dma_regs + CPDMA_SOFTRESET);

	priv->data.control(0);
}

static int cpsw_send(struct eth_device *dev, void *packet, int length)
{
	struct cpsw_priv	*priv = dev->priv;
	void *buffer;
	int len;
	int timeout = CPDMA_TIMEOUT;

	if (!cpsw_check_link(priv))
		return -EIO;

	flush_dcache_range((unsigned long)packet,
			   (unsigned long)packet + length);

	/* first reap completed packets */
	while (timeout-- &&
		(cpdma_process(priv, &priv->tx_chan, &buffer, &len) >= 0))
		;

	if (timeout == -1) {
		printf("cpdma_process timeout\n");
		return -ETIMEDOUT;
	}

	return cpdma_submit(priv, &priv->tx_chan, packet, length);
}

static int cpsw_recv(struct eth_device *dev)
{
	struct cpsw_priv	*priv = dev->priv;
	void *buffer;
	int len;

	cpsw_check_link(priv);

	while (cpdma_process(priv, &priv->rx_chan, &buffer, &len) >= 0) {
		invalidate_dcache_range((unsigned long)buffer,
					(unsigned long)buffer + PKTSIZE_ALIGN);
		NetReceive(buffer, len);
		cpdma_submit(priv, &priv->rx_chan, buffer, PKTSIZE);
	}

	return 0;
}

static void cpsw_slave_setup(struct cpsw_slave *slave, int slave_num,
			    struct cpsw_priv *priv)
{
	void			*regs = priv->regs;
	struct cpsw_slave_data	*data = priv->data.slave_data + slave_num;
	slave->slave_num = slave_num;
	slave->data	= data;
	slave->regs	= regs + data->slave_reg_ofs;
	slave->sliver	= regs + data->sliver_reg_ofs;
}

static int cpsw_phy_init(struct eth_device *dev, struct cpsw_slave *slave)
{
	struct cpsw_priv *priv = (struct cpsw_priv *)dev->priv;
	struct phy_device *phydev;
	u32 supported = (SUPPORTED_10baseT_Half |
			SUPPORTED_10baseT_Full |
			SUPPORTED_100baseT_Half |
			SUPPORTED_100baseT_Full |
			SUPPORTED_1000baseT_Full);

	phydev = phy_connect(priv->bus,
			CONFIG_PHY_ADDR,
			dev,
			slave->data->phy_if);

	if (!phydev)
		return -1;

	phydev->supported &= supported;
	phydev->advertising = phydev->supported;

	priv->phydev = phydev;
	phy_config(phydev);

	return 1;
}

int cpsw_register(struct cpsw_platform_data *data)
{
	struct cpsw_priv	*priv;
	struct cpsw_slave	*slave;
	void			*regs = (void *)data->cpsw_base;
	struct eth_device	*dev;

	dev = calloc(sizeof(*dev), 1);
	if (!dev)
		return -ENOMEM;

	priv = calloc(sizeof(*priv), 1);
	if (!priv) {
		free(dev);
		return -ENOMEM;
	}

	priv->data = *data;
	priv->dev = dev;

	priv->slaves = malloc(sizeof(struct cpsw_slave) * data->slaves);
	if (!priv->slaves) {
		free(dev);
		free(priv);
		return -ENOMEM;
	}

	priv->host_port		= data->host_port_num;
	priv->regs		= regs;
	priv->host_port_regs	= regs + data->host_port_reg_ofs;
	priv->dma_regs		= regs + data->cpdma_reg_ofs;
	priv->ale_regs		= regs + data->ale_reg_ofs;
	priv->descs		= (void *)regs + data->bd_ram_ofs;

	int idx = 0;

	for_each_slave(slave, priv) {
		cpsw_slave_setup(slave, idx, priv);
		idx = idx + 1;
	}

	strcpy(dev->name, "cpsw");
	dev->iobase	= 0;
	dev->init	= cpsw_init;
	dev->halt	= cpsw_halt;
	dev->send	= cpsw_send;
	dev->recv	= cpsw_recv;
	dev->priv	= priv;

	eth_register(dev);

	cpsw_mdio_init(dev->name, data->mdio_base, data->mdio_div);
	priv->bus = miiphy_get_dev_by_name(dev->name);
	for_each_slave(slave, priv)
		cpsw_phy_init(dev, slave);

	return 1;
}
