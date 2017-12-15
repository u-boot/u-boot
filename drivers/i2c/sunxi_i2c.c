/*
 * SUN8I V3S GPIO-I2C driver.
 *
 * Copyright(c) 2017 I4VINE Inc.,
 * All right reserved by JuYoung Ryu <jyryu@stcube.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
//#define DEBUG	1
#include <asm/io.h>
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <i2c.h>
#include <linux/bitops.h>
#include <asm/arch/sunxi_i2c.h>
#include <asm/arch/gpio.h>

DECLARE_GLOBAL_DATA_PTR;
struct sun8i_i2c_regs i2c_regs;
struct sun8i_i2c_regs *i2c_reg;

#define V3S_I2C_GPIO	

# ifndef I2C_DELAY
#  define I2C_DELAY udelay(25)	/* 1/4 I2C clock duration */
# endif

enum {
	PIN_SDA = 0,
	PIN_SCL,
	PIN_COUNT,
};

void scl_ctl(struct sun8i_i2c_regs *reg, u8 val)
{
	u32 read_val=0;
	read_val = readl(&reg->lcr);		
	if(val){
		writel(read_val|SCL_CTL, &reg->lcr);	
	} else {
		writel(read_val&~SCL_CTL, &reg->lcr);	
	}
}

void sda_ctl(struct sun8i_i2c_regs *reg, u8 val)
{
	u32 read_val=0;
	read_val = readl(&reg->lcr);	
	if(val){
		writel(read_val|SDA_CTL, &reg->lcr);	
	} else {
		writel(read_val&~SDA_CTL, &reg->lcr);	
	}
}

static int write_byte(struct sun8i_i2c_regs *reg, unsigned char data)
{
	int i = 0;
	int nack;
	for(i=0; i<8; i++){
		scl_ctl(reg,0);
		I2C_DELAY;
		sda_ctl(reg,data & 0x80);
		I2C_DELAY;
		scl_ctl(reg,1);
		I2C_DELAY;
		I2C_DELAY;

		data <<= 1;
	}
	scl_ctl(reg,0);
	I2C_DELAY;

	sda_ctl(reg,1);
	I2C_DELAY;
	scl_ctl(reg,1);
	I2C_DELAY;
	I2C_DELAY;

	nack = 	readl(&reg->lcr) & SDA_STATE ;
	scl_ctl(reg,0);
	I2C_DELAY;
	I2C_DELAY;
	return (nack&0x01);
}

static void send_ack(struct sun8i_i2c_regs *reg, int ack)
{
	scl_ctl(reg,0);
	I2C_DELAY;

	sda_ctl(reg,ack);
	I2C_DELAY;
	scl_ctl(reg,1);
	I2C_DELAY;
	I2C_DELAY;
	scl_ctl(reg,0);
	I2C_DELAY;
}

static unsigned char read_byte(struct sun8i_i2c_regs *reg, int ack)
{
	unsigned char data = 0;
	unsigned char read_bit = 0;
	int i=0;

	sda_ctl(reg,1);	
	for(i=0; i<8; i++){
		scl_ctl(reg,0);
		I2C_DELAY;
		scl_ctl(reg,1);
		I2C_DELAY;
		data <<= 1;
		read_bit = !!(readl(&reg->lcr) & SDA_STATE);
		data |= read_bit;
		I2C_DELAY;
	}
	send_ack(reg,ack);
	
	return data;
}

static void send_start(struct sun8i_i2c_regs *reg)
{
	I2C_DELAY;
	sda_ctl(reg,1);
	I2C_DELAY;
	scl_ctl(reg,1);
	I2C_DELAY;
	sda_ctl(reg,0);
	I2C_DELAY;
}

static void send_stop(struct sun8i_i2c_regs *reg)
{
	scl_ctl(reg,0);
	I2C_DELAY;
	sda_ctl(reg,0);
	I2C_DELAY;
	scl_ctl(reg,1);
	I2C_DELAY;
	sda_ctl(reg,1);
	I2C_DELAY;
}

static void sun8i_i2c_init(struct  sun8i_i2c_regs *reg)
{
	i2c_reg = &i2c_regs;
	i2c_reg = reg;

	writel(0x28, &reg->ccr);
	writel(BUS_EN, &reg->cntr);
	writel(SCL_CTL_EN|SDA_CTL_EN|SCL_CTL|SDA_CTL, &reg->lcr);
	
#ifdef V3S_I2C_GPIO
	sunxi_gpio_set_cfgpin(SUNXI_GPB(6), SUN8I_GPB_TWI0);//SUNXI_GPIO_OUTPUT);//SUN8I_GPB_TWI0);
	sunxi_gpio_set_cfgpin(SUNXI_GPB(7), SUN8I_GPB_TWI0);//SUNXI_GPIO_OUTPUT);//SUN8I_GPB_TWI0);
	clock_twi_onoff(0, 1);
	sunxi_gpio_set_cfgpin(SUNXI_GPE(21), SUN8I_GPE_TWI1);//SUNXI_GPIO_OUTPUT);//SUN8I_GPE_TWI1);
	sunxi_gpio_set_cfgpin(SUNXI_GPE(22), SUN8I_GPE_TWI1);//SUNXI_GPIO_OUTPUT);//SUN8I_GPE_TWI1);
	clock_twi_onoff(1, 1);	
#endif
//slave mode
	//addr = readl(&reg->addr);
	//addr = (addr&0x01) | ((slaveaddr << 1)&0xFE);
	//writel(addr, &&reg->addr);
	
	debug("%s exit\n", __func__);
}

int sun8i_i2c_read(struct sun8i_i2c_regs* reg, u8 chip, uint addr, int alen, u8 *data, int len)
{
	unsigned char shift = 0;	
	int ret;
	debug("%s enter chip 0x%X addr 0x%X alen %d len %d data 0x%X \n", __func__, chip,addr,alen,len,*data);	
	send_start(reg);
#if 0
	if(alen > 0) {
		if(alen > 1)
			debug("Not support yet over two bytes.\n");
		
		ret = write_byte(reg,chip << 1);
		if(ret) {	/* write cycle */
			send_stop(reg);
			debug("i2c_read, no chip responded %02X\n", chip);
			return ret;
		}
		
	
	/*	shift = (alen-1) * 8;
		while(alen-- > 0) {
			ret = write_byte(reg,addr >> shift);
			if(ret) {
				debug("i2c_read, address not <ACK>ed\n");
				return ret;
			}
			shift -= 8;
		}	*/
#ifdef CONFIG_SOFT_I2C_READ_REPEATED_START
		send_start(reg);
#else
		send_stop(reg);
		send_start(reg);
#endif
	}	
#endif
	write_byte(reg,(chip << 1) | 1);	/* read cycle */
	
	while(len-- > 0) {
		*data++ = read_byte(reg,len == 0);
	}
	send_stop(reg);
	debug("%s exit\n", __func__);		
	return 0;
}

int sun8i_i2c_write(struct sun8i_i2c_regs *reg, u8 chip, uint addr, int alen, u8 *buffer, int len)
{
	unsigned char data = *buffer;
	int shift, failures = 0;
	int ret=0;
	debug("%s enter chip 0x%X, addr 0x%X val 0x%X alen %d len %d buf 0x%X\n", __func__,chip,addr,data,alen,len,buffer);	

	send_start(reg);

	if(alen > 0){
		ret = write_byte(reg,chip << 1);
		if(ret) {	/* write cycle */
			send_stop(reg);
			debug("i2c_write, no chip responded %02X\n", chip);
			return ret;
		}			
	/*		shift = (alen-1) * 8;
	while(alen-- > 0) {
			ret = write_byte(reg,chip >> shift);
			if(ret) {
				debug("i2c_write, address not <ACK>ed\n");
				return ret;
			}
			shift -= 8;
		}*/
	}

	while(len-- > 0) {
		ret = write_byte(reg,*buffer++);
		if(ret) {
			failures++;
		}
	}
	send_stop(reg);
	debug("%s exit\n", __func__);		
	return(failures);
}

static int sun8i_i2c_xfer(struct udevice *dev, struct i2c_msg *msg, int nmsgs)
{
	struct sun8i_i2c_bus *bus = dev_get_priv(dev);
	struct sun8i_i2c_regs *reg = bus->regs;
	struct i2c_msg *msgs = msg;
	bool is_read;
	u32 int_addr_flag = 0;
	int ret = 0;

	debug("%s enter msgs %d chip=0x%x, len=0x%x\n\n", __func__,nmsgs,msgs->addr, msgs->len);	

	for(; nmsgs>0; nmsgs--, msgs++){

		if(msgs->flags & I2C_M_RD){
			ret = sun8i_i2c_read(reg, msgs->addr,0,1,msgs->buf,msgs->len);
		} else { 
			ret = sun8i_i2c_write(reg, msgs->addr,0, 1, msgs->buf, msgs->len);
		}
		
		if(ret) {
			debug("i2c_write: error sending\n");
			return -EREMOTEIO;
		}
	}
	debug("%s exit\n", __func__);	
	return ret;
}

static int sun8i_i2c_probe_chip(struct udevice *dev, uint chip, uint chip_flags)
{
	struct sun8i_i2c_bus *bus = dev_get_priv(dev);
	struct sun8i_i2c_regs *reg = bus->regs;
	int ret;
	u8 data = 0;
	debug("%s enter\n", __func__);	

	bus->regs = (struct sun8i_i2c_regs *)dev_get_addr(dev);
	bus->speed = 400000;
	
	sun8i_i2c_init(reg);

	ret = sun8i_i2c_write(reg,chip,0x00,1,&data,1);
	
	debug("%s exit ret 0x%X\n", __func__,ret);  
	return ret;
}

static int sun8i_i2c_probe(struct udevice *dev)
{
	struct sun8i_i2c_bus *bus = dev_get_priv(dev);
	struct sun8i_i2c_regs *reg = bus->regs;

	int ret;

	debug("%s enter\n", __func__);	

	bus->regs = (struct sun8i_i2c_regs *)dev_get_addr(dev);
	bus->speed = 400000;
	
	sun8i_i2c_init(reg);

	debug("%s exit\n", __func__);  
	return 0;
}

static int sun8i_i2c_ofdata_to_platdata(struct udevice *dev)
{
	const void *blob = gd->fdt_blob;
	struct sun8i_i2c_bus *bus = dev_get_priv(dev);
	int node = dev->of_offset;
	debug("%s enter \n", __func__);
	bus->regs = (struct sun8i_i2c_regs *)dev_get_addr(dev);
	bus->pdata = (struct sun8i_i2c_pdata *)dev_get_driver_data(dev);
	bus->clock_frequency = fdtdec_get_int(blob, node,
					      "clock-frequency", 100000);
	debug("%s exit \n", __func__);
	return 0;
}

static int sun8i_i2c_set_bus_speed(struct udevice *dev, unsigned int speed)
{
	struct sun8i_i2c_bus *bus = dev_get_priv(dev);
	struct sun8i_i2c_regs *reg = bus->regs;
	bus->speed = 400000;
	writel(0x28, &reg->ccr);
	
	return 0;	
}

static int sun8i_i2c_reset(struct udevice *dev)
{
	struct sun8i_i2c_bus *bus = dev_get_priv(dev);
	struct sun8i_i2c_regs *reg = bus->regs;
	debug("%s enter\n", __func__);
	writel(0x01, &reg->srst);
	sun8i_i2c_init(reg);	
	debug("%s exit\n", __func__);	
	return 0;	
}

static const struct dm_i2c_ops sun8i_i2c_ops = {
	.xfer           = sun8i_i2c_xfer,
	.probe_chip     = sun8i_i2c_probe_chip,
	.set_bus_speed  = sun8i_i2c_set_bus_speed,
	.deblock		= sun8i_i2c_reset,
/*	.get_bus_speed	= sun8i_i2c_get_bus_speed,	*/
};

static const struct udevice_id sun8i_i2c_ids[] = {
	{ .compatible = "i4vine,i2c-sunxi",},
	{ /* sentinel */ }
};



U_BOOT_DRIVER(i2c_sunxi) = {
	.name	= "i2c_sunxi",
	.id	= UCLASS_I2C,
	.of_match = sun8i_i2c_ids,
	.ofdata_to_platdata = sun8i_i2c_ofdata_to_platdata,
	.per_child_auto_alloc_size = sizeof(struct dm_i2c_chip),	
	.priv_auto_alloc_size = sizeof(struct sun8i_i2c_bus),
	.ops	= &sun8i_i2c_ops,
	.probe = sun8i_i2c_probe,
};

