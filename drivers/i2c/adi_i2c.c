// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Converted to driver model by Nathan Barrett-Morrison
 *
 * Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 * Contact: Greg Malysa <greg.malysa@timesys.com>
 */

#include <clk.h>
#include <dm.h>
#include <i2c.h>
#include <mapmem.h>
#include <linux/io.h>

#define CLKLOW(x) ((x) & 0xFF)     // Periods Clock Is Held Low
#define CLKHI(y) (((y) & 0xFF) << 0x8) // Periods Clock Is High

#define PRESCALE        0x007F     // SCLKs Per Internal Time Reference (10MHz)
#define TWI_ENA         0x0080     // TWI Enable
#define SCCB            0x0200     // SCCB Compatibility Enable

#define SEN             0x0001     // Slave Enable
#define SADD_LEN        0x0002     // Slave Address Length
#define STDVAL          0x0004     // Slave Transmit Data Valid
#define TSC_NAK         0x0008     // NAK Generated At Conclusion Of Transfer
#define GEN             0x0010     // General Call Adrress Matching Enabled

#define SDIR            0x0001     // Slave Transfer Direction
#define GCALL           0x0002     // General Call Indicator

#define MEN             0x0001     // Master Mode Enable
#define MADD_LEN        0x0002     // Master Address Length
#define MDIR            0x0004     // Master Transmit Direction (RX/TX*)
#define FAST            0x0008     // Use Fast Mode Timing Specs
#define STOP            0x0010     // Issue Stop Condition
#define RSTART          0x0020     // Repeat Start or Stop* At End Of Transfer
#define DCNT            0x3FC0     // Data Bytes To Transfer
#define SDAOVR          0x4000     // Serial Data Override
#define SCLOVR          0x8000     // Serial Clock Override

#define MPROG           0x0001     // Master Transfer In Progress
#define LOSTARB         0x0002     // Lost Arbitration Indicator (Xfer Aborted)
#define ANAK            0x0004     // Address Not Acknowledged
#define DNAK            0x0008     // Data Not Acknowledged
#define BUFRDERR        0x0010     // Buffer Read Error
#define BUFWRERR        0x0020     // Buffer Write Error
#define SDASEN          0x0040     // Serial Data Sense
#define SCLSEN          0x0080     // Serial Clock Sense
#define BUSBUSY         0x0100     // Bus Busy Indicator

#define SINIT           0x0001     // Slave Transfer Initiated
#define SCOMP           0x0002     // Slave Transfer Complete
#define SERR            0x0004     // Slave Transfer Error
#define SOVF            0x0008     // Slave Overflow
#define MCOMP           0x0010     // Master Transfer Complete
#define MERR            0x0020     // Master Transfer Error
#define XMTSERV         0x0040     // Transmit FIFO Service
#define RCVSERV         0x0080     // Receive FIFO Service

#define XMTFLUSH        0x0001     // Transmit Buffer Flush
#define RCVFLUSH        0x0002     // Receive Buffer Flush
#define XMTINTLEN       0x0004     // Transmit Buffer Interrupt Length
#define RCVINTLEN       0x0008     // Receive Buffer Interrupt Length

#define XMTSTAT         0x0003     // Transmit FIFO Status
#define XMT_EMPTY       0x0000     // Transmit FIFO Empty
#define XMT_HALF        0x0001     // Transmit FIFO Has 1 Byte To Write
#define XMT_FULL        0x0003     // Transmit FIFO Full (2 Bytes To Write)

#define RCVSTAT         0x000C     // Receive FIFO Status
#define RCV_EMPTY       0x0000     // Receive FIFO Empty
#define RCV_HALF        0x0004     // Receive FIFO Has 1 Byte To Read
#define RCV_FULL        0x000C     // Receive FIFO Full (2 Bytes To Read)

/* Every register is 32bit aligned, but only 16bits in size */
#define ureg(name) u16 name; u16 __pad_##name

struct twi_regs {
	ureg(clkdiv);
	ureg(control);
	ureg(slave_ctl);
	ureg(slave_stat);
	ureg(slave_addr);
	ureg(master_ctl);
	ureg(master_stat);
	ureg(master_addr);
	ureg(int_stat);
	ureg(int_mask);
	ureg(fifo_ctl);
	ureg(fifo_stat);
	u8 __pad[0x50];

	ureg(xmt_data8);
	ureg(xmt_data16);
	ureg(rcv_data8);
	ureg(rcv_data16);
};

#undef ureg

/*
 * The way speed is changed into duty often results in integer truncation
 * with 50% duty, so we'll force rounding up to the next duty by adding 1
 * to the max. In practice this will get us a speed of something like
 * 385 KHz. The other limit is easy to handle as it is only 8 bits.
 */
#define I2C_SPEED_MAX             400000
#define I2C_SPEED_TO_DUTY(speed)  (5000000 / (speed))
#define I2C_DUTY_MAX              (I2C_SPEED_TO_DUTY(I2C_SPEED_MAX) + 1)
#define I2C_DUTY_MIN              0xff	/* 8 bit limited */

#define I2C_M_COMBO		0x4
#define I2C_M_STOP		0x2
#define I2C_M_READ		0x1

/*
 * All transfers are described by this data structure
 */
struct adi_i2c_msg {
	u8 flags;
	u32 len;		/* msg length */
	u8 *buf;		/* pointer to msg data */
	u32 olen;		/* addr length */
	u8 *obuf;		/* addr buffer */
};

struct adi_i2c_dev {
	struct twi_regs  __iomem *base;
	u32 i2c_clk;
	uint speed;
};

/* Allow msec timeout per ~byte transfer */
#define I2C_TIMEOUT 10

/**
 * wait_for_completion - manage the actual i2c transfer
 *	@msg: the i2c msg
 */
static int wait_for_completion(struct twi_regs *twi, struct adi_i2c_msg *msg)
{
	u16 int_stat;
	ulong timebase = get_timer(0);

	do {
		int_stat = ioread16(&twi->int_stat);

		if (int_stat & XMTSERV) {
			iowrite16(XMTSERV, &twi->int_stat);
			if (msg->olen) {
				iowrite16(*(msg->obuf++), &twi->xmt_data8);
				--msg->olen;
			} else if (!(msg->flags & I2C_M_COMBO) && msg->len) {
				iowrite16(*(msg->buf++), &twi->xmt_data8);
				--msg->len;
			} else {
				if (msg->flags & I2C_M_COMBO)
					setbits_16(&twi->master_ctl, RSTART | MDIR);
				else
					setbits_16(&twi->master_ctl, STOP);
			}
		}
		if (int_stat & RCVSERV) {
			iowrite16(RCVSERV, &twi->int_stat);
			if (msg->len) {
				*(msg->buf++) = ioread16(&twi->rcv_data8);
				--msg->len;
			} else if (msg->flags & I2C_M_STOP) {
				setbits_16(&twi->master_ctl, STOP);
			}
		}
		if (int_stat & MERR) {
			pr_err("%s: master transmit terror: %d\n", __func__,
			       ioread16(&twi->master_stat));
			iowrite16(MERR, &twi->int_stat);
			return -EIO;
		}
		if (int_stat & MCOMP) {
			iowrite16(MCOMP, &twi->int_stat);
			if (msg->flags & I2C_M_COMBO && msg->len) {
				u16 mlen = min(msg->len, 0xffu) << 6;
				clrsetbits_16(&twi->master_ctl, RSTART, mlen | MEN | MDIR);
			} else {
				break;
			}
		}

		/* If we were able to do something, reset timeout */
		if (int_stat)
			timebase = get_timer(0);

	} while (get_timer(timebase) < I2C_TIMEOUT);

	return 0;
}

static int i2c_transfer(struct twi_regs *twi, u8 chip, u8 *offset,
			int olen, u8 *buffer, int len, u8 flags)
{
	int ret;
	u16 ctl;

	struct adi_i2c_msg msg = {
		.flags = flags | (len >= 0xff ? I2C_M_STOP : 0),
		.buf   = buffer,
		.len   = len,
		.obuf  = offset,
		.olen  = olen,
	};

	/* wait for things to settle */
	while (ioread16(&twi->master_stat) & BUSBUSY)
		if (!IS_ENABLED(CONFIG_SPL_BUILD) && ctrlc())
			return -EINTR;

	/* Set Transmit device address */
	iowrite16(chip, &twi->master_addr);

	/* Clear the FIFO before starting things */
	iowrite16(XMTFLUSH | RCVFLUSH, &twi->fifo_ctl);
	iowrite16(0, &twi->fifo_ctl);

	/* Prime the pump */
	if (msg.olen) {
		len = (msg.flags & I2C_M_COMBO) ? msg.olen : msg.olen + len;
		iowrite16(*(msg.obuf++), &twi->xmt_data8);
		--msg.olen;
	} else if (!(msg.flags & I2C_M_READ) && msg.len) {
		iowrite16(*(msg.buf++), &twi->xmt_data8);
		--msg.len;
	}

	/* clear int stat */
	iowrite16(-1, &twi->master_stat);
	iowrite16(-1, &twi->int_stat);
	iowrite16(0, &twi->int_mask);

	/* Master enable */
	ctl = ioread16(&twi->master_ctl);
	ctl = (ctl & FAST) | (min(len, 0xff) << 6) | MEN |
		((msg.flags & I2C_M_READ) ? MDIR : 0);
	iowrite16(ctl, &twi->master_ctl);

	/* Process the rest */
	ret = wait_for_completion(twi, &msg);

	clrbits_16(&twi->master_ctl, MEN);
	clrbits_16(&twi->control, TWI_ENA);
	setbits_16(&twi->control, TWI_ENA);
	return ret;
}

static int adi_i2c_read(struct twi_regs *twi, u8 chip,
			u8 *offset, int olen, u8 *buffer, int len)
{
	return i2c_transfer(twi, chip, offset, olen, buffer,
			len, olen ? I2C_M_COMBO : I2C_M_READ);
}

static int adi_i2c_write(struct twi_regs *twi, u8 chip,
			 u8 *offset, int olen, u8 *buffer, int len)
{
	return i2c_transfer(twi, chip, offset, olen, buffer, len, 0);
}

static int adi_i2c_set_bus_speed(struct udevice *bus, uint speed)
{
	struct adi_i2c_dev *dev = dev_get_priv(bus);
	struct twi_regs *twi = dev->base;
	u16 clkdiv = I2C_SPEED_TO_DUTY(speed);

	/* Set TWI interface clock */
	if (clkdiv < I2C_DUTY_MAX || clkdiv > I2C_DUTY_MIN)
		return -1;
	clkdiv = (clkdiv << 8) | (clkdiv & 0xff);
	iowrite16(clkdiv, &twi->clkdiv);

	/* Don't turn it on */
	iowrite16(speed > 100000 ? FAST : 0, &twi->master_ctl);

	return 0;
}

static int adi_i2c_of_to_plat(struct udevice *bus)
{
	struct adi_i2c_dev *dev = dev_get_priv(bus);
	struct clk clock;
	u32 ret;

	dev->base = map_sysmem(dev_read_addr(bus), sizeof(struct twi_regs));

	if (!dev->base)
		return -ENOMEM;

	dev->speed = dev_read_u32_default(bus, "clock-frequency",
					  I2C_SPEED_FAST_RATE);

	ret = clk_get_by_name(bus, "i2c", &clock);
	if (ret < 0)
		printf("%s: Can't get I2C clk: %d\n", __func__, ret);
	else
		dev->i2c_clk = clk_get_rate(&clock);

	return 0;
}

static int adi_i2c_probe_chip(struct udevice *bus, u32 chip_addr,
			      u32 chip_flags)
{
	struct adi_i2c_dev *dev = dev_get_priv(bus);
	u8 byte;

	return adi_i2c_read(dev->base, chip_addr, NULL, 0, &byte, 1);
}

static int adi_i2c_xfer(struct udevice *bus, struct i2c_msg *msg, int nmsgs)
{
	struct adi_i2c_dev *dev = dev_get_priv(bus);
	struct i2c_msg *dmsg, *omsg, dummy;

	memset(&dummy, 0, sizeof(struct i2c_msg));

	/*
	 * We expect either two messages (one with an offset and one with the
	 * actual data) or one message (just data)
	 */
	if (nmsgs > 2 || nmsgs == 0) {
		debug("%s: Only one or two messages are supported.", __func__);
		return -EINVAL;
	}

	omsg = nmsgs == 1 ? &dummy : msg;
	dmsg = nmsgs == 1 ? msg : msg + 1;

	if (dmsg->flags & I2C_M_RD)
		return adi_i2c_read(dev->base, dmsg->addr, omsg->buf, omsg->len,
				  dmsg->buf, dmsg->len);
	else
		return adi_i2c_write(dev->base, dmsg->addr, omsg->buf, omsg->len,
				   dmsg->buf, dmsg->len);
}

int adi_i2c_probe(struct udevice *bus)
{
	struct adi_i2c_dev *dev = dev_get_priv(bus);
	struct twi_regs *twi = dev->base;

	u16 prescale = ((dev->i2c_clk / 1000 / 1000 + 5) / 10) & 0x7F;

	/* Set TWI internal clock as 10MHz */
	iowrite16(prescale, &twi->control);

	/* Set TWI interface clock as specified */
	adi_i2c_set_bus_speed(bus, dev->speed);

	/* Enable it */
	iowrite16(TWI_ENA | prescale, &twi->control);

	return 0;
}

static const struct dm_i2c_ops adi_i2c_ops = {
	.xfer           = adi_i2c_xfer,
	.probe_chip     = adi_i2c_probe_chip,
	.set_bus_speed  = adi_i2c_set_bus_speed,
};

static const struct udevice_id adi_i2c_ids[] = {
	{ .compatible = "adi-i2c", },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(i2c_adi) = {
	.name = "i2c_adi",
	.id = UCLASS_I2C,
	.of_match = adi_i2c_ids,
	.probe = adi_i2c_probe,
	.of_to_plat = adi_i2c_of_to_plat,
	.priv_auto = sizeof(struct adi_i2c_dev),
	.ops = &adi_i2c_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
