/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * Copyright (C) 2009 NVIDIA, Corporation
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm/unaligned.h>
#include <common.h>
#include <usb.h>
#include <linux/mii.h>
#include "usb_ether.h"

/* SMSC LAN95xx based USB 2.0 Ethernet Devices */

/* Tx command words */
#define TX_CMD_A_FIRST_SEG_		0x00002000
#define TX_CMD_A_LAST_SEG_		0x00001000

/* Rx status word */
#define RX_STS_FL_			0x3FFF0000	/* Frame Length */
#define RX_STS_ES_			0x00008000	/* Error Summary */

/* SCSRs */
#define ID_REV				0x00

#define INT_STS				0x08

#define TX_CFG				0x10
#define TX_CFG_ON_			0x00000004

#define HW_CFG				0x14
#define HW_CFG_BIR_			0x00001000
#define HW_CFG_RXDOFF_			0x00000600
#define HW_CFG_MEF_			0x00000020
#define HW_CFG_BCE_			0x00000002
#define HW_CFG_LRST_			0x00000008

#define PM_CTRL				0x20
#define PM_CTL_PHY_RST_			0x00000010

#define AFC_CFG				0x2C

/*
 * Hi watermark = 15.5Kb (~10 mtu pkts)
 * low watermark = 3k (~2 mtu pkts)
 * backpressure duration = ~ 350us
 * Apply FC on any frame.
 */
#define AFC_CFG_DEFAULT			0x00F830A1

#define E2P_CMD				0x30
#define E2P_CMD_BUSY_			0x80000000
#define E2P_CMD_READ_			0x00000000
#define E2P_CMD_TIMEOUT_		0x00000400
#define E2P_CMD_LOADED_			0x00000200
#define E2P_CMD_ADDR_			0x000001FF

#define E2P_DATA			0x34

#define BURST_CAP			0x38

#define INT_EP_CTL			0x68
#define INT_EP_CTL_PHY_INT_		0x00008000

#define BULK_IN_DLY			0x6C

/* MAC CSRs */
#define MAC_CR				0x100
#define MAC_CR_MCPAS_			0x00080000
#define MAC_CR_PRMS_			0x00040000
#define MAC_CR_HPFILT_			0x00002000
#define MAC_CR_TXEN_			0x00000008
#define MAC_CR_RXEN_			0x00000004

#define ADDRH				0x104

#define ADDRL				0x108

#define MII_ADDR			0x114
#define MII_WRITE_			0x02
#define MII_BUSY_			0x01
#define MII_READ_			0x00 /* ~of MII Write bit */

#define MII_DATA			0x118

#define FLOW				0x11C

#define VLAN1				0x120

#define COE_CR				0x130
#define Tx_COE_EN_			0x00010000
#define Rx_COE_EN_			0x00000001

/* Vendor-specific PHY Definitions */
#define PHY_INT_SRC			29

#define PHY_INT_MASK			30
#define PHY_INT_MASK_ANEG_COMP_		((u16)0x0040)
#define PHY_INT_MASK_LINK_DOWN_		((u16)0x0010)
#define PHY_INT_MASK_DEFAULT_		(PHY_INT_MASK_ANEG_COMP_ | \
					 PHY_INT_MASK_LINK_DOWN_)

/* USB Vendor Requests */
#define USB_VENDOR_REQUEST_WRITE_REGISTER	0xA0
#define USB_VENDOR_REQUEST_READ_REGISTER	0xA1

/* Some extra defines */
#define HS_USB_PKT_SIZE			512
#define FS_USB_PKT_SIZE			64
#define DEFAULT_HS_BURST_CAP_SIZE	(16 * 1024 + 5 * HS_USB_PKT_SIZE)
#define DEFAULT_FS_BURST_CAP_SIZE	(6 * 1024 + 33 * FS_USB_PKT_SIZE)
#define DEFAULT_BULK_IN_DELAY		0x00002000
#define MAX_SINGLE_PACKET_SIZE		2048
#define EEPROM_MAC_OFFSET		0x01
#define SMSC95XX_INTERNAL_PHY_ID	1
#define ETH_P_8021Q	0x8100          /* 802.1Q VLAN Extended Header  */

/* local defines */
#define SMSC95XX_BASE_NAME "sms"
#define USB_CTRL_SET_TIMEOUT 5000
#define USB_CTRL_GET_TIMEOUT 5000
#define USB_BULK_SEND_TIMEOUT 5000
#define USB_BULK_RECV_TIMEOUT 5000

#define AX_RX_URB_SIZE 2048
#define PHY_CONNECT_TIMEOUT 5000

#define TURBO_MODE

/* local vars */
static int curr_eth_dev; /* index for name of next device detected */


/*
 * Smsc95xx infrastructure commands
 */
static int smsc95xx_write_reg(struct ueth_data *dev, u32 index, u32 data)
{
	int len;
	ALLOC_CACHE_ALIGN_BUFFER(u32, tmpbuf, 1);

	cpu_to_le32s(&data);
	tmpbuf[0] = data;

	len = usb_control_msg(dev->pusb_dev, usb_sndctrlpipe(dev->pusb_dev, 0),
		USB_VENDOR_REQUEST_WRITE_REGISTER,
		USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		00, index, tmpbuf, sizeof(data), USB_CTRL_SET_TIMEOUT);
	if (len != sizeof(data)) {
		debug("smsc95xx_write_reg failed: index=%d, data=%d, len=%d",
		      index, data, len);
		return -1;
	}
	return 0;
}

static int smsc95xx_read_reg(struct ueth_data *dev, u32 index, u32 *data)
{
	int len;
	ALLOC_CACHE_ALIGN_BUFFER(u32, tmpbuf, 1);

	len = usb_control_msg(dev->pusb_dev, usb_rcvctrlpipe(dev->pusb_dev, 0),
		USB_VENDOR_REQUEST_READ_REGISTER,
		USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		00, index, tmpbuf, sizeof(data), USB_CTRL_GET_TIMEOUT);
	*data = tmpbuf[0];
	if (len != sizeof(data)) {
		debug("smsc95xx_read_reg failed: index=%d, len=%d",
		      index, len);
		return -1;
	}

	le32_to_cpus(data);
	return 0;
}

/* Loop until the read is completed with timeout */
static int smsc95xx_phy_wait_not_busy(struct ueth_data *dev)
{
	unsigned long start_time = get_timer(0);
	u32 val;

	do {
		smsc95xx_read_reg(dev, MII_ADDR, &val);
		if (!(val & MII_BUSY_))
			return 0;
	} while (get_timer(start_time) < 1 * 1000 * 1000);

	return -1;
}

static int smsc95xx_mdio_read(struct ueth_data *dev, int phy_id, int idx)
{
	u32 val, addr;

	/* confirm MII not busy */
	if (smsc95xx_phy_wait_not_busy(dev)) {
		debug("MII is busy in smsc95xx_mdio_read\n");
		return -1;
	}

	/* set the address, index & direction (read from PHY) */
	addr = (phy_id << 11) | (idx << 6) | MII_READ_;
	smsc95xx_write_reg(dev, MII_ADDR, addr);

	if (smsc95xx_phy_wait_not_busy(dev)) {
		debug("Timed out reading MII reg %02X\n", idx);
		return -1;
	}

	smsc95xx_read_reg(dev, MII_DATA, &val);

	return (u16)(val & 0xFFFF);
}

static void smsc95xx_mdio_write(struct ueth_data *dev, int phy_id, int idx,
				int regval)
{
	u32 val, addr;

	/* confirm MII not busy */
	if (smsc95xx_phy_wait_not_busy(dev)) {
		debug("MII is busy in smsc95xx_mdio_write\n");
		return;
	}

	val = regval;
	smsc95xx_write_reg(dev, MII_DATA, val);

	/* set the address, index & direction (write to PHY) */
	addr = (phy_id << 11) | (idx << 6) | MII_WRITE_;
	smsc95xx_write_reg(dev, MII_ADDR, addr);

	if (smsc95xx_phy_wait_not_busy(dev))
		debug("Timed out writing MII reg %02X\n", idx);
}

static int smsc95xx_eeprom_confirm_not_busy(struct ueth_data *dev)
{
	unsigned long start_time = get_timer(0);
	u32 val;

	do {
		smsc95xx_read_reg(dev, E2P_CMD, &val);
		if (!(val & E2P_CMD_LOADED_)) {
			debug("No EEPROM present\n");
			return -1;
		}
		if (!(val & E2P_CMD_BUSY_))
			return 0;
		udelay(40);
	} while (get_timer(start_time) < 1 * 1000 * 1000);

	debug("EEPROM is busy\n");
	return -1;
}

static int smsc95xx_wait_eeprom(struct ueth_data *dev)
{
	unsigned long start_time = get_timer(0);
	u32 val;

	do {
		smsc95xx_read_reg(dev, E2P_CMD, &val);
		if (!(val & E2P_CMD_BUSY_) || (val & E2P_CMD_TIMEOUT_))
			break;
		udelay(40);
	} while (get_timer(start_time) < 1 * 1000 * 1000);

	if (val & (E2P_CMD_TIMEOUT_ | E2P_CMD_BUSY_)) {
		debug("EEPROM read operation timeout\n");
		return -1;
	}
	return 0;
}

static int smsc95xx_read_eeprom(struct ueth_data *dev, u32 offset, u32 length,
				u8 *data)
{
	u32 val;
	int i, ret;

	ret = smsc95xx_eeprom_confirm_not_busy(dev);
	if (ret)
		return ret;

	for (i = 0; i < length; i++) {
		val = E2P_CMD_BUSY_ | E2P_CMD_READ_ | (offset & E2P_CMD_ADDR_);
		smsc95xx_write_reg(dev, E2P_CMD, val);

		ret = smsc95xx_wait_eeprom(dev);
		if (ret < 0)
			return ret;

		smsc95xx_read_reg(dev, E2P_DATA, &val);
		data[i] = val & 0xFF;
		offset++;
	}
	return 0;
}

/*
 * mii_nway_restart - restart NWay (autonegotiation) for this interface
 *
 * Returns 0 on success, negative on error.
 */
static int mii_nway_restart(struct ueth_data *dev)
{
	int bmcr;
	int r = -1;

	/* if autoneg is off, it's an error */
	bmcr = smsc95xx_mdio_read(dev, dev->phy_id, MII_BMCR);

	if (bmcr & BMCR_ANENABLE) {
		bmcr |= BMCR_ANRESTART;
		smsc95xx_mdio_write(dev, dev->phy_id, MII_BMCR, bmcr);
		r = 0;
	}
	return r;
}

static int smsc95xx_phy_initialize(struct ueth_data *dev)
{
	smsc95xx_mdio_write(dev, dev->phy_id, MII_BMCR, BMCR_RESET);
	smsc95xx_mdio_write(dev, dev->phy_id, MII_ADVERTISE,
		ADVERTISE_ALL | ADVERTISE_CSMA | ADVERTISE_PAUSE_CAP |
		ADVERTISE_PAUSE_ASYM);

	/* read to clear */
	smsc95xx_mdio_read(dev, dev->phy_id, PHY_INT_SRC);

	smsc95xx_mdio_write(dev, dev->phy_id, PHY_INT_MASK,
		PHY_INT_MASK_DEFAULT_);
	mii_nway_restart(dev);

	debug("phy initialised succesfully\n");
	return 0;
}

static int smsc95xx_init_mac_address(struct eth_device *eth,
		struct ueth_data *dev)
{
	/* try reading mac address from EEPROM */
	if (smsc95xx_read_eeprom(dev, EEPROM_MAC_OFFSET, ETH_ALEN,
			eth->enetaddr) == 0) {
		if (is_valid_ether_addr(eth->enetaddr)) {
			/* eeprom values are valid so use them */
			debug("MAC address read from EEPROM\n");
			return 0;
		}
	}

	/*
	 * No eeprom, or eeprom values are invalid. Generating a random MAC
	 * address is not safe. Just return an error.
	 */
	return -1;
}

static int smsc95xx_write_hwaddr(struct eth_device *eth)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	u32 addr_lo = __get_unaligned_le32(&eth->enetaddr[0]);
	u32 addr_hi = __get_unaligned_le16(&eth->enetaddr[4]);
	int ret;

	/* set hardware address */
	debug("** %s()\n", __func__);
	ret = smsc95xx_write_reg(dev, ADDRL, addr_lo);
	if (ret < 0)
		return ret;

	ret = smsc95xx_write_reg(dev, ADDRH, addr_hi);
	if (ret < 0)
		return ret;

	debug("MAC %pM\n", eth->enetaddr);
	dev->have_hwaddr = 1;
	return 0;
}

/* Enable or disable Tx & Rx checksum offload engines */
static int smsc95xx_set_csums(struct ueth_data *dev,
		int use_tx_csum, int use_rx_csum)
{
	u32 read_buf;
	int ret = smsc95xx_read_reg(dev, COE_CR, &read_buf);
	if (ret < 0)
		return ret;

	if (use_tx_csum)
		read_buf |= Tx_COE_EN_;
	else
		read_buf &= ~Tx_COE_EN_;

	if (use_rx_csum)
		read_buf |= Rx_COE_EN_;
	else
		read_buf &= ~Rx_COE_EN_;

	ret = smsc95xx_write_reg(dev, COE_CR, read_buf);
	if (ret < 0)
		return ret;

	debug("COE_CR = 0x%08x\n", read_buf);
	return 0;
}

static void smsc95xx_set_multicast(struct ueth_data *dev)
{
	/* No multicast in u-boot */
	dev->mac_cr &= ~(MAC_CR_PRMS_ | MAC_CR_MCPAS_ | MAC_CR_HPFILT_);
}

/* starts the TX path */
static void smsc95xx_start_tx_path(struct ueth_data *dev)
{
	u32 reg_val;

	/* Enable Tx at MAC */
	dev->mac_cr |= MAC_CR_TXEN_;

	smsc95xx_write_reg(dev, MAC_CR, dev->mac_cr);

	/* Enable Tx at SCSRs */
	reg_val = TX_CFG_ON_;
	smsc95xx_write_reg(dev, TX_CFG, reg_val);
}

/* Starts the Receive path */
static void smsc95xx_start_rx_path(struct ueth_data *dev)
{
	dev->mac_cr |= MAC_CR_RXEN_;
	smsc95xx_write_reg(dev, MAC_CR, dev->mac_cr);
}

/*
 * Smsc95xx callbacks
 */
static int smsc95xx_init(struct eth_device *eth, bd_t *bd)
{
	int ret;
	u32 write_buf;
	u32 read_buf;
	u32 burst_cap;
	int timeout;
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
#define TIMEOUT_RESOLUTION 50	/* ms */
	int link_detected;

	debug("** %s()\n", __func__);
	dev->phy_id = SMSC95XX_INTERNAL_PHY_ID; /* fixed phy id */

	write_buf = HW_CFG_LRST_;
	ret = smsc95xx_write_reg(dev, HW_CFG, write_buf);
	if (ret < 0)
		return ret;

	timeout = 0;
	do {
		ret = smsc95xx_read_reg(dev, HW_CFG, &read_buf);
		if (ret < 0)
			return ret;
		udelay(10 * 1000);
		timeout++;
	} while ((read_buf & HW_CFG_LRST_) && (timeout < 100));

	if (timeout >= 100) {
		debug("timeout waiting for completion of Lite Reset\n");
		return -1;
	}

	write_buf = PM_CTL_PHY_RST_;
	ret = smsc95xx_write_reg(dev, PM_CTRL, write_buf);
	if (ret < 0)
		return ret;

	timeout = 0;
	do {
		ret = smsc95xx_read_reg(dev, PM_CTRL, &read_buf);
		if (ret < 0)
			return ret;
		udelay(10 * 1000);
		timeout++;
	} while ((read_buf & PM_CTL_PHY_RST_) && (timeout < 100));
	if (timeout >= 100) {
		debug("timeout waiting for PHY Reset\n");
		return -1;
	}
	if (!dev->have_hwaddr && smsc95xx_init_mac_address(eth, dev) == 0)
		dev->have_hwaddr = 1;
	if (!dev->have_hwaddr) {
		puts("Error: SMSC95xx: No MAC address set - set usbethaddr\n");
		return -1;
	}
	if (smsc95xx_write_hwaddr(eth) < 0)
		return -1;

	ret = smsc95xx_read_reg(dev, HW_CFG, &read_buf);
	if (ret < 0)
		return ret;
	debug("Read Value from HW_CFG : 0x%08x\n", read_buf);

	read_buf |= HW_CFG_BIR_;
	ret = smsc95xx_write_reg(dev, HW_CFG, read_buf);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(dev, HW_CFG, &read_buf);
	if (ret < 0)
		return ret;
	debug("Read Value from HW_CFG after writing "
		"HW_CFG_BIR_: 0x%08x\n", read_buf);

#ifdef TURBO_MODE
	if (dev->pusb_dev->speed == USB_SPEED_HIGH) {
		burst_cap = DEFAULT_HS_BURST_CAP_SIZE / HS_USB_PKT_SIZE;
		dev->rx_urb_size = DEFAULT_HS_BURST_CAP_SIZE;
	} else {
		burst_cap = DEFAULT_FS_BURST_CAP_SIZE / FS_USB_PKT_SIZE;
		dev->rx_urb_size = DEFAULT_FS_BURST_CAP_SIZE;
	}
#else
	burst_cap = 0;
	dev->rx_urb_size = MAX_SINGLE_PACKET_SIZE;
#endif
	debug("rx_urb_size=%ld\n", (ulong)dev->rx_urb_size);

	ret = smsc95xx_write_reg(dev, BURST_CAP, burst_cap);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(dev, BURST_CAP, &read_buf);
	if (ret < 0)
		return ret;
	debug("Read Value from BURST_CAP after writing: 0x%08x\n", read_buf);

	read_buf = DEFAULT_BULK_IN_DELAY;
	ret = smsc95xx_write_reg(dev, BULK_IN_DLY, read_buf);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(dev, BULK_IN_DLY, &read_buf);
	if (ret < 0)
		return ret;
	debug("Read Value from BULK_IN_DLY after writing: "
			"0x%08x\n", read_buf);

	ret = smsc95xx_read_reg(dev, HW_CFG, &read_buf);
	if (ret < 0)
		return ret;
	debug("Read Value from HW_CFG: 0x%08x\n", read_buf);

#ifdef TURBO_MODE
	read_buf |= (HW_CFG_MEF_ | HW_CFG_BCE_);
#endif
	read_buf &= ~HW_CFG_RXDOFF_;

#define NET_IP_ALIGN 0
	read_buf |= NET_IP_ALIGN << 9;

	ret = smsc95xx_write_reg(dev, HW_CFG, read_buf);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(dev, HW_CFG, &read_buf);
	if (ret < 0)
		return ret;
	debug("Read Value from HW_CFG after writing: 0x%08x\n", read_buf);

	write_buf = 0xFFFFFFFF;
	ret = smsc95xx_write_reg(dev, INT_STS, write_buf);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(dev, ID_REV, &read_buf);
	if (ret < 0)
		return ret;
	debug("ID_REV = 0x%08x\n", read_buf);

	/* Init Tx */
	write_buf = 0;
	ret = smsc95xx_write_reg(dev, FLOW, write_buf);
	if (ret < 0)
		return ret;

	read_buf = AFC_CFG_DEFAULT;
	ret = smsc95xx_write_reg(dev, AFC_CFG, read_buf);
	if (ret < 0)
		return ret;

	ret = smsc95xx_read_reg(dev, MAC_CR, &dev->mac_cr);
	if (ret < 0)
		return ret;

	/* Init Rx. Set Vlan */
	write_buf = (u32)ETH_P_8021Q;
	ret = smsc95xx_write_reg(dev, VLAN1, write_buf);
	if (ret < 0)
		return ret;

	/* Disable checksum offload engines */
	ret = smsc95xx_set_csums(dev, 0, 0);
	if (ret < 0) {
		debug("Failed to set csum offload: %d\n", ret);
		return ret;
	}
	smsc95xx_set_multicast(dev);

	if (smsc95xx_phy_initialize(dev) < 0)
		return -1;
	ret = smsc95xx_read_reg(dev, INT_EP_CTL, &read_buf);
	if (ret < 0)
		return ret;

	/* enable PHY interrupts */
	read_buf |= INT_EP_CTL_PHY_INT_;

	ret = smsc95xx_write_reg(dev, INT_EP_CTL, read_buf);
	if (ret < 0)
		return ret;

	smsc95xx_start_tx_path(dev);
	smsc95xx_start_rx_path(dev);

	timeout = 0;
	do {
		link_detected = smsc95xx_mdio_read(dev, dev->phy_id, MII_BMSR)
			& BMSR_LSTATUS;
		if (!link_detected) {
			if (timeout == 0)
				printf("Waiting for Ethernet connection... ");
			udelay(TIMEOUT_RESOLUTION * 1000);
			timeout += TIMEOUT_RESOLUTION;
		}
	} while (!link_detected && timeout < PHY_CONNECT_TIMEOUT);
	if (link_detected) {
		if (timeout != 0)
			printf("done.\n");
	} else {
		printf("unable to connect.\n");
		return -1;
	}
	return 0;
}

static int smsc95xx_send(struct eth_device *eth, void* packet, int length)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	int err;
	int actual_len;
	u32 tx_cmd_a;
	u32 tx_cmd_b;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, msg,
				 PKTSIZE + sizeof(tx_cmd_a) + sizeof(tx_cmd_b));

	debug("** %s(), len %d, buf %#x\n", __func__, length, (int)msg);
	if (length > PKTSIZE)
		return -1;

	tx_cmd_a = (u32)length | TX_CMD_A_FIRST_SEG_ | TX_CMD_A_LAST_SEG_;
	tx_cmd_b = (u32)length;
	cpu_to_le32s(&tx_cmd_a);
	cpu_to_le32s(&tx_cmd_b);

	/* prepend cmd_a and cmd_b */
	memcpy(msg, &tx_cmd_a, sizeof(tx_cmd_a));
	memcpy(msg + sizeof(tx_cmd_a), &tx_cmd_b, sizeof(tx_cmd_b));
	memcpy(msg + sizeof(tx_cmd_a) + sizeof(tx_cmd_b), (void *)packet,
	       length);
	err = usb_bulk_msg(dev->pusb_dev,
				usb_sndbulkpipe(dev->pusb_dev, dev->ep_out),
				(void *)msg,
				length + sizeof(tx_cmd_a) + sizeof(tx_cmd_b),
				&actual_len,
				USB_BULK_SEND_TIMEOUT);
	debug("Tx: len = %u, actual = %u, err = %d\n",
	      length + sizeof(tx_cmd_a) + sizeof(tx_cmd_b),
	      actual_len, err);
	return err;
}

static int smsc95xx_recv(struct eth_device *eth)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	DEFINE_CACHE_ALIGN_BUFFER(unsigned char, recv_buf, AX_RX_URB_SIZE);
	unsigned char *buf_ptr;
	int err;
	int actual_len;
	u32 packet_len;
	int cur_buf_align;

	debug("** %s()\n", __func__);
	err = usb_bulk_msg(dev->pusb_dev,
				usb_rcvbulkpipe(dev->pusb_dev, dev->ep_in),
				(void *)recv_buf,
				AX_RX_URB_SIZE,
				&actual_len,
				USB_BULK_RECV_TIMEOUT);
	debug("Rx: len = %u, actual = %u, err = %d\n", AX_RX_URB_SIZE,
	      actual_len, err);
	if (err != 0) {
		debug("Rx: failed to receive\n");
		return -1;
	}
	if (actual_len > AX_RX_URB_SIZE) {
		debug("Rx: received too many bytes %d\n", actual_len);
		return -1;
	}

	buf_ptr = recv_buf;
	while (actual_len > 0) {
		/*
		 * 1st 4 bytes contain the length of the actual data plus error
		 * info. Extract data length.
		 */
		if (actual_len < sizeof(packet_len)) {
			debug("Rx: incomplete packet length\n");
			return -1;
		}
		memcpy(&packet_len, buf_ptr, sizeof(packet_len));
		le32_to_cpus(&packet_len);
		if (packet_len & RX_STS_ES_) {
			debug("Rx: Error header=%#x", packet_len);
			return -1;
		}
		packet_len = ((packet_len & RX_STS_FL_) >> 16);

		if (packet_len > actual_len - sizeof(packet_len)) {
			debug("Rx: too large packet: %d\n", packet_len);
			return -1;
		}

		/* Notify net stack */
		NetReceive(buf_ptr + sizeof(packet_len), packet_len - 4);

		/* Adjust for next iteration */
		actual_len -= sizeof(packet_len) + packet_len;
		buf_ptr += sizeof(packet_len) + packet_len;
		cur_buf_align = (int)buf_ptr - (int)recv_buf;

		if (cur_buf_align & 0x03) {
			int align = 4 - (cur_buf_align & 0x03);

			actual_len -= align;
			buf_ptr += align;
		}
	}
	return err;
}

static void smsc95xx_halt(struct eth_device *eth)
{
	debug("** %s()\n", __func__);
}

/*
 * SMSC probing functions
 */
void smsc95xx_eth_before_probe(void)
{
	curr_eth_dev = 0;
}

struct smsc95xx_dongle {
	unsigned short vendor;
	unsigned short product;
};

static const struct smsc95xx_dongle smsc95xx_dongles[] = {
	{ 0x0424, 0xec00 },	/* LAN9512/LAN9514 Ethernet */
	{ 0x0424, 0x9500 },	/* LAN9500 Ethernet */
	{ 0x0000, 0x0000 }	/* END - Do not remove */
};

/* Probe to see if a new device is actually an SMSC device */
int smsc95xx_eth_probe(struct usb_device *dev, unsigned int ifnum,
		      struct ueth_data *ss)
{
	struct usb_interface *iface;
	struct usb_interface_descriptor *iface_desc;
	int i;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];
	iface_desc = &dev->config.if_desc[ifnum].desc;

	for (i = 0; smsc95xx_dongles[i].vendor != 0; i++) {
		if (dev->descriptor.idVendor == smsc95xx_dongles[i].vendor &&
		    dev->descriptor.idProduct == smsc95xx_dongles[i].product)
			/* Found a supported dongle */
			break;
	}
	if (smsc95xx_dongles[i].vendor == 0)
		return 0;

	/* At this point, we know we've got a live one */
	debug("\n\nUSB Ethernet device detected\n");
	memset(ss, '\0', sizeof(struct ueth_data));

	/* Initialize the ueth_data structure with some useful info */
	ss->ifnum = ifnum;
	ss->pusb_dev = dev;
	ss->subclass = iface_desc->bInterfaceSubClass;
	ss->protocol = iface_desc->bInterfaceProtocol;

	/*
	 * We are expecting a minimum of 3 endpoints - in, out (bulk), and int.
	 * We will ignore any others.
	 */
	for (i = 0; i < iface_desc->bNumEndpoints; i++) {
		/* is it an BULK endpoint? */
		if ((iface->ep_desc[i].bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
			if (iface->ep_desc[i].bEndpointAddress & USB_DIR_IN)
				ss->ep_in =
					iface->ep_desc[i].bEndpointAddress &
					USB_ENDPOINT_NUMBER_MASK;
			else
				ss->ep_out =
					iface->ep_desc[i].bEndpointAddress &
					USB_ENDPOINT_NUMBER_MASK;
		}

		/* is it an interrupt endpoint? */
		if ((iface->ep_desc[i].bmAttributes &
		    USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT) {
			ss->ep_int = iface->ep_desc[i].bEndpointAddress &
				USB_ENDPOINT_NUMBER_MASK;
			ss->irqinterval = iface->ep_desc[i].bInterval;
		}
	}
	debug("Endpoints In %d Out %d Int %d\n",
		  ss->ep_in, ss->ep_out, ss->ep_int);

	/* Do some basic sanity checks, and bail if we find a problem */
	if (usb_set_interface(dev, iface_desc->bInterfaceNumber, 0) ||
	    !ss->ep_in || !ss->ep_out || !ss->ep_int) {
		debug("Problems with device\n");
		return 0;
	}
	dev->privptr = (void *)ss;
	return 1;
}

int smsc95xx_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
				struct eth_device *eth)
{
	debug("** %s()\n", __func__);
	if (!eth) {
		debug("%s: missing parameter.\n", __func__);
		return 0;
	}
	sprintf(eth->name, "%s%d", SMSC95XX_BASE_NAME, curr_eth_dev++);
	eth->init = smsc95xx_init;
	eth->send = smsc95xx_send;
	eth->recv = smsc95xx_recv;
	eth->halt = smsc95xx_halt;
	eth->write_hwaddr = smsc95xx_write_hwaddr;
	eth->priv = ss;
	return 1;
}
