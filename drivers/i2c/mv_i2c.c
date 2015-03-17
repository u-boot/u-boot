/*
 * (C) Copyright 2000
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
 *
 * (C) Copyright 2000 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2003 Pengutronix e.K.
 * Robert Schwebel <r.schwebel@pengutronix.de>
 *
 * (C) Copyright 2011 Marvell Inc.
 * Lei Wen <leiwen@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Back ported to the 8xx platform (from the 8260 platform) by
 * Murray.Jensen@cmst.csiro.au, 27-Jan-01.
 */

#include <common.h>
#include <asm/io.h>

#ifdef CONFIG_HARD_I2C
#include <i2c.h>
#include "mv_i2c.h"

#ifdef DEBUG_I2C
#define PRINTD(x) printf x
#else
#define PRINTD(x)
#endif

/* All transfers are described by this data structure */
struct mv_i2c_msg {
	u8 condition;
	u8 acknack;
	u8 direction;
	u8 data;
};

struct mv_i2c {
	u32 ibmr;
	u32 pad0;
	u32 idbr;
	u32 pad1;
	u32 icr;
	u32 pad2;
	u32 isr;
	u32 pad3;
	u32 isar;
};

static struct mv_i2c *base;
static void i2c_board_init(struct mv_i2c *base)
{
#ifdef CONFIG_SYS_I2C_INIT_BOARD
	u32 icr;
	/*
	 * call board specific i2c bus reset routine before accessing the
	 * environment, which might be in a chip on that bus. For details
	 * about this problem see doc/I2C_Edge_Conditions.
	 *
	 * disable I2C controller first, otherwhise it thinks we want to
	 * talk to the slave port...
	 */
	icr = readl(&base->icr);
	writel(readl(&base->icr) & ~(ICR_SCLE | ICR_IUE), &base->icr);

	i2c_init_board();

	writel(icr, &base->icr);
#endif
}

#ifdef CONFIG_I2C_MULTI_BUS
static unsigned long i2c_regs[CONFIG_MV_I2C_NUM] = CONFIG_MV_I2C_REG;
static unsigned int bus_initialized[CONFIG_MV_I2C_NUM];
static unsigned int current_bus;

int i2c_set_bus_num(unsigned int bus)
{
	if ((bus < 0) || (bus >= CONFIG_MV_I2C_NUM)) {
		printf("Bad bus: %d\n", bus);
		return -1;
	}

	base = (struct mv_i2c *)i2c_regs[bus];
	current_bus = bus;

	if (!bus_initialized[current_bus]) {
		i2c_board_init(base);
		bus_initialized[current_bus] = 1;
	}

	return 0;
}

unsigned int i2c_get_bus_num(void)
{
	return current_bus;
}
#endif

/*
 * i2c_reset: - reset the host controller
 *
 */
static void i2c_reset(void)
{
	writel(readl(&base->icr) & ~ICR_IUE, &base->icr); /* disable unit */
	writel(readl(&base->icr) | ICR_UR, &base->icr);	  /* reset the unit */
	udelay(100);
	writel(readl(&base->icr) & ~ICR_IUE, &base->icr); /* disable unit */

	i2c_clk_enable();

	writel(CONFIG_SYS_I2C_SLAVE, &base->isar); /* set our slave address */
	writel(I2C_ICR_INIT, &base->icr); /* set control reg values */
	writel(I2C_ISR_INIT, &base->isr); /* set clear interrupt bits */
	writel(readl(&base->icr) | ICR_IUE, &base->icr); /* enable unit */
	udelay(100);
}

/*
 * i2c_isr_set_cleared: - wait until certain bits of the I2C status register
 *	                  are set and cleared
 *
 * @return: 1 in case of success, 0 means timeout (no match within 10 ms).
 */
static int i2c_isr_set_cleared(unsigned long set_mask,
			       unsigned long cleared_mask)
{
	int timeout = 1000, isr;

	do {
		isr = readl(&base->isr);
		udelay(10);
		if (timeout-- < 0)
			return 0;
	} while (((isr & set_mask) != set_mask)
		|| ((isr & cleared_mask) != 0));

	return 1;
}

/*
 * i2c_transfer: - Transfer one byte over the i2c bus
 *
 * This function can tranfer a byte over the i2c bus in both directions.
 * It is used by the public API functions.
 *
 * @return:  0: transfer successful
 *          -1: message is empty
 *          -2: transmit timeout
 *          -3: ACK missing
 *          -4: receive timeout
 *          -5: illegal parameters
 *          -6: bus is busy and couldn't be aquired
 */
int i2c_transfer(struct mv_i2c_msg *msg)
{
	int ret;

	if (!msg)
		goto transfer_error_msg_empty;

	switch (msg->direction) {
	case I2C_WRITE:
		/* check if bus is not busy */
		if (!i2c_isr_set_cleared(0, ISR_IBB))
			goto transfer_error_bus_busy;

		/* start transmission */
		writel(readl(&base->icr) & ~ICR_START, &base->icr);
		writel(readl(&base->icr) & ~ICR_STOP, &base->icr);
		writel(msg->data, &base->idbr);
		if (msg->condition == I2C_COND_START)
			writel(readl(&base->icr) | ICR_START, &base->icr);
		if (msg->condition == I2C_COND_STOP)
			writel(readl(&base->icr) | ICR_STOP, &base->icr);
		if (msg->acknack == I2C_ACKNAK_SENDNAK)
			writel(readl(&base->icr) | ICR_ACKNAK, &base->icr);
		if (msg->acknack == I2C_ACKNAK_SENDACK)
			writel(readl(&base->icr) & ~ICR_ACKNAK, &base->icr);
		writel(readl(&base->icr) & ~ICR_ALDIE, &base->icr);
		writel(readl(&base->icr) | ICR_TB, &base->icr);

		/* transmit register empty? */
		if (!i2c_isr_set_cleared(ISR_ITE, 0))
			goto transfer_error_transmit_timeout;

		/* clear 'transmit empty' state */
		writel(readl(&base->isr) | ISR_ITE, &base->isr);

		/* wait for ACK from slave */
		if (msg->acknack == I2C_ACKNAK_WAITACK)
			if (!i2c_isr_set_cleared(0, ISR_ACKNAK))
				goto transfer_error_ack_missing;
		break;

	case I2C_READ:

		/* check if bus is not busy */
		if (!i2c_isr_set_cleared(0, ISR_IBB))
			goto transfer_error_bus_busy;

		/* start receive */
		writel(readl(&base->icr) & ~ICR_START, &base->icr);
		writel(readl(&base->icr) & ~ICR_STOP, &base->icr);
		if (msg->condition == I2C_COND_START)
			writel(readl(&base->icr) | ICR_START, &base->icr);
		if (msg->condition == I2C_COND_STOP)
			writel(readl(&base->icr) | ICR_STOP, &base->icr);
		if (msg->acknack == I2C_ACKNAK_SENDNAK)
			writel(readl(&base->icr) | ICR_ACKNAK, &base->icr);
		if (msg->acknack == I2C_ACKNAK_SENDACK)
			writel(readl(&base->icr) & ~ICR_ACKNAK, &base->icr);
		writel(readl(&base->icr) & ~ICR_ALDIE, &base->icr);
		writel(readl(&base->icr) | ICR_TB, &base->icr);

		/* receive register full? */
		if (!i2c_isr_set_cleared(ISR_IRF, 0))
			goto transfer_error_receive_timeout;

		msg->data = readl(&base->idbr);

		/* clear 'receive empty' state */
		writel(readl(&base->isr) | ISR_IRF, &base->isr);
		break;
	default:
		goto transfer_error_illegal_param;
	}

	return 0;

transfer_error_msg_empty:
		PRINTD(("i2c_transfer: error: 'msg' is empty\n"));
		ret = -1; goto i2c_transfer_finish;

transfer_error_transmit_timeout:
		PRINTD(("i2c_transfer: error: transmit timeout\n"));
		ret = -2; goto i2c_transfer_finish;

transfer_error_ack_missing:
		PRINTD(("i2c_transfer: error: ACK missing\n"));
		ret = -3; goto i2c_transfer_finish;

transfer_error_receive_timeout:
		PRINTD(("i2c_transfer: error: receive timeout\n"));
		ret = -4; goto i2c_transfer_finish;

transfer_error_illegal_param:
		PRINTD(("i2c_transfer: error: illegal parameters\n"));
		ret = -5; goto i2c_transfer_finish;

transfer_error_bus_busy:
		PRINTD(("i2c_transfer: error: bus is busy\n"));
		ret = -6; goto i2c_transfer_finish;

i2c_transfer_finish:
		PRINTD(("i2c_transfer: ISR: 0x%04x\n", readl(&base->isr)));
		i2c_reset();
		return ret;
}

/* ------------------------------------------------------------------------ */
/* API Functions                                                            */
/* ------------------------------------------------------------------------ */
void i2c_init(int speed, int slaveaddr)
{
#ifdef CONFIG_I2C_MULTI_BUS
	current_bus = 0;
	base = (struct mv_i2c *)i2c_regs[current_bus];
#else
	base = (struct mv_i2c *)CONFIG_MV_I2C_REG;
#endif

	i2c_board_init(base);
}

/*
 * i2c_probe: - Test if a chip answers for a given i2c address
 *
 * @chip:	address of the chip which is searched for
 * @return:	0 if a chip was found, -1 otherwhise
 */
int i2c_probe(uchar chip)
{
	struct mv_i2c_msg msg;

	i2c_reset();

	msg.condition = I2C_COND_START;
	msg.acknack   = I2C_ACKNAK_WAITACK;
	msg.direction = I2C_WRITE;
	msg.data      = (chip << 1) + 1;
	if (i2c_transfer(&msg))
		return -1;

	msg.condition = I2C_COND_STOP;
	msg.acknack   = I2C_ACKNAK_SENDNAK;
	msg.direction = I2C_READ;
	msg.data      = 0x00;
	if (i2c_transfer(&msg))
		return -1;

	return 0;
}

/*
 * i2c_read: - Read multiple bytes from an i2c device
 *
 * The higher level routines take into account that this function is only
 * called with len < page length of the device (see configuration file)
 *
 * @chip:	address of the chip which is to be read
 * @addr:	i2c data address within the chip
 * @alen:	length of the i2c data address (1..2 bytes)
 * @buffer:	where to write the data
 * @len:	how much byte do we want to read
 * @return:	0 in case of success
 */
int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	struct mv_i2c_msg msg;
	u8 addr_bytes[3]; /* lowest...highest byte of data address */

	PRINTD(("i2c_read(chip=0x%02x, addr=0x%02x, alen=0x%02x, "
		"len=0x%02x)\n", chip, addr, alen, len));

	i2c_reset();

	/* dummy chip address write */
	PRINTD(("i2c_read: dummy chip address write\n"));
	msg.condition = I2C_COND_START;
	msg.acknack   = I2C_ACKNAK_WAITACK;
	msg.direction = I2C_WRITE;
	msg.data = (chip << 1);
	msg.data &= 0xFE;
	if (i2c_transfer(&msg))
		return -1;

	/*
	 * send memory address bytes;
	 * alen defines how much bytes we have to send.
	 */
	/*addr &= ((1 << CONFIG_SYS_EEPROM_PAGE_WRITE_BITS)-1); */
	addr_bytes[0] = (u8)((addr >>  0) & 0x000000FF);
	addr_bytes[1] = (u8)((addr >>  8) & 0x000000FF);
	addr_bytes[2] = (u8)((addr >> 16) & 0x000000FF);

	while (--alen >= 0) {
		PRINTD(("i2c_read: send memory word address byte %1d\n", alen));
		msg.condition = I2C_COND_NORMAL;
		msg.acknack   = I2C_ACKNAK_WAITACK;
		msg.direction = I2C_WRITE;
		msg.data      = addr_bytes[alen];
		if (i2c_transfer(&msg))
			return -1;
	}

	/* start read sequence */
	PRINTD(("i2c_read: start read sequence\n"));
	msg.condition = I2C_COND_START;
	msg.acknack   = I2C_ACKNAK_WAITACK;
	msg.direction = I2C_WRITE;
	msg.data      = (chip << 1);
	msg.data     |= 0x01;
	if (i2c_transfer(&msg))
		return -1;

	/* read bytes; send NACK at last byte */
	while (len--) {
		if (len == 0) {
			msg.condition = I2C_COND_STOP;
			msg.acknack   = I2C_ACKNAK_SENDNAK;
		} else {
			msg.condition = I2C_COND_NORMAL;
			msg.acknack   = I2C_ACKNAK_SENDACK;
		}

		msg.direction = I2C_READ;
		msg.data      = 0x00;
		if (i2c_transfer(&msg))
			return -1;

		*buffer = msg.data;
		PRINTD(("i2c_read: reading byte (0x%08x)=0x%02x\n",
			(unsigned int)buffer, *buffer));
		buffer++;
	}

	i2c_reset();

	return 0;
}

/*
 * i2c_write: -  Write multiple bytes to an i2c device
 *
 * The higher level routines take into account that this function is only
 * called with len < page length of the device (see configuration file)
 *
 * @chip:	address of the chip which is to be written
 * @addr:	i2c data address within the chip
 * @alen:	length of the i2c data address (1..2 bytes)
 * @buffer:	where to find the data to be written
 * @len:	how much byte do we want to read
 * @return:	0 in case of success
 */
int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	struct mv_i2c_msg msg;
	u8 addr_bytes[3]; /* lowest...highest byte of data address */

	PRINTD(("i2c_write(chip=0x%02x, addr=0x%02x, alen=0x%02x, "
		"len=0x%02x)\n", chip, addr, alen, len));

	i2c_reset();

	/* chip address write */
	PRINTD(("i2c_write: chip address write\n"));
	msg.condition = I2C_COND_START;
	msg.acknack   = I2C_ACKNAK_WAITACK;
	msg.direction = I2C_WRITE;
	msg.data = (chip << 1);
	msg.data &= 0xFE;
	if (i2c_transfer(&msg))
		return -1;

	/*
	 * send memory address bytes;
	 * alen defines how much bytes we have to send.
	 */
	addr_bytes[0] = (u8)((addr >>  0) & 0x000000FF);
	addr_bytes[1] = (u8)((addr >>  8) & 0x000000FF);
	addr_bytes[2] = (u8)((addr >> 16) & 0x000000FF);

	while (--alen >= 0) {
		PRINTD(("i2c_write: send memory word address\n"));
		msg.condition = I2C_COND_NORMAL;
		msg.acknack   = I2C_ACKNAK_WAITACK;
		msg.direction = I2C_WRITE;
		msg.data      = addr_bytes[alen];
		if (i2c_transfer(&msg))
			return -1;
	}

	/* write bytes; send NACK at last byte */
	while (len--) {
		PRINTD(("i2c_write: writing byte (0x%08x)=0x%02x\n",
			(unsigned int)buffer, *buffer));

		if (len == 0)
			msg.condition = I2C_COND_STOP;
		else
			msg.condition = I2C_COND_NORMAL;

		msg.acknack   = I2C_ACKNAK_WAITACK;
		msg.direction = I2C_WRITE;
		msg.data      = *(buffer++);

		if (i2c_transfer(&msg))
			return -1;
	}

	i2c_reset();

	return 0;
}
#endif	/* CONFIG_HARD_I2C */
