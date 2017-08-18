/*
 * Copyright (c) 2011 The Chromium OS Authors.
 *
 * Patched for AX88772B by Antmicro Ltd <www.antmicro.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <usb.h>
#include <malloc.h>
#include <memalign.h>
#include <linux/mii.h>
#include "usb_ether.h"

/* ASIX AX8817X based USB 2.0 Ethernet Devices */

#define AX_CMD_SET_SW_MII		0x06
#define AX_CMD_READ_MII_REG		0x07
#define AX_CMD_WRITE_MII_REG		0x08
#define AX_CMD_SET_HW_MII		0x0a
#define AX_CMD_READ_EEPROM		0x0b
#define AX_CMD_WRITE_ENABLE		0x0d
#define AX_CMD_WRITE_DISABLE	0x0e
#define AX_CMD_READ_RX_CTL		0x0f
#define AX_CMD_WRITE_RX_CTL		0x10
#define AX_CMD_WRITE_IPG0		0x12
#define AX_CMD_READ_NODE_ID		0x13
#define AX_CMD_WRITE_NODE_ID	0x14
#define AX_CMD_READ_PHY_ID		0x19
#define AX_CMD_WRITE_MEDIUM_MODE	0x1b
#define AX_CMD_READ_GPIOS		0x1e
#define AX_CMD_WRITE_GPIOS		0x1f
#define AX_CMD_SW_RESET			0x20
#define AX_CMD_SW_PHY_SELECT		0x22

#define AX_SWRESET_CLEAR		0x00
#define AX_SWRESET_PRTE			0x04
#define AX_SWRESET_PRL			0x08
#define AX_SWRESET_IPRL			0x20
#define AX_SWRESET_IPPD			0x40

#define AX88772_IPG0_DEFAULT		0x15
#define AX88772_IPG1_DEFAULT		0x0c
#define AX88772_IPG2_DEFAULT		0x12

/* AX88772 & AX88178 Medium Mode Register */
#define AX_MEDIUM_PF		0x0080
#define AX_MEDIUM_JFE		0x0040
#define AX_MEDIUM_TFC		0x0020
#define AX_MEDIUM_RFC		0x0010
#define AX_MEDIUM_ENCK		0x0008
#define AX_MEDIUM_AC		0x0004
#define AX_MEDIUM_FD		0x0002
#define AX_MEDIUM_GM		0x0001
#define AX_MEDIUM_SM		0x1000
#define AX_MEDIUM_SBP		0x0800
#define AX_MEDIUM_PS		0x0200
#define AX_MEDIUM_RE		0x0100

#define AX88178_MEDIUM_DEFAULT	\
	(AX_MEDIUM_PS | AX_MEDIUM_FD | AX_MEDIUM_AC | \
	 AX_MEDIUM_RFC | AX_MEDIUM_TFC | AX_MEDIUM_JFE | \
	 AX_MEDIUM_RE)

#define AX88772_MEDIUM_DEFAULT	\
	(AX_MEDIUM_FD | AX_MEDIUM_RFC | \
	 AX_MEDIUM_TFC | AX_MEDIUM_PS | \
	 AX_MEDIUM_AC | AX_MEDIUM_RE)

/* AX88772 & AX88178 RX_CTL values */
#define AX_RX_CTL_RH2M		0x0200	/* 32-bit aligned RX IP header */
#define AX_RX_CTL_RH1M		0x0100	/* Enable RX header format type 1 */
#define AX_RX_CTL_SO			0x0080
#define AX_RX_CTL_AB			0x0008
#define AX_RX_HEADER_DEFAULT	(AX_RX_CTL_RH1M | AX_RX_CTL_RH2M)

#define AX_DEFAULT_RX_CTL	\
	(AX_RX_CTL_SO | AX_RX_CTL_AB)

/* GPIO 0..2 toggles */
#define AX_GPIO_GPO0EN		0x01	/* GPIO0 Output enable */
#define AX_GPIO_GPO_0		0x02	/* GPIO0 Output value */
#define AX_GPIO_GPO1EN		0x04	/* GPIO1 Output enable */
#define AX_GPIO_GPO_1		0x08	/* GPIO1 Output value */
#define AX_GPIO_GPO2EN		0x10	/* GPIO2 Output enable */
#define AX_GPIO_GPO_2		0x20	/* GPIO2 Output value */
#define AX_GPIO_RESERVED	0x40	/* Reserved */
#define AX_GPIO_RSE		0x80	/* Reload serial EEPROM */

/* local defines */
#define ASIX_BASE_NAME "asx"
#define USB_CTRL_SET_TIMEOUT 5000
#define USB_CTRL_GET_TIMEOUT 5000
#define USB_BULK_SEND_TIMEOUT 5000
#define USB_BULK_RECV_TIMEOUT 5000

#define AX_RX_URB_SIZE 2048
#define AX88178_RX_URB_SIZE 1024 * 0x12
#define PHY_CONNECT_TIMEOUT 5000

/* PHY MODE */
#define PHY_MODE_MARVELL	0x0000
#define	PHY_MODE_RTL8211CL	0x000C

/* asix_flags defines */
#define FLAG_NONE			0
#define FLAG_TYPE_AX88172	(1U << 0)
#define FLAG_TYPE_AX88772	(1U << 1)
#define FLAG_TYPE_AX88772B	(1U << 2)
#define FLAG_TYPE_AX88178	(1U << 3)
#define FLAG_EEPROM_MAC		(1U << 8) /* initial mac address in eeprom */

#define ASIX_USB_VENDOR_ID	0x0b95
#define AX88772B_USB_PRODUCT_ID	0x772b
#define AX88178_USB_PRODUCT_ID	0x1780

/* driver private */
struct asix_private {
	int flags;
	int phymode; /* For ASIX88178 */
	int ledmode;
	int rx_urb_size;
	int supports_gmii;
	int advertising;
	int full_duplex;		
#ifdef CONFIG_DM_ETH
	struct ueth_data ueth;
#endif
	uint8_t rcv_buf[AX_RX_URB_SIZE * 8];
};

#ifndef CONFIG_DM_ETH
/* local vars */
static int curr_eth_dev; /* index for name of next device detected */
#endif

/*
 * Asix infrastructure commands
 */
static int asix_write_cmd(struct ueth_data *dev, u8 cmd, u16 value, u16 index,
			     u16 size, void *data)
{
	int len;

	debug("asix_write_cmd() cmd=0x%02x value=0x%04x index=0x%04x "
		"size=%d\n", cmd, value, index, size);

	len = usb_control_msg(
		dev->pusb_dev,
		usb_sndctrlpipe(dev->pusb_dev, 0),
		cmd,
		USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		value,
		index,
		data,
		size,
		USB_CTRL_SET_TIMEOUT);

	return len == size ? 0 : -1;
}

static int asix_read_cmd(struct ueth_data *dev, u8 cmd, u16 value, u16 index,
			    u16 size, void *data)
{
	int len;

	debug("asix_read_cmd() cmd=0x%02x value=0x%04x index=0x%04x size=%d\n",
		cmd, value, index, size);

	len = usb_control_msg(
		dev->pusb_dev,
		usb_rcvctrlpipe(dev->pusb_dev, 0),
		cmd,
		USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		value,
		index,
		data,
		size,
		USB_CTRL_GET_TIMEOUT);
	return len == size ? 0 : -1;
}

static inline int asix_set_sw_mii(struct ueth_data *dev)
{
	int ret;

	ret = asix_write_cmd(dev, AX_CMD_SET_SW_MII, 0x0000, 0, 0, NULL);
	if (ret < 0)
		debug("Failed to enable software MII access\n");
	return ret;
}

static inline int asix_set_hw_mii(struct ueth_data *dev)
{
	int ret;

	ret = asix_write_cmd(dev, AX_CMD_SET_HW_MII, 0x0000, 0, 0, NULL);
	if (ret < 0)
		debug("Failed to enable hardware MII access\n");
	return ret;
}

static int asix_mdio_read(struct ueth_data *dev, int phy_id, int loc)
{
	ALLOC_CACHE_ALIGN_BUFFER(__le16, res, 1);

	asix_set_sw_mii(dev);
	asix_read_cmd(dev, AX_CMD_READ_MII_REG, phy_id, (__u16)loc, 2, res);
	asix_set_hw_mii(dev);

	debug("asix_mdio_read() phy_id=0x%02x, loc=0x%02x, returns=0x%04x\n",
			phy_id, loc, le16_to_cpu(*res));

	return le16_to_cpu(*res);
}

static void
asix_mdio_write(struct ueth_data *dev, int phy_id, int loc, int val)
{
	ALLOC_CACHE_ALIGN_BUFFER(__le16, res, 1);
	*res = cpu_to_le16(val);

	debug("asix_mdio_write() phy_id=0x%02x, loc=0x%02x, val=0x%04x\n",
			phy_id, loc, val);
	asix_set_sw_mii(dev);
	asix_write_cmd(dev, AX_CMD_WRITE_MII_REG, phy_id, (__u16)loc, 2, res);
	asix_set_hw_mii(dev);
}

/*
 * Asix "high level" commands
 */
static int asix_sw_reset(struct ueth_data *dev, u8 flags)
{
	int ret;

	ret = asix_write_cmd(dev, AX_CMD_SW_RESET, flags, 0, 0, NULL);
	if (ret < 0)
		debug("Failed to send software reset: %02x\n", ret);
	else
		udelay(150 * 1000);

	return ret;
}

static inline int asix_get_phy_addr(struct ueth_data *dev)
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, buf, 2);

	int ret = asix_read_cmd(dev, AX_CMD_READ_PHY_ID, 0, 0, 2, buf);

	debug("asix_get_phy_addr()\n");

	if (ret < 0) {
		debug("Error reading PHYID register: %02x\n", ret);
		goto out;
	}
	debug("asix_get_phy_addr() returning 0x%02x%02x\n", buf[0], buf[1]);
	ret = buf[1];

out:
	return ret;
}

static int asix_write_medium_mode(struct ueth_data *dev, u16 mode)
{
	int ret;

	debug("asix_write_medium_mode() - mode = 0x%04x\n", mode);
	ret = asix_write_cmd(dev, AX_CMD_WRITE_MEDIUM_MODE, mode,
			0, 0, NULL);
	if (ret < 0) {
		debug("Failed to write Medium Mode mode to 0x%04x: %02x\n",
			mode, ret);
	}
	return ret;
}

static u16 asix_read_rx_ctl(struct ueth_data *dev)
{
	ALLOC_CACHE_ALIGN_BUFFER(__le16, v, 1);

	int ret = asix_read_cmd(dev, AX_CMD_READ_RX_CTL, 0, 0, 2, v);

	if (ret < 0)
		debug("Error reading RX_CTL register: %02x\n", ret);
	else
		ret = le16_to_cpu(*v);
	return ret;
}

static int asix_write_rx_ctl(struct ueth_data *dev, u16 mode)
{
	int ret;

	debug("asix_write_rx_ctl() - mode = 0x%04x\n", mode);
	ret = asix_write_cmd(dev, AX_CMD_WRITE_RX_CTL, mode, 0, 0, NULL);
	if (ret < 0) {
		debug("Failed to write RX_CTL mode to 0x%04x: %02x\n",
				mode, ret);
	}
	return ret;
}

static int asix_write_gpio(struct ueth_data *dev, u16 value, int sleep)
{
	int ret;

	debug("asix_write_gpio() - value = 0x%04x\n", value);
	ret = asix_write_cmd(dev, AX_CMD_WRITE_GPIOS, value, 0, 0, NULL);
	if (ret < 0) {
		debug("Failed to write GPIO value 0x%04x: %02x\n",
			value, ret);
	}
	if (sleep)
		udelay(sleep * 1000);

	return ret;
}

static int asix_write_hwaddr_common(struct ueth_data *dev, uint8_t *enetaddr)
{
	int ret;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buf, ETH_ALEN);

	memcpy(buf, enetaddr, ETH_ALEN);

	ret = asix_write_cmd(dev, AX_CMD_WRITE_NODE_ID, 0, 0, ETH_ALEN, buf);
	if (ret < 0)
		debug("Failed to set MAC address: %02x\n", ret);

	return ret;
}

/*
 * mii commands
 */

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
	bmcr = asix_mdio_read(dev, dev->phy_id, MII_BMCR);

	if (bmcr & BMCR_ANENABLE) {
		bmcr |= BMCR_ANRESTART;
		asix_mdio_write(dev, dev->phy_id, MII_BMCR, bmcr);
		r = 0;
	}

	return r;
}

static int asix_read_mac_common(struct ueth_data *dev,
				struct asix_private *priv, uint8_t *enetaddr)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buf, ETH_ALEN);
	int i;

	if (priv->flags & FLAG_EEPROM_MAC) {
		for (i = 0; i < (ETH_ALEN >> 1); i++) {
			if (asix_read_cmd(dev, AX_CMD_READ_EEPROM,
					  0x04 + i, 0, 2, buf) < 0) {
				debug("Failed to read SROM address 04h.\n");
				return -1;
			}
			memcpy(enetaddr + i * 2, buf, 2);
		}
	} else {
		if (asix_read_cmd(dev, AX_CMD_READ_NODE_ID, 0, 0, ETH_ALEN, buf)
		     < 0) {
			debug("Failed to read MAC address.\n");
			return -1;
		}
		memcpy(enetaddr, buf, ETH_ALEN);
	}

	return 0;
}

static int asix_basic_reset(struct ueth_data *dev, struct asix_private *priv)
{
	int embd_phy;
	u16 rx_ctl;

	if (asix_write_gpio(dev,
			AX_GPIO_RSE | AX_GPIO_GPO_2 | AX_GPIO_GPO2EN, 5) < 0)
		return -1;

	/* 0x10 is the phy id of the embedded 10/100 ethernet phy */
	embd_phy = ((asix_get_phy_addr(dev) & 0x1f) == 0x10 ? 1 : 0);
	if (asix_write_cmd(dev, AX_CMD_SW_PHY_SELECT,
				embd_phy, 0, 0, NULL) < 0) {
		debug("Select PHY #1 failed\n");
		return -1;
	}

	if (asix_sw_reset(dev, AX_SWRESET_IPPD | AX_SWRESET_PRL) < 0)
		return -1;

	if (asix_sw_reset(dev, AX_SWRESET_CLEAR) < 0)
		return -1;

	if (embd_phy) {
		if (asix_sw_reset(dev, AX_SWRESET_IPRL) < 0)
			return -1;
	} else {
		if (asix_sw_reset(dev, AX_SWRESET_PRTE) < 0)
			return -1;
	}

	rx_ctl = asix_read_rx_ctl(dev);
	debug("RX_CTL is 0x%04x after software reset\n", rx_ctl);
	if (asix_write_rx_ctl(dev, 0x0000) < 0)
		return -1;

	rx_ctl = asix_read_rx_ctl(dev);
	debug("RX_CTL is 0x%04x setting to 0x0000\n", rx_ctl);

	dev->phy_id = asix_get_phy_addr(dev);
	if (dev->phy_id < 0)
		debug("Failed to read phy id\n");

	asix_mdio_write(dev, dev->phy_id, MII_BMCR, BMCR_RESET);
	asix_mdio_write(dev, dev->phy_id, MII_ADVERTISE,
			ADVERTISE_ALL | ADVERTISE_CSMA);
	mii_nway_restart(dev);

	if (asix_write_medium_mode(dev, AX88772_MEDIUM_DEFAULT) < 0)
		return -1;

	if (asix_write_cmd(dev, AX_CMD_WRITE_IPG0,
				AX88772_IPG0_DEFAULT | AX88772_IPG1_DEFAULT,
				AX88772_IPG2_DEFAULT, 0, NULL) < 0) {
		debug("Write IPG,IPG1,IPG2 failed\n");
		return -1;
	}

	return 0;
}

static int asix_init_common(struct ueth_data *dev, uint8_t *enetaddr)
{
	int timeout = 0;
#define TIMEOUT_RESOLUTION 50	/* ms */
	int link_detected;
	u32 ctl = AX_DEFAULT_RX_CTL;	
	
	debug("** %s()\n", __func__);

	if ((dev->pusb_dev->descriptor.idVendor == ASIX_USB_VENDOR_ID) &&
	    (dev->pusb_dev->descriptor.idProduct == AX88772B_USB_PRODUCT_ID))
		ctl |= AX_RX_HEADER_DEFAULT;

	if ((dev->pusb_dev->descriptor.idVendor == ASIX_USB_VENDOR_ID) &&
	    (dev->pusb_dev->descriptor.idProduct == AX88178_USB_PRODUCT_ID))
		ctl |= AX_RX_HEADER_DEFAULT;

	if (asix_write_rx_ctl(dev, ctl) < 0)
		goto out_err;

	if (asix_write_hwaddr_common(dev, enetaddr) < 0)
		goto out_err;

	do {
		link_detected = asix_mdio_read(dev, dev->phy_id, MII_BMSR) &
			BMSR_LSTATUS;
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
		goto out_err;
	}

	/*
	 * Wait some more to avoid timeout on first transfer
	 * (e.g. EHCI timed out on TD - token=0x8008d80)
	 */
	mdelay(25);

	return 0;
out_err:
	return -1;
}


/* Get the PHY Identifier from the PHYSID1 & PHYSID2 MII registers */
static u32 asix_get_phyid(struct ueth_data *dev)
{
	int phy_reg;
	u32 phy_id;
	int i;
	printf("%s enter\n", __func__);
	/* Poll for the rare case the FW or phy isn't ready yet.  */
	for (i = 0; i < 100; i++) {
		phy_reg = asix_mdio_read(dev, dev->phy_id, MII_PHYSID1);
		if (phy_reg != 0 && phy_reg != 0xFFFF)
			break;
		mdelay(1);
	}

	if (phy_reg <= 0 || phy_reg == 0xFFFF)
		return 0;

	phy_id = (phy_reg & 0xffff) << 16;

	phy_reg = asix_mdio_read(dev, dev->phy_id, MII_PHYSID2);
	if (phy_reg < 0)
		return 0;

	phy_id |= (phy_reg & 0xffff);
	printf("%s exit\n", __func__);
	return phy_id;
}


static int rtl8211cl_phy_init(struct ueth_data *dev, struct asix_private *priv)
{
	printf("%s enter\n", __func__);

	asix_mdio_write (dev, dev->phy_id, 0x1f, 0x0005);
	asix_mdio_write (dev, dev->phy_id, 0x0c, 0);
	asix_mdio_write (dev, dev->phy_id, 0x01,
		asix_mdio_read (dev, dev->phy_id, 0x01) | 0x0080);
	asix_mdio_write (dev, dev->phy_id, 0x1f, 0);

	if (priv->ledmode == 12) {
		asix_mdio_write (dev, dev->phy_id, 0x1f, 0x0002);
		asix_mdio_write (dev, dev->phy_id, 0x1a, 0x00cb);
		asix_mdio_write (dev, dev->phy_id, 0x1f, 0);
	}
	printf("%s enter\n", __func__);
	return 0;
}

static int asix_send_common(struct ueth_data *dev, void *packet, int length)
{
	int err;
	u32 packet_len;
	int actual_len;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, msg,
		PKTSIZE + sizeof(packet_len));

	debug("** %s(), len %d\n", __func__, length);

	packet_len = (((length) ^ 0x0000ffff) << 16) + (length);
	cpu_to_le32s(&packet_len);

	memcpy(msg, &packet_len, sizeof(packet_len));
	memcpy(msg + sizeof(packet_len), (void *)packet, length);

	err = usb_bulk_msg(dev->pusb_dev,
				usb_sndbulkpipe(dev->pusb_dev, dev->ep_out),
				(void *)msg,
				length + sizeof(packet_len),
				&actual_len,
				USB_BULK_SEND_TIMEOUT);
	debug("Tx: len = %zu, actual = %u, err = %d\n",
			length + sizeof(packet_len), actual_len, err);

	return err;
}

#ifndef CONFIG_DM_ETH
/*
 * Asix callbacks
 */
static int asix_init(struct eth_device *eth, bd_t *bd)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;

	return asix_init_common(dev, eth->enetaddr);
}

static int asix_send(struct eth_device *eth, void *packet, int length)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;

	return asix_send_common(dev, packet, length);
}

static int asix_recv(struct eth_device *eth)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, recv_buf, AX_RX_URB_SIZE);
	unsigned char *buf_ptr;
	int err;
	int actual_len;
	u32 packet_len;

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
		 * 1st 4 bytes contain the length of the actual data as two
		 * complementary 16-bit words. Extract the length of the data.
		 */
		if (actual_len < sizeof(packet_len)) {
			debug("Rx: incomplete packet length\n");
			return -1;
		}
		memcpy(&packet_len, buf_ptr, sizeof(packet_len));
		le32_to_cpus(&packet_len);
		if (((~packet_len >> 16) & 0x7ff) != (packet_len & 0x7ff)) {
			debug("Rx: malformed packet length: %#x (%#x:%#x)\n",
			      packet_len, (~packet_len >> 16) & 0x7ff,
			      packet_len & 0x7ff);
			return -1;
		}
		packet_len = packet_len & 0x7ff;
		if (packet_len > actual_len - sizeof(packet_len)) {
			debug("Rx: too large packet: %d\n", packet_len);
			return -1;
		}

		if ((dev->pusb_dev->descriptor.idVendor ==
		     ASIX_USB_VENDOR_ID) &&
		    (dev->pusb_dev->descriptor.idProduct ==
		     AX88772B_USB_PRODUCT_ID))
			buf_ptr += 2;	
			
		/* Notify net stack */
		net_process_received_packet(buf_ptr + sizeof(packet_len),
					    packet_len);

		/* Adjust for next iteration. Packets are padded to 16-bits */
		if (packet_len & 1)
			packet_len++;
		actual_len -= sizeof(packet_len) + packet_len;
		buf_ptr += sizeof(packet_len) + packet_len;
	}

	return err;
}

static void asix_halt(struct eth_device *eth)
{
	debug("** %s()\n", __func__);
}

static int asix_write_hwaddr(struct eth_device *eth)
{
	struct ueth_data *dev = (struct ueth_data *)eth->priv;

	return asix_write_hwaddr_common(dev, eth->enetaddr);
}

/*
 * Asix probing functions
 */
void asix_eth_before_probe(void)
{
	curr_eth_dev = 0;
}

struct asix_dongle {
	unsigned short vendor;
	unsigned short product;
	int flags;
};

static const struct asix_dongle asix_dongles[] = {
	{ 0x05ac, 0x1402, FLAG_TYPE_AX88772 },	/* Apple USB Ethernet Adapter */
	{ 0x07d1, 0x3c05, FLAG_TYPE_AX88772 },	/* D-Link DUB-E100 H/W Ver B1 */
	{ 0x2001, 0x1a02, FLAG_TYPE_AX88772 },	/* D-Link DUB-E100 H/W Ver C1 */
	/* Cables-to-Go USB Ethernet Adapter */
	{ 0x0b95, 0x772a, FLAG_TYPE_AX88772 },
	{ 0x0b95, 0x7720, FLAG_TYPE_AX88772 },	/* Trendnet TU2-ET100 V3.0R */
	{ 0x0b95, 0x1720, FLAG_TYPE_AX88172 },	/* SMC */
	{ 0x0db0, 0xa877, FLAG_TYPE_AX88772 },	/* MSI - ASIX 88772a */
	{ 0x13b1, 0x0018, FLAG_TYPE_AX88172 },	/* Linksys 200M v2.1 */
	{ 0x1557, 0x7720, FLAG_TYPE_AX88772 },	/* 0Q0 cable ethernet */
	/* DLink DUB-E100 H/W Ver B1 Alternate */
	{ 0x2001, 0x3c05, FLAG_TYPE_AX88772 },
	/* ASIX 88772B */
	{ 0x0b95, 0x772b, FLAG_TYPE_AX88772B | FLAG_EEPROM_MAC },
	{ 0x0b95, 0x772b, FLAG_TYPE_AX88772B },	
	{ 0x0b95, 0x7e2b, FLAG_TYPE_AX88772B },
	{ 0x0000, 0x0000, FLAG_NONE }	/* END - Do not remove */
};

/* Probe to see if a new device is actually an asix device */
int asix_eth_probe(struct usb_device *dev, unsigned int ifnum,
		      struct ueth_data *ss)
{
	struct usb_interface *iface;
	struct usb_interface_descriptor *iface_desc;
	int ep_in_found = 0, ep_out_found = 0;
	int i;

	/* let's examine the device now */
	iface = &dev->config.if_desc[ifnum];
	iface_desc = &dev->config.if_desc[ifnum].desc;

	for (i = 0; asix_dongles[i].vendor != 0; i++) {
		if (dev->descriptor.idVendor == asix_dongles[i].vendor &&
		    dev->descriptor.idProduct == asix_dongles[i].product)
			/* Found a supported dongle */
			break;
	}

	if (asix_dongles[i].vendor == 0)
		return 0;

	memset(ss, 0, sizeof(struct ueth_data));

	/* At this point, we know we've got a live one */
	debug("\n\nUSB Ethernet device detected: %#04x:%#04x\n",
	      dev->descriptor.idVendor, dev->descriptor.idProduct);

	/* Initialize the ueth_data structure with some useful info */
	ss->ifnum = ifnum;
	ss->pusb_dev = dev;
	ss->subclass = iface_desc->bInterfaceSubClass;
	ss->protocol = iface_desc->bInterfaceProtocol;

	/* alloc driver private */
	ss->dev_priv = calloc(1, sizeof(struct asix_private));
	if (!ss->dev_priv)
		return 0;

	((struct asix_private *)ss->dev_priv)->flags = asix_dongles[i].flags;

	/*
	 * We are expecting a minimum of 3 endpoints - in, out (bulk), and
	 * int. We will ignore any others.
	 */
	for (i = 0; i < iface_desc->bNumEndpoints; i++) {
		/* is it an BULK endpoint? */
		if ((iface->ep_desc[i].bmAttributes &
		     USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK) {
			u8 ep_addr = iface->ep_desc[i].bEndpointAddress;
			if (ep_addr & USB_DIR_IN) {
				if (!ep_in_found) {
					ss->ep_in = ep_addr &
						USB_ENDPOINT_NUMBER_MASK;
					ep_in_found = 1;
				}
			} else {
				if (!ep_out_found) {
					ss->ep_out = ep_addr &
						USB_ENDPOINT_NUMBER_MASK;
					ep_out_found = 1;
				}
			}
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


static int ax88178_reset(struct ueth_data *dev, struct asix_private *priv)
{
	int ret;
	int gpio0 = 0;
	u32 phyid;
	__le16 eeprom;
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, status, 4);
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, eep, 4);
	printf("%s enter\n", __func__);
	*status = 0;
	asix_read_cmd(dev, AX_CMD_READ_GPIOS, 0, 0, 1, status);
	debug("GPIO Status: 0x%04x\n", *status);

	*eep = 0;
	asix_write_cmd(dev, AX_CMD_WRITE_ENABLE, 0, 0, 0, NULL);
	asix_read_cmd(dev, AX_CMD_READ_EEPROM, 0x0017, 0, 2, eep);
	asix_write_cmd(dev, AX_CMD_WRITE_DISABLE, 0, 0, 0, NULL);

	eeprom = *eep;
	debug("EEPROM index 0x17 is 0x%04x\n", eeprom);

	if (eeprom == cpu_to_le16(0xffff)) {
		priv->phymode = PHY_MODE_MARVELL;
		priv->ledmode = 0;
		gpio0 = 1;
	} else {
		priv->phymode = le16_to_cpu(eeprom) & 0x7F;
		priv->ledmode = le16_to_cpu(eeprom) >> 8;
		gpio0 = (le16_to_cpu(eeprom) & 0x80) ? 0 : 1;
	}
	debug("GPIO0: %d, PhyMode: %d\n", gpio0, priv->phymode);

	/* Power up external GigaPHY through AX88178 GPIO pin */
	asix_write_gpio(dev, AX_GPIO_RSE | AX_GPIO_GPO_1 | AX_GPIO_GPO1EN, 40);
	if ((le16_to_cpu(eeprom) >> 8) != 1) {
		asix_write_gpio(dev, 0x003c, 30);
		asix_write_gpio(dev, 0x001c, 300);
		asix_write_gpio(dev, 0x003c, 30);
	} else {
		debug("gpio phymode == 1 path");
		asix_write_gpio(dev, AX_GPIO_GPO1EN, 30);
		asix_write_gpio(dev, AX_GPIO_GPO1EN | AX_GPIO_GPO_1, 30);
	}

	dev->phy_id = asix_get_phy_addr(dev);

	/* Read PHYID register *AFTER* powering up PHY */
	phyid = asix_get_phyid(dev);
	debug("PHYID=0x%08x\n", phyid);

	/* Set AX88178 to enable MII/GMII/RGMII interface for external PHY */
	asix_write_cmd(dev, AX_CMD_SW_PHY_SELECT, 0, 0, 0, NULL);

	asix_sw_reset(dev, 0);
	mdelay(150);

	asix_sw_reset(dev, AX_SWRESET_PRL | AX_SWRESET_IPPD);
	mdelay(150);

	asix_write_rx_ctl(dev, 0);

	if (priv->phymode == PHY_MODE_MARVELL) {
//		marvell_phy_init(dev, priv);
		mdelay(60);
	} else if (priv->phymode == PHY_MODE_RTL8211CL)
		rtl8211cl_phy_init(dev, priv);

	asix_mdio_write(dev, dev->phy_id, MII_BMCR,
			BMCR_RESET | BMCR_ANENABLE);
	asix_mdio_write(dev, dev->phy_id, MII_ADVERTISE,
			ADVERTISE_ALL | ADVERTISE_CSMA | ADVERTISE_PAUSE_CAP);
	asix_mdio_write(dev, dev->phy_id, MII_CTRL1000,
			ADVERTISE_1000FULL);

	mii_nway_restart(dev);

	ret = asix_write_medium_mode(dev, AX88178_MEDIUM_DEFAULT | AX_MEDIUM_ENCK);
	if (ret < 0)
		return ret;
#if 0 /* Rewrite MAC ?? */
	/* Rewrite MAC address */
	memcpy(data->mac_addr, dev->net->dev_addr, ETH_ALEN);
	ret = asix_write_cmd(dev, AX_CMD_WRITE_NODE_ID, 0, 0, ETH_ALEN,
							data->mac_addr);
	if (ret < 0)
		return ret;
#endif

	ret = asix_write_rx_ctl(dev, AX_DEFAULT_RX_CTL);
	if (ret < 0)
		return ret;
	printf("%s exit\n", __func__);
	return 0;
}       

int asix_eth_get_info(struct usb_device *dev, struct ueth_data *ss,
				struct eth_device *eth)
{
	struct asix_private *priv = (struct asix_private *)ss->dev_priv;

	if (!eth) {
		debug("%s: missing parameter.\n", __func__);
		return 0;
	}
	sprintf(eth->name, "%s%d", ASIX_BASE_NAME, curr_eth_dev++);
	eth->init = asix_init;
	eth->send = asix_send;
	eth->recv = asix_recv;
	eth->halt = asix_halt;
	if (!(priv->flags & FLAG_TYPE_AX88172))
		eth->write_hwaddr = asix_write_hwaddr;
	eth->priv = ss;

	if (priv->flags & FLAG_TYPE_AX88178) {
		priv->rx_urb_size = AX88178_RX_URB_SIZE;
	} else {
		priv->rx_urb_size = AX_RX_URB_SIZE;
	}
	
	/* Check AX88178 for another initialize. */
	if (priv->flags & FLAG_TYPE_AX88172) {
		if (ax88178_reset(ss, priv))
			return 0;
	} else {
		if (asix_basic_reset(ss, priv))
			return 0;
	}

	/* Get the MAC address */
	if (asix_read_mac_common(ss, priv, eth->enetaddr))
		return 0;
	debug("MAC %pM\n", eth->enetaddr);

	return 1;
}
#endif

#ifdef CONFIG_DM_ETH
static int asix_eth_start(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct asix_private *priv = dev_get_priv(dev);

	return asix_init_common(&priv->ueth, pdata->enetaddr);
}

void asix_eth_stop(struct udevice *dev)
{
	debug("** %s()\n", __func__);
}

int asix_eth_send(struct udevice *dev, void *packet, int length)
{
	struct asix_private *priv = dev_get_priv(dev);

	return asix_send_common(&priv->ueth, packet, length);
}

int asix_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct asix_private *priv = dev_get_priv(dev);
	struct ueth_data *ueth = &priv->ueth;
	uint8_t *ptr;
	int ret, len;
	u32 packet_len;

	len = usb_ether_get_rx_bytes(ueth, &ptr);
	debug("%s: first try, len=%d\n", __func__, len);
	if (!len) {
		if (!(flags & ETH_RECV_CHECK_DEVICE))
			return -EAGAIN;
		ret = usb_ether_receive(ueth, AX_RX_URB_SIZE);
		if (ret == -EAGAIN)
			return ret;

		len = usb_ether_get_rx_bytes(ueth, &ptr);
		debug("%s: second try, len=%d\n", __func__, len);
	}

	/*
	 * 1st 4 bytes contain the length of the actual data as two
	 * complementary 16-bit words. Extract the length of the data.
	 */
	if (len < sizeof(packet_len)) {
		debug("Rx: incomplete packet length\n");
		goto err;
	}
	memcpy(&packet_len, ptr, sizeof(packet_len));
	le32_to_cpus(&packet_len);
	if (((~packet_len >> 16) & 0x7ff) != (packet_len & 0x7ff)) {
		debug("Rx: malformed packet length: %#x (%#x:%#x)\n",
		      packet_len, (~packet_len >> 16) & 0x7ff,
		      packet_len & 0x7ff);
		goto err;
	}
	packet_len = packet_len & 0x7ff;
	if (packet_len > len - sizeof(packet_len)) {
		debug("Rx: too large packet: %d\n", packet_len);
		goto err;
	}

	*packetp = ptr + sizeof(packet_len);
	return packet_len;

err:
	usb_ether_advance_rxbuf(ueth, -1);
	return -EINVAL;
}

static int asix_free_pkt(struct udevice *dev, uchar *packet, int packet_len)
{
	struct asix_private *priv = dev_get_priv(dev);

	if (packet_len & 1)
		packet_len++;
	usb_ether_advance_rxbuf(&priv->ueth, sizeof(u32) + packet_len);

	return 0;
}

int asix_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct asix_private *priv = dev_get_priv(dev);

	if (priv->flags & FLAG_TYPE_AX88172)
		return -ENOSYS;

	return asix_write_hwaddr_common(&priv->ueth, pdata->enetaddr);
}

static int asix_eth_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct asix_private *priv = dev_get_priv(dev);
	struct ueth_data *ss = &priv->ueth;
	int ret;

	priv->flags = dev->driver_data;
	
	if (priv->flags & FLAG_TYPE_AX88178) {
		priv->rx_urb_size = AX88178_RX_URB_SIZE;
	} else {
		priv->rx_urb_size = AX_RX_URB_SIZE;
	}	
	
	ret = usb_ether_register(dev, ss, AX_RX_URB_SIZE);
	if (ret)
		return ret;

	if (priv->flags & FLAG_TYPE_AX88178) {
		ret = ax88178_reset(ss, priv);
		if (ret)
			goto err;
	} else {
		ret = asix_basic_reset(ss, priv);
		if (ret)                    
			goto err;
	}
	
	/* Get the MAC address */
	ret = asix_read_mac_common(ss, priv, pdata->enetaddr);
	if (ret)
		goto err;
	debug("MAC %pM\n", pdata->enetaddr);

	return 0;

err:
	return usb_ether_deregister(ss);
}

static const struct eth_ops asix_eth_ops = {
	.start	= asix_eth_start,
	.send	= asix_eth_send,
	.recv	= asix_eth_recv,
	.free_pkt = asix_free_pkt,
	.stop	= asix_eth_stop,
	.write_hwaddr = asix_write_hwaddr,
};

U_BOOT_DRIVER(asix_eth) = {
	.name	= "asix_eth",
	.id	= UCLASS_ETH,
	.probe = asix_eth_probe,
	.ops	= &asix_eth_ops,
	.priv_auto_alloc_size = sizeof(struct asix_private),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};

static const struct usb_device_id asix_eth_id_table[] = {
	/* Apple USB Ethernet Adapter */
	{ USB_DEVICE(0x05ac, 0x1402), .driver_info = FLAG_TYPE_AX88772 },
	/* D-Link DUB-E100 H/W Ver B1 */
	{ USB_DEVICE(0x07d1, 0x3c05), .driver_info = FLAG_TYPE_AX88772 },
	/* D-Link DUB-E100 H/W Ver C1 */
	{ USB_DEVICE(0x2001, 0x1a02), .driver_info = FLAG_TYPE_AX88772 },
	/* Cables-to-Go USB Ethernet Adapter */
	{ USB_DEVICE(0x0b95, 0x772a), .driver_info = FLAG_TYPE_AX88772 },
	/* Trendnet TU2-ET100 V3.0R */
	{ USB_DEVICE(0x0b95, 0x7720), .driver_info = FLAG_TYPE_AX88772 },
	/* SMC */
	{ USB_DEVICE(0x0b95, 0x1720), .driver_info = FLAG_TYPE_AX88172 },
	/* MSI - ASIX 88772a */
	{ USB_DEVICE(0x0db0, 0xa877), .driver_info = FLAG_TYPE_AX88772 },
	/* Linksys 200M v2.1 */
	{ USB_DEVICE(0x13b1, 0x0018), .driver_info = FLAG_TYPE_AX88172 },
	/* 0Q0 cable ethernet */
	{ USB_DEVICE(0x1557, 0x7720), .driver_info = FLAG_TYPE_AX88772 },
	/* DLink DUB-E100 H/W Ver B1 Alternate */
	{ USB_DEVICE(0x2001, 0x3c05), .driver_info = FLAG_TYPE_AX88772 },
	{ USB_DEVICE(0x0a46, 0x1269), .driver_info = FLAG_TYPE_AX88772 },		
	/* ASIX 88772B */
	{ USB_DEVICE(0x0b95, 0x772b),
		.driver_info = FLAG_TYPE_AX88772B | FLAG_EEPROM_MAC },
	{ USB_DEVICE(0x0b95, 0x7e2b), .driver_info = FLAG_TYPE_AX88772B },
	{ USB_DEVICE(0x0b95, 0x1780), .driver_info = FLAG_TYPE_AX88178 },		
	{ }		/* Terminating entry */
};

U_BOOT_USB_DEVICE(asix_eth, asix_eth_id_table);
#endif
