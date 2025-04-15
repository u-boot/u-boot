// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016-2017 Google, Inc
 *
 * Fairchild FUSB302 Type-C Chip Driver
 */

#include <dm.h>
#include <i2c.h>
#include <asm/gpio.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <dm/device_compat.h>
#include <usb/tcpm.h>
#include "fusb302_reg.h"

#define FUSB302_MAX_MSG_LEN 0x1F

enum toggling_mode {
	TOGGLING_MODE_OFF,
	TOGGLING_MODE_DRP,
	TOGGLING_MODE_SNK,
	TOGGLING_MODE_SRC,
};

enum src_current_status {
	SRC_CURRENT_DEFAULT,
	SRC_CURRENT_MEDIUM,
	SRC_CURRENT_HIGH,
};

static const u8 ra_mda_value[] = {
	[SRC_CURRENT_DEFAULT] = 4,	/* 210mV */
	[SRC_CURRENT_MEDIUM] = 9,	/* 420mV */
	[SRC_CURRENT_HIGH] = 18,	/* 798mV */
};

static const u8 rd_mda_value[] = {
	[SRC_CURRENT_DEFAULT] = 38,	/* 1638mV */
	[SRC_CURRENT_MEDIUM] = 38,	/* 1638mV */
	[SRC_CURRENT_HIGH] = 61,	/* 2604mV */
};

struct fusb302_chip {
	enum toggling_mode toggling_mode;
	enum src_current_status src_current_status;
	bool intr_togdone;
	bool intr_bc_lvl;
	bool intr_comp_chng;

	/* port status */
	bool vconn_on;
	bool vbus_present;
	enum typec_cc_polarity cc_polarity;
	enum typec_cc_status cc1;
	enum typec_cc_status cc2;
};

static int fusb302_i2c_write(struct udevice *dev, u8 address, u8 data)
{
	int ret;

	ret = dm_i2c_write(dev, address, &data, 1);
	if (ret)
		dev_err(dev, "cannot write 0x%02x to 0x%02x, ret=%d\n",
			data, address, ret);

	return ret;
}

static int fusb302_i2c_block_write(struct udevice *dev, u8 address,
				   u8 length, const u8 *data)
{
	int ret;

	if (!length)
		return 0;

	ret = dm_i2c_write(dev, address, data, length);
	if (ret)
		dev_err(dev, "cannot block write 0x%02x, len=%d, ret=%d\n",
			address, length, ret);

	return ret;
}

static int fusb302_i2c_read(struct udevice *dev, u8 address, u8 *data)
{
	int ret, retries;

	for (retries = 0; retries < 3; retries++) {
		ret = dm_i2c_read(dev, address, data, 1);
		if (ret == 0)
			return ret;
		dev_err(dev, "cannot read %02x, ret=%d\n", address, ret);
	}

	return ret;
}

static int fusb302_i2c_block_read(struct udevice *dev, u8 address,
				  u8 length, u8 *data)
{
	int ret;

	if (!length)
		return 0;

	ret = dm_i2c_read(dev, address, data, length);
	if (ret)
		dev_err(dev, "cannot block read 0x%02x, len=%d, ret=%d\n",
			address, length, ret);
	return ret;
}

static int fusb302_i2c_mask_write(struct udevice *dev, u8 address,
				  u8 mask, u8 value)
{
	int ret;
	u8 data;

	ret = fusb302_i2c_read(dev, address, &data);
	if (ret)
		return ret;
	data &= ~mask;
	data |= value;
	ret = fusb302_i2c_write(dev, address, data);
	if (ret)
		return ret;

	return ret;
}

static int fusb302_i2c_set_bits(struct udevice *dev, u8 address, u8 set_bits)
{
	return fusb302_i2c_mask_write(dev, address, 0x00, set_bits);
}

static int fusb302_i2c_clear_bits(struct udevice *dev, u8 address, u8 clear_bits)
{
	return fusb302_i2c_mask_write(dev, address, clear_bits, 0x00);
}

static int fusb302_sw_reset(struct udevice *dev)
{
	int ret = fusb302_i2c_write(dev, FUSB_REG_RESET, FUSB_REG_RESET_SW_RESET);

	if (ret)
		dev_err(dev, "cannot sw reset the fusb302: %d\n", ret);

	return ret;
}

static int fusb302_enable_tx_auto_retries(struct udevice *dev, u8 retry_count)
{
	int ret;

	ret = fusb302_i2c_set_bits(dev, FUSB_REG_CONTROL3, retry_count |
				   FUSB_REG_CONTROL3_AUTO_RETRY);

	return ret;
}

/*
 * mask all interrupt on the chip
 */
static int fusb302_mask_interrupt(struct udevice *dev)
{
	int ret;

	ret = fusb302_i2c_write(dev, FUSB_REG_MASK, 0xFF);
	if (ret)
		return ret;
	ret = fusb302_i2c_write(dev, FUSB_REG_MASKA, 0xFF);
	if (ret)
		return ret;
	ret = fusb302_i2c_write(dev, FUSB_REG_MASKB, 0xFF);
	if (ret)
		return ret;
	ret = fusb302_i2c_set_bits(dev, FUSB_REG_CONTROL0,
				   FUSB_REG_CONTROL0_INT_MASK);
		return ret;
}

/*
 * initialize interrupt on the chip
 * - unmasked interrupt: VBUS_OK
 */
static int fusb302_init_interrupt(struct udevice *dev)
{
	int ret;

	ret = fusb302_i2c_write(dev, FUSB_REG_MASK,
				0xFF & ~FUSB_REG_MASK_VBUSOK);
	if (ret)
		return ret;
	ret = fusb302_i2c_write(dev, FUSB_REG_MASKA, 0xFF);
	if (ret)
		return ret;
	ret = fusb302_i2c_write(dev, FUSB_REG_MASKB, 0xFF);
	if (ret)
		return ret;
	ret = fusb302_i2c_clear_bits(dev, FUSB_REG_CONTROL0,
				     FUSB_REG_CONTROL0_INT_MASK);
	return ret;
}

static int fusb302_set_power_mode(struct udevice *dev, u8 power_mode)
{
	int ret;

	ret = fusb302_i2c_write(dev, FUSB_REG_POWER, power_mode);

	return ret;
}

static int fusb302_init(struct udevice *dev)
{
	struct fusb302_chip *chip = dev_get_priv(dev);
	int ret;
	u8 data;

	ret = fusb302_sw_reset(dev);
	if (ret)
		return ret;
	ret = fusb302_enable_tx_auto_retries(dev, FUSB_REG_CONTROL3_N_RETRIES_3);
	if (ret)
		return ret;
	ret = fusb302_init_interrupt(dev);
	if (ret)
		return ret;
	ret = fusb302_set_power_mode(dev, FUSB_REG_POWER_PWR_ALL);
	if (ret)
		return ret;
	ret = fusb302_i2c_read(dev, FUSB_REG_STATUS0, &data);
	if (ret)
		return ret;
	chip->vbus_present = !!(data & FUSB_REG_STATUS0_VBUSOK);
	ret = fusb302_i2c_read(dev, FUSB_REG_DEVICE_ID, &data);
	if (ret)
		return ret;
	dev_info(dev, "fusb302 device ID: 0x%02x\n", data);

	return ret;
}

static int fusb302_get_vbus(struct udevice *dev)
{
	struct fusb302_chip *chip = dev_get_priv(dev);

	return chip->vbus_present ? 1 : 0;
}

static int fusb302_set_src_current(struct udevice *dev,
				   enum src_current_status status)
{
	struct fusb302_chip *chip = dev_get_priv(dev);
	int ret;

	chip->src_current_status = status;
	switch (status) {
	case SRC_CURRENT_DEFAULT:
		ret = fusb302_i2c_mask_write(dev, FUSB_REG_CONTROL0,
					     FUSB_REG_CONTROL0_HOST_CUR_MASK,
					     FUSB_REG_CONTROL0_HOST_CUR_DEF);
		break;
	case SRC_CURRENT_MEDIUM:
		ret = fusb302_i2c_mask_write(dev, FUSB_REG_CONTROL0,
					     FUSB_REG_CONTROL0_HOST_CUR_MASK,
					     FUSB_REG_CONTROL0_HOST_CUR_MED);
		break;
	case SRC_CURRENT_HIGH:
		ret = fusb302_i2c_mask_write(dev, FUSB_REG_CONTROL0,
					     FUSB_REG_CONTROL0_HOST_CUR_MASK,
					     FUSB_REG_CONTROL0_HOST_CUR_HIGH);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int fusb302_set_toggling(struct udevice *dev,
				enum toggling_mode mode)
{
	struct fusb302_chip *chip = dev_get_priv(dev);
	int ret;

	/* first disable toggling */
	ret = fusb302_i2c_clear_bits(dev, FUSB_REG_CONTROL2,
				     FUSB_REG_CONTROL2_TOGGLE);
	if (ret)
		return ret;
	/* mask interrupts for SRC or SNK */
	ret = fusb302_i2c_set_bits(dev, FUSB_REG_MASK,
				   FUSB_REG_MASK_BC_LVL |
				   FUSB_REG_MASK_COMP_CHNG);
	if (ret)
		return ret;
	chip->intr_bc_lvl = false;
	chip->intr_comp_chng = false;
	/* configure toggling mode: none/snk/src/drp */
	switch (mode) {
	case TOGGLING_MODE_OFF:
		ret = fusb302_i2c_mask_write(dev, FUSB_REG_CONTROL2,
					     FUSB_REG_CONTROL2_MODE_MASK,
					     FUSB_REG_CONTROL2_MODE_NONE);
		break;
	case TOGGLING_MODE_SNK:
		ret = fusb302_i2c_mask_write(dev, FUSB_REG_CONTROL2,
					     FUSB_REG_CONTROL2_MODE_MASK,
					     FUSB_REG_CONTROL2_MODE_UFP);
		break;
	case TOGGLING_MODE_SRC:
		ret = fusb302_i2c_mask_write(dev, FUSB_REG_CONTROL2,
					     FUSB_REG_CONTROL2_MODE_MASK,
					     FUSB_REG_CONTROL2_MODE_DFP);
		break;
	case TOGGLING_MODE_DRP:
		ret = fusb302_i2c_mask_write(dev, FUSB_REG_CONTROL2,
					     FUSB_REG_CONTROL2_MODE_MASK,
					     FUSB_REG_CONTROL2_MODE_DRP);
		break;
	default:
		break;
	}

	if (ret)
		return ret;

	if (mode == TOGGLING_MODE_OFF) {
		/* mask TOGDONE interrupt */
		ret = fusb302_i2c_set_bits(dev, FUSB_REG_MASKA,
					   FUSB_REG_MASKA_TOGDONE);
		if (ret)
			return ret;
		chip->intr_togdone = false;
	} else {
		/* Datasheet says vconn MUST be off when toggling */
		if (chip->vconn_on)
			dev_warn(dev, "Vconn is on during toggle start\n");
		/* unmask TOGDONE interrupt */
		ret = fusb302_i2c_clear_bits(dev, FUSB_REG_MASKA,
					     FUSB_REG_MASKA_TOGDONE);
		if (ret)
			return ret;
		chip->intr_togdone = true;
		/* start toggling */
		ret = fusb302_i2c_set_bits(dev, FUSB_REG_CONTROL2,
					   FUSB_REG_CONTROL2_TOGGLE);
		if (ret)
			return ret;
		/* during toggling, consider cc as Open */
		chip->cc1 = TYPEC_CC_OPEN;
		chip->cc2 = TYPEC_CC_OPEN;
	}
	chip->toggling_mode = mode;

	return ret;
}

static const enum src_current_status cc_src_current[] = {
	[TYPEC_CC_OPEN]		= SRC_CURRENT_DEFAULT,
	[TYPEC_CC_RA]		= SRC_CURRENT_DEFAULT,
	[TYPEC_CC_RD]		= SRC_CURRENT_DEFAULT,
	[TYPEC_CC_RP_DEF]	= SRC_CURRENT_DEFAULT,
	[TYPEC_CC_RP_1_5]	= SRC_CURRENT_MEDIUM,
	[TYPEC_CC_RP_3_0]	= SRC_CURRENT_HIGH,
};

static int fusb302_set_cc(struct udevice *dev, enum typec_cc_status cc)
{
	struct fusb302_chip *chip = dev_get_priv(dev);
	const u8 switches0_mask = FUSB_REG_SWITCHES0_CC1_PU_EN |
				  FUSB_REG_SWITCHES0_CC2_PU_EN |
				  FUSB_REG_SWITCHES0_CC1_PD_EN |
				  FUSB_REG_SWITCHES0_CC2_PD_EN;
	u8 rd_mda, switches0_data = 0x00;
	int ret;

	switch (cc) {
	case TYPEC_CC_OPEN:
		break;
	case TYPEC_CC_RD:
		switches0_data |= FUSB_REG_SWITCHES0_CC1_PD_EN |
				  FUSB_REG_SWITCHES0_CC2_PD_EN;
		break;
	case TYPEC_CC_RP_DEF:
	case TYPEC_CC_RP_1_5:
	case TYPEC_CC_RP_3_0:
		switches0_data |= (chip->cc_polarity == TYPEC_POLARITY_CC1) ?
				  FUSB_REG_SWITCHES0_CC1_PU_EN :
				  FUSB_REG_SWITCHES0_CC2_PU_EN;
		break;
	default:
		dev_err(dev, "unsupported CC value: %s\n",
			typec_cc_status_name[cc]);
		ret = -EINVAL;
		goto done;
	}

	ret = fusb302_set_toggling(dev, TOGGLING_MODE_OFF);
	if (ret) {
		dev_err(dev, "cannot set toggling mode: %d\n", ret);
		goto done;
	}

	ret = fusb302_i2c_mask_write(dev, FUSB_REG_SWITCHES0,
				     switches0_mask, switches0_data);
	if (ret) {
		dev_err(dev, "cannot set pull-up/-down: %d\n", ret);
		goto done;
	}
	/* reset the cc status */
	chip->cc1 = TYPEC_CC_OPEN;
	chip->cc2 = TYPEC_CC_OPEN;

	/* adjust current for SRC */
	ret = fusb302_set_src_current(dev, cc_src_current[cc]);
	if (ret) {
		dev_err(dev, "cannot set src current %s: %d\n",
			typec_cc_status_name[cc], ret);
		goto done;
	}

	/* enable/disable interrupts, BC_LVL for SNK and COMP_CHNG for SRC */
	switch (cc) {
	case TYPEC_CC_RP_DEF:
	case TYPEC_CC_RP_1_5:
	case TYPEC_CC_RP_3_0:
		rd_mda = rd_mda_value[cc_src_current[cc]];
		ret = fusb302_i2c_write(dev, FUSB_REG_MEASURE, rd_mda);
		if (ret) {
			dev_err(dev, "cannot set SRC measure value: %d\n", ret);
			goto done;
		}
		ret = fusb302_i2c_mask_write(dev, FUSB_REG_MASK,
					     FUSB_REG_MASK_BC_LVL |
					     FUSB_REG_MASK_COMP_CHNG,
					     FUSB_REG_MASK_BC_LVL);
		if (ret) {
			dev_err(dev, "cannot set SRC irq: %d\n", ret);
			goto done;
		}
		chip->intr_comp_chng = true;
		break;
	case TYPEC_CC_RD:
		ret = fusb302_i2c_mask_write(dev, FUSB_REG_MASK,
					     FUSB_REG_MASK_BC_LVL |
					     FUSB_REG_MASK_COMP_CHNG,
					     FUSB_REG_MASK_COMP_CHNG);
		if (ret) {
			dev_err(dev, "cannot set SRC irq: %d\n", ret);
			goto done;
		}
		chip->intr_bc_lvl = true;
		break;
	default:
		break;
	}
done:
	return ret;
}

static int fusb302_get_cc(struct udevice *dev, enum typec_cc_status *cc1,
			  enum typec_cc_status *cc2)
{
	struct fusb302_chip *chip = dev_get_priv(dev);

	*cc1 = chip->cc1;
	*cc2 = chip->cc2;
	dev_dbg(dev, "get cc1 = %s, cc2 = %s\n", typec_cc_status_name[*cc1],
		typec_cc_status_name[*cc2]);

	return 0;
}

static int fusb302_set_vconn(struct udevice *dev, bool on)
{
	struct fusb302_chip *chip = dev_get_priv(dev);
	int ret;
	u8 switches0_data = 0x00;
	u8 switches0_mask = FUSB_REG_SWITCHES0_VCONN_CC1 |
			    FUSB_REG_SWITCHES0_VCONN_CC2;

	if (chip->vconn_on == on) {
		ret = 0;
		dev_dbg(dev, "vconn is already %s\n", on ? "on" : "off");
		goto done;
	}
	if (on) {
		switches0_data = (chip->cc_polarity == TYPEC_POLARITY_CC1) ?
				 FUSB_REG_SWITCHES0_VCONN_CC2 :
				 FUSB_REG_SWITCHES0_VCONN_CC1;
	}
	ret = fusb302_i2c_mask_write(dev, FUSB_REG_SWITCHES0,
				     switches0_mask, switches0_data);
	if (ret)
		goto done;
	dev_dbg(dev, "set vconn = %s\n", on ? "on" : "off");
done:
	return ret;
}

static int fusb302_set_vbus(struct udevice *dev, bool on, bool charge)
{
	return 0;
}

static int fusb302_pd_tx_flush(struct udevice *dev)
{
	return fusb302_i2c_set_bits(dev, FUSB_REG_CONTROL0,
				    FUSB_REG_CONTROL0_TX_FLUSH);
}

static int fusb302_pd_rx_flush(struct udevice *dev)
{
	return fusb302_i2c_set_bits(dev, FUSB_REG_CONTROL1,
				    FUSB_REG_CONTROL1_RX_FLUSH);
}

static int fusb302_pd_set_auto_goodcrc(struct udevice *dev, bool on)
{
	if (on)
		return fusb302_i2c_set_bits(dev, FUSB_REG_SWITCHES1,
					    FUSB_REG_SWITCHES1_AUTO_GCRC);
	return fusb302_i2c_clear_bits(dev, FUSB_REG_SWITCHES1,
				      FUSB_REG_SWITCHES1_AUTO_GCRC);
}

static int fusb302_pd_set_interrupts(struct udevice *dev, bool on)
{
	int ret;
	u8 mask_interrupts = FUSB_REG_MASK_COLLISION;
	u8 maska_interrupts = FUSB_REG_MASKA_RETRYFAIL |
			      FUSB_REG_MASKA_HARDSENT |
			      FUSB_REG_MASKA_TX_SUCCESS |
			      FUSB_REG_MASKA_HARDRESET;
	u8 maskb_interrupts = FUSB_REG_MASKB_GCRCSENT;

	ret = on ?
		fusb302_i2c_clear_bits(dev, FUSB_REG_MASK, mask_interrupts) :
		fusb302_i2c_set_bits(dev, FUSB_REG_MASK, mask_interrupts);
	if (ret)
		return ret;
	ret = on ?
		fusb302_i2c_clear_bits(dev, FUSB_REG_MASKA, maska_interrupts) :
		fusb302_i2c_set_bits(dev, FUSB_REG_MASKA, maska_interrupts);
	if (ret)
		return ret;
	ret = on ?
		fusb302_i2c_clear_bits(dev, FUSB_REG_MASKB, maskb_interrupts) :
		fusb302_i2c_set_bits(dev, FUSB_REG_MASKB, maskb_interrupts);
	return ret;
}

static int fusb302_set_pd_rx(struct udevice *dev, bool on)
{
	int ret;

	ret = fusb302_pd_rx_flush(dev);
	if (ret) {
		dev_err(dev, "cannot flush pd rx buffer: %d\n", ret);
		goto done;
	}
	ret = fusb302_pd_tx_flush(dev);
	if (ret) {
		dev_err(dev, "cannot flush pd tx buffer: %d\n", ret);
		goto done;
	}
	ret = fusb302_pd_set_auto_goodcrc(dev, on);
	if (ret) {
		dev_err(dev, "cannot turn %s auto GoodCRC: %d\n",
			on ? "on" : "off", ret);
		goto done;
	}
	ret = fusb302_pd_set_interrupts(dev, on);
	if (ret) {
		dev_err(dev, "cannot turn %s pd interrupts: %d\n",
			on ? "on" : "off", ret);
		goto done;
	}
	dev_dbg(dev, "set pd RX %s\n", on ? "on" : "off");
done:
	return ret;
}

static int fusb302_set_roles(struct udevice *dev, bool attached,
			     enum typec_role pwr, enum typec_data_role data)
{
	int ret;
	u8 switches1_mask = FUSB_REG_SWITCHES1_POWERROLE |
			    FUSB_REG_SWITCHES1_DATAROLE;
	u8 switches1_data = 0x00;

	if (pwr == TYPEC_SOURCE)
		switches1_data |= FUSB_REG_SWITCHES1_POWERROLE;
	if (data == TYPEC_HOST)
		switches1_data |= FUSB_REG_SWITCHES1_DATAROLE;
	ret = fusb302_i2c_mask_write(dev, FUSB_REG_SWITCHES1,
				     switches1_mask, switches1_data);
	if (ret) {
		dev_err(dev, "unable to set pd header %s, %s, ret=%d\n",
			typec_role_name[pwr], typec_data_role_name[data], ret);
		goto done;
	}
	dev_dbg(dev, "pd header : %s, %s\n", typec_role_name[pwr],
		typec_data_role_name[data]);
done:

	return ret;
}

static int fusb302_start_toggling(struct udevice *dev,
				  enum typec_port_type port_type,
				  enum typec_cc_status cc)
{
	enum toggling_mode mode = TOGGLING_MODE_OFF;
	int ret;

	switch (port_type) {
	case TYPEC_PORT_SRC:
		mode = TOGGLING_MODE_SRC;
		break;
	case TYPEC_PORT_SNK:
		mode = TOGGLING_MODE_SNK;
		break;
	case TYPEC_PORT_DRP:
		mode = TOGGLING_MODE_DRP;
		break;
	}

	ret = fusb302_set_src_current(dev, cc_src_current[cc]);
	if (ret) {
		dev_err(dev, "unable to set src current %s, ret=%d",
			typec_cc_status_name[cc], ret);
		goto done;
	}
	ret = fusb302_set_toggling(dev, mode);
	if (ret) {
		dev_err(dev, "unable to start drp toggling: %d\n", ret);
		goto done;
	}
	dev_info(dev, "fusb302 start drp toggling\n");
done:

	return ret;
}

static int fusb302_pd_send_message(struct udevice *dev,
				   const struct pd_message *msg)
{
	int ret;
	/* SOP tokens */
	u8 buf[40] = {FUSB302_TKN_SYNC1, FUSB302_TKN_SYNC1, FUSB302_TKN_SYNC1,
		      FUSB302_TKN_SYNC2};
	u8 pos = 4;
	int len;

	len = pd_header_cnt_le(msg->header) * 4;
	/* plug 2 for header */
	len += 2;
	if (len > FUSB302_MAX_MSG_LEN) {
		dev_err(dev, "PD message too long %d (incl. header)", len);
		return -EINVAL;
	}
	/* packsym tells the FUSB302 chip that the next X bytes are payload */
	buf[pos++] = FUSB302_TKN_PACKSYM | (len & FUSB302_MAX_MSG_LEN);
	memcpy(&buf[pos], &msg->header, sizeof(msg->header));
	pos += sizeof(msg->header);

	len -= 2;
	memcpy(&buf[pos], msg->payload, len);
	pos += len;

	/* CRC */
	buf[pos++] = FUSB302_TKN_JAMCRC;
	/* EOP */
	buf[pos++] = FUSB302_TKN_EOP;
	/* turn tx off after sending message */
	buf[pos++] = FUSB302_TKN_TXOFF;
	/* start transmission */
	buf[pos++] = FUSB302_TKN_TXON;

	ret = fusb302_i2c_block_write(dev, FUSB_REG_FIFOS, pos, buf);
	if (ret)
		return ret;
	dev_dbg(dev, "Send PD message (header=0x%x len=%d)\n", msg->header, len);

	return ret;
}

static int fusb302_pd_send_hardreset(struct udevice *dev)
{
	return fusb302_i2c_set_bits(dev, FUSB_REG_CONTROL3,
				    FUSB_REG_CONTROL3_SEND_HARDRESET);
}

static const char * const transmit_type_name[] = {
	[TCPC_TX_SOP]			= "SOP",
	[TCPC_TX_SOP_PRIME]		= "SOP'",
	[TCPC_TX_SOP_PRIME_PRIME]	= "SOP''",
	[TCPC_TX_SOP_DEBUG_PRIME]	= "DEBUG'",
	[TCPC_TX_SOP_DEBUG_PRIME_PRIME]	= "DEBUG''",
	[TCPC_TX_HARD_RESET]		= "HARD_RESET",
	[TCPC_TX_CABLE_RESET]		= "CABLE_RESET",
	[TCPC_TX_BIST_MODE_2]		= "BIST_MODE_2",
};

static int fusb302_pd_transmit(struct udevice *dev, enum tcpm_transmit_type type,
			       const struct pd_message *msg, unsigned int negotiated_rev)
{
	int ret;

	switch (type) {
	case TCPC_TX_SOP:
		/* nRetryCount 3 in P2.0 spec, whereas 2 in PD3.0 spec */
		ret = fusb302_enable_tx_auto_retries(dev, negotiated_rev > PD_REV20 ?
						     FUSB_REG_CONTROL3_N_RETRIES_2 :
						     FUSB_REG_CONTROL3_N_RETRIES_3);
		if (ret)
			dev_err(dev, "cannot update retry count: %d\n", ret);

		ret = fusb302_pd_send_message(dev, msg);
		if (ret)
			dev_err(dev, "cannot send PD message: %d\n", ret);
		break;
	case TCPC_TX_HARD_RESET:
		ret = fusb302_pd_send_hardreset(dev);
		if (ret)
			dev_err(dev, "cannot send hardreset: %d\n", ret);
		break;
	default:
		dev_err(dev, "type %s not supported", transmit_type_name[type]);
		ret = -EINVAL;
	}

	return ret;
}

static enum typec_cc_status fusb302_bc_lvl_to_cc(u8 bc_lvl)
{
	if (bc_lvl == FUSB_REG_STATUS0_BC_LVL_1230_MAX)
		return TYPEC_CC_RP_3_0;
	if (bc_lvl == FUSB_REG_STATUS0_BC_LVL_600_1230)
		return TYPEC_CC_RP_1_5;
	if (bc_lvl == FUSB_REG_STATUS0_BC_LVL_200_600)
		return TYPEC_CC_RP_DEF;
	return TYPEC_CC_OPEN;
}

static void fusb302_bc_lvl_handler(struct udevice *dev)
{
	struct fusb302_chip *chip = dev_get_priv(dev);
	enum typec_cc_status cc_status;
	u8 status0, bc_lvl;
	int ret;

	if (!chip->intr_bc_lvl) {
		dev_err(dev, "BC_LVL interrupt is turned off, abort\n");
		goto done;
	}
	ret = fusb302_i2c_read(dev, FUSB_REG_STATUS0, &status0);
	if (ret)
		goto done;

	dev_dbg(dev, "BC_LVL handler, status0 = 0x%02x\n", status0);
	if (status0 & FUSB_REG_STATUS0_ACTIVITY)
		dev_info(dev, "CC activities detected, delay handling\n");
	bc_lvl = status0 & FUSB_REG_STATUS0_BC_LVL_MASK;
	cc_status = fusb302_bc_lvl_to_cc(bc_lvl);
	if (chip->cc_polarity == TYPEC_POLARITY_CC1) {
		if (chip->cc1 != cc_status) {
			dev_dbg(dev, "cc1: %s -> %s\n",
				typec_cc_status_name[chip->cc1],
				typec_cc_status_name[cc_status]);
			chip->cc1 = cc_status;
			tcpm_cc_change(dev);
		}
	} else {
		if (chip->cc2 != cc_status) {
			dev_dbg(dev, "cc2: %s -> %s\n",
				typec_cc_status_name[chip->cc2],
				typec_cc_status_name[cc_status]);
			chip->cc2 = cc_status;
			tcpm_cc_change(dev);
		}
	}

done:
	return;
}

static int fusb302_enter_low_power_mode(struct udevice *dev,
					bool attached, bool pd_capable)
{
	unsigned int reg;
	int ret;

	ret = fusb302_mask_interrupt(dev);
	if (ret)
		return ret;
	if (attached && pd_capable)
		reg = FUSB_REG_POWER_PWR_MEDIUM;
	else if (attached)
		reg = FUSB_REG_POWER_PWR_LOW;
	else
		reg = 0;

	return fusb302_set_power_mode(dev, reg);
}

static const char * const cc_polarity_name[] = {
	[TYPEC_POLARITY_CC1]	= "Polarity_CC1",
	[TYPEC_POLARITY_CC2]	= "Polarity_CC2",
};

static int fusb302_set_cc_polarity_and_pull(struct udevice *dev,
					    enum typec_cc_polarity cc_polarity,
					    bool pull_up, bool pull_down)
{
	struct fusb302_chip *chip = dev_get_priv(dev);
	int ret;
	u8 switches0_data = 0x00;
	u8 switches1_mask = FUSB_REG_SWITCHES1_TXCC1_EN |
			    FUSB_REG_SWITCHES1_TXCC2_EN;
	u8 switches1_data = 0x00;

	if (pull_down)
		switches0_data |= FUSB_REG_SWITCHES0_CC1_PD_EN |
				  FUSB_REG_SWITCHES0_CC2_PD_EN;

	if (cc_polarity == TYPEC_POLARITY_CC1) {
		switches0_data |= FUSB_REG_SWITCHES0_MEAS_CC1;
		if (chip->vconn_on)
			switches0_data |= FUSB_REG_SWITCHES0_VCONN_CC2;
		if (pull_up)
			switches0_data |= FUSB_REG_SWITCHES0_CC1_PU_EN;
		switches1_data = FUSB_REG_SWITCHES1_TXCC1_EN;
	} else {
		switches0_data |= FUSB_REG_SWITCHES0_MEAS_CC2;
		if (chip->vconn_on)
			switches0_data |= FUSB_REG_SWITCHES0_VCONN_CC1;
		if (pull_up)
			switches0_data |= FUSB_REG_SWITCHES0_CC2_PU_EN;
		switches1_data = FUSB_REG_SWITCHES1_TXCC2_EN;
	}
	ret = fusb302_i2c_write(dev, FUSB_REG_SWITCHES0, switches0_data);
	if (ret)
		return ret;
	ret = fusb302_i2c_mask_write(dev, FUSB_REG_SWITCHES1,
				     switches1_mask, switches1_data);
	if (ret)
		return ret;
	chip->cc_polarity = cc_polarity;

	return ret;
}

static int fusb302_handle_togdone_snk(struct udevice *dev,
				      u8 togdone_result)
{
	struct fusb302_chip *chip = dev_get_priv(dev);
	int ret;
	u8 status0;
	u8 bc_lvl;
	enum typec_cc_polarity cc_polarity;
	enum typec_cc_status cc_status_active, cc1, cc2;

	/* set polarity and pull_up, pull_down */
	cc_polarity = (togdone_result == FUSB_REG_STATUS1A_TOGSS_SNK1) ?
		      TYPEC_POLARITY_CC1 : TYPEC_POLARITY_CC2;
	ret = fusb302_set_cc_polarity_and_pull(dev, cc_polarity, false, true);
	if (ret) {
		dev_err(dev, "cannot set cc polarity %s, ret = %d\n",
			cc_polarity_name[cc_polarity], ret);
		return ret;
	}
	/* fusb302_set_cc_polarity() has set the correct measure block */
	ret = fusb302_i2c_read(dev, FUSB_REG_STATUS0, &status0);
	if (ret < 0)
		return ret;
	bc_lvl = status0 & FUSB_REG_STATUS0_BC_LVL_MASK;
	cc_status_active = fusb302_bc_lvl_to_cc(bc_lvl);
	/* restart toggling if the cc status on the active line is OPEN */
	if (cc_status_active == TYPEC_CC_OPEN) {
		dev_info(dev, "restart toggling as CC_OPEN detected\n");
		ret = fusb302_set_toggling(dev, chip->toggling_mode);
		return ret;
	}
	/* update tcpm with the new cc value */
	cc1 = (cc_polarity == TYPEC_POLARITY_CC1) ?
	      cc_status_active : TYPEC_CC_OPEN;
	cc2 = (cc_polarity == TYPEC_POLARITY_CC2) ?
	      cc_status_active : TYPEC_CC_OPEN;
	if (chip->cc1 != cc1 || chip->cc2 != cc2) {
		chip->cc1 = cc1;
		chip->cc2 = cc2;
		tcpm_cc_change(dev);
	}
	/* turn off toggling */
	ret = fusb302_set_toggling(dev, TOGGLING_MODE_OFF);
	if (ret) {
		dev_err(dev, "cannot set toggling mode off, ret=%d\n", ret);
		return ret;
	}
	/* unmask bc_lvl interrupt */
	ret = fusb302_i2c_clear_bits(dev, FUSB_REG_MASK, FUSB_REG_MASK_BC_LVL);
	if (ret) {
		dev_err(dev, "cannot unmask bc_lcl irq, ret=%d\n", ret);
		return ret;
	}
	chip->intr_bc_lvl = true;
	dev_dbg(dev, "detected cc1=%s, cc2=%s\n",
		typec_cc_status_name[cc1],
		typec_cc_status_name[cc2]);

	return ret;
}

/* On error returns < 0, otherwise a typec_cc_status value */
static int fusb302_get_src_cc_status(struct udevice *dev,
				     enum typec_cc_polarity cc_polarity,
				     enum typec_cc_status *cc)
{
	struct fusb302_chip *chip = dev_get_priv(dev);
	u8 ra_mda = ra_mda_value[chip->src_current_status];
	u8 rd_mda = rd_mda_value[chip->src_current_status];
	u8 switches0_data, status0;
	int ret;

	/* Step 1: Set switches so that we measure the right CC pin */
	switches0_data = (cc_polarity == TYPEC_POLARITY_CC1) ?
		FUSB_REG_SWITCHES0_CC1_PU_EN | FUSB_REG_SWITCHES0_MEAS_CC1 :
		FUSB_REG_SWITCHES0_CC2_PU_EN | FUSB_REG_SWITCHES0_MEAS_CC2;
	ret = fusb302_i2c_write(dev, FUSB_REG_SWITCHES0, switches0_data);
	if (ret < 0)
		return ret;

	fusb302_i2c_read(dev, FUSB_REG_SWITCHES0, &status0);
	dev_dbg(dev, "get_src_cc_status switches: 0x%0x", status0);

	/* Step 2: Set compararator volt to differentiate between Open and Rd */
	ret = fusb302_i2c_write(dev, FUSB_REG_MEASURE, rd_mda);
	if (ret)
		return ret;

	udelay(100);
	ret = fusb302_i2c_read(dev, FUSB_REG_STATUS0, &status0);
	if (ret)
		return ret;

	dev_dbg(dev, "get_src_cc_status rd_mda status0: 0x%0x", status0);
	if (status0 & FUSB_REG_STATUS0_COMP) {
		*cc = TYPEC_CC_OPEN;
		return 0;
	}

	/* Step 3: Set compararator input to differentiate between Rd and Ra. */
	ret = fusb302_i2c_write(dev, FUSB_REG_MEASURE, ra_mda);
	if (ret)
		return ret;

	udelay(100);
	ret = fusb302_i2c_read(dev, FUSB_REG_STATUS0, &status0);
	if (ret)
		return ret;

	dev_dbg(dev, "get_src_cc_status ra_mda status0: 0x%0x", status0);
	if (status0 & FUSB_REG_STATUS0_COMP)
		*cc = TYPEC_CC_RD;
	else
		*cc = TYPEC_CC_RA;

	return 0;
}

static int fusb302_handle_togdone_src(struct udevice *dev,
				      u8 togdone_result)
{
	/*
	 * - set polarity (measure cc, vconn, tx)
	 * - set pull_up, pull_down
	 * - set cc1, cc2, and update to tcpm state machine
	 * - set I_COMP interrupt on
	 */
	struct fusb302_chip *chip = dev_get_priv(dev);
	u8 rd_mda = rd_mda_value[chip->src_current_status];
	enum toggling_mode toggling_mode = chip->toggling_mode;
	enum typec_cc_polarity cc_polarity;
	enum typec_cc_status cc1, cc2;
	int ret;

	/*
	 * The toggle-engine will stop in a src state if it sees either Ra or
	 * Rd. Determine the status for both CC pins, starting with the one
	 * where toggling stopped, as that is where the switches point now.
	 */
	if (togdone_result == FUSB_REG_STATUS1A_TOGSS_SRC1)
		ret = fusb302_get_src_cc_status(dev, TYPEC_POLARITY_CC1, &cc1);
	else
		ret = fusb302_get_src_cc_status(dev, TYPEC_POLARITY_CC2, &cc2);
	if (ret)
		return ret;
	/* we must turn off toggling before we can measure the other pin */
	ret = fusb302_set_toggling(dev, TOGGLING_MODE_OFF);
	if (ret) {
		dev_err(dev, "cannot set toggling mode off, ret=%d\n", ret);
		return ret;
	}
	/* get the status of the other pin */
	if (togdone_result == FUSB_REG_STATUS1A_TOGSS_SRC1)
		ret = fusb302_get_src_cc_status(dev, TYPEC_POLARITY_CC2, &cc2);
	else
		ret = fusb302_get_src_cc_status(dev, TYPEC_POLARITY_CC1, &cc1);
	if (ret)
		return ret;

	/* determine polarity based on the status of both pins */
	if (cc1 == TYPEC_CC_RD && (cc2 == TYPEC_CC_OPEN || cc2 == TYPEC_CC_RA)) {
		cc_polarity = TYPEC_POLARITY_CC1;
	} else if (cc2 == TYPEC_CC_RD &&
		    (cc1 == TYPEC_CC_OPEN || cc1 == TYPEC_CC_RA)) {
		cc_polarity = TYPEC_POLARITY_CC2;
	} else {
		dev_err(dev, "unexpected CC status cc1=%s, cc2=%s, restarting toggling\n",
			typec_cc_status_name[cc1],
			typec_cc_status_name[cc2]);
		return fusb302_set_toggling(dev, toggling_mode);
	}
	/* set polarity and pull_up, pull_down */
	ret = fusb302_set_cc_polarity_and_pull(dev, cc_polarity, true, false);
	if (ret < 0) {
		dev_err(dev, "cannot set cc polarity %s, ret=%d\n",
			cc_polarity_name[cc_polarity], ret);
		return ret;
	}
	/* update tcpm with the new cc value */
	if (chip->cc1 != cc1 || chip->cc2 != cc2) {
		chip->cc1 = cc1;
		chip->cc2 = cc2;
		tcpm_cc_change(dev);
	}
	/* set MDAC to Rd threshold, and unmask I_COMP for unplug detection */
	ret = fusb302_i2c_write(dev, FUSB_REG_MEASURE, rd_mda);
	if (ret)
		return ret;
	/* unmask comp_chng interrupt */
	ret = fusb302_i2c_clear_bits(dev, FUSB_REG_MASK,
				     FUSB_REG_MASK_COMP_CHNG);
	if (ret) {
		dev_err(dev, "cannot unmask comp_chng irq, ret=%d\n", ret);
		return ret;
	}
	chip->intr_comp_chng = true;
	dev_dbg(dev, "detected cc1=%s, cc2=%s\n",
		typec_cc_status_name[cc1],
		typec_cc_status_name[cc2]);

	return ret;
}

static int fusb302_handle_togdone(struct udevice *dev)
{
	struct fusb302_chip *chip = dev_get_priv(dev);
	u8 togdone_result, status1a;
	int ret;

	ret = fusb302_i2c_read(dev, FUSB_REG_STATUS1A, &status1a);
	if (ret < 0)
		return ret;
	togdone_result = (status1a >> FUSB_REG_STATUS1A_TOGSS_POS) &
			 FUSB_REG_STATUS1A_TOGSS_MASK;
	switch (togdone_result) {
	case FUSB_REG_STATUS1A_TOGSS_SNK1:
	case FUSB_REG_STATUS1A_TOGSS_SNK2:
		return fusb302_handle_togdone_snk(dev, togdone_result);
	case FUSB_REG_STATUS1A_TOGSS_SRC1:
	case FUSB_REG_STATUS1A_TOGSS_SRC2:
		return fusb302_handle_togdone_src(dev, togdone_result);
	case FUSB_REG_STATUS1A_TOGSS_AA:
		/* doesn't support */
		dev_err(dev, "AudioAccessory not supported\n");
		fusb302_set_toggling(dev, chip->toggling_mode);
		break;
	default:
		dev_err(dev, "TOGDONE with an invalid state: %d\n",
			togdone_result);
		fusb302_set_toggling(dev, chip->toggling_mode);
		break;
	}
	return ret;
}

static int fusb302_pd_reset(struct udevice *dev)
{
	return fusb302_i2c_set_bits(dev, FUSB_REG_RESET,
				    FUSB_REG_RESET_PD_RESET);
}

static int fusb302_pd_read_message(struct udevice *dev,
				   struct pd_message *msg)
{
	int len, ret;
	u8 crc[4];
	u8 token;

	/* first SOP token */
	ret = fusb302_i2c_read(dev, FUSB_REG_FIFOS, &token);
	if (ret)
		return ret;
	ret = fusb302_i2c_block_read(dev, FUSB_REG_FIFOS, 2,
				     (u8 *)&msg->header);
	if (ret)
		return ret;
	len = pd_header_cnt_le(msg->header) * 4;
	/* add 4 to length to include the CRC */
	if (len > PD_MAX_PAYLOAD * 4) {
		dev_err(dev, "PD message too long %d\n", len);
		return -EINVAL;
	}
	if (len > 0) {
		ret = fusb302_i2c_block_read(dev, FUSB_REG_FIFOS, len,
					     (u8 *)msg->payload);
		if (ret)
			return ret;
	}
	/* another 4 bytes to read CRC out */
	ret = fusb302_i2c_block_read(dev, FUSB_REG_FIFOS, 4, crc);
	if (ret)
		return ret;
	dev_dbg(dev, "Received PD message (header=0x%x len=%d)\n", msg->header, len);

	/*
	 * Check if we've read off a GoodCRC message. If so then indicate to
	 * TCPM that the previous transmission has completed. Otherwise we pass
	 * the received message over to TCPM for processing.
	 *
	 * We make this check here instead of basing the reporting decision on
	 * the IRQ event type, as it's possible for the chip to report the
	 * TX_SUCCESS and GCRCSENT events out of order on occasion, so we need
	 * to check the message type to ensure correct reporting to TCPM.
	 */
	if (!len && (pd_header_type_le(msg->header) == PD_CTRL_GOOD_CRC))
		tcpm_pd_transmit_complete(dev, TCPC_TX_SUCCESS);
	else
		tcpm_pd_receive(dev, msg);

	return ret;
}

static void fusb302_interrupt_handle(struct udevice *dev)
{
	struct fusb302_chip *chip = dev_get_priv(dev);
	u8 interrupt;
	u8 interrupta;
	u8 interruptb;
	u8 status0;
	bool vbus_present;
	bool comp_result;
	bool intr_togdone;
	bool intr_bc_lvl;
	bool intr_comp_chng;
	struct pd_message pd_msg;
	int ret;

	/* grab a snapshot of intr flags */
	intr_togdone = chip->intr_togdone;
	intr_bc_lvl = chip->intr_bc_lvl;
	intr_comp_chng = chip->intr_comp_chng;

	ret = fusb302_i2c_read(dev, FUSB_REG_INTERRUPT, &interrupt);
	if (ret)
		return;
	ret = fusb302_i2c_read(dev, FUSB_REG_INTERRUPTA, &interrupta);
	if (ret)
		return;
	ret = fusb302_i2c_read(dev, FUSB_REG_INTERRUPTB, &interruptb);
	if (ret)
		return;
	ret = fusb302_i2c_read(dev, FUSB_REG_STATUS0, &status0);
	if (ret)
		return;

	/*
	 * Since we are polling the IRQs, avoid printing messages when there
	 * no interrupts at all to avoid spamming the log.
	 */
	if (interrupt != 0 || interrupta != 0 || interruptb != 0)
		dev_dbg(dev, "IRQ: 0x%02x, a: 0x%02x, b: 0x%02x, status0: 0x%02x\n",
			interrupt, interrupta, interruptb, status0);

	if (interrupt & FUSB_REG_INTERRUPT_VBUSOK) {
		vbus_present = !!(status0 & FUSB_REG_STATUS0_VBUSOK);
		dev_dbg(dev, "IRQ: VBUS_OK, vbus=%s\n",
			vbus_present ? "On" : "Off");
		if (vbus_present != chip->vbus_present) {
			chip->vbus_present = vbus_present;
			tcpm_vbus_change(dev);
		}
	}

	if ((interrupta & FUSB_REG_INTERRUPTA_TOGDONE) && intr_togdone) {
		dev_dbg(dev, "IRQ: TOGDONE\n");
		ret = fusb302_handle_togdone(dev);
		if (ret) {
			dev_err(dev, "handle togdone error: %d\n", ret);
			return;
		}
	}

	if ((interrupt & FUSB_REG_INTERRUPT_BC_LVL) && intr_bc_lvl) {
		dev_dbg(dev, "IRQ: BC_LVL, handler pending\n");
		fusb302_bc_lvl_handler(dev);
	}

	if ((interrupt & FUSB_REG_INTERRUPT_COMP_CHNG) && intr_comp_chng) {
		comp_result = !!(status0 & FUSB_REG_STATUS0_COMP);
		dev_dbg(dev, "IRQ: COMP_CHNG, comp=%s\n",
			comp_result ? "true" : "false");
		if (comp_result) {
			/* cc level > Rd_threshold, detach */
			chip->cc1 = TYPEC_CC_OPEN;
			chip->cc2 = TYPEC_CC_OPEN;
			tcpm_cc_change(dev);
		}
	}

	if (interrupt & FUSB_REG_INTERRUPT_COLLISION) {
		dev_dbg(dev, "IRQ: PD collision\n");
		tcpm_pd_transmit_complete(dev, TCPC_TX_FAILED);
	}

	if (interrupta & FUSB_REG_INTERRUPTA_RETRYFAIL) {
		dev_dbg(dev, "IRQ: PD retry failed\n");
		tcpm_pd_transmit_complete(dev, TCPC_TX_FAILED);
	}

	if (interrupta & FUSB_REG_INTERRUPTA_HARDSENT) {
		dev_dbg(dev, "IRQ: PD hardreset sent\n");
		ret = fusb302_pd_reset(dev);
		if (ret) {
			dev_err(dev, "cannot PD reset, ret=%d\n", ret);
			return;
		}
		tcpm_pd_transmit_complete(dev, TCPC_TX_SUCCESS);
	}

	if (interrupta & FUSB_REG_INTERRUPTA_TX_SUCCESS) {
		dev_dbg(dev, "IRQ: PD tx success\n");
		ret = fusb302_pd_read_message(dev, &pd_msg);
		if (ret) {
			dev_err(dev, "cannot read in PD message, ret=%d\n", ret);
			return;
		}
	}

	if (interrupta & FUSB_REG_INTERRUPTA_HARDRESET) {
		dev_dbg(dev, "IRQ: PD received hardreset\n");
		ret = fusb302_pd_reset(dev);
		if (ret) {
			dev_err(dev, "cannot PD reset, ret=%d\n", ret);
			return;
		}
		tcpm_pd_hard_reset(dev);
	}

	if (interruptb & FUSB_REG_INTERRUPTB_GCRCSENT) {
		dev_dbg(dev, "IRQ: PD sent good CRC\n");
		ret = fusb302_pd_read_message(dev, &pd_msg);
		if (ret) {
			dev_err(dev, "cannot read in PD message, ret=%d\n", ret);
			return;
		}
	}
}

static void fusb302_poll_event(struct udevice *dev)
{
	fusb302_interrupt_handle(dev);
}

static int fusb302_get_connector_node(struct udevice *dev, ofnode *connector_node)
{
	*connector_node = dev_read_subnode(dev, "connector");
	if (!ofnode_valid(*connector_node)) {
		dev_err(dev, "'connector' node is not found\n");
		return -ENODEV;
	}

	return 0;
}

static struct dm_tcpm_ops fusb302_ops = {
	.get_connector_node = fusb302_get_connector_node,
	.init = fusb302_init,
	.get_vbus = fusb302_get_vbus,
	.set_cc = fusb302_set_cc,
	.get_cc = fusb302_get_cc,
	.set_vconn = fusb302_set_vconn,
	.set_vbus = fusb302_set_vbus,
	.set_pd_rx = fusb302_set_pd_rx,
	.set_roles = fusb302_set_roles,
	.start_toggling = fusb302_start_toggling,
	.pd_transmit = fusb302_pd_transmit,
	.poll_event = fusb302_poll_event,
	.enter_low_power_mode = fusb302_enter_low_power_mode,
};

static const struct udevice_id fusb302_ids[] = {
	{ .compatible = "fcs,fusb302" },
	{ }
};

U_BOOT_DRIVER(fusb302) = {
	.name = "fusb302",
	.id = UCLASS_TCPM,
	.of_match = fusb302_ids,
	.ops = &fusb302_ops,
	.priv_auto = sizeof(struct fusb302_chip),
};
