/*
 *  Copyright (C) 2005 Sandburst Corporation
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Ported from cpu/ppc4xx/i2c.c by AS HARNOIS by
 * Travis B. Sawyer
 * Sandburst Corporation.
 */
#include <common.h>
#include <ppc4xx.h>
#include <4xx_i2c.h>
#include <i2c.h>
#include <command.h>
#include "ppc440gx_i2c.h"
#include <asm-ppc/io.h>

#ifdef CONFIG_I2C_BUS1

#define IIC_OK		0
#define IIC_NOK		1
#define IIC_NOK_LA	2		/* Lost arbitration */
#define IIC_NOK_ICT	3		/* Incomplete transfer */
#define IIC_NOK_XFRA	4		/* Transfer aborted */
#define IIC_NOK_DATA	5		/* No data in buffer */
#define IIC_NOK_TOUT	6		/* Transfer timeout */

#define IIC_TIMEOUT 1			/* 1 second */
#if defined(CONFIG_SYS_I2C_NOPROBES)
static uchar i2c_no_probes[] = CONFIG_SYS_I2C_NOPROBES;
#endif

static struct ppc4xx_i2c *i2c = (struct ppc4xx_i2c *)I2C_REGISTERS_BUS1_BASE_ADDRESS;

static void _i2c_bus1_reset (void)
{
	int i, status;

	/* Reset status register */
	/* write 1 in SCMP and IRQA to clear these fields */
	out_8 (IIC_STS1, 0x0A);

	/* write 1 in IRQP IRQD LA ICT XFRA to clear these fields */
	out_8 (IIC_EXTSTS1, 0x8F);
	__asm__ volatile ("eieio");

	/*
	 * Get current state, reset bus
	 * only if no transfers are pending.
	 */
	i = 10;
	do {
		/* Get status */
		status = in_8 (IIC_STS1);
		udelay (500);			/* 500us */
		i--;
	} while ((status & IIC_STS_PT) && (i > 0));
	/* Soft reset controller */
	status = in_8 (IIC_XTCNTLSS1);
	out_8 (IIC_XTCNTLSS1, (status | IIC_XTCNTLSS_SRST));
	__asm__ volatile ("eieio");

	/* make sure where in initial state, data hi, clock hi */
	out_8 (IIC_DIRECTCNTL1, 0xC);
	for (i = 0; i < 10; i++) {
		if ((in_8 (IIC_DIRECTCNTL1) & 0x3) != 0x3) {
			/* clock until we get to known state */
			out_8 (IIC_DIRECTCNTL1, 0x8);	/* clock lo */
			udelay (100);		/* 100us */
			out_8 (IIC_DIRECTCNTL1, 0xC);	/* clock hi */
			udelay (100);		/* 100us */
		} else {
			break;
		}
	}
	/* send start condition */
	out_8 (IIC_DIRECTCNTL1, 0x4);
	udelay (1000);				/* 1ms */
	/* send stop condition */
	out_8 (IIC_DIRECTCNTL1, 0xC);
	udelay (1000);				/* 1ms */
	/* Unreset controller */
	out_8 (IIC_XTCNTLSS1, (status & ~IIC_XTCNTLSS_SRST));
	udelay (1000);				/* 1ms */
}

void i2c1_init (int speed, int slaveadd)
{
	sys_info_t sysInfo;
	unsigned long freqOPB;
	int val, divisor;

#ifdef CONFIG_SYS_I2C_INIT_BOARD
	/* call board specific i2c bus reset routine before accessing the   */
	/* environment, which might be in a chip on that bus. For details   */
	/* about this problem see doc/I2C_Edge_Conditions.                  */
	i2c_init_board();
#endif

	/* Handle possible failed I2C state */
	/* FIXME: put this into i2c_init_board()? */
	_i2c_bus1_reset ();

	/* clear lo master address */
	out_8 (IIC_LMADR1, 0);

	/* clear hi master address */
	out_8 (IIC_HMADR1, 0);

	/* clear lo slave address */
	out_8 (IIC_LSADR1, 0);

	/* clear hi slave address */
	out_8 (IIC_HSADR1, 0);

	/* Clock divide Register */
	/* get OPB frequency */
	get_sys_info (&sysInfo);
	freqOPB = sysInfo.freqPLB / sysInfo.pllOpbDiv;
	/* set divisor according to freqOPB */
	divisor = (freqOPB - 1) / 10000000;
	if (divisor == 0)
		divisor = 1;
	out_8 (IIC_CLKDIV1, divisor);

	/* no interrupts */
	out_8 (IIC_INTRMSK1, 0);

	/* clear transfer count */
	out_8 (IIC_XFRCNT1, 0);

	/* clear extended control & stat */
	/* write 1 in SRC SRS SWC SWS to clear these fields */
	out_8 (IIC_XTCNTLSS1, 0xF0);

	/* Mode Control Register
	   Flush Slave/Master data buffer */
	out_8 (IIC_MDCNTL1, IIC_MDCNTL_FSDB | IIC_MDCNTL_FMDB);
	__asm__ volatile ("eieio");


	val = in_8(IIC_MDCNTL1);
	__asm__ volatile ("eieio");

	/* Ignore General Call, slave transfers are ignored,
	   disable interrupts, exit unknown bus state, enable hold
	   SCL
	   100kHz normaly or FastMode for 400kHz and above
	*/

	val |= IIC_MDCNTL_EUBS|IIC_MDCNTL_HSCL;
	if( speed >= 400000 ){
		val |= IIC_MDCNTL_FSM;
	}
	out_8 (IIC_MDCNTL1, val);

	/* clear control reg */
	out_8 (IIC_CNTL1, 0x00);
	__asm__ volatile ("eieio");

}

/*
  This code tries to use the features of the 405GP i2c
  controller. It will transfer up to 4 bytes in one pass
  on the loop. It only does out_8(lbz) to the buffer when it
  is possible to do out16(lhz) transfers.

  cmd_type is 0 for write 1 for read.

  addr_len can take any value from 0-255, it is only limited
  by the char, we could make it larger if needed. If it is
  0 we skip the address write cycle.

  Typical case is a Write of an addr followd by a Read. The
  IBM FAQ does not cover this. On the last byte of the write
  we don't set the creg CHT bit, and on the first bytes of the
  read we set the RPST bit.

  It does not support address only transfers, there must be
  a data part. If you want to write the address yourself, put
  it in the data pointer.

  It does not support transfer to/from address 0.

  It does not check XFRCNT.
*/
static
int i2c_transfer1(unsigned char cmd_type,
		  unsigned char chip,
		  unsigned char addr[],
		  unsigned char addr_len,
		  unsigned char data[],
		  unsigned short data_len )
{
	unsigned char* ptr;
	int reading;
	int tran,cnt;
	int result;
	int status;
	int i;
	uchar creg;

	if( data == 0 || data_len == 0 ){
		/*Don't support data transfer of no length or to address 0*/
		printf( "i2c_transfer: bad call\n" );
		return IIC_NOK;
	}
	if( addr && addr_len ){
		ptr = addr;
		cnt = addr_len;
		reading = 0;
	}else{
		ptr = data;
		cnt = data_len;
		reading = cmd_type;
	}

	/*Clear Stop Complete Bit*/
	out_8(IIC_STS1,IIC_STS_SCMP);
	/* Check init */
	i=10;
	do {
		/* Get status */
		status = in_8(IIC_STS1);
		__asm__ volatile("eieio");
		i--;
	} while ((status & IIC_STS_PT) && (i>0));

	if (status & IIC_STS_PT) {
		result = IIC_NOK_TOUT;
		return(result);
	}
	/*flush the Master/Slave Databuffers*/
	out_8(IIC_MDCNTL1, ((in_8(IIC_MDCNTL1))|IIC_MDCNTL_FMDB|IIC_MDCNTL_FSDB));
	/*need to wait 4 OPB clocks? code below should take that long*/

	/* 7-bit adressing */
	out_8(IIC_HMADR1,0);
	out_8(IIC_LMADR1, chip);
	__asm__ volatile("eieio");

	tran = 0;
	result = IIC_OK;
	creg = 0;

	while ( tran != cnt && (result == IIC_OK)) {
		int  bc,j;

		/* Control register =
		   Normal transfer, 7-bits adressing, Transfer up to bc bytes, Normal start,
		   Transfer is a sequence of transfers
		*/
		creg |= IIC_CNTL_PT;

		bc = (cnt - tran) > 4 ? 4 :
			cnt - tran;
		creg |= (bc-1)<<4;
		/* if the real cmd type is write continue trans*/
		if ( (!cmd_type && (ptr == addr)) || ((tran+bc) != cnt) )
			creg |= IIC_CNTL_CHT;

		if (reading)
			creg |= IIC_CNTL_READ;
		else {
			for(j=0; j<bc; j++) {
				/* Set buffer */
				out_8(IIC_MDBUF1,ptr[tran+j]);
				__asm__ volatile("eieio");
			}
		}
		out_8(IIC_CNTL1, creg );
		__asm__ volatile("eieio");

		/* Transfer is in progress
		   we have to wait for upto 5 bytes of data
		   1 byte chip address+r/w bit then bc bytes
		   of data.
		   udelay(10) is 1 bit time at 100khz
		   Doubled for slop. 20 is too small.
		*/
		i=2*5*8;
		do {
			/* Get status */
			status = in_8(IIC_STS1);
			__asm__ volatile("eieio");
			udelay (10);
			i--;
		} while ((status & IIC_STS_PT) && !(status & IIC_STS_ERR)
			 && (i>0));

		if (status & IIC_STS_ERR) {
			result = IIC_NOK;
			status = in_8 (IIC_EXTSTS1);
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
				  even if we have data we have to wait 4OPB clocks
				  for it to hit the front of the FIFO, after that
				  we can just read. We should check XFCNT here and
				  if the FIFO is full there is no need to wait.
				*/
				udelay (1);
				for(j=0;j<bc;j++) {
					ptr[tran+j] = in_8(IIC_MDBUF1);
					__asm__ volatile("eieio");
				}
			} else
				result = IIC_NOK_DATA;
		}
		creg = 0;
		tran+=bc;
		if( ptr == addr && tran == cnt ) {
			ptr = data;
			cnt = data_len;
			tran = 0;
			reading = cmd_type;
			if( reading )
				creg = IIC_CNTL_RPST;
		}
	}
	return (result);
}

int i2c_probe1 (uchar chip)
{
	uchar buf[1];

	buf[0] = 0;

	/*
	 * What is needed is to send the chip address and verify that the
	 * address was <ACK>ed (i.e. there was a chip at that address which
	 * drove the data line low).
	 */
	return(i2c_transfer1 (1, chip << 1, 0,0, buf, 1) != 0);
}


int i2c_read1 (uchar chip, uint addr, int alen, uchar * buffer, int len)
{
	uchar xaddr[4];
	int ret;

	if ( alen > 4 ) {
		printf ("I2C read: addr len %d not supported\n", alen);
		return 1;
	}

	if ( alen > 0 ) {
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
	if( alen > 0 )
		chip |= ((addr >> (alen * 8)) & CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW);
#endif
	if( (ret = i2c_transfer1( 1, chip<<1, &xaddr[4-alen], alen, buffer, len )) != 0) {
		printf( "I2c read: failed %d\n", ret);
		return 1;
	}
	return 0;
}

int i2c_write1 (uchar chip, uint addr, int alen, uchar * buffer, int len)
{
	uchar xaddr[4];

	if ( alen > 4 ) {
		printf ("I2C write: addr len %d not supported\n", alen);
		return 1;

	}
	if ( alen > 0 ) {
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
	if( alen > 0 )
		chip |= ((addr >> (alen * 8)) & CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW);
#endif

	return (i2c_transfer1( 0, chip<<1, &xaddr[4-alen], alen, buffer, len ) != 0);
}

/*-----------------------------------------------------------------------
 * Read a register
 */
uchar i2c_reg_read1(uchar i2c_addr, uchar reg)
{
	uchar buf;

	i2c_read1(i2c_addr, reg, 1, &buf, (uchar)1);

	return(buf);
}

/*-----------------------------------------------------------------------
 * Write a register
 */
void i2c_reg_write1(uchar i2c_addr, uchar reg, uchar val)
{
	i2c_write1(i2c_addr, reg, 1, &val, 1);
}


int do_i2c1_probe(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int j;
#if defined(CONFIG_SYS_I2C_NOPROBES)
	int k, skip;
#endif

	puts ("Valid chip addresses:");
	for(j = 0; j < 128; j++) {
#if defined(CONFIG_SYS_I2C_NOPROBES)
		skip = 0;
		for (k = 0; k < sizeof(i2c_no_probes); k++){
			if (j == i2c_no_probes[k]){
				skip = 1;
				break;
			}
		}
		if (skip)
			continue;
#endif
		if(i2c_probe1(j) == 0) {
			printf(" %02X", j);
		}
	}
	putc ('\n');

#if defined(CONFIG_SYS_I2C_NOPROBES)
	puts ("Excluded chip addresses:");
	for( k = 0; k < sizeof(i2c_no_probes); k++ )
		printf(" %02X", i2c_no_probes[k] );
	putc ('\n');
#endif

	return 0;
}

U_BOOT_CMD(
	iprobe1,	1,	1,	do_i2c1_probe,
	"probe to discover valid I2C chip addresses",
	""
);

#endif	/* CONFIG_I2C_BUS1 */
