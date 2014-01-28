/*
 * i2c.c - driver for ADI TWI/I2C
 *
 * Copyright (c) 2006-2013 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <i2c.h>

#include <asm/clock.h>
#include <asm/twi.h>
#include <asm/io.h>

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

/* U-Boot I2C framework allows only one active device at a time.  */
#ifdef TWI_CLKDIV
#define TWI0_CLKDIV TWI_CLKDIV
#endif
static struct twi_regs *twi = (void *)TWI0_CLKDIV;

#ifdef DEBUG
# define dmemset(s, c, n) memset(s, c, n)
#else
# define dmemset(s, c, n)
#endif
#define debugi(fmt, args...) \
	debug( \
		"MSTAT:0x%03x FSTAT:0x%x ISTAT:0x%02x\t%-20s:%-3i: " fmt "\n", \
		twi->master_stat, twi->fifo_stat, twi->int_stat, \
		__func__, __LINE__, ## args)

#ifdef CONFIG_TWICLK_KHZ
# error do not define CONFIG_TWICLK_KHZ ... use CONFIG_SYS_I2C_SPEED
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
# error "The Blackfin I2C hardware can only operate 20KHz - 400KHz"
#endif

/* All transfers are described by this data structure */
struct i2c_msg {
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
static int wait_for_completion(struct i2c_msg *msg)
{
	u16 int_stat, ctl;
	ulong timebase = get_timer(0);

	do {
		int_stat = readw(&twi->int_stat);

		if (int_stat & XMTSERV) {
			debugi("processing XMTSERV");
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
			debugi("processing RCVSERV");
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
			debugi("processing MERR");
			writew(MERR, &twi->int_stat);
			return msg->len;
		}
		if (int_stat & MCOMP) {
			debugi("processing MCOMP");
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

/**
 * i2c_transfer - setup an i2c transfer
 *	@return: 0 if things worked, non-0 if things failed
 *
 *	Here we just get the i2c stuff all prepped and ready, and then tail off
 *	into wait_for_completion() for all the bits to go.
 */
static int i2c_transfer(uchar chip, uint addr, int alen, uchar *buffer,
			int len, u8 flags)
{
	int ret;
	u16 ctl;
	uchar addr_buffer[] = {
		(addr >>  0),
		(addr >>  8),
		(addr >> 16),
	};
	struct i2c_msg msg = {
		.flags = flags | (len >= 0xff ? I2C_M_STOP : 0),
		.buf   = buffer,
		.len   = len,
		.abuf  = addr_buffer,
		.alen  = alen,
	};

	dmemset(buffer, 0xff, len);
	debugi("chip=0x%x addr=0x%02x alen=%i buf[0]=0x%02x len=%i ",
		chip, addr, alen, buffer[0], len);
	debugi("flags=0x%02x[%s] ", flags,
		(flags & I2C_M_READ ? "rd" : "wr"));

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
		debugi("first byte=0x%02x", *msg.abuf);
		writew(*(msg.abuf++), &twi->xmt_data8);
		--msg.alen;
	} else if (!(msg.flags & I2C_M_READ) && msg.len) {
		debugi("first byte=0x%02x", *msg.buf);
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
	ret = wait_for_completion(&msg);
	debugi("ret=%d", ret);

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

/**
 * i2c_set_bus_speed - set i2c bus speed
 *	@speed: bus speed (in HZ)
 */
int i2c_set_bus_speed(unsigned int speed)
{
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

/**
 * i2c_get_bus_speed - get i2c bus speed
 *	@speed: bus speed (in HZ)
 */
unsigned int i2c_get_bus_speed(void)
{
	u16 clkdiv = readw(&twi->clkdiv) & 0xff;
	/* 10 MHz / (2 * CLKDIV) -> 5 MHz / CLKDIV */
	return 5000000 / clkdiv;
}

/**
 * i2c_init - initialize the i2c bus
 *	@speed: bus speed (in HZ)
 *	@slaveaddr: address of device in slave mode (0 - not slave)
 *
 *	Slave mode isn't actually implemented.  It'll stay that way until
 *	we get a real request for it.
 */
void i2c_init(int speed, int slaveaddr)
{
	u16 prescale = ((get_i2c_clk() / 1000 / 1000 + 5) / 10) & 0x7F;

	/* Set TWI internal clock as 10MHz */
	writew(prescale, &twi->control);

	/* Set TWI interface clock as specified */
	i2c_set_bus_speed(speed);

	/* Enable it */
	writew(TWI_ENA | prescale, &twi->control);

	debugi("CONTROL:0x%04x CLKDIV:0x%04x", readw(&twi->control),
		readw(&twi->clkdiv));

#if CONFIG_SYS_I2C_SLAVE
# error I2C slave support not tested/supported
#endif
}

/**
 * i2c_probe - test if a chip exists at a given i2c address
 *	@chip: i2c chip addr to search for
 *	@return: 0 if found, non-0 if not found
 */
int i2c_probe(uchar chip)
{
	u8 byte;
	return i2c_read(chip, 0, 0, &byte, 1);
}

/**
 * i2c_read - read data from an i2c device
 *	@chip: i2c chip addr
 *	@addr: memory (register) address in the chip
 *	@alen: byte size of address
 *	@buffer: buffer to store data read from chip
 *	@len: how many bytes to read
 *	@return: 0 on success, non-0 on failure
 */
int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	return i2c_transfer(chip, addr, alen, buffer,
			len, (alen ? I2C_M_COMBO : I2C_M_READ));
}

/**
 * i2c_write - write data to an i2c device
 *	@chip: i2c chip addr
 *	@addr: memory (register) address in the chip
 *	@alen: byte size of address
 *	@buffer: buffer holding data to write to chip
 *	@len: how many bytes to write
 *	@return: 0 on success, non-0 on failure
 */
int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	return i2c_transfer(chip, addr, alen, buffer, len, 0);
}

/**
 * i2c_set_bus_num - change active I2C bus
 *	@bus: bus index, zero based
 *	@returns: 0 on success, non-0 on failure
 */
int i2c_set_bus_num(unsigned int bus)
{
	switch (bus) {
#if CONFIG_SYS_MAX_I2C_BUS > 0
	case 0:
		twi = (void *)TWI0_CLKDIV;
		return 0;
#endif
#if CONFIG_SYS_MAX_I2C_BUS > 1
	case 1:
		twi = (void *)TWI1_CLKDIV;
		return 0;
#endif
#if CONFIG_SYS_MAX_I2C_BUS > 2
	case 2:
		twi = (void *)TWI2_CLKDIV;
		return 0;
#endif
	default: return -1;
	}
}

/**
 * i2c_get_bus_num - returns index of active I2C bus
 */
unsigned int i2c_get_bus_num(void)
{
	switch ((unsigned long)twi) {
#if CONFIG_SYS_MAX_I2C_BUS > 0
	case TWI0_CLKDIV:
		return 0;
#endif
#if CONFIG_SYS_MAX_I2C_BUS > 1
	case TWI1_CLKDIV:
		return 1;
#endif
#if CONFIG_SYS_MAX_I2C_BUS > 2
	case TWI2_CLKDIV:
		return 2;
#endif
	default: return -1;
	}
}
