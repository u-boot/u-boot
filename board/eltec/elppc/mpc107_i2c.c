/*
 * (C) Copyright 2002 ELTEC Elektronik AG
 * Frank Gottschling <fgottschling@eltec.de>
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

/* includes */
#include <common.h>
#include "srom.h"

/* locals */
static unsigned long mpc107_eumb_addr = 0;

/*----------------------------------------------------------------------------*/

/*
 * calculate checksum for ELTEC revision srom
 */
unsigned long el_srom_checksum (ptr, size)
register unsigned char *ptr;
unsigned long size;
{
    u_long f, accu = 0;
    u_int  i;
    u_char byte;

    for (; size; size--)
    {
	byte = *ptr++;
	for (i = 8; i; i--)
	{
	    f =  ((byte & 1) ^ (accu & 1)) ? 0x84083001 : 0;
	    accu >>= 1; accu ^= f;
	    byte >>= 1;
	}
    }
    return(accu);
}

/*----------------------------------------------------------------------------*/

static int mpc107_i2c_wait ( unsigned long timeout )
{
    unsigned long x;

    while (((x = in32r(MPC107_I2CSR)) & 0x82) != 0x82)
    {
	if (!timeout--)
	    return -1;
    }

    if (x & 0x10)
    {
	return -1;
    }
    out32r(MPC107_I2CSR, 0);

    return 0;
}

/*----------------------------------------------------------------------------*/

static int mpc107_i2c_wait_idle ( unsigned long timeout )
{
    while (in32r(MPC107_I2CSR) & 0x20)
    {
	if (!timeout--)
	    return -1;
    }
    return 0;
}


/*----------------------------------------------------------------------------*/

int mpc107_i2c_read_byte (
    unsigned char device,
    unsigned char block,
    unsigned char offset )
{
    unsigned long timeout = MPC107_I2C_TIMEOUT;
    int data;

    if (!mpc107_eumb_addr)
	return -6;

    mpc107_i2c_wait_idle (timeout);

    /* Start with MEN */
    out32r(MPC107_I2CCR, 0x80);

    /* Start as master */
    out32r(MPC107_I2CCR, 0xB0);
    out32r(MPC107_I2CDR, (0xA0 | device | block));

    if (mpc107_i2c_wait(timeout) < 0)
    {
	printf("mpc107_i2c_read Error 1\n");
	return -2;
    }

    if (in32r(MPC107_I2CSR)&0x1)
    {
	/* Generate STOP condition; device busy or not existing */
	out32r(MPC107_I2CCR, 0x80);
	return -1;
    }

    /* Data address */
    out32r(MPC107_I2CDR, offset);

    if (mpc107_i2c_wait(timeout) < 0)
    {
	printf("mpc107_i2c_read Error 2\n");
	return -3;
    }

    /* Switch to read - restart */
    out32r(MPC107_I2CCR, 0xB4);
    out32r(MPC107_I2CDR, (0xA1 | device | block));

    if (mpc107_i2c_wait(timeout) < 0)
    {
	printf("mpc107_i2c_read Error 3\n");
	return -4;
    }

    out32r(MPC107_I2CCR, 0xA8); /* no ACK */
    in32r(MPC107_I2CDR);

    if (mpc107_i2c_wait(timeout) < 0)
    {
	printf("mpc107_i2c_read Error 4\n");
	return -5;
    }
    /* Generate STOP condition */
    out32r(MPC107_I2CCR, 0x88);

    /* read */
    data = in32r(MPC107_I2CDR);

    return (data);
}

/*----------------------------------------------------------------------------*/

int mpc107_i2c_write_byte (
    unsigned char device,
    unsigned char block,
    unsigned char offset,
    unsigned char val )
{

    unsigned long timeout = MPC107_I2C_TIMEOUT;

    if (!mpc107_eumb_addr)
	return -6;

    mpc107_i2c_wait_idle(timeout);

    /* Start with MEN */
    out32r(MPC107_I2CCR, 0x80);

    /* Start as master */
    out32r(MPC107_I2CCR, 0xB0);
    out32r(MPC107_I2CDR, (0xA0 | device | block));

    if (mpc107_i2c_wait(timeout) < 0)
    {
	printf("mpc107_i2c_write Error 1\n");
	return -1;
    }

    /* Data address */
    out32r(MPC107_I2CDR, offset);

    if (mpc107_i2c_wait(timeout) < 0)
    {
	printf("mpc107_i2c_write Error 2\n");
	return -1;
    }

    /* Write */
    out32r(MPC107_I2CDR, val);
    if (mpc107_i2c_wait(timeout) < 0)
    {
	printf("mpc107_i2c_write Error 3\n");
	return -1;
    }

    /* Generate Stop Condition */
    out32r(MPC107_I2CCR, 0x80);

    /* Return ACK or no ACK */
    return (in32r(MPC107_I2CSR) & 0x01);
}

/*----------------------------------------------------------------------------*/

int mpc107_srom_load (
    unsigned char addr,
    unsigned char *pBuf,
    int          cnt,
    unsigned char device,
    unsigned char block )
{
    register int i;
    int val;
    int timeout;

    for (i = 0; i < cnt; i++)
    {
	timeout=100;
	do
	{
	    val = mpc107_i2c_read_byte (device, block, addr);
	    if (val < -1)
	    {
	    printf("i2c_read_error %d at dev %x block %x addr %x\n",
		   val, device, block, addr);
	    return -1;
	    }
	    else if (timeout==0)
	    {
		printf ("i2c_read_error: timeout at dev %x block %x addr %x\n",
			device, block, addr);
		return -1;
	    }
	    timeout--;
	} while (val == -1); /* if no ack: try again! */

	*pBuf++ = (unsigned char)val;
	addr++;

	if ((addr == 0) && (i != cnt-1))    /* is it the same block ? */
	{
	    if (block == FIRST_BLOCK)
		block = SECOND_BLOCK;
	    else
	    {
		printf ("ic2_read_error: read beyond 2. block !\n");
		return -1;
	    }
	}
    }
    udelay(100000);
    return (cnt);
}

/*----------------------------------------------------------------------------*/

int mpc107_srom_store (
    unsigned char addr,
    unsigned char *pBuf,
    int          cnt,
    unsigned char device,
    unsigned char block )
{
    register int i;

    for (i = 0; i < cnt; i++)
    {
	while (mpc107_i2c_write_byte (device,block,addr,*pBuf) == 1);
	addr++;
	pBuf++;

	if ((addr == 0) && (i != cnt-1))     /* is it the same block ? */
	{
	    if (block == FIRST_BLOCK)
		block = SECOND_BLOCK;
	    else
	    {
		printf ("ic2_write_error: write beyond 2. block !\n");
		return -1;
	    }
	}
    }
    udelay(100000);
    return(cnt);
}

/*----------------------------------------------------------------------------*/

int mpc107_i2c_init ( unsigned long eumb_addr, unsigned long divider )
{
    unsigned long x;

    if (eumb_addr)
	mpc107_eumb_addr = eumb_addr;
    else
	return -1;

    /* Set I2C clock */
    x = in32r(MPC107_I2CFDR) & 0xffffff00;
    out32r(MPC107_I2CFDR, (x | divider));

    /* Clear arbitration */
    out32r(MPC107_I2CSR, 0);

    return mpc107_eumb_addr;
}

/*----------------------------------------------------------------------------*/
