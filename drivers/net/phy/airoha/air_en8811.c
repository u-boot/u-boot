// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for the Airoha EN8811H 2.5 Gigabit PHY.
 *
 * Limitations of the EN8811H:
 * - Only full duplex supported
 * - Forced speed (AN off) is not supported by hardware (100Mbps)
 *
 * Source originated from linux air_en8811h.c
 *
 * Copyright (C) 2025 Airoha Technology Corp.
 */

#include <phy.h>
#include <errno.h>
#include <log.h>
#include <env.h>
#include <malloc.h>
#include <fw_loader.h>
#include <asm/unaligned.h>
#include <linux/iopoll.h>
#include <linux/bitops.h>
#include <linux/compat.h>
#include <dm/device_compat.h>
#include <u-boot/crc.h>

#define EN8811H_PHY_ID		0x03a2a411

#define AIR_FW_ADDR_DM		0x00000000
#define AIR_FW_ADDR_DSP		0x00100000

#define EN8811H_MD32_DM_SIZE	0x4000
#define EN8811H_MD32_DSP_SIZE	0x20000

#define EN8811H_FW_CTRL_1		0x0f0018
#define EN8811H_FW_CTRL_1_START		0x0
#define EN8811H_FW_CTRL_1_FINISH	0x1
#define EN8811H_FW_CTRL_2		0x800000
#define EN8811H_FW_CTRL_2_LOADING	BIT(11)

/* MII Registers */
#define AIR_AUX_CTRL_STATUS		0x1d
#define AIR_AUX_CTRL_STATUS_SPEED_MASK	GENMASK(4, 2)
#define AIR_AUX_CTRL_STATUS_SPEED_100	0x4
#define AIR_AUX_CTRL_STATUS_SPEED_1000	0x8
#define AIR_AUX_CTRL_STATUS_SPEED_2500	0xc

#define AIR_EXT_PAGE_ACCESS		0x1f
#define AIR_PHY_PAGE_STANDARD		0x0000
#define AIR_PHY_PAGE_EXTENDED_4		0x0004

/* MII Registers Page 4 */
#define AIR_BPBUS_MODE			0x10
#define AIR_BPBUS_MODE_ADDR_FIXED	0x0000
#define AIR_BPBUS_MODE_ADDR_INCR	BIT(15)
#define AIR_BPBUS_WR_ADDR_HIGH		0x11
#define AIR_BPBUS_WR_ADDR_LOW		0x12
#define AIR_BPBUS_WR_DATA_HIGH		0x13
#define AIR_BPBUS_WR_DATA_LOW		0x14
#define AIR_BPBUS_RD_ADDR_HIGH		0x15
#define AIR_BPBUS_RD_ADDR_LOW		0x16
#define AIR_BPBUS_RD_DATA_HIGH		0x17
#define AIR_BPBUS_RD_DATA_LOW		0x18

/* Registers on MDIO_MMD_VEND1 */
#define EN8811H_PHY_FW_STATUS		0x8009
#define EN8811H_PHY_READY		0x02

/* Registers on MDIO_MMD_VEND2 */
#define AIR_PHY_LED_BCR			0x021
#define AIR_PHY_LED_BCR_MODE_MASK	GENMASK(1, 0)
#define AIR_PHY_LED_BCR_TIME_TEST	BIT(2)
#define AIR_PHY_LED_BCR_CLK_EN		BIT(3)
#define AIR_PHY_LED_BCR_EXT_CTRL	BIT(15)

#define AIR_PHY_LED_DUR_ON		0x022

#define AIR_PHY_LED_DUR_BLINK		0x023

#define AIR_PHY_LED_ON(i)	       (0x024 + ((i) * 2))
#define AIR_PHY_LED_ON_MASK		(GENMASK(6, 0) | BIT(8))
#define AIR_PHY_LED_ON_LINK1000		BIT(0)
#define AIR_PHY_LED_ON_LINK100		BIT(1)
#define AIR_PHY_LED_ON_LINK10		BIT(2)
#define AIR_PHY_LED_ON_LINKDOWN		BIT(3)
#define AIR_PHY_LED_ON_FDX		BIT(4) /* Full duplex */
#define AIR_PHY_LED_ON_HDX		BIT(5) /* Half duplex */
#define AIR_PHY_LED_ON_FORCE_ON		BIT(6)
#define AIR_PHY_LED_ON_LINK2500		BIT(8)
#define AIR_PHY_LED_ON_POLARITY		BIT(14)
#define AIR_PHY_LED_ON_ENABLE		BIT(15)

#define AIR_PHY_LED_BLINK(i)	       (0x025 + ((i) * 2))
#define AIR_PHY_LED_BLINK_1000TX	BIT(0)
#define AIR_PHY_LED_BLINK_1000RX	BIT(1)
#define AIR_PHY_LED_BLINK_100TX		BIT(2)
#define AIR_PHY_LED_BLINK_100RX		BIT(3)
#define AIR_PHY_LED_BLINK_10TX		BIT(4)
#define AIR_PHY_LED_BLINK_10RX		BIT(5)
#define AIR_PHY_LED_BLINK_COLLISION	BIT(6)
#define AIR_PHY_LED_BLINK_RX_CRC_ERR	BIT(7)
#define AIR_PHY_LED_BLINK_RX_IDLE_ERR	BIT(8)
#define AIR_PHY_LED_BLINK_FORCE_BLINK	BIT(9)
#define AIR_PHY_LED_BLINK_2500TX	BIT(10)
#define AIR_PHY_LED_BLINK_2500RX	BIT(11)

#define EN8811H_FW_VERSION		0x3b3c

#define EN8811H_POLARITY		0xca0f8
#define EN8811H_POLARITY_TX_NORMAL	BIT(0)
#define EN8811H_POLARITY_RX_REVERSE	BIT(1)

#define EN8811H_CLK_CGM			0xcf958
#define EN8811H_CLK_CGM_CKO		BIT(26)
#define EN8811H_HWTRAP1			0xcf914
#define EN8811H_HWTRAP1_CKO		BIT(12)

#define clear_bit(bit, bitmap)	__clear_bit(bit, bitmap)

/* Led definitions */
#define EN8811H_LED_COUNT	3

struct led {
	unsigned long rules;
	unsigned long state;
};

enum {
	AIR_PHY_LED_STATE_FORCE_ON,
	AIR_PHY_LED_STATE_FORCE_BLINK,
};

enum {
	AIR_PHY_LED_DUR_BLINK_32MS,
	AIR_PHY_LED_DUR_BLINK_64MS,
	AIR_PHY_LED_DUR_BLINK_128MS,
	AIR_PHY_LED_DUR_BLINK_256MS,
	AIR_PHY_LED_DUR_BLINK_512MS,
	AIR_PHY_LED_DUR_BLINK_1024MS,
};

enum {
	AIR_LED_DISABLE,
	AIR_LED_ENABLE,
};

enum {
	AIR_ACTIVE_LOW,
	AIR_ACTIVE_HIGH,
};

enum {
	AIR_LED_MODE_DISABLE,
	AIR_LED_MODE_USER_DEFINE,
};

/* Trigger specific enum */
enum air_led_trigger_netdev_modes {
	AIR_TRIGGER_NETDEV_LINK = 0,
	AIR_TRIGGER_NETDEV_LINK_10,
	AIR_TRIGGER_NETDEV_LINK_100,
	AIR_TRIGGER_NETDEV_LINK_1000,
	AIR_TRIGGER_NETDEV_LINK_2500,
	AIR_TRIGGER_NETDEV_LINK_5000,
	AIR_TRIGGER_NETDEV_LINK_10000,
	AIR_TRIGGER_NETDEV_HALF_DUPLEX,
	AIR_TRIGGER_NETDEV_FULL_DUPLEX,
	AIR_TRIGGER_NETDEV_TX,
	AIR_TRIGGER_NETDEV_RX,
	AIR_TRIGGER_NETDEV_TX_ERR,
	AIR_TRIGGER_NETDEV_RX_ERR,

	/* Keep last */
	__AIR_TRIGGER_NETDEV_MAX,
};

/* Default LED setup:
 * GPIO5 <-> LED0  On: Link detected, blink Rx/Tx
 * GPIO4 <-> LED1  On: Link detected at 2500 and 1000 Mbps
 * GPIO3 <-> LED2  On: Link detected at 2500 and  100 Mbps
 */
#define AIR_DEFAULT_TRIGGER_LED0 (BIT(AIR_TRIGGER_NETDEV_LINK)      | \
				  BIT(AIR_TRIGGER_NETDEV_RX)        | \
				  BIT(AIR_TRIGGER_NETDEV_TX))
#define AIR_DEFAULT_TRIGGER_LED1 (BIT(AIR_TRIGGER_NETDEV_LINK_2500) | \
				  BIT(AIR_TRIGGER_NETDEV_LINK_1000))
#define AIR_DEFAULT_TRIGGER_LED2 (BIT(AIR_TRIGGER_NETDEV_LINK_2500) | \
				  BIT(AIR_TRIGGER_NETDEV_LINK_100))

#define AIR_PHY_LED_DUR_UNIT	781
#define AIR_PHY_LED_DUR (AIR_PHY_LED_DUR_UNIT << AIR_PHY_LED_DUR_BLINK_64MS)

struct en8811h_priv {
	int firmware_version;
	bool		mcu_needs_restart;
	struct led	led[EN8811H_LED_COUNT];
};

static int air_buckpbus_reg_write(struct phy_device *phydev,
				  u32 pbus_address, u32 pbus_data)
{
	int ret, saved_page;

	saved_page = phy_select_page(phydev, AIR_PHY_PAGE_EXTENDED_4);
	if (saved_page < 0)
		return saved_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_MODE,
			AIR_BPBUS_MODE_ADDR_FIXED);
	if (ret < 0)
		goto restore_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_WR_ADDR_HIGH,
			upper_16_bits(pbus_address));
	if (ret < 0)
		goto restore_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_WR_ADDR_LOW,
			lower_16_bits(pbus_address));
	if (ret < 0)
		goto restore_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_WR_DATA_HIGH,
			upper_16_bits(pbus_data));
	if (ret < 0)
		goto restore_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_WR_DATA_LOW,
			lower_16_bits(pbus_data));
	if (ret < 0)
		goto restore_page;

restore_page:
	if (ret < 0)
		dev_err(phydev->dev, "%s 0x%08x failed: %d\n", __func__,
			pbus_address, ret);

	return phy_restore_page(phydev, saved_page, ret);
}

static int air_buckpbus_reg_read(struct phy_device *phydev,
				 u32 pbus_address, u32 *pbus_data)
{
	int pbus_data_low, pbus_data_high;
	int ret = 0, saved_page;

	saved_page = phy_select_page(phydev, AIR_PHY_PAGE_EXTENDED_4);
	if (saved_page < 0)
		return saved_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_MODE,
			AIR_BPBUS_MODE_ADDR_FIXED);
	if (ret < 0)
		goto restore_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_RD_ADDR_HIGH,
			upper_16_bits(pbus_address));
	if (ret < 0)
		goto restore_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_RD_ADDR_LOW,
			lower_16_bits(pbus_address));
	if (ret < 0)
		goto restore_page;

	pbus_data_high = phy_read(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_RD_DATA_HIGH);
	if (pbus_data_high < 0) {
		ret = pbus_data_high;
		goto restore_page;
	}

	pbus_data_low = phy_read(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_RD_DATA_LOW);
	if (pbus_data_low < 0) {
		ret = pbus_data_low;
		goto restore_page;
	}

	*pbus_data = pbus_data_low | (pbus_data_high << 16);

restore_page:
	if (ret < 0)
		dev_err(phydev->dev, "%s 0x%08x failed: %d\n", __func__,
			pbus_address, ret);

	return phy_restore_page(phydev, saved_page, ret);
}

static int air_buckpbus_reg_modify(struct phy_device *phydev,
				   u32 pbus_address, u32 mask, u32 set)
{
	int pbus_data_low, pbus_data_high;
	u32 pbus_data_old, pbus_data_new;
	int ret = 0, saved_page;

	saved_page = phy_select_page(phydev, AIR_PHY_PAGE_EXTENDED_4);
	if (saved_page < 0)
		return saved_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_MODE,
			AIR_BPBUS_MODE_ADDR_FIXED);
	if (ret < 0)
		goto restore_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_RD_ADDR_HIGH,
			upper_16_bits(pbus_address));
	if (ret < 0)
		goto restore_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_RD_ADDR_LOW,
			lower_16_bits(pbus_address));
	if (ret < 0)
		goto restore_page;

	pbus_data_high = phy_read(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_RD_DATA_HIGH);
	if (pbus_data_high < 0) {
		ret = pbus_data_high;
		goto restore_page;
	}

	pbus_data_low = phy_read(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_RD_DATA_LOW);
	if (pbus_data_low < 0) {
		ret = pbus_data_low;
		goto restore_page;
	}

	pbus_data_old = pbus_data_low | (pbus_data_high << 16);
	pbus_data_new = (pbus_data_old & ~mask) | set;
	if (pbus_data_new == pbus_data_old)
		goto restore_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_WR_ADDR_HIGH,
			upper_16_bits(pbus_address));
	if (ret < 0)
		goto restore_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_WR_ADDR_LOW,
			lower_16_bits(pbus_address));
	if (ret < 0)
		goto restore_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_WR_DATA_HIGH,
			upper_16_bits(pbus_data_new));
	if (ret < 0)
		goto restore_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_WR_DATA_LOW,
			lower_16_bits(pbus_data_new));
	if (ret < 0)
		goto restore_page;

restore_page:
	if (ret < 0)
		dev_err(phydev->dev, "%s 0x%08x failed: %d\n", __func__,
			pbus_address, ret);

	return phy_restore_page(phydev, saved_page, ret);
}

static int air_write_buf(struct phy_device *phydev, unsigned long address,
			 unsigned long array_size, const unsigned char *buffer)
{
	unsigned int offset;
	int ret, saved_page;
	u16 val;

	saved_page = phy_select_page(phydev, AIR_PHY_PAGE_EXTENDED_4);
	if (saved_page < 0)
		return saved_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_MODE,
			AIR_BPBUS_MODE_ADDR_INCR);
	if (ret < 0)
		goto restore_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_WR_ADDR_HIGH,
			upper_16_bits(address));
	if (ret < 0)
		goto restore_page;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_WR_ADDR_LOW,
			lower_16_bits(address));
	if (ret < 0)
		goto restore_page;

	for (offset = 0; offset < array_size; offset += 4) {
		val = get_unaligned_le16(&buffer[offset + 2]);
		ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_WR_DATA_HIGH, val);
		if (ret < 0)
			goto restore_page;

		val = get_unaligned_le16(&buffer[offset]);
		ret = phy_write(phydev, MDIO_DEVAD_NONE, AIR_BPBUS_WR_DATA_LOW, val);
		if (ret < 0)
			goto restore_page;
	}

restore_page:
	if (ret < 0)
		dev_err(phydev->dev, "%s 0x%08lx failed: %d\n", __func__,
			address, ret);

	return phy_restore_page(phydev, saved_page, ret);
}

static int en8811h_wait_mcu_ready(struct phy_device *phydev)
{
	int ret, reg_value;

	/* Because of mdio-lock, may have to wait for multiple loads */
	ret = phy_read_mmd_poll_timeout(phydev, MDIO_MMD_VEND1,
					EN8811H_PHY_FW_STATUS, reg_value,
					reg_value == EN8811H_PHY_READY,
					20000, 7500000, true);
	if (ret) {
		dev_err(phydev->dev, "MCU not ready: 0x%x\n", reg_value);
		return ret;
	}

	return ret;
}

int en8811h_read_fw(void **fw, size_t *fwsize)
{
	void *buffer;
	int ret;

	buffer = malloc(EN8811H_MD32_DM_SIZE + EN8811H_MD32_DSP_SIZE);
	if (!buffer)
		return -ENOMEM;

	ret = request_firmware_into_buf_via_script(buffer,
						   EN8811H_MD32_DM_SIZE + EN8811H_MD32_DSP_SIZE,
						   "en8811h_load_firmware", fwsize);
	if (ret) {
		free(buffer);
		return ret;
	}

	*fw = buffer;

	debug("Found Airoha Firmware.\n");

	return 0;
}

static int en8811h_load_firmware(struct phy_device *phydev)
{
	struct en8811h_priv *priv = phydev->priv;
	size_t fw_size;
	void *buffer;
	int ret;

	ret = en8811h_read_fw(&buffer, &fw_size);
	if (ret < 0) {
		dev_err(phydev->dev, "Failed to get firmware data\n");
		return -EINVAL;
	}

	if (fw_size != EN8811H_MD32_DM_SIZE + EN8811H_MD32_DSP_SIZE) {
		dev_err(phydev->dev,
			"MD32 firmware size mismatch (0x%zx != 0x%x)\n",
			fw_size, EN8811H_MD32_DM_SIZE + EN8811H_MD32_DSP_SIZE);
		ret = -EINVAL;
		goto en8811h_load_firmware_out;
	}

	ret = air_buckpbus_reg_write(phydev, EN8811H_FW_CTRL_1,
				     EN8811H_FW_CTRL_1_START);
	if (ret < 0)
		goto en8811h_load_firmware_out;

	ret = air_buckpbus_reg_modify(phydev, EN8811H_FW_CTRL_2,
				      EN8811H_FW_CTRL_2_LOADING,
				      EN8811H_FW_CTRL_2_LOADING);
	if (ret < 0)
		goto en8811h_load_firmware_out;

	ret = air_write_buf(phydev, AIR_FW_ADDR_DM, EN8811H_MD32_DM_SIZE,
			    (unsigned char *)buffer);
	if (ret < 0)
		goto en8811h_load_firmware_out;

	ret = air_write_buf(phydev, AIR_FW_ADDR_DSP, EN8811H_MD32_DSP_SIZE,
			    (unsigned char *)buffer + EN8811H_MD32_DM_SIZE);
	if (ret < 0)
		goto en8811h_load_firmware_out;

	ret = air_buckpbus_reg_modify(phydev, EN8811H_FW_CTRL_2,
				      EN8811H_FW_CTRL_2_LOADING, 0);
	if (ret < 0)
		goto en8811h_load_firmware_out;

	ret = air_buckpbus_reg_write(phydev, EN8811H_FW_CTRL_1,
				     EN8811H_FW_CTRL_1_FINISH);
	if (ret < 0)
		goto en8811h_load_firmware_out;

	ret = en8811h_wait_mcu_ready(phydev);

	air_buckpbus_reg_read(phydev, EN8811H_FW_VERSION,
			      &priv->firmware_version);
	dev_info(phydev->dev, "MD32 firmware version: %08x\n",
		 priv->firmware_version);

en8811h_load_firmware_out:
	free(buffer);
	if (ret < 0)
		dev_err(phydev->dev, "Firmware loading failed: %d\n", ret);

	return ret;
}

static int en8811h_restart_mcu(struct phy_device *phydev)
{
	int ret;

	ret = phy_write_mmd(phydev, 0x1e, 0x8009, 0x0);
	if (ret < 0)
		return ret;

	ret = air_buckpbus_reg_write(phydev, EN8811H_FW_CTRL_1,
				     EN8811H_FW_CTRL_1_START);
	if (ret < 0)
		return ret;

	return air_buckpbus_reg_write(phydev, EN8811H_FW_CTRL_1,
				      EN8811H_FW_CTRL_1_FINISH);
}

static int air_led_hw_control_set(struct phy_device *phydev, u8 index,
				  unsigned long rules)
{
	struct en8811h_priv *priv = phydev->priv;
	u16 on = 0, blink = 0;
	int ret;

	if (index >= EN8811H_LED_COUNT)
		return -EINVAL;

	priv->led[index].rules = rules;

	if (rules & BIT(AIR_TRIGGER_NETDEV_FULL_DUPLEX))
		on |= AIR_PHY_LED_ON_FDX;

	if (rules & (BIT(AIR_TRIGGER_NETDEV_LINK_10) | BIT(AIR_TRIGGER_NETDEV_LINK)))
		on |= AIR_PHY_LED_ON_LINK10;

	if (rules & (BIT(AIR_TRIGGER_NETDEV_LINK_100) | BIT(AIR_TRIGGER_NETDEV_LINK)))
		on |= AIR_PHY_LED_ON_LINK100;

	if (rules & (BIT(AIR_TRIGGER_NETDEV_LINK_1000) | BIT(AIR_TRIGGER_NETDEV_LINK)))
		on |= AIR_PHY_LED_ON_LINK1000;

	if (rules & (BIT(AIR_TRIGGER_NETDEV_LINK_2500) | BIT(AIR_TRIGGER_NETDEV_LINK)))
		on |= AIR_PHY_LED_ON_LINK2500;

	if (rules & BIT(AIR_TRIGGER_NETDEV_RX)) {
		blink |= AIR_PHY_LED_BLINK_10RX   |
			 AIR_PHY_LED_BLINK_100RX  |
			 AIR_PHY_LED_BLINK_1000RX |
			 AIR_PHY_LED_BLINK_2500RX;
	}

	if (rules & BIT(AIR_TRIGGER_NETDEV_TX)) {
		blink |= AIR_PHY_LED_BLINK_10TX   |
			 AIR_PHY_LED_BLINK_100TX  |
			 AIR_PHY_LED_BLINK_1000TX |
			 AIR_PHY_LED_BLINK_2500TX;
	}

	if (blink || on) {
		/* switch hw-control on, so led-on and led-blink are off */
		clear_bit(AIR_PHY_LED_STATE_FORCE_ON,
			  &priv->led[index].state);
		clear_bit(AIR_PHY_LED_STATE_FORCE_BLINK,
			  &priv->led[index].state);
	} else {
		priv->led[index].rules = 0;
	}

	ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, AIR_PHY_LED_ON(index),
			     AIR_PHY_LED_ON_MASK, on);

	if (ret < 0)
		return ret;

	return phy_write_mmd(phydev, MDIO_MMD_VEND2, AIR_PHY_LED_BLINK(index),
			     blink);
};

static int air_led_init(struct phy_device *phydev, u8 index, u8 state, u8 pol)
{
	int val = 0;
	int err;

	if (index >= EN8811H_LED_COUNT)
		return -EINVAL;

	if (state == AIR_LED_ENABLE)
		val |= AIR_PHY_LED_ON_ENABLE;
	else
		val &= ~AIR_PHY_LED_ON_ENABLE;

	if (pol == AIR_ACTIVE_HIGH)
		val |= AIR_PHY_LED_ON_POLARITY;
	else
		val &= ~AIR_PHY_LED_ON_POLARITY;

	err = phy_write_mmd(phydev, 0x1f, AIR_PHY_LED_ON(index), val);
	if (err < 0)
		return err;

	return 0;
}

static int air_leds_init(struct phy_device *phydev, int num, u16 dur, int mode)
{
	struct en8811h_priv *priv = phydev->priv;
	int ret, i;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, AIR_PHY_LED_DUR_BLINK,
			    dur);
	if (ret < 0)
		return ret;

	ret = phy_write_mmd(phydev, MDIO_MMD_VEND2, AIR_PHY_LED_DUR_ON,
			    dur >> 1);
	if (ret < 0)
		return ret;

	switch (mode) {
	case AIR_LED_MODE_DISABLE:
		ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, AIR_PHY_LED_BCR,
				     AIR_PHY_LED_BCR_EXT_CTRL |
				     AIR_PHY_LED_BCR_MODE_MASK, 0);
		break;
	case AIR_LED_MODE_USER_DEFINE:
		ret = phy_modify_mmd(phydev, MDIO_MMD_VEND2, AIR_PHY_LED_BCR,
				     AIR_PHY_LED_BCR_EXT_CTRL |
				     AIR_PHY_LED_BCR_CLK_EN,
				     AIR_PHY_LED_BCR_EXT_CTRL |
				     AIR_PHY_LED_BCR_CLK_EN);
		if (ret < 0)
			return ret;
		break;
	default:
		dev_err(phydev->dev, "LED mode %d is not supported\n", mode);
		return -EINVAL;
	}

	for (i = 0; i < num; ++i) {
		ret = air_led_init(phydev, i, AIR_LED_ENABLE, AIR_ACTIVE_HIGH);
		if (ret < 0) {
			dev_err(phydev->dev, "LED%d init failed: %d\n", i, ret);
			return ret;
		}
		air_led_hw_control_set(phydev, i, priv->led[i].rules);
	}

	return 0;
}

static int en8811h_config(struct phy_device *phydev)
{
	struct en8811h_priv *priv = phydev->priv;
	ofnode node = phy_get_ofnode(phydev);
	u32 pbus_value = 0;
	int ret = 0;

	/* If restart happened in .probe(), no need to restart now */
	if (priv->mcu_needs_restart) {
		ret = en8811h_restart_mcu(phydev);
		if (ret < 0)
			return ret;
	} else {
		ret = en8811h_load_firmware(phydev);
		if (ret) {
			dev_err(phydev->dev, "Load firmware fail.\n");
			return ret;
		}
		/* Next calls to .config() mcu needs to restart */
		priv->mcu_needs_restart = true;
	}

	ret = phy_write_mmd(phydev, 0x1e, 0x800c, 0x0);
	if (ret < 0)
		return ret;
	ret = phy_write_mmd(phydev, 0x1e, 0x800d, 0x0);
	if (ret < 0)
		return ret;
	ret = phy_write_mmd(phydev, 0x1e, 0x800e, 0x1101);
	if (ret < 0)
		return ret;
	ret = phy_write_mmd(phydev, 0x1e, 0x800f, 0x0002);
	if (ret < 0)
		return ret;

	/* Serdes polarity */
	pbus_value = 0;
	if (ofnode_read_bool(node, "airoha,pnswap-rx"))
		pbus_value |=  EN8811H_POLARITY_RX_REVERSE;
	else
		pbus_value &= ~EN8811H_POLARITY_RX_REVERSE;
	if (ofnode_read_bool(node, "airoha,pnswap-tx"))
		pbus_value &= ~EN8811H_POLARITY_TX_NORMAL;
	else
		pbus_value |=  EN8811H_POLARITY_TX_NORMAL;
	ret = air_buckpbus_reg_modify(phydev, EN8811H_POLARITY,
				      EN8811H_POLARITY_RX_REVERSE |
				      EN8811H_POLARITY_TX_NORMAL, pbus_value);
	if (ret < 0)
		return ret;

	ret = air_leds_init(phydev, EN8811H_LED_COUNT, AIR_PHY_LED_DUR,
			    AIR_LED_MODE_USER_DEFINE);
	if (ret < 0) {
		dev_err(phydev->dev, "Failed to disable leds: %d\n", ret);
		return ret;
	}

	return 0;
}

static int en8811h_parse_status(struct phy_device *phydev)
{
	int ret = 0, reg_value;

	phydev->duplex = DUPLEX_FULL;

	reg_value = phy_read(phydev, MDIO_DEVAD_NONE, AIR_AUX_CTRL_STATUS);
	if (reg_value < 0)
		return reg_value;

	switch (reg_value & AIR_AUX_CTRL_STATUS_SPEED_MASK) {
	case AIR_AUX_CTRL_STATUS_SPEED_2500:
		phydev->speed = SPEED_2500;
		break;
	case AIR_AUX_CTRL_STATUS_SPEED_1000:
		phydev->speed = SPEED_1000;
		break;
	case AIR_AUX_CTRL_STATUS_SPEED_100:
		phydev->speed = SPEED_100;
		break;
	default:
		dev_err(phydev->dev, "Auto-neg error, defaulting to 2500M/FD\n");
		phydev->speed = SPEED_2500;
		break;
	}

	return ret;
}

static int en8811h_startup(struct phy_device *phydev)
{
	int ret = 0;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	return en8811h_parse_status(phydev);
}

static int en8811h_probe(struct phy_device *phydev)
{
	struct en8811h_priv *priv;

	priv = malloc(sizeof(*priv));
	if (!priv)
		return -ENOMEM;
	memset(priv, 0, sizeof(*priv));

	priv->led[0].rules = AIR_DEFAULT_TRIGGER_LED0;
	priv->led[1].rules = AIR_DEFAULT_TRIGGER_LED1;
	priv->led[2].rules = AIR_DEFAULT_TRIGGER_LED2;

	/* mcu has just restarted after firmware load */
	priv->mcu_needs_restart = false;

	phydev->priv = priv;

	return 0;
}

static int en8811h_read_page(struct phy_device *phydev)
{
	return phy_read(phydev, MDIO_DEVAD_NONE, AIR_EXT_PAGE_ACCESS);
}

static int en8811h_write_page(struct phy_device *phydev, int page)
{
	return phy_write(phydev, MDIO_DEVAD_NONE, AIR_EXT_PAGE_ACCESS, page);
}

U_BOOT_PHY_DRIVER(en8811h) = {
	.name = "Airoha EN8811H",
	.uid = EN8811H_PHY_ID,
	.mask = 0x0ffffff0,
	.config = &en8811h_config,
	.probe = &en8811h_probe,
	.read_page = &en8811h_read_page,
	.write_page = &en8811h_write_page,
	.startup = &en8811h_startup,
	.shutdown = &genphy_shutdown,
};
