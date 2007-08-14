/*
 * TI DaVinci (TMS320DM644x) I2C driver.
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * --------------------------------------------------------
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

#ifdef CONFIG_DRIVER_DAVINCI_I2C

#include <i2c.h>
#include <asm/arch/hardware.h>
#include <asm/arch/i2c_defs.h>

#define CHECK_NACK() \
	do {\
		if (tmp & (I2C_TIMEOUT | I2C_STAT_NACK)) {\
			REG(I2C_CON) = 0;\
			return(1);\
		}\
	} while (0)


static int wait_for_bus(void)
{
	int	stat, timeout;

	REG(I2C_STAT) = 0xffff;

	for (timeout = 0; timeout < 10; timeout++) {
		if (!((stat = REG(I2C_STAT)) & I2C_STAT_BB)) {
			REG(I2C_STAT) = 0xffff;
			return(0);
		}

		REG(I2C_STAT) = stat;
		udelay(50000);
	}

	REG(I2C_STAT) = 0xffff;
	return(1);
}


static int poll_i2c_irq(int mask)
{
	int	stat, timeout;

	for (timeout = 0; timeout < 10; timeout++) {
		udelay(1000);
		stat = REG(I2C_STAT);
		if (stat & mask) {
			return(stat);
		}
	}

	REG(I2C_STAT) = 0xffff;
	return(stat | I2C_TIMEOUT);
}


void flush_rx(void)
{
	int	dummy;

	while (1) {
		if (!(REG(I2C_STAT) & I2C_STAT_RRDY))
			break;

		dummy = REG(I2C_DRR);
		REG(I2C_STAT) = I2C_STAT_RRDY;
		udelay(1000);
	}
}


void i2c_init(int speed, int slaveadd)
{
	u_int32_t	div, psc;

	if (REG(I2C_CON) & I2C_CON_EN) {
		REG(I2C_CON) = 0;
		udelay (50000);
	}

	psc = 2;
	div = (CFG_HZ_CLOCK / ((psc + 1) * speed)) - 10;	/* SCLL + SCLH */
	REG(I2C_PSC) = psc;			/* 27MHz / (2 + 1) = 9MHz */
	REG(I2C_SCLL) = (div * 50) / 100;	/* 50% Duty */
	REG(I2C_SCLH) = div - REG(I2C_SCLL);

	REG(I2C_OA) = slaveadd;
	REG(I2C_CNT) = 0;

	/* Interrupts must be enabled or I2C module won't work */
	REG(I2C_IE) = I2C_IE_SCD_IE | I2C_IE_XRDY_IE |
		I2C_IE_RRDY_IE | I2C_IE_ARDY_IE | I2C_IE_NACK_IE;

	/* Now enable I2C controller (get it out of reset) */
	REG(I2C_CON) = I2C_CON_EN;

	udelay(1000);
}


int i2c_probe(u_int8_t chip)
{
	int	rc = 1;

	if (chip == REG(I2C_OA)) {
		return(rc);
	}

	REG(I2C_CON) = 0;
	if (wait_for_bus()) {return(1);}

	/* try to read one byte from current (or only) address */
	REG(I2C_CNT) = 1;
	REG(I2C_SA) = chip;
	REG(I2C_CON) = (I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_STP);
	udelay (50000);

	if (!(REG(I2C_STAT) & I2C_STAT_NACK)) {
		rc = 0;
		flush_rx();
		REG(I2C_STAT) = 0xffff;
	} else {
		REG(I2C_STAT) = 0xffff;
		REG(I2C_CON) |= I2C_CON_STP;
		udelay(20000);
		if (wait_for_bus()) {return(1);}
	}

	flush_rx();
	REG(I2C_STAT) = 0xffff;
	REG(I2C_CNT) = 0;
	return(rc);
}


int i2c_read(u_int8_t chip, u_int32_t addr, int alen, u_int8_t *buf, int len)
{
	u_int32_t	tmp;
	int		i;

	if ((alen < 0) || (alen > 2)) {
		printf("%s(): bogus address length %x\n", __FUNCTION__, alen);
		return(1);
	}

	if (wait_for_bus()) {return(1);}

	if (alen != 0) {
		/* Start address phase */
		tmp = I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX;
		REG(I2C_CNT) = alen;
		REG(I2C_SA) = chip;
		REG(I2C_CON) = tmp;

		tmp = poll_i2c_irq(I2C_STAT_XRDY | I2C_STAT_NACK);

		CHECK_NACK();

		switch (alen) {
			case 2:
				/* Send address MSByte */
				if (tmp & I2C_STAT_XRDY) {
					REG(I2C_DXR) = (addr >> 8) & 0xff;
				} else {
					REG(I2C_CON) = 0;
					return(1);
				}

				tmp = poll_i2c_irq(I2C_STAT_XRDY | I2C_STAT_NACK);

				CHECK_NACK();
				/* No break, fall through */
			case 1:
				/* Send address LSByte */
				if (tmp & I2C_STAT_XRDY) {
					REG(I2C_DXR) = addr & 0xff;
				} else {
					REG(I2C_CON) = 0;
					return(1);
				}

				tmp = poll_i2c_irq(I2C_STAT_XRDY | I2C_STAT_NACK | I2C_STAT_ARDY);

				CHECK_NACK();

				if (!(tmp & I2C_STAT_ARDY)) {
					REG(I2C_CON) = 0;
					return(1);
				}
		}
	}

	/* Address phase is over, now read 'len' bytes and stop */
	tmp = I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_STP;
	REG(I2C_CNT) = len & 0xffff;
	REG(I2C_SA) = chip;
	REG(I2C_CON) = tmp;

	for (i = 0; i < len; i++) {
		tmp = poll_i2c_irq(I2C_STAT_RRDY | I2C_STAT_NACK | I2C_STAT_ROVR);

		CHECK_NACK();

		if (tmp & I2C_STAT_RRDY) {
			buf[i] = REG(I2C_DRR);
		} else {
			REG(I2C_CON) = 0;
			return(1);
		}
	}

	tmp = poll_i2c_irq(I2C_STAT_SCD | I2C_STAT_NACK);

	CHECK_NACK();

	if (!(tmp & I2C_STAT_SCD)) {
		REG(I2C_CON) = 0;
		return(1);
	}

	flush_rx();
	REG(I2C_STAT) = 0xffff;
	REG(I2C_CNT) = 0;
	REG(I2C_CON) = 0;

	return(0);
}


int i2c_write(u_int8_t chip, u_int32_t addr, int alen, u_int8_t *buf, int len)
{
	u_int32_t	tmp;
	int		i;

	if ((alen < 0) || (alen > 2)) {
		printf("%s(): bogus address length %x\n", __FUNCTION__, alen);
		return(1);
	}
	if (len < 0) {
		printf("%s(): bogus length %x\n", __FUNCTION__, len);
		return(1);
	}

	if (wait_for_bus()) {return(1);}

	/* Start address phase */
	tmp = I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX | I2C_CON_STP;
	REG(I2C_CNT) = (alen == 0) ? len & 0xffff : (len & 0xffff) + alen;
	REG(I2C_SA) = chip;
	REG(I2C_CON) = tmp;

	switch (alen) {
		case 2:
			/* Send address MSByte */
			tmp = poll_i2c_irq(I2C_STAT_XRDY | I2C_STAT_NACK);

			CHECK_NACK();

			if (tmp & I2C_STAT_XRDY) {
				REG(I2C_DXR) = (addr >> 8) & 0xff;
			} else {
				REG(I2C_CON) = 0;
				return(1);
			}
			/* No break, fall through */
		case 1:
			/* Send address LSByte */
			tmp = poll_i2c_irq(I2C_STAT_XRDY | I2C_STAT_NACK);

			CHECK_NACK();

			if (tmp & I2C_STAT_XRDY) {
				REG(I2C_DXR) = addr & 0xff;
			} else {
				REG(I2C_CON) = 0;
				return(1);
			}
	}

	for (i = 0; i < len; i++) {
		tmp = poll_i2c_irq(I2C_STAT_XRDY | I2C_STAT_NACK);

		CHECK_NACK();

		if (tmp & I2C_STAT_XRDY) {
			REG(I2C_DXR) = buf[i];
		} else {
			return(1);
		}
	}

	tmp = poll_i2c_irq(I2C_STAT_SCD | I2C_STAT_NACK);

	CHECK_NACK();

	if (!(tmp & I2C_STAT_SCD)) {
		REG(I2C_CON) = 0;
		return(1);
	}

	flush_rx();
	REG(I2C_STAT) = 0xffff;
	REG(I2C_CNT) = 0;
	REG(I2C_CON) = 0;

	return(0);
}


u_int8_t i2c_reg_read(u_int8_t chip, u_int8_t reg)
{
	u_int8_t	tmp;

	i2c_read(chip, reg, 1, &tmp, 1);
	return(tmp);
}


void i2c_reg_write(u_int8_t chip, u_int8_t reg, u_int8_t val)
{
	u_int8_t	tmp;

	i2c_write(chip, reg, 1, &tmp, 1);
}

#endif /* CONFIG_DRIVER_DAVINCI_I2C */
