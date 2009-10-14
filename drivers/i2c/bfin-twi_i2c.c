/*
 * i2c.c - driver for Blackfin on-chip TWI/I2C
 *
 * Copyright (c) 2006-2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <i2c.h>

#include <asm/blackfin.h>
#include <asm/mach-common/bits/twi.h>

#ifdef DEBUG
# define dmemset(s, c, n) memset(s, c, n)
#else
# define dmemset(s, c, n)
#endif
#define debugi(fmt, args...) \
	debug( \
		"MSTAT:0x%03x FSTAT:0x%x ISTAT:0x%02x\t" \
		"%-20s:%-3i: " fmt "\n", \
		bfin_read_TWI_MASTER_STAT(), bfin_read_TWI_FIFO_STAT(), bfin_read_TWI_INT_STAT(), \
		__func__, __LINE__, ## args)

#ifdef TWI0_CLKDIV
#define bfin_write_TWI_CLKDIV(val)           bfin_write_TWI0_CLKDIV(val)
#define bfin_read_TWI_CLKDIV(val)            bfin_read_TWI0_CLKDIV(val)
#define bfin_write_TWI_CONTROL(val)          bfin_write_TWI0_CONTROL(val)
#define bfin_read_TWI_CONTROL(val)           bfin_read_TWI0_CONTROL(val)
#define bfin_write_TWI_MASTER_ADDR(val)      bfin_write_TWI0_MASTER_ADDR(val)
#define bfin_write_TWI_XMT_DATA8(val)        bfin_write_TWI0_XMT_DATA8(val)
#define bfin_read_TWI_RCV_DATA8()            bfin_read_TWI0_RCV_DATA8()
#define bfin_read_TWI_INT_STAT()             bfin_read_TWI0_INT_STAT()
#define bfin_write_TWI_INT_STAT(val)         bfin_write_TWI0_INT_STAT(val)
#define bfin_read_TWI_MASTER_STAT()          bfin_read_TWI0_MASTER_STAT()
#define bfin_write_TWI_MASTER_STAT(val)      bfin_write_TWI0_MASTER_STAT(val)
#define bfin_read_TWI_MASTER_CTL()           bfin_read_TWI0_MASTER_CTL()
#define bfin_write_TWI_MASTER_CTL(val)       bfin_write_TWI0_MASTER_CTL(val)
#define bfin_write_TWI_INT_MASK(val)         bfin_write_TWI0_INT_MASK(val)
#define bfin_write_TWI_FIFO_CTL(val)         bfin_write_TWI0_FIFO_CTL(val)
#endif

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
	uint16_t int_stat;
	ulong timebase = get_timer(0);

	do {
		int_stat = bfin_read_TWI_INT_STAT();

		if (int_stat & XMTSERV) {
			debugi("processing XMTSERV");
			bfin_write_TWI_INT_STAT(XMTSERV);
			SSYNC();
			if (msg->alen) {
				bfin_write_TWI_XMT_DATA8(*(msg->abuf++));
				--msg->alen;
			} else if (!(msg->flags & I2C_M_COMBO) && msg->len) {
				bfin_write_TWI_XMT_DATA8(*(msg->buf++));
				--msg->len;
			} else {
				bfin_write_TWI_MASTER_CTL(bfin_read_TWI_MASTER_CTL() |
					(msg->flags & I2C_M_COMBO ? RSTART | MDIR : STOP));
				SSYNC();
			}
		}
		if (int_stat & RCVSERV) {
			debugi("processing RCVSERV");
			bfin_write_TWI_INT_STAT(RCVSERV);
			SSYNC();
			if (msg->len) {
				*(msg->buf++) = bfin_read_TWI_RCV_DATA8();
				--msg->len;
			} else if (msg->flags & I2C_M_STOP) {
				bfin_write_TWI_MASTER_CTL(bfin_read_TWI_MASTER_CTL() | STOP);
				SSYNC();
			}
		}
		if (int_stat & MERR) {
			debugi("processing MERR");
			bfin_write_TWI_INT_STAT(MERR);
			SSYNC();
			return msg->len;
		}
		if (int_stat & MCOMP) {
			debugi("processing MCOMP");
			bfin_write_TWI_INT_STAT(MCOMP);
			SSYNC();
			if (msg->flags & I2C_M_COMBO && msg->len) {
				bfin_write_TWI_MASTER_CTL((bfin_read_TWI_MASTER_CTL() & ~RSTART) |
					(min(msg->len, 0xff) << 6) | MEN | MDIR);
				SSYNC();
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
static int i2c_transfer(uchar chip, uint addr, int alen, uchar *buffer, int len, u8 flags)
{
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
	int ret;

	dmemset(buffer, 0xff, len);
	debugi("chip=0x%x addr=0x%02x alen=%i buf[0]=0x%02x len=%i flags=0x%02x[%s] ",
		chip, addr, alen, buffer[0], len, flags, (flags & I2C_M_READ ? "rd" : "wr"));

	/* wait for things to settle */
	while (bfin_read_TWI_MASTER_STAT() & BUSBUSY)
		if (ctrlc())
			return 1;

	/* Set Transmit device address */
	bfin_write_TWI_MASTER_ADDR(chip);

	/* Clear the FIFO before starting things */
	bfin_write_TWI_FIFO_CTL(XMTFLUSH | RCVFLUSH);
	SSYNC();
	bfin_write_TWI_FIFO_CTL(0);
	SSYNC();

	/* prime the pump */
	if (msg.alen) {
		len = (msg.flags & I2C_M_COMBO) ? msg.alen : msg.alen + len;
		debugi("first byte=0x%02x", *msg.abuf);
		bfin_write_TWI_XMT_DATA8(*(msg.abuf++));
		--msg.alen;
	} else if (!(msg.flags & I2C_M_READ) && msg.len) {
		debugi("first byte=0x%02x", *msg.buf);
		bfin_write_TWI_XMT_DATA8(*(msg.buf++));
		--msg.len;
	}

	/* clear int stat */
	bfin_write_TWI_MASTER_STAT(-1);
	bfin_write_TWI_INT_STAT(-1);
	bfin_write_TWI_INT_MASK(0);
	SSYNC();

	/* Master enable */
	bfin_write_TWI_MASTER_CTL(
			(bfin_read_TWI_MASTER_CTL() & FAST) |
			(min(len, 0xff) << 6) | MEN |
			((msg.flags & I2C_M_READ) ? MDIR : 0)
	);
	SSYNC();
	debugi("CTL=0x%04x", bfin_read_TWI_MASTER_CTL());

	/* process the rest */
	ret = wait_for_completion(&msg);
	debugi("ret=%d", ret);

	if (ret) {
		bfin_write_TWI_MASTER_CTL(bfin_read_TWI_MASTER_CTL() & ~MEN);
		bfin_write_TWI_CONTROL(bfin_read_TWI_CONTROL() & ~TWI_ENA);
		SSYNC();
		bfin_write_TWI_CONTROL(bfin_read_TWI_CONTROL() | TWI_ENA);
		SSYNC();
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
	bfin_write_TWI_CLKDIV((clkdiv << 8) | (clkdiv & 0xff));

	/* Don't turn it on */
	bfin_write_TWI_MASTER_CTL(speed > 100000 ? FAST : 0);

	return 0;
}

/**
 * i2c_get_bus_speed - get i2c bus speed
 *	@speed: bus speed (in HZ)
 */
unsigned int i2c_get_bus_speed(void)
{
	/* 10 MHz / (2 * CLKDIV) -> 5 MHz / CLKDIV */
	return 5000000 / (bfin_read_TWI_CLKDIV() & 0xff);
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
	uint8_t prescale = ((get_sclk() / 1024 / 1024 + 5) / 10) & 0x7F;

	/* Set TWI internal clock as 10MHz */
	bfin_write_TWI_CONTROL(prescale);

	/* Set TWI interface clock as specified */
	i2c_set_bus_speed(speed);

	/* Enable it */
	bfin_write_TWI_CONTROL(TWI_ENA | prescale);
	SSYNC();

	debugi("CONTROL:0x%04x CLKDIV:0x%04x",
		bfin_read_TWI_CONTROL(), bfin_read_TWI_CLKDIV());

#if CONFIG_SYS_I2C_SLAVE
# error I2C slave support not tested/supported
	/* If they want us as a slave, do it */
	if (slaveaddr) {
		bfin_write_TWI_SLAVE_ADDR(slaveaddr);
		bfin_write_TWI_SLAVE_CTL(SEN);
	}
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
	return i2c_transfer(chip, addr, alen, buffer, len, (alen ? I2C_M_COMBO : I2C_M_READ));
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
