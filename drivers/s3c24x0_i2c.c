/*
 * (C) Copyright 2002
 * David Mueller, ELSOFT AG, d.mueller@elsoft.ch
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

/* This code should work for both the S3C2400 and the S3C2410
 * as they seem to have the same I2C controller inside.
 * The different address mapping is handled by the s3c24xx.h files below.
 */

#include <common.h>

#ifdef CONFIG_DRIVER_S3C24X0_I2C

#if defined(CONFIG_S3C2400)
#include <s3c2400.h>
#elif defined(CONFIG_S3C2410)
#include <s3c2410.h>
#endif
#include <i2c.h>

#ifdef CONFIG_HARD_I2C

#define	IIC_WRITE	0
#define IIC_READ	1

#define IIC_OK		0
#define IIC_NOK		1
#define IIC_NACK	2
#define IIC_NOK_LA	3		/* Lost arbitration */
#define IIC_NOK_TOUT	4		/* time out */

#define IICSTAT_BSY	0x20		/* Busy bit */
#define IICSTAT_NACK	0x01		/* Nack bit */
#define IICCON_IRPND	0x10		/* Interrupt pending bit */
#define IIC_MODE_MT	0xC0		/* Master Transmit Mode */
#define IIC_MODE_MR	0x80		/* Master Receive Mode */
#define IIC_START_STOP	0x20		/* START / STOP */
#define IIC_TXRX_ENA	0x10		/* I2C Tx/Rx enable */

#define IIC_TIMEOUT 1			/* 1 seconde */


static int GetIICSDA(void)
{
	return (rGPEDAT & 0x8000) >> 15;
}

#if 0
static void SetIICSDA(int x)
{
	rGPEDAT = (rGPEDAT & ~0x8000) | (x&1) << 15;
}
#endif

static void SetIICSCL(int x)
{
	rGPEDAT = (rGPEDAT & ~0x4000) | (x&1) << 14;
}


static int WaitForXfer(void)
{
    int i, status;

    i = IIC_TIMEOUT * 1000;
    status = rIICCON;
    while ((i > 0) && !(status & IICCON_IRPND)) {
    	udelay(1000);
	status = rIICCON;
	i--;
    }

    return(status & IICCON_IRPND) ? IIC_OK : IIC_NOK_TOUT;
}

static int IsACK(void)
{
    return(!(rIICSTAT & IICSTAT_NACK));
}

static void ReadWriteByte(void)
{
    rIICCON &= ~IICCON_IRPND;
}

void i2c_init (int speed, int slaveadd)
{
    ulong freq, pres = 16, div;
    int i, status;

    /* wait for some time to give previous transfer a chance to finish */

    i = IIC_TIMEOUT * 1000;
    status = rIICSTAT;
    while ((i > 0) && (status & IICSTAT_BSY)) {
	udelay(1000);
	status = rIICSTAT;
	i--;
    }

    if ((status & IICSTAT_BSY) || GetIICSDA() == 0) {
	ulong old_gpecon = rGPECON;
	/* bus still busy probably by (most) previously interrupted transfer */

	/* set IICSDA and IICSCL (GPE15, GPE14) to GPIO */
	rGPECON = (rGPECON & ~0xF0000000) | 0x10000000;

	/* toggle IICSCL until bus idle */
	SetIICSCL(0); udelay(1000);
	i = 10;
	while ((i > 0) && (GetIICSDA() != 1)) {
		SetIICSCL(1); udelay(1000);
		SetIICSCL(0); udelay(1000);
		i--;
	}
	SetIICSCL(1); udelay(1000);

	/* restore pin functions */
	rGPECON = old_gpecon;
    }

    /* calculate prescaler and divisor values */
    freq = get_PCLK();
    if ((freq / pres / (16+1)) > speed)
	/* set prescaler to 512 */
	pres = 512;

    div = 0;
    while ((freq / pres / (div+1)) > speed)
	div++;

    /* set prescaler, divisor according to freq, also set
       ACKGEN, IRQ */
    rIICCON = (div & 0x0F) | 0xA0 | ((pres == 512) ? 0x40 : 0);

    /* init to SLAVE REVEIVE and set slaveaddr */
    rIICSTAT = 0;
    rIICADD = slaveadd;
    /* program Master Transmit (and implicit STOP) */
    rIICSTAT = IIC_MODE_MT | IIC_TXRX_ENA;

}

/*
  cmd_type is 0 for write 1 for read.

  addr_len can take any value from 0-255, it is only limited
  by the char, we could make it larger if needed. If it is
  0 we skip the address write cycle.

*/
static
int i2c_transfer(unsigned char cmd_type,
                 unsigned char chip,
                 unsigned char addr[],
                 unsigned char addr_len,
                 unsigned char data[],
		 unsigned short data_len)
{
    int i, status, result;

    if (data == 0 || data_len == 0) {
	/*Don't support data transfer of no length or to address 0*/
	printf( "i2c_transfer: bad call\n" );
	return IIC_NOK;
    }

    //CheckDelay();

    /* Check I2C bus idle */
    i = IIC_TIMEOUT * 1000;
    status = rIICSTAT;
    while ((i > 0) && (status & IICSTAT_BSY)) {
	udelay(1000);
	status = rIICSTAT;
	i--;
    }


    if (status & IICSTAT_BSY) {
	result = IIC_NOK_TOUT;
        return(result);
    }

    rIICCON |= 0x80;

    result = IIC_OK;

    switch (cmd_type) {
	case IIC_WRITE:
	    if (addr && addr_len) {
		rIICDS = chip;
		/* send START */
		rIICSTAT = IIC_MODE_MT | IIC_TXRX_ENA | IIC_START_STOP;
		i = 0;
		while ((i < addr_len) && (result == IIC_OK)) {
		    result = WaitForXfer();
		    rIICDS = addr[i];
		    ReadWriteByte();
		    i++;
		}
		i = 0;
		while ((i < data_len) && (result == IIC_OK)) {
		    result = WaitForXfer();
		    rIICDS = data[i];
		    ReadWriteByte();
		    i++;
		}
	    } else {
		rIICDS = chip;
		/* send START */
		rIICSTAT = IIC_MODE_MT | IIC_TXRX_ENA | IIC_START_STOP;
		i = 0;
		while ((i < data_len) && (result = IIC_OK)) {
		    result = WaitForXfer();
		    rIICDS = data[i];
		    ReadWriteByte();
		    i++;
		}
	    }

	    if (result == IIC_OK)
	        result = WaitForXfer();

	    /* send STOP */
	    rIICSTAT = IIC_MODE_MR | IIC_TXRX_ENA;
	    ReadWriteByte();
	    break;

	case IIC_READ:
	    if (addr && addr_len) {
		rIICSTAT = IIC_MODE_MT | IIC_TXRX_ENA;
		rIICDS = chip;
		/* send START */
		rIICSTAT |= IIC_START_STOP;
		result = WaitForXfer();
		if (IsACK()) {
		    i = 0;
		    while ((i < addr_len) && (result == IIC_OK)) {
			rIICDS = addr[i];
			ReadWriteByte();
			result = WaitForXfer();
			i++;
		    }

		    rIICDS = chip;
		    /* resend START */
		    rIICSTAT = IIC_MODE_MR | IIC_TXRX_ENA | IIC_START_STOP;
		    ReadWriteByte();
		    result = WaitForXfer();
		    i = 0;
		    while ((i < data_len) && (result == IIC_OK)) {
			/* disable ACK for final READ */
			if (i == data_len - 1)
			    rIICCON &= ~0x80;
			ReadWriteByte();
			result = WaitForXfer();
			data[i] = rIICDS;
			i++;
		    }
		} else {
		    result = IIC_NACK;
		}

	    } else {
		rIICSTAT = IIC_MODE_MR | IIC_TXRX_ENA;
		rIICDS = chip;
		/* send START */
		rIICSTAT |= IIC_START_STOP;
		result = WaitForXfer();

		if (IsACK()) {
		    i = 0;
		    while ((i < data_len) && (result == IIC_OK)) {
			/* disable ACK for final READ */
			if (i == data_len - 1)
			    rIICCON &= ~0x80;
		        ReadWriteByte();
			result = WaitForXfer();
			data[i] = rIICDS;
			i++;
		    }
		} else {
		    result = IIC_NACK;
		}
	    }

	    /* send STOP */
	    rIICSTAT = IIC_MODE_MR | IIC_TXRX_ENA;
	    ReadWriteByte();
	    break;

	default:
	    printf( "i2c_transfer: bad call\n" );
    	    result = IIC_NOK;
	    break;
    }

    return (result);
}

int i2c_probe (uchar chip)
{
    uchar buf[1];

    buf[0] = 0;

    /*
     * What is needed is to send the chip address and verify that the
     * address was <ACK>ed (i.e. there was a chip at that address which
     * drove the data line low).
     */
    return(i2c_transfer (IIC_READ, chip << 1, 0, 0, buf, 1) != IIC_OK);
}

int i2c_read (uchar chip, uint addr, int alen, uchar * buffer, int len)
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


#ifdef CFG_I2C_EEPROM_ADDR_OVERFLOW
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
	chip |= ((addr >> (alen * 8)) & CFG_I2C_EEPROM_ADDR_OVERFLOW);
#endif
    if( (ret = i2c_transfer(IIC_READ, chip<<1, &xaddr[4-alen], alen, buffer, len )) != 0) {
        printf( "I2c read: failed %d\n", ret);
        return 1;
    }
    return 0;
}

int i2c_write (uchar chip, uint addr, int alen, uchar * buffer, int len)
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

#ifdef CFG_I2C_EEPROM_ADDR_OVERFLOW
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
        chip |= ((addr >> (alen * 8)) & CFG_I2C_EEPROM_ADDR_OVERFLOW);
#endif
    return (i2c_transfer(IIC_WRITE, chip<<1, &xaddr[4-alen], alen, buffer, len ) != 0);
}

#endif	/* CONFIG_HARD_I2C */

#endif /* CONFIG_DRIVER_S3C24X0_I2C */
