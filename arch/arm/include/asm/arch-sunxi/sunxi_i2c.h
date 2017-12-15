/* 
 * Support for GPIO-I2C on SUN8I V3S
 *
 * Copyright(c) 2017 I4VINE Inc.,
 * All right reserved by JuYoung Ryu<jyryu@stcube.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

		
#ifndef _SUNXI_I2C_H
#define _SUNXI_I2C_H

/* Register Map */
#define TWI_ADDR	0x0000
		#define	SLA				GENMASK(7,1)
		#define GCE				BIT(0)
#define TWI_XADDR	0x0004
		#define	SLAX			GENMASK(7,0)
#define TWI_DATA	0x0008
		#define	TWI_DATA_MASK	GENMASK(7,0)
#define TWI_CNTR	0x000C
		#define INT_EN			BIT(7)
		#define BUS_EN			BIT(6)
		#define M_STA			BIT(5)
		#define M_STP			BIT(4)
		#define INT_FLAG		BIT(3)
		#define A_ACK			BIT(2)
		
#define TWI_STAT	0x0010
		#define TWI_M_ACK		0x50
		#define TWI_M_NACK		0x58		
#define TWI_CCR		0x0014	
		#define CLK_M			GENMASK(6,3)
		#define CLK_N			GENMASK(2,0)
#define TWI_SRST	0x0018	
		#define TWI_SRST_MASK	BIT(0)
#define TWI_EFR		0x001C
		#define DBN				GENMASK(1,0)
#define TWI_LCR		0x0020
		#define SCL_STATE		BIT(5)
		#define SDA_STATE		BIT(4)
		#define SCL_CTL			BIT(3)
		#define SCL_CTL_EN		BIT(2)
		#define SDA_CTL			BIT(1)
		#define SDA_CTL_EN		BIT(0)
#define TWI_DVFSCR	0x0024 /*TWI0 only*/
		#define MS_PRIORITY		BIT(2)
		#define CPU_BUSY_SET	BIT(1)
		#define DVFC_BUSY_SET	BIT(0)
		

struct sun8i_i2c_regs {
	u32 addr;	//slavemode
	u32 xaddr;	//slavemode
	u32 data;
	u32 cntr;
	u32 stat;
	u32 ccr;
	u32 srst;
	u32 efr;
	u32 lcr;	
	u32 dvfscr;	//slavemode, twi0 only.
};

struct sun8i_i2c_pdata {
	unsigned clk_max_div;
	unsigned clk_offset;
};

struct sun8i_i2c_bus {
	struct sun8i_i2c_regs *regs;
	u32 status;
	ulong bus_clk_rate;
	u32 clock_frequency;
	u32 speed;
	u32 ccr_val;
	const struct sun8i_i2c_pdata *pdata;
};

int sun8i_i2c_read(struct sun8i_i2c_regs* reg, u8 chip, uint addr, int alen, u8 *data, int len);
int sun8i_i2c_write(struct sun8i_i2c_regs *reg, u8 chip, uint addr, int alen, u8 *buffer, int len);

#endif /* _SUNXI_I2C_H */
