/*
 * (C) Copyright 2004
 * DAVE Srl
 * http://www.dave-tech.it
 * http://www.wawnet.biz
 * mailto:info@wawnet.biz
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
 */

#include <common.h>
#include <command.h>
#include <asm/hardware.h>

/*
 * Initialization, must be called once on start up, may be called
 * repeatedly to change the speed and slave addresses.
 */
void i2c_init(int speed, int slaveaddr)
{
	/*
		setting up I2C support
	*/
	unsigned int save_F,save_PF,rIICCON,rPCONA,rPDATA,rPCONF,rPUPF;

	save_F = PCONF;
	save_PF = PUPF;

	rPCONF = ((save_F & ~(0xF))| 0xa);
	rPUPF = (save_PF | 0x3);
	PCONF = rPCONF; /*PF0:IICSCL, PF1:IICSDA*/
	PUPF = rPUPF; /* Disable pull-up */

	/* Configuring pin for WC pin of EEprom */
	rPCONA = PCONA;
	rPCONA &= ~(1<<9);
	PCONA = rPCONA;

	rPDATA = PDATA;
	rPDATA &= ~(1<<9);
	PDATA = rPDATA;

	/*
		Enable ACK, IICCLK=MCLK/16, enable interrupt
		75MHz/16/(12+1) = 390625 Hz
	*/
	rIICCON=(1<<7)|(0<<6)|(1<<5)|(0xC);
	IICCON = rIICCON;

	IICADD = slaveaddr;
}

/*
 * Probe the given I2C chip address.  Returns 0 if a chip responded,
 * not 0 on failure.
 */
int i2c_probe(uchar chip)
{
	/*
		not implemented
	*/

	printf("i2c_probe chip %d\n", (int) chip);
	return -1;
}

/*
 * Read/Write interface:
 *   chip:    I2C chip address, range 0..127
 *   addr:    Memory (register) address within the chip
 *   alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   buffer:  Where to read/write the data
 *   len:     How many bytes to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */

#define S3C44B0X_rIIC_INTPEND               (1<<4)
#define S3C44B0X_rIIC_LAST_RECEIV_BIT       (1<<0)
#define S3C44B0X_rIIC_INTERRUPT_ENABLE      (1<<5)
#define S3C44B0_IIC_TIMEOUT 100

int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{

	int k, j, temp;
	u32 rIICSTAT;

	/*
		send the device offset
	*/

	rIICSTAT = 0xD0;
	IICSTAT = rIICSTAT;

	IICDS = chip;	/* this is a write operation... */

	rIICSTAT |= (1<<5);
	IICSTAT = rIICSTAT;

	for(k=0; k<S3C44B0_IIC_TIMEOUT; k++) {
		temp = IICCON;
		if( (temp & S3C44B0X_rIIC_INTPEND) == S3C44B0X_rIIC_INTPEND)
		break;
		udelay(2000);
	}
	if (k==S3C44B0_IIC_TIMEOUT)
		return -1;

	/* wait and check ACK */
	temp = IICSTAT;
	if ((temp & S3C44B0X_rIIC_LAST_RECEIV_BIT) == S3C44B0X_rIIC_LAST_RECEIV_BIT )
		return -1;

	IICDS = addr;
	IICCON = IICCON & ~(S3C44B0X_rIIC_INTPEND);

	/* wait and check ACK */
	for(k=0; k<S3C44B0_IIC_TIMEOUT; k++) {
		temp = IICCON;
		if( (temp & S3C44B0X_rIIC_INTPEND) == S3C44B0X_rIIC_INTPEND)
		break;
		udelay(2000);
	}
	if (k==S3C44B0_IIC_TIMEOUT)
		return -1;

	temp = IICSTAT;
	if ((temp & S3C44B0X_rIIC_LAST_RECEIV_BIT) == S3C44B0X_rIIC_LAST_RECEIV_BIT )
		return -1;

	/*
		now we can start with the read operation...
	*/

	IICDS = chip | 0x01;	/* this is a read operation... */

	rIICSTAT = 0x90; /*master recv*/
	rIICSTAT |= (1<<5);
	IICSTAT = rIICSTAT;

	IICCON = IICCON & ~(S3C44B0X_rIIC_INTPEND);

	/* wait and check ACK */
	for(k=0; k<S3C44B0_IIC_TIMEOUT; k++) {
		temp = IICCON;
		if( (temp & S3C44B0X_rIIC_INTPEND) == S3C44B0X_rIIC_INTPEND)
		break;
		udelay(2000);
	}
	if (k==S3C44B0_IIC_TIMEOUT)
		return -1;

	temp = IICSTAT;
	if ((temp & S3C44B0X_rIIC_LAST_RECEIV_BIT) == S3C44B0X_rIIC_LAST_RECEIV_BIT )
		return -1;

	for (j=0; j<len-1; j++) {

	/*clear pending bit to resume */

	temp = IICCON & ~(S3C44B0X_rIIC_INTPEND);
	IICCON = temp;

	/* wait and check ACK */
	for(k=0; k<S3C44B0_IIC_TIMEOUT; k++) {
		temp = IICCON;
		if( (temp & S3C44B0X_rIIC_INTPEND) == S3C44B0X_rIIC_INTPEND)
		break;
		udelay(2000);
	}
	if (k==S3C44B0_IIC_TIMEOUT)
		return -1;


		buffer[j] = IICDS; /*save readed data*/

    } /*end for(j)*/

	/*
		reading the last data
		unset ACK generation
	*/
	temp = IICCON & ~(S3C44B0X_rIIC_INTPEND | (1<<7));
	IICCON = temp;

	/* wait but NOT check ACK */
	for(k=0; k<S3C44B0_IIC_TIMEOUT; k++) {
		temp = IICCON;
		if( (temp & S3C44B0X_rIIC_INTPEND) == S3C44B0X_rIIC_INTPEND)
		break;
		udelay(2000);
	}
	if (k==S3C44B0_IIC_TIMEOUT)
		return -1;

	buffer[j] = IICDS; /*save readed data*/

	rIICSTAT = 0x90; /*master recv*/

	/* Write operation Terminate sending STOP */
	IICSTAT = rIICSTAT;
	/*Clear Int Pending Bit to RESUME*/
	temp = IICCON;
	IICCON = temp & (~S3C44B0X_rIIC_INTPEND);

	IICCON = IICCON | (1<<7);	/*restore ACK generation*/

	return 0;
}

int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	int j, k;
	u32 rIICSTAT, temp;


	/*
		send the device offset
	*/

	rIICSTAT = 0xD0;
	IICSTAT = rIICSTAT;

	IICDS = chip;	/* this is a write operation... */

	rIICSTAT |= (1<<5);
	IICSTAT = rIICSTAT;

	IICCON = IICCON & ~(S3C44B0X_rIIC_INTPEND);

	/* wait and check ACK */
	for(k=0; k<S3C44B0_IIC_TIMEOUT; k++) {
		temp = IICCON;
		if( (temp & S3C44B0X_rIIC_INTPEND) == S3C44B0X_rIIC_INTPEND)
		break;
		udelay(2000);
	}
	if (k==S3C44B0_IIC_TIMEOUT)
		return -1;

	temp = IICSTAT;
	if ((temp & S3C44B0X_rIIC_LAST_RECEIV_BIT) == S3C44B0X_rIIC_LAST_RECEIV_BIT )
		return -1;

	IICDS = addr;
	IICCON = IICCON & ~(S3C44B0X_rIIC_INTPEND);

	/* wait and check ACK */
	for(k=0; k<S3C44B0_IIC_TIMEOUT; k++) {
		temp = IICCON;
		if( (temp & S3C44B0X_rIIC_INTPEND) == S3C44B0X_rIIC_INTPEND)
		break;
		udelay(2000);
	}
	if (k==S3C44B0_IIC_TIMEOUT)
	  return -1;

	temp = IICSTAT;
	if ((temp & S3C44B0X_rIIC_LAST_RECEIV_BIT) == S3C44B0X_rIIC_LAST_RECEIV_BIT )
		return -1;

	/*
		now we can start with the read write operation
	*/
	for (j=0; j<len; j++) {

		IICDS = buffer[j]; /*prerare data to write*/

		/*clear pending bit to resume*/

		temp = IICCON & ~(S3C44B0X_rIIC_INTPEND);
		IICCON = temp;

		/* wait but NOT check ACK */
		for(k=0; k<S3C44B0_IIC_TIMEOUT; k++) {
			temp = IICCON;
			if( (temp & S3C44B0X_rIIC_INTPEND) == S3C44B0X_rIIC_INTPEND)
			break;

			udelay(2000);
		}

		if (k==S3C44B0_IIC_TIMEOUT)
			return -1;

	} /* end for(j) */

	/* sending stop to terminate */
	rIICSTAT = 0xD0;  /*master send*/
	IICSTAT = rIICSTAT;
	/*Clear Int Pending Bit to RESUME*/
	temp = IICCON;
	IICCON = temp & (~S3C44B0X_rIIC_INTPEND);

	return 0;
}
