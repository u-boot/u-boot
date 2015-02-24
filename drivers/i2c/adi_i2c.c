/*
 * i2c.c - driver for ADI TWI/I2C
 *
 * Copyright (c) 2006-2014 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <i2c.h>

#include <asm/clock.h>
#include <asm/twi.h>
#include <asm/io.h>

static struct twi_regs *i2c_get_base(struct i2c_adapter *adap);

/* Every register is 32bit aligned, but only 16bits in size */
#define ureg(name) u16 name; u16 __pad_##name;
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
	char __pad[0x50];
	ureg(xmt_data8);
	ureg(xmt_data16);
	ureg(rcv_data8);
	ureg(rcv_data16);
};
#undef ureg

#ifdef TWI_CLKDIV
#define TWI0_CLKDIV TWI_CLKDIV
# ifdef CONFIG_SYS_MAX_I2C_BUS
# undef CONFIG_SYS_MAX_I2C_BUS
# endif
#define CONFIG_SYS_MAX_I2C_BUS 1
#endif

/*
 * The way speed is changed into duty often results in integer truncation
 * with 50% duty, so we'll force rounding up to the next duty by adding 1
 * to the max.  In practice this will get us a speed of something like
 * 385 KHz.  The other limit is easy to handle as it is only 8 bits.
 */
#define I2C_SPEED_MAX             400000
#define I2C_SPEED_TO_DUTY(speed)  (5000000 / (speed))
#define I2C_DUTY_MAX              (I2C_SPEED_TO_DUTY(I2C_SPEED_MAX) + 1)
#define I2C_DUTY_MIN              0xff	/* 8 bit limited */
#define SYS_I2C_DUTY              I2C_SPEED_TO_DUTY(CONFIG_SYS_I2C_SPEED)
/* Note: duty is inverse of speed, so the comparisons below are correct */
#if SYS_I2C_DUTY < I2C_DUTY_MAX || SYS_I2C_DUTY > I2C_DUTY_MIN
# error "The I2C hardware can only operate 20KHz - 400KHz"
#endif

/* All transfers are described by this data structure */
struct adi_i2c_msg {
	u8 flags;
#define I2C_M_COMBO		0x4
#define I2C_M_STOP		0x2
#define I2C_M_READ		0x1
	int len;		/* msg length */
	u8 *buf;		/* pointer to msg data */
	int alen;		/* addr length */
	u8 *abuf;		/* addr buffer */
};

/* Allow msec timeout per ~byte transfer */
#define I2C_TIMEOUT 10

/**
 * wait_for_completion - manage the actual i2c transfer
 *	@msg: the i2c msg
 */
static int wait_for_completion(struct twi_regs *twi, struct adi_i2c_msg *msg)
{
	u16 int_stat, ctl;
	ulong timebase = get_timer(0);

	do {
		int_stat = readw(&twi->int_stat);

		if (int_stat & XMTSERV) {
			writew(XMTSERV, &twi->int_stat);
			if (msg->alen) {
				writew(*(msg->abuf++), &twi->xmt_data8);
				--msg->alen;
			} else if (!(msg->flags & I2C_M_COMBO) && msg->len) {
				writew(*(msg->buf++), &twi->xmt_data8);
				--msg->len;
			} else {
				ctl = readw(&twi->master_ctl);
				if (msg->flags & I2C_M_COMBO)
					writew(ctl | RSTART | MDIR,
							&twi->master_ctl);
				else
					writew(ctl | STOP, &twi->master_ctl);
			}
		}
		if (int_stat & RCVSERV) {
			writew(RCVSERV, &twi->int_stat);
			if (msg->len) {
				*(msg->buf++) = readw(&twi->rcv_data8);
				--msg->len;
			} else if (msg->flags & I2C_M_STOP) {
				ctl = readw(&twi->master_ctl);
				writew(ctl | STOP, &twi->master_ctl);
			}
		}
		if (int_stat & MERR) {
			writew(MERR, &twi->int_stat);
			return msg->len;
		}
		if (int_stat & MCOMP) {
			writew(MCOMP, &twi->int_stat);
			if (msg->flags & I2C_M_COMBO && msg->len) {
				ctl = readw(&twi->master_ctl);
				ctl = (ctl & ~RSTART) |
					(min(msg->len, 0xff) << 6) | MEN | MDIR;
				writew(ctl, &twi->master_ctl);
			} else
				break;
		}

		/* If we were able to do something, reset timeout */
		if (int_stat)
			timebase = get_timer(0);

	} while (get_timer(timebase) < I2C_TIMEOUT);

	return msg->len;
}

static int i2c_transfer(struct i2c_adapter *adap, uint8_t chip, uint addr,
			int alen, uint8_t *buffer, int len, uint8_t flags)
{
	struct twi_regs *twi = i2c_get_base(adap);
	int ret;
	u16 ctl;
	uchar addr_buffer[] = {
		(addr >>  0),
		(addr >>  8),
		(addr >> 16),
	};
	struct adi_i2c_msg msg = {
		.flags = flags | (len >= 0xff ? I2C_M_STOP : 0),
		.buf   = buffer,
		.len   = len,
		.abuf  = addr_buffer,
		.alen  = alen,
	};

	/* wait for things to settle */
	while (readw(&twi->master_stat) & BUSBUSY)
		if (ctrlc())
			return 1;

	/* Set Transmit device address */
	writew(chip, &twi->master_addr);

	/* Clear the FIFO before starting things */
	writew(XMTFLUSH | RCVFLUSH, &twi->fifo_ctl);
	writew(0, &twi->fifo_ctl);

	/* prime the pump */
	if (msg.alen) {
		len = (msg.flags & I2C_M_COMBO) ? msg.alen : msg.alen + len;
		writew(*(msg.abuf++), &twi->xmt_data8);
		--msg.alen;
	} else if (!(msg.flags & I2C_M_READ) && msg.len) {
		writew(*(msg.buf++), &twi->xmt_data8);
		--msg.len;
	}

	/* clear int stat */
	writew(-1, &twi->master_stat);
	writew(-1, &twi->int_stat);
	writew(0, &twi->int_mask);

	/* Master enable */
	ctl = readw(&twi->master_ctl);
	ctl = (ctl & FAST) | (min(len, 0xff) << 6) | MEN |
		((msg.flags & I2C_M_READ) ? MDIR : 0);
	writew(ctl, &twi->master_ctl);

	/* process the rest */
	ret = wait_for_completion(twi, &msg);

	if (ret) {
		ctl = readw(&twi->master_ctl) & ~MEN;
		writew(ctl, &twi->master_ctl);
		ctl = readw(&twi->control) & ~TWI_ENA;
		writew(ctl, &twi->control);
		ctl = readw(&twi->control) | TWI_ENA;
		writew(ctl, &twi->control);
	}

	return ret;
}

static uint adi_i2c_setspeed(struct i2c_adapter *adap, uint speed)
{
	struct twi_regs *twi = i2c_get_base(adap);
	u16 clkdiv = I2C_SPEED_TO_DUTY(speed);

	/* Set TWI interface clock */
	if (clkdiv < I2C_DUTY_MAX || clkdiv > I2C_DUTY_MIN)
		return -1;
	clkdiv = (clkdiv << 8) | (clkdiv & 0xff);
	writew(clkdiv, &twi->clkdiv);

	/* Don't turn it on */
	writew(speed > 100000 ? FAST : 0, &twi->master_ctl);

	return 0;
}

static void adi_i2c_init(struct i2c_adapter *adap, int speed, int slaveaddr)
{
	struct twi_regs *twi = i2c_get_base(adap);
	u16 prescale = ((get_i2c_clk() / 1000 / 1000 + 5) / 10) & 0x7F;

	/* Set TWI internal clock as 10MHz */
	writew(prescale, &twi->control);

	/* Set TWI interface clock as specified */
	i2c_set_bus_speed(speed);

	/* Enable it */
	writew(TWI_ENA | prescale, &twi->control);
}

static int adi_i2c_read(struct i2c_adapter *adap, uint8_t chip,
			uint addr, int alen, uint8_t *buffer, int len)
{
	return i2c_transfer(adap, chip, addr, alen, buffer,
			len, alen ? I2C_M_COMBO : I2C_M_READ);
}

static int adi_i2c_write(struct i2c_adapter *adap, uint8_t chip,
			uint addr, int alen, uint8_t *buffer, int len)
{
	return i2c_transfer(adap, chip, addr, alen, buffer, len, 0);
}

static int adi_i2c_probe(struct i2c_adapter *adap, uint8_t chip)
{
	u8 byte;
	return adi_i2c_read(adap, chip, 0, 0, &byte, 1);
}

static struct twi_regs *i2c_get_base(struct i2c_adapter *adap)
{
	switch (adap->hwadapnr) {
#if CONFIG_SYS_MAX_I2C_BUS > 2
	case 2:
		return (struct twi_regs *)TWI2_CLKDIV;
#endif
#if CONFIG_SYS_MAX_I2C_BUS > 1
	case 1:
		return (struct twi_regs *)TWI1_CLKDIV;
#endif
	case 0:
		return (struct twi_regs *)TWI0_CLKDIV;

	default:
		printf("wrong hwadapnr: %d\n", adap->hwadapnr);
	}

	return NULL;
}

U_BOOT_I2C_ADAP_COMPLETE(adi_i2c0, adi_i2c_init, adi_i2c_probe,
			 adi_i2c_read, adi_i2c_write,
			 adi_i2c_setspeed,
			 CONFIG_SYS_I2C_SPEED,
			 0,
			 0)

#if CONFIG_SYS_MAX_I2C_BUS > 1
U_BOOT_I2C_ADAP_COMPLETE(adi_i2c1, adi_i2c_init, adi_i2c_probe,
			 adi_i2c_read, adi_i2c_write,
			 adi_i2c_setspeed,
			 CONFIG_SYS_I2C_SPEED,
			 0,
			 1)
#endif

#if CONFIG_SYS_MAX_I2C_BUS > 2
U_BOOT_I2C_ADAP_COMPLETE(adi_i2c2, adi_i2c_init, adi_i2c_probe,
			 adi_i2c_read, adi_i2c_write,
			 adi_i2c_setspeed,
			 CONFIG_SYS_I2C_SPEED,
			 0,
			 2)
#endif
