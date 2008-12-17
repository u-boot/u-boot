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
 * Ported from i2c driver for ppc4xx by AS HARNOIS by
 * Travis B. Sawyer
 * Sandburst Corporation
 */
#include <common.h>
#include <ppc4xx.h>
#include <4xx_i2c.h>
#include <i2c.h>

#ifdef CONFIG_HARD_I2C

#define I2C_BUS1_BASE_ADDR (CONFIG_SYS_PERIPHERAL_BASE + 0x00000500)
#define	   I2C_REGISTERS_BUS1_BASE_ADDRESS I2C_BUS1_BASE_ADDR
#define    IIC_MDBUF1	(I2C_REGISTERS_BUS1_BASE_ADDRESS+IICMDBUF)
#define    IIC_SDBUF1	(I2C_REGISTERS_BUS1_BASE_ADDRESS+IICSDBUF)
#define    IIC_LMADR1	(I2C_REGISTERS_BUS1_BASE_ADDRESS+IICLMADR)
#define    IIC_HMADR1	(I2C_REGISTERS_BUS1_BASE_ADDRESS+IICHMADR)
#define    IIC_CNTL1	(I2C_REGISTERS_BUS1_BASE_ADDRESS+IICCNTL)
#define    IIC_MDCNTL1	(I2C_REGISTERS_BUS1_BASE_ADDRESS+IICMDCNTL)
#define    IIC_STS1	(I2C_REGISTERS_BUS1_BASE_ADDRESS+IICSTS)
#define    IIC_EXTSTS1	(I2C_REGISTERS_BUS1_BASE_ADDRESS+IICEXTSTS)
#define    IIC_LSADR1	(I2C_REGISTERS_BUS1_BASE_ADDRESS+IICLSADR)
#define    IIC_HSADR1	(I2C_REGISTERS_BUS1_BASE_ADDRESS+IICHSADR)
#define    IIC_CLKDIV1	(I2C_REGISTERS_BUS1_BASE_ADDRESS+IICCLKDIV)
#define    IIC_INTRMSK1	(I2C_REGISTERS_BUS1_BASE_ADDRESS+IICINTRMSK)
#define    IIC_XFRCNT1	(I2C_REGISTERS_BUS1_BASE_ADDRESS+IICXFRCNT)
#define    IIC_XTCNTLSS1	(I2C_REGISTERS_BUS1_BASE_ADDRESS+IICXTCNTLSS)
#define    IIC_DIRECTCNTL1 (I2C_REGISTERS_BUS1_BASE_ADDRESS+IICDIRECTCNTL)

void i2c1_init (int speed, int slaveadd);
int i2c_probe1 (uchar chip);
int i2c_read1 (uchar chip, uint addr, int alen, uchar * buffer, int len);
int i2c_write1 (uchar chip, uint addr, int alen, uchar * buffer, int len);
uchar i2c_reg_read1(uchar i2c_addr, uchar reg);
void i2c_reg_write1(uchar i2c_addr, uchar reg, uchar val);

#endif	/* CONFIG_HARD_I2C */
