/*
 * Copyright (C) 2012 Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * Copyright (C) 2012 Renesas Solutions Corp.
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
#include <i2c.h>
#include <asm/io.h>

struct sh_i2c {
	u8 iccr1;
	u8 iccr2;
	u8 icmr;
	u8 icier;
	u8 icsr;
	u8 sar;
	u8 icdrt;
	u8 icdrr;
	u8 nf2cyc;
	u8 __pad0;
	u8 __pad1;
};

static struct sh_i2c *base;
static u8 iccr1_cks, nf2cyc;

/* ICCR1 */
#define SH_I2C_ICCR1_ICE	(1 << 7)
#define SH_I2C_ICCR1_RCVD	(1 << 6)
#define SH_I2C_ICCR1_MST	(1 << 5)
#define SH_I2C_ICCR1_TRS	(1 << 4)
#define SH_I2C_ICCR1_MTRS	\
	(SH_I2C_ICCR1_MST | SH_I2C_ICCR1_TRS)

/* ICCR1 */
#define SH_I2C_ICCR2_BBSY	(1 << 7)
#define SH_I2C_ICCR2_SCP	(1 << 6)
#define SH_I2C_ICCR2_SDAO	(1 << 5)
#define SH_I2C_ICCR2_SDAOP	(1 << 4)
#define SH_I2C_ICCR2_SCLO	(1 << 3)
#define SH_I2C_ICCR2_IICRST	(1 << 1)

#define SH_I2C_ICIER_TIE	(1 << 7)
#define SH_I2C_ICIER_TEIE	(1 << 6)
#define SH_I2C_ICIER_RIE	(1 << 5)
#define SH_I2C_ICIER_NAKIE	(1 << 4)
#define SH_I2C_ICIER_STIE	(1 << 3)
#define SH_I2C_ICIER_ACKE	(1 << 2)
#define SH_I2C_ICIER_ACKBR	(1 << 1)
#define SH_I2C_ICIER_ACKBT	(1 << 0)

#define SH_I2C_ICSR_TDRE	(1 << 7)
#define SH_I2C_ICSR_TEND	(1 << 6)
#define SH_I2C_ICSR_RDRF	(1 << 5)
#define SH_I2C_ICSR_NACKF	(1 << 4)
#define SH_I2C_ICSR_STOP	(1 << 3)
#define SH_I2C_ICSR_ALOVE	(1 << 2)
#define SH_I2C_ICSR_AAS		(1 << 1)
#define SH_I2C_ICSR_ADZ		(1 << 0)

#define IRQ_WAIT 1000

static void sh_i2c_send_stop(struct sh_i2c *base)
{
	clrbits_8(&base->iccr2, SH_I2C_ICCR2_BBSY | SH_I2C_ICCR2_SCP);
}

static int check_icsr_bits(struct sh_i2c *base, u8 bits)
{
	int i;

	for (i = 0; i < IRQ_WAIT; i++) {
		if (bits & readb(&base->icsr))
			return 0;
		udelay(10);
	}

	return 1;
}

static int check_stop(struct sh_i2c *base)
{
	int ret = check_icsr_bits(base, SH_I2C_ICSR_STOP);
	clrbits_8(&base->icsr, SH_I2C_ICSR_STOP);

	return ret;
}

static int check_tend(struct sh_i2c *base, int stop)
{
	int ret = check_icsr_bits(base, SH_I2C_ICSR_TEND);

	if (stop) {
		clrbits_8(&base->icsr, SH_I2C_ICSR_STOP);
		sh_i2c_send_stop(base);
	}

	clrbits_8(&base->icsr, SH_I2C_ICSR_TEND);
	return ret;
}

static int check_tdre(struct sh_i2c *base)
{
	return check_icsr_bits(base, SH_I2C_ICSR_TDRE);
}

static int check_rdrf(struct sh_i2c *base)
{
	return check_icsr_bits(base, SH_I2C_ICSR_RDRF);
}

static int check_bbsy(struct sh_i2c *base)
{
	int i;

	for (i = 0 ; i < IRQ_WAIT ; i++) {
		if (!(SH_I2C_ICCR2_BBSY & readb(&base->iccr2)))
			return 0;
		udelay(10);
	}
	return 1;
}

static int check_ackbr(struct sh_i2c *base)
{
	int i;

	for (i = 0 ; i < IRQ_WAIT ; i++) {
		if (!(SH_I2C_ICIER_ACKBR & readb(&base->icier)))
			return 0;
		udelay(10);
	}

	return 1;
}

static void sh_i2c_reset(struct sh_i2c *base)
{
	setbits_8(&base->iccr2, SH_I2C_ICCR2_IICRST);

	udelay(100);

	clrbits_8(&base->iccr2, SH_I2C_ICCR2_IICRST);
}

static int i2c_set_addr(struct sh_i2c *base, u8 id, u8 reg)
{
	if (check_bbsy(base)) {
		puts("i2c bus busy\n");
		goto fail;
	}

	setbits_8(&base->iccr1, SH_I2C_ICCR1_MTRS);
	clrsetbits_8(&base->iccr2, SH_I2C_ICCR2_SCP, SH_I2C_ICCR2_BBSY);

	writeb((id << 1), &base->icdrt);

	if (check_tend(base, 0)) {
		puts("TEND check fail...\n");
		goto fail;
	}

	if (check_ackbr(base)) {
		check_tend(base, 0);
		sh_i2c_send_stop(base);
		goto fail;
	}

	writeb(reg, &base->icdrt);

	if (check_tdre(base)) {
		puts("TDRE check fail...\n");
		goto fail;
	}

	if (check_tend(base, 0)) {
		puts("TEND check fail...\n");
		goto fail;
	}

	return 0;
fail:

	return 1;
}

static int
i2c_raw_write(struct sh_i2c *base, u8 id, u8 reg, u8 *val, int size)
{
	int i;

	if (i2c_set_addr(base, id, reg)) {
		puts("Fail set slave address\n");
		return 1;
	}

	for (i = 0; i < size; i++) {
		writeb(val[i], &base->icdrt);
		check_tdre(base);
	}

	check_tend(base, 1);
	check_stop(base);

	udelay(100);

	clrbits_8(&base->iccr1, SH_I2C_ICCR1_MTRS);
	clrbits_8(&base->icsr, SH_I2C_ICSR_TDRE);
	sh_i2c_reset(base);

	return 0;
}

static u8 i2c_raw_read(struct sh_i2c *base, u8 id, u8 reg)
{
	u8 ret = 0;

	if (i2c_set_addr(base, id, reg)) {
		puts("Fail set slave address\n");
		goto fail;
	}

	clrsetbits_8(&base->iccr2, SH_I2C_ICCR2_SCP, SH_I2C_ICCR2_BBSY);
	writeb((id << 1) | 1, &base->icdrt);

	if (check_tend(base, 0))
		puts("TDRE check fail...\n");

	clrsetbits_8(&base->iccr1, SH_I2C_ICCR1_TRS, SH_I2C_ICCR1_MST);
	clrbits_8(&base->icsr, SH_I2C_ICSR_TDRE);
	setbits_8(&base->icier, SH_I2C_ICIER_ACKBT);
	setbits_8(&base->iccr1, SH_I2C_ICCR1_RCVD);

	/* read data (dummy) */
	ret = readb(&base->icdrr);

	if (check_rdrf(base)) {
		puts("check RDRF error\n");
		goto fail;
	}

	clrbits_8(&base->icsr, SH_I2C_ICSR_STOP);
	udelay(1000);

	sh_i2c_send_stop(base);

	if (check_stop(base)) {
		puts("check STOP error\n");
		goto fail;
	}

	clrbits_8(&base->iccr1, SH_I2C_ICCR1_MTRS);
	clrbits_8(&base->icsr, SH_I2C_ICSR_TDRE);

	/* data read */
	ret = readb(&base->icdrr);

fail:
	clrbits_8(&base->iccr1, SH_I2C_ICCR1_RCVD);

	return ret;
}

#ifdef CONFIG_I2C_MULTI_BUS
static unsigned int current_bus;

/**
 * i2c_set_bus_num - change active I2C bus
 *	@bus: bus index, zero based
 *	@returns: 0 on success, non-0 on failure
 */
int i2c_set_bus_num(unsigned int bus)
{
	switch (bus) {
	case 0:
		base = (void *)CONFIG_SH_I2C_BASE0;
		break;
	case 1:
		base = (void *)CONFIG_SH_I2C_BASE1;
		break;
	default:
		printf("Bad bus: %d\n", bus);
		return -1;
	}

	current_bus = bus;

	return 0;
}

/**
 * i2c_get_bus_num - returns index of active I2C bus
 */
unsigned int i2c_get_bus_num(void)
{
	return current_bus;
}
#endif

void i2c_init(int speed, int slaveaddr)
{
#ifdef CONFIG_I2C_MULTI_BUS
	current_bus = 0;
#endif
	base = (struct sh_i2c *)CONFIG_SH_I2C_BASE0;

	if (speed == 400000)
		iccr1_cks = 0x07;
	else
		iccr1_cks = 0x0F;

	nf2cyc = 1;

	/* Reset */
	sh_i2c_reset(base);

	/* ICE enable and set clock */
	writeb(SH_I2C_ICCR1_ICE | iccr1_cks, &base->iccr1);
	writeb(nf2cyc, &base->nf2cyc);
}

/*
 * i2c_read: - Read multiple bytes from an i2c device
 *
 * The higher level routines take into account that this function is only
 * called with len < page length of the device (see configuration file)
 *
 * @chip:   address of the chip which is to be read
 * @addr:   i2c data address within the chip
 * @alen:   length of the i2c data address (1..2 bytes)
 * @buffer: where to write the data
 * @len:    how much byte do we want to read
 * @return: 0 in case of success
 */
int i2c_read(u8 chip, u32 addr, int alen, u8 *buffer, int len)
{
	int i = 0;
	for (i = 0; i < len; i++)
		buffer[i] = i2c_raw_read(base, chip, addr + i);

	return 0;
}

/*
 * i2c_write: -  Write multiple bytes to an i2c device
 *
 * The higher level routines take into account that this function is only
 * called with len < page length of the device (see configuration file)
 *
 * @chip:   address of the chip which is to be written
 * @addr:   i2c data address within the chip
 * @alen:   length of the i2c data address (1..2 bytes)
 * @buffer: where to find the data to be written
 * @len:    how much byte do we want to read
 * @return: 0 in case of success
 */
int i2c_write(u8 chip, u32 addr, int alen, u8 *buffer, int len)
{
	return i2c_raw_write(base, chip, addr, buffer, len);
}

/*
 * i2c_probe: - Test if a chip answers for a given i2c address
 *
 * @chip:   address of the chip which is searched for
 * @return: 0 if a chip was found, -1 otherwhise
 */
int i2c_probe(u8 chip)
{
	u8 byte;
	return i2c_read(chip, 0, 0, &byte, 1);
}
