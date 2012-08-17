/*
 * (C) Copyright 2007-2009
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * based on work by Anne Sophie Harnois <anne-sophie.harnois@nextream.fr>
 *
 * (C) Copyright 2001
 * Bill Hunter,  Wave 7 Optics, williamhunter@mediaone.net
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/ppc4xx.h>
#include <asm/ppc4xx-i2c.h>
#include <i2c.h>
#include <asm/io.h>

#ifdef CONFIG_HARD_I2C

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_I2C_MULTI_BUS)
/*
 * Initialize the bus pointer to whatever one the SPD EEPROM is on.
 * Default is bus 0.  This is necessary because the DDR initialization
 * runs from ROM, and we can't switch buses because we can't modify
 * the global variables.
 */
#ifndef CONFIG_SYS_SPD_BUS_NUM
#define CONFIG_SYS_SPD_BUS_NUM	0
#endif
static unsigned int i2c_bus_num __attribute__ ((section (".data"))) =
	CONFIG_SYS_SPD_BUS_NUM;
#endif /* CONFIG_I2C_MULTI_BUS */

static void _i2c_bus_reset(void)
{
	struct ppc4xx_i2c *i2c = (struct ppc4xx_i2c *)I2C_BASE_ADDR;
	int i;
	u8 dc;

	/* Reset status register */
	/* write 1 in SCMP and IRQA to clear these fields */
	out_8(&i2c->sts, 0x0A);

	/* write 1 in IRQP IRQD LA ICT XFRA to clear these fields */
	out_8(&i2c->extsts, 0x8F);

	/* Place chip in the reset state */
	out_8(&i2c->xtcntlss, IIC_XTCNTLSS_SRST);

	/* Check if bus is free */
	dc = in_8(&i2c->directcntl);
	if (!DIRCTNL_FREE(dc)){
		/* Try to set bus free state */
		out_8(&i2c->directcntl, IIC_DIRCNTL_SDAC | IIC_DIRCNTL_SCC);

		/* Wait until we regain bus control */
		for (i = 0; i < 100; ++i) {
			dc = in_8(&i2c->directcntl);
			if (DIRCTNL_FREE(dc))
				break;

			/* Toggle SCL line */
			dc ^= IIC_DIRCNTL_SCC;
			out_8(&i2c->directcntl, dc);
			udelay(10);
			dc ^= IIC_DIRCNTL_SCC;
			out_8(&i2c->directcntl, dc);
		}
	}

	/* Remove reset */
	out_8(&i2c->xtcntlss, 0);
}

void i2c_init(int speed, int slaveaddr)
{
	struct ppc4xx_i2c *i2c;
	int val, divisor;
	int bus;

#ifdef CONFIG_SYS_I2C_INIT_BOARD
	/*
	 * Call board specific i2c bus reset routine before accessing the
	 * environment, which might be in a chip on that bus. For details
	 * about this problem see doc/I2C_Edge_Conditions.
	 */
	i2c_init_board();
#endif

	for (bus = 0; bus < CONFIG_SYS_MAX_I2C_BUS; bus++) {
		I2C_SET_BUS(bus);

		/* Set i2c pointer after calling I2C_SET_BUS() */
		i2c = (struct ppc4xx_i2c *)I2C_BASE_ADDR;

		/* Handle possible failed I2C state */
		/* FIXME: put this into i2c_init_board()? */
		_i2c_bus_reset();

		/* clear lo master address */
		out_8(&i2c->lmadr, 0);

		/* clear hi master address */
		out_8(&i2c->hmadr, 0);

		/* clear lo slave address */
		out_8(&i2c->lsadr, 0);

		/* clear hi slave address */
		out_8(&i2c->hsadr, 0);

		/* Clock divide Register */
		/* set divisor according to freq_opb */
		divisor = (get_OPB_freq() - 1) / 10000000;
		if (divisor == 0)
			divisor = 1;
		out_8(&i2c->clkdiv, divisor);

		/* no interrupts */
		out_8(&i2c->intrmsk, 0);

		/* clear transfer count */
		out_8(&i2c->xfrcnt, 0);

		/* clear extended control & stat */
		/* write 1 in SRC SRS SWC SWS to clear these fields */
		out_8(&i2c->xtcntlss, 0xF0);

		/* Mode Control Register
		   Flush Slave/Master data buffer */
		out_8(&i2c->mdcntl, IIC_MDCNTL_FSDB | IIC_MDCNTL_FMDB);

		val = in_8(&i2c->mdcntl);

		/* Ignore General Call, slave transfers are ignored,
		 * disable interrupts, exit unknown bus state, enable hold
		 * SCL 100kHz normaly or FastMode for 400kHz and above
		 */

		val |= IIC_MDCNTL_EUBS | IIC_MDCNTL_HSCL;
		if (speed >= 400000)
			val |= IIC_MDCNTL_FSM;
		out_8(&i2c->mdcntl, val);

		/* clear control reg */
		out_8(&i2c->cntl, 0x00);
	}

	/* set to SPD bus as default bus upon powerup */
	I2C_SET_BUS(CONFIG_SYS_SPD_BUS_NUM);
}

/*
 * This code tries to use the features of the 405GP i2c
 * controller. It will transfer up to 4 bytes in one pass
 * on the loop. It only does out_8((u8 *)lbz) to the buffer when it
 * is possible to do out16(lhz) transfers.
 *
 * cmd_type is 0 for write 1 for read.
 *
 * addr_len can take any value from 0-255, it is only limited
 * by the char, we could make it larger if needed. If it is
 * 0 we skip the address write cycle.
 *
 * Typical case is a Write of an addr followd by a Read. The
 * IBM FAQ does not cover this. On the last byte of the write
 * we don't set the creg CHT bit, and on the first bytes of the
 * read we set the RPST bit.
 *
 * It does not support address only transfers, there must be
 * a data part. If you want to write the address yourself, put
 * it in the data pointer.
 *
 * It does not support transfer to/from address 0.
 *
 * It does not check XFRCNT.
 */
static int i2c_transfer(unsigned char cmd_type,
			unsigned char chip,
			unsigned char addr[],
			unsigned char addr_len,
			unsigned char data[],
			unsigned short data_len)
{
	struct ppc4xx_i2c *i2c = (struct ppc4xx_i2c *)I2C_BASE_ADDR;
	u8 *ptr;
	int reading;
	int tran, cnt;
	int result;
	int status;
	int i;
	u8 creg;

	if (data == 0 || data_len == 0) {
		/* Don't support data transfer of no length or to address 0 */
		printf( "i2c_transfer: bad call\n" );
		return IIC_NOK;
	}
	if (addr && addr_len) {
		ptr = addr;
		cnt = addr_len;
		reading = 0;
	} else {
		ptr = data;
		cnt = data_len;
		reading = cmd_type;
	}

	/* Clear Stop Complete Bit */
	out_8(&i2c->sts, IIC_STS_SCMP);

	/* Check init */
	i = 10;
	do {
		/* Get status */
		status = in_8(&i2c->sts);
		i--;
	} while ((status & IIC_STS_PT) && (i > 0));

	if (status & IIC_STS_PT) {
		result = IIC_NOK_TOUT;
		return(result);
	}

	/* flush the Master/Slave Databuffers */
	out_8(&i2c->mdcntl, in_8(&i2c->mdcntl) |
	      IIC_MDCNTL_FMDB | IIC_MDCNTL_FSDB);

	/* need to wait 4 OPB clocks? code below should take that long */

	/* 7-bit adressing */
	out_8(&i2c->hmadr, 0);
	out_8(&i2c->lmadr, chip);

	tran = 0;
	result = IIC_OK;
	creg = 0;

	while (tran != cnt && (result == IIC_OK)) {
		int  bc,j;

		/*
		 * Control register =
		 * Normal transfer, 7-bits adressing, Transfer up to
		 * bc bytes, Normal start, Transfer is a sequence of transfers
		 */
		creg |= IIC_CNTL_PT;

		bc = (cnt - tran) > 4 ? 4 : cnt - tran;
		creg |= (bc - 1) << 4;
		/* if the real cmd type is write continue trans */
		if ((!cmd_type && (ptr == addr)) || ((tran + bc) != cnt))
			creg |= IIC_CNTL_CHT;

		if (reading) {
			creg |= IIC_CNTL_READ;
		} else {
			for(j = 0; j < bc; j++) {
				/* Set buffer */
				out_8(&i2c->mdbuf, ptr[tran + j]);
			}
		}
		out_8(&i2c->cntl, creg);

		/*
		 * Transfer is in progress
		 * we have to wait for upto 5 bytes of data
		 * 1 byte chip address+r/w bit then bc bytes
		 * of data.
		 * udelay(10) is 1 bit time at 100khz
		 * Doubled for slop. 20 is too small.
		 */
		i = 2 * 5 * 8;
		do {
			/* Get status */
			status = in_8(&i2c->sts);
			udelay(10);
			i--;
		} while ((status & IIC_STS_PT) && !(status & IIC_STS_ERR) &&
			 (i > 0));

		if (status & IIC_STS_ERR) {
			result = IIC_NOK;
			status = in_8(&i2c->extsts);
			/* Lost arbitration? */
			if (status & IIC_EXTSTS_LA)
				result = IIC_NOK_LA;
			/* Incomplete transfer? */
			if (status & IIC_EXTSTS_ICT)
				result = IIC_NOK_ICT;
			/* Transfer aborted? */
			if (status & IIC_EXTSTS_XFRA)
				result = IIC_NOK_XFRA;
		} else if ( status & IIC_STS_PT) {
			result = IIC_NOK_TOUT;
		}

		/* Command is reading => get buffer */
		if ((reading) && (result == IIC_OK)) {
			/* Are there data in buffer */
			if (status & IIC_STS_MDBS) {
				/*
				 * even if we have data we have to wait 4OPB
				 * clocks for it to hit the front of the FIFO,
				 * after that we can just read. We should check
				 * XFCNT here and if the FIFO is full there is
				 * no need to wait.
				 */
				udelay(1);
				for (j = 0; j < bc; j++)
					ptr[tran + j] = in_8(&i2c->mdbuf);
			} else
				result = IIC_NOK_DATA;
		}
		creg = 0;
		tran += bc;
		if (ptr == addr && tran == cnt) {
			ptr = data;
			cnt = data_len;
			tran = 0;
			reading = cmd_type;
			if (reading)
				creg = IIC_CNTL_RPST;
		}
	}
	return result;
}

int i2c_probe(uchar chip)
{
	uchar buf[1];

	buf[0] = 0;

	/*
	 * What is needed is to send the chip address and verify that the
	 * address was <ACK>ed (i.e. there was a chip at that address which
	 * drove the data line low).
	 */
	return (i2c_transfer(1, chip << 1, 0, 0, buf, 1) != 0);
}

static int ppc4xx_i2c_transfer(uchar chip, uint addr, int alen, uchar *buffer,
			       int len, int read)
{
	uchar xaddr[4];
	int ret;

	if (alen > 4) {
		printf("I2C: addr len %d not supported\n", alen);
		return 1;
	}

	if (alen > 0) {
		xaddr[0] = (addr >> 24) & 0xFF;
		xaddr[1] = (addr >> 16) & 0xFF;
		xaddr[2] = (addr >> 8) & 0xFF;
		xaddr[3] = addr & 0xFF;
	}


#ifdef CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW
	/*
	 * EEPROM chips that implement "address overflow" are ones
	 * like Catalyst 24WC04/08/16 which has 9/10/11 bits of
	 * address and the extra bits end up in the "chip address"
	 * bit slots. This makes a 24WC08 (1Kbyte) chip look like
	 * four 256 byte chips.
	 *
	 * Note that we consider the length of the address field to
	 * still be one byte because the extra address bits are
	 * hidden in the chip address.
	 */
	if (alen > 0)
		chip |= ((addr >> (alen * 8)) &
			 CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW);
#endif
	if ((ret = i2c_transfer(read, chip << 1, &xaddr[4 - alen], alen,
				buffer, len)) != 0) {
		printf("I2C %s: failed %d\n", read ? "read" : "write", ret);

		return 1;
	}

	return 0;
}

int i2c_read(uchar chip, uint addr, int alen, uchar * buffer, int len)
{
	return ppc4xx_i2c_transfer(chip, addr, alen, buffer, len, 1);
}

int i2c_write(uchar chip, uint addr, int alen, uchar * buffer, int len)
{
	return ppc4xx_i2c_transfer(chip, addr, alen, buffer, len, 0);
}

#if defined(CONFIG_I2C_MULTI_BUS)
/*
 * Functions for multiple I2C bus handling
 */
unsigned int i2c_get_bus_num(void)
{
	return i2c_bus_num;
}

int i2c_set_bus_num(unsigned int bus)
{
	if (bus >= CONFIG_SYS_MAX_I2C_BUS)
		return -1;

	i2c_bus_num = bus;

	return 0;
}
#endif	/* CONFIG_I2C_MULTI_BUS */
#endif	/* CONFIG_HARD_I2C */
