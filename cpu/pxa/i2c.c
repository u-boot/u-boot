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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Back ported to the 8xx platform (from the 8260 platform) by
 * Murray.Jensen@cmst.csiro.au, 27-Jan-01.
 */

/* FIXME: this file is PXA255 specific! What about other XScales? */

#include <common.h>

#ifdef CONFIG_HARD_I2C

/*
 *	- CFG_I2C_SPEED
 *	- I2C_PXA_SLAVE_ADDR
 */

#include <asm/arch/hardware.h>
#include <asm/arch/pxa-regs.h>
#include <i2c.h>

/*#define	DEBUG_I2C 	1	/###* activate local debugging output  */
#define I2C_PXA_SLAVE_ADDR	0x1	/* slave pxa unit address           */

#if (CFG_I2C_SPEED == 400000)
#define I2C_ICR_INIT	(ICR_FM | ICR_BEIE | ICR_IRFIE | ICR_ITEIE | ICR_GCD | ICR_SCLE)
#else
#define I2C_ICR_INIT	(ICR_BEIE | ICR_IRFIE | ICR_ITEIE | ICR_GCD | ICR_SCLE)
#endif

#define I2C_ISR_INIT		0x7FF

#ifdef DEBUG_I2C
#define PRINTD(x) printf x
#else
#define PRINTD(x)
#endif


/* Shall the current transfer have a start/stop condition? */
#define I2C_COND_NORMAL		0
#define I2C_COND_START		1
#define I2C_COND_STOP		2

/* Shall the current transfer be ack/nacked or being waited for it? */
#define I2C_ACKNAK_WAITACK	1
#define I2C_ACKNAK_SENDACK	2
#define I2C_ACKNAK_SENDNAK	4

/* Specify who shall transfer the data (master or slave) */
#define I2C_READ		0
#define I2C_WRITE		1

/* All transfers are described by this data structure */
struct i2c_msg {
	u8 condition;
	u8 acknack;
	u8 direction;
	u8 data;
};


/**
 * i2c_pxa_reset: - reset the host controller
 *
 */

static void i2c_reset( void )
{
	ICR &= ~ICR_IUE;		/* disable unit */
	ICR |= ICR_UR;			/* reset the unit */
	udelay(100);
	ICR &= ~ICR_IUE;		/* disable unit */
#ifdef CONFIG_CPU_MONAHANS
	CKENB |= (CKENB_4_I2C); /*  | CKENB_1_PWM1 | CKENB_0_PWM0); */
#else /* CONFIG_CPU_MONAHANS */
	CKEN |= CKEN14_I2C;		/* set the global I2C clock on */
#endif
	ISAR = I2C_PXA_SLAVE_ADDR;	/* set our slave address */
	ICR = I2C_ICR_INIT;		/* set control register values */
	ISR = I2C_ISR_INIT;		/* set clear interrupt bits */
	ICR |= ICR_IUE;			/* enable unit */
	udelay(100);
}


/**
 * i2c_isr_set_cleared: - wait until certain bits of the I2C status register
 *	                  are set and cleared
 *
 * @return: 1 in case of success, 0 means timeout (no match within 10 ms).
 */
static int i2c_isr_set_cleared( unsigned long set_mask, unsigned long cleared_mask )
{
	int timeout = 10000;

	while( ((ISR & set_mask)!=set_mask) || ((ISR & cleared_mask)!=0) ){
		udelay( 10 );
		if( timeout-- < 0 ) return 0;
	}

	return 1;
}


/**
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
int i2c_transfer(struct i2c_msg *msg)
{
	int ret;

	if (!msg)
		goto transfer_error_msg_empty;

	switch(msg->direction) {

	case I2C_WRITE:

		/* check if bus is not busy */
		if (!i2c_isr_set_cleared(0,ISR_IBB))
			goto transfer_error_bus_busy;

		/* start transmission */
		ICR &= ~ICR_START;
		ICR &= ~ICR_STOP;
		IDBR = msg->data;
		if (msg->condition == I2C_COND_START)     ICR |=  ICR_START;
		if (msg->condition == I2C_COND_STOP)      ICR |=  ICR_STOP;
		if (msg->acknack   == I2C_ACKNAK_SENDNAK) ICR |=  ICR_ACKNAK;
		if (msg->acknack   == I2C_ACKNAK_SENDACK) ICR &= ~ICR_ACKNAK;
		ICR &= ~ICR_ALDIE;
		ICR |= ICR_TB;

		/* transmit register empty? */
		if (!i2c_isr_set_cleared(ISR_ITE,0))
			goto transfer_error_transmit_timeout;

		/* clear 'transmit empty' state */
		ISR |= ISR_ITE;

		/* wait for ACK from slave */
		if (msg->acknack == I2C_ACKNAK_WAITACK)
			if (!i2c_isr_set_cleared(0,ISR_ACKNAK))
				goto transfer_error_ack_missing;
		break;

	case I2C_READ:

		/* check if bus is not busy */
		if (!i2c_isr_set_cleared(0,ISR_IBB))
			goto transfer_error_bus_busy;

		/* start receive */
		ICR &= ~ICR_START;
		ICR &= ~ICR_STOP;
		if (msg->condition == I2C_COND_START) 	  ICR |= ICR_START;
		if (msg->condition == I2C_COND_STOP)  	  ICR |= ICR_STOP;
		if (msg->acknack   == I2C_ACKNAK_SENDNAK) ICR |=  ICR_ACKNAK;
		if (msg->acknack   == I2C_ACKNAK_SENDACK) ICR &= ~ICR_ACKNAK;
		ICR &= ~ICR_ALDIE;
		ICR |= ICR_TB;

		/* receive register full? */
		if (!i2c_isr_set_cleared(ISR_IRF,0))
			goto transfer_error_receive_timeout;

		msg->data = IDBR;

		/* clear 'receive empty' state */
		ISR |= ISR_IRF;

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
		PRINTD(("i2c_transfer: ISR: 0x%04x\n",ISR));
		i2c_reset();
		return ret;

}

/* ------------------------------------------------------------------------ */
/* API Functions                                                            */
/* ------------------------------------------------------------------------ */

void i2c_init(int speed, int slaveaddr)
{
#ifdef CFG_I2C_INIT_BOARD
	/* call board specific i2c bus reset routine before accessing the   */
	/* environment, which might be in a chip on that bus. For details   */
	/* about this problem see doc/I2C_Edge_Conditions.                  */
	i2c_init_board();
#endif
}


/**
 * i2c_probe: - Test if a chip answers for a given i2c address
 *
 * @chip:	address of the chip which is searched for
 * @return: 	0 if a chip was found, -1 otherwhise
 */

int i2c_probe(uchar chip)
{
	struct i2c_msg msg;

	i2c_reset();

	msg.condition = I2C_COND_START;
	msg.acknack   = I2C_ACKNAK_WAITACK;
	msg.direction = I2C_WRITE;
	msg.data      = (chip << 1) + 1;
	if (i2c_transfer(&msg)) return -1;

	msg.condition = I2C_COND_STOP;
	msg.acknack   = I2C_ACKNAK_SENDNAK;
	msg.direction = I2C_READ;
	msg.data      = 0x00;
	if (i2c_transfer(&msg)) return -1;

	return 0;
}


/**
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
	struct i2c_msg msg;
	u8 addr_bytes[3]; /* lowest...highest byte of data address */
	int ret;

	PRINTD(("i2c_read(chip=0x%02x, addr=0x%02x, alen=0x%02x, len=0x%02x)\n",chip,addr,alen,len));

	i2c_reset();

	/* dummy chip address write */
	PRINTD(("i2c_read: dummy chip address write\n"));
	msg.condition = I2C_COND_START;
	msg.acknack   = I2C_ACKNAK_WAITACK;
	msg.direction = I2C_WRITE;
	msg.data      = (chip << 1);
	msg.data     &= 0xFE;
	if ((ret=i2c_transfer(&msg))) return -1;

	/*
	 * send memory address bytes;
	 * alen defines how much bytes we have to send.
	 */
	/*addr &= ((1 << CFG_EEPROM_PAGE_WRITE_BITS)-1); */
	addr_bytes[0] = (u8)((addr >>  0) & 0x000000FF);
	addr_bytes[1] = (u8)((addr >>  8) & 0x000000FF);
	addr_bytes[2] = (u8)((addr >> 16) & 0x000000FF);

	while (--alen >= 0) {

		PRINTD(("i2c_read: send memory word address byte %1d\n",alen));
		msg.condition = I2C_COND_NORMAL;
		msg.acknack   = I2C_ACKNAK_WAITACK;
		msg.direction = I2C_WRITE;
		msg.data      = addr_bytes[alen];
		if ((ret=i2c_transfer(&msg))) return -1;
	}


	/* start read sequence */
	PRINTD(("i2c_read: start read sequence\n"));
	msg.condition = I2C_COND_START;
	msg.acknack   = I2C_ACKNAK_WAITACK;
	msg.direction = I2C_WRITE;
	msg.data      = (chip << 1);
	msg.data     |= 0x01;
	if ((ret=i2c_transfer(&msg))) return -1;

	/* read bytes; send NACK at last byte */
	while (len--) {

		if (len==0) {
			msg.condition = I2C_COND_STOP;
			msg.acknack   = I2C_ACKNAK_SENDNAK;
		} else {
			msg.condition = I2C_COND_NORMAL;
			msg.acknack   = I2C_ACKNAK_SENDACK;
		}

		msg.direction = I2C_READ;
		msg.data      = 0x00;
		if ((ret=i2c_transfer(&msg))) return -1;

		*buffer = msg.data;
		PRINTD(("i2c_read: reading byte (0x%08x)=0x%02x\n",(unsigned int)buffer,*buffer));
		buffer++;

	}

	i2c_reset();

	return 0;
}


/**
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
	struct i2c_msg msg;
	u8 addr_bytes[3]; /* lowest...highest byte of data address */

	PRINTD(("i2c_write(chip=0x%02x, addr=0x%02x, alen=0x%02x, len=0x%02x)\n",chip,addr,alen,len));

	i2c_reset();

	/* chip address write */
	PRINTD(("i2c_write: chip address write\n"));
	msg.condition = I2C_COND_START;
	msg.acknack   = I2C_ACKNAK_WAITACK;
	msg.direction = I2C_WRITE;
	msg.data      = (chip << 1);
	msg.data     &= 0xFE;
	if (i2c_transfer(&msg)) return -1;

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
		if (i2c_transfer(&msg)) return -1;
	}

	/* write bytes; send NACK at last byte */
	while (len--) {

		PRINTD(("i2c_write: writing byte (0x%08x)=0x%02x\n",(unsigned int)buffer,*buffer));

		if (len==0)
			msg.condition = I2C_COND_STOP;
		else
			msg.condition = I2C_COND_NORMAL;

		msg.acknack   = I2C_ACKNAK_WAITACK;
		msg.direction = I2C_WRITE;
		msg.data      = *(buffer++);

		if (i2c_transfer(&msg)) return -1;

	}

	i2c_reset();

	return 0;

}

uchar i2c_reg_read (uchar chip, uchar reg)
{
	char buf;

	PRINTD(("i2c_reg_read(chip=0x%02x, reg=0x%02x)\n",chip,reg));
	i2c_read(chip, reg, 1, &buf, 1);
	return (buf);
}

void  i2c_reg_write(uchar chip, uchar reg, uchar val)
{
	PRINTD(("i2c_reg_write(chip=0x%02x, reg=0x%02x, val=0x%02x)\n",chip,reg,val));
	i2c_write(chip, reg, 1, &val, 1);
}

#endif	/* CONFIG_HARD_I2C */
