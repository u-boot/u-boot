/*
 * TI DaVinci (TMS320DM644x) I2C driver.
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 * (C) Copyright 2007 Sergey Kubushyn <ksi@koi8.net>
 * --------------------------------------------------------
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <asm/arch/hardware.h>
#include <asm/arch/i2c_defs.h>
#include <asm/io.h>
#include "davinci_i2c.h"

#define CHECK_NACK() \
	do {\
		if (tmp & (I2C_TIMEOUT | I2C_STAT_NACK)) {\
			REG(&(i2c_base->i2c_con)) = 0;\
			return 1;\
		} \
	} while (0)

static struct i2c_regs *davinci_get_base(struct i2c_adapter *adap);

static int wait_for_bus(struct i2c_adapter *adap)
{
	struct i2c_regs *i2c_base = davinci_get_base(adap);
	int	stat, timeout;

	REG(&(i2c_base->i2c_stat)) = 0xffff;

	for (timeout = 0; timeout < 10; timeout++) {
		stat = REG(&(i2c_base->i2c_stat));
		if (!((stat) & I2C_STAT_BB)) {
			REG(&(i2c_base->i2c_stat)) = 0xffff;
			return 0;
		}

		REG(&(i2c_base->i2c_stat)) = stat;
		udelay(50000);
	}

	REG(&(i2c_base->i2c_stat)) = 0xffff;
	return 1;
}


static int poll_i2c_irq(struct i2c_adapter *adap, int mask)
{
	struct i2c_regs *i2c_base = davinci_get_base(adap);
	int	stat, timeout;

	for (timeout = 0; timeout < 10; timeout++) {
		udelay(1000);
		stat = REG(&(i2c_base->i2c_stat));
		if (stat & mask)
			return stat;
	}

	REG(&(i2c_base->i2c_stat)) = 0xffff;
	return stat | I2C_TIMEOUT;
}

static void flush_rx(struct i2c_adapter *adap)
{
	struct i2c_regs *i2c_base = davinci_get_base(adap);

	while (1) {
		if (!(REG(&(i2c_base->i2c_stat)) & I2C_STAT_RRDY))
			break;

		REG(&(i2c_base->i2c_drr));
		REG(&(i2c_base->i2c_stat)) = I2C_STAT_RRDY;
		udelay(1000);
	}
}

static uint davinci_i2c_setspeed(struct i2c_adapter *adap, uint speed)
{
	struct i2c_regs *i2c_base = davinci_get_base(adap);
	uint32_t	div, psc;

	psc = 2;
	/* SCLL + SCLH */
	div = (CONFIG_SYS_HZ_CLOCK / ((psc + 1) * speed)) - 10;
	REG(&(i2c_base->i2c_psc)) = psc; /* 27MHz / (2 + 1) = 9MHz */
	REG(&(i2c_base->i2c_scll)) = (div * 50) / 100; /* 50% Duty */
	REG(&(i2c_base->i2c_sclh)) = div - REG(&(i2c_base->i2c_scll));

	adap->speed	= speed;
	return 0;
}

static void davinci_i2c_init(struct i2c_adapter *adap, int speed, int slaveadd)
{
	struct i2c_regs *i2c_base = davinci_get_base(adap);

	if (REG(&(i2c_base->i2c_con)) & I2C_CON_EN) {
		REG(&(i2c_base->i2c_con)) = 0;
		udelay(50000);
	}

	davinci_i2c_setspeed(adap, speed);

	REG(&(i2c_base->i2c_oa)) = slaveadd;
	REG(&(i2c_base->i2c_cnt)) = 0;

	/* Interrupts must be enabled or I2C module won't work */
	REG(&(i2c_base->i2c_ie)) = I2C_IE_SCD_IE | I2C_IE_XRDY_IE |
		I2C_IE_RRDY_IE | I2C_IE_ARDY_IE | I2C_IE_NACK_IE;

	/* Now enable I2C controller (get it out of reset) */
	REG(&(i2c_base->i2c_con)) = I2C_CON_EN;

	udelay(1000);
}

static int davinci_i2c_probe(struct i2c_adapter *adap, uint8_t chip)
{
	struct i2c_regs *i2c_base = davinci_get_base(adap);
	int	rc = 1;

	if (chip == REG(&(i2c_base->i2c_oa)))
		return rc;

	REG(&(i2c_base->i2c_con)) = 0;
	if (wait_for_bus(adap))
		return 1;

	/* try to read one byte from current (or only) address */
	REG(&(i2c_base->i2c_cnt)) = 1;
	REG(&(i2c_base->i2c_sa))  = chip;
	REG(&(i2c_base->i2c_con)) = (I2C_CON_EN | I2C_CON_MST | I2C_CON_STT |
				     I2C_CON_STP);
	udelay(50000);

	if (!(REG(&(i2c_base->i2c_stat)) & I2C_STAT_NACK)) {
		rc = 0;
		flush_rx(adap);
		REG(&(i2c_base->i2c_stat)) = 0xffff;
	} else {
		REG(&(i2c_base->i2c_stat)) = 0xffff;
		REG(&(i2c_base->i2c_con)) |= I2C_CON_STP;
		udelay(20000);
		if (wait_for_bus(adap))
			return 1;
	}

	flush_rx(adap);
	REG(&(i2c_base->i2c_stat)) = 0xffff;
	REG(&(i2c_base->i2c_cnt)) = 0;
	return rc;
}

static int davinci_i2c_read(struct i2c_adapter *adap, uint8_t chip,
				uint32_t addr, int alen, uint8_t *buf, int len)
{
	struct i2c_regs *i2c_base = davinci_get_base(adap);
	uint32_t	tmp;
	int		i;

	if ((alen < 0) || (alen > 2)) {
		printf("%s(): bogus address length %x\n", __func__, alen);
		return 1;
	}

	if (wait_for_bus(adap))
		return 1;

	if (alen != 0) {
		/* Start address phase */
		tmp = I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX;
		REG(&(i2c_base->i2c_cnt)) = alen;
		REG(&(i2c_base->i2c_sa)) = chip;
		REG(&(i2c_base->i2c_con)) = tmp;

		tmp = poll_i2c_irq(adap, I2C_STAT_XRDY | I2C_STAT_NACK);

		CHECK_NACK();

		switch (alen) {
		case 2:
			/* Send address MSByte */
			if (tmp & I2C_STAT_XRDY) {
				REG(&(i2c_base->i2c_dxr)) = (addr >> 8) & 0xff;
			} else {
				REG(&(i2c_base->i2c_con)) = 0;
				return 1;
			}

			tmp = poll_i2c_irq(adap, I2C_STAT_XRDY | I2C_STAT_NACK);

			CHECK_NACK();
			/* No break, fall through */
		case 1:
			/* Send address LSByte */
			if (tmp & I2C_STAT_XRDY) {
				REG(&(i2c_base->i2c_dxr)) = addr & 0xff;
			} else {
				REG(&(i2c_base->i2c_con)) = 0;
				return 1;
			}

			tmp = poll_i2c_irq(adap, I2C_STAT_XRDY |
					   I2C_STAT_NACK | I2C_STAT_ARDY);

			CHECK_NACK();

			if (!(tmp & I2C_STAT_ARDY)) {
				REG(&(i2c_base->i2c_con)) = 0;
				return 1;
			}
		}
	}

	/* Address phase is over, now read 'len' bytes and stop */
	tmp = I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_STP;
	REG(&(i2c_base->i2c_cnt)) = len & 0xffff;
	REG(&(i2c_base->i2c_sa)) = chip;
	REG(&(i2c_base->i2c_con)) = tmp;

	for (i = 0; i < len; i++) {
		tmp = poll_i2c_irq(adap, I2C_STAT_RRDY | I2C_STAT_NACK |
				   I2C_STAT_ROVR);

		CHECK_NACK();

		if (tmp & I2C_STAT_RRDY) {
			buf[i] = REG(&(i2c_base->i2c_drr));
		} else {
			REG(&(i2c_base->i2c_con)) = 0;
			return 1;
		}
	}

	tmp = poll_i2c_irq(adap, I2C_STAT_SCD | I2C_STAT_NACK);

	CHECK_NACK();

	if (!(tmp & I2C_STAT_SCD)) {
		REG(&(i2c_base->i2c_con)) = 0;
		return 1;
	}

	flush_rx(adap);
	REG(&(i2c_base->i2c_stat)) = 0xffff;
	REG(&(i2c_base->i2c_cnt)) = 0;
	REG(&(i2c_base->i2c_con)) = 0;

	return 0;
}

static int davinci_i2c_write(struct i2c_adapter *adap, uint8_t chip,
				uint32_t addr, int alen, uint8_t *buf, int len)
{
	struct i2c_regs *i2c_base = davinci_get_base(adap);
	uint32_t	tmp;
	int		i;

	if ((alen < 0) || (alen > 2)) {
		printf("%s(): bogus address length %x\n", __func__, alen);
		return 1;
	}
	if (len < 0) {
		printf("%s(): bogus length %x\n", __func__, len);
		return 1;
	}

	if (wait_for_bus(adap))
		return 1;

	/* Start address phase */
	tmp = I2C_CON_EN | I2C_CON_MST | I2C_CON_STT |
		I2C_CON_TRX | I2C_CON_STP;
	REG(&(i2c_base->i2c_cnt)) = (alen == 0) ?
		len & 0xffff : (len & 0xffff) + alen;
	REG(&(i2c_base->i2c_sa)) = chip;
	REG(&(i2c_base->i2c_con)) = tmp;

	switch (alen) {
	case 2:
		/* Send address MSByte */
		tmp = poll_i2c_irq(adap, I2C_STAT_XRDY | I2C_STAT_NACK);

		CHECK_NACK();

		if (tmp & I2C_STAT_XRDY) {
			REG(&(i2c_base->i2c_dxr)) = (addr >> 8) & 0xff;
		} else {
			REG(&(i2c_base->i2c_con)) = 0;
			return 1;
		}
		/* No break, fall through */
	case 1:
		/* Send address LSByte */
		tmp = poll_i2c_irq(adap, I2C_STAT_XRDY | I2C_STAT_NACK);

		CHECK_NACK();

		if (tmp & I2C_STAT_XRDY) {
			REG(&(i2c_base->i2c_dxr)) = addr & 0xff;
		} else {
			REG(&(i2c_base->i2c_con)) = 0;
			return 1;
		}
	}

	for (i = 0; i < len; i++) {
		tmp = poll_i2c_irq(adap, I2C_STAT_XRDY | I2C_STAT_NACK);

		CHECK_NACK();

		if (tmp & I2C_STAT_XRDY)
			REG(&(i2c_base->i2c_dxr)) = buf[i];
		else
			return 1;
	}

	tmp = poll_i2c_irq(adap, I2C_STAT_SCD | I2C_STAT_NACK);

	CHECK_NACK();

	if (!(tmp & I2C_STAT_SCD)) {
		REG(&(i2c_base->i2c_con)) = 0;
		return 1;
	}

	flush_rx(adap);
	REG(&(i2c_base->i2c_stat)) = 0xffff;
	REG(&(i2c_base->i2c_cnt)) = 0;
	REG(&(i2c_base->i2c_con)) = 0;

	return 0;
}

static struct i2c_regs *davinci_get_base(struct i2c_adapter *adap)
{
	switch (adap->hwadapnr) {
#if I2C_BUS_MAX >= 3
	case 2:
		return (struct i2c_regs *)I2C2_BASE;
#endif
#if I2C_BUS_MAX >= 2
	case 1:
		return (struct i2c_regs *)I2C1_BASE;
#endif
	case 0:
		return (struct i2c_regs *)I2C_BASE;

	default:
		printf("wrong hwadapnr: %d\n", adap->hwadapnr);
	}

	return NULL;
}

U_BOOT_I2C_ADAP_COMPLETE(davinci_0, davinci_i2c_init, davinci_i2c_probe,
			 davinci_i2c_read, davinci_i2c_write,
			 davinci_i2c_setspeed,
			 CONFIG_SYS_DAVINCI_I2C_SPEED,
			 CONFIG_SYS_DAVINCI_I2C_SLAVE,
			 0)

#if I2C_BUS_MAX >= 2
U_BOOT_I2C_ADAP_COMPLETE(davinci_1, davinci_i2c_init, davinci_i2c_probe,
			 davinci_i2c_read, davinci_i2c_write,
			 davinci_i2c_setspeed,
			 CONFIG_SYS_DAVINCI_I2C_SPEED1,
			 CONFIG_SYS_DAVINCI_I2C_SLAVE1,
			 1)
#endif

#if I2C_BUS_MAX >= 3
U_BOOT_I2C_ADAP_COMPLETE(davinci_2, davinci_i2c_init, davinci_i2c_probe,
			 davinci_i2c_read, davinci_i2c_write,
			 davinci_i2c_setspeed,
			 CONFIG_SYS_DAVINCI_I2C_SPEED2,
			 CONFIG_SYS_DAVINCI_I2C_SLAVE2,
			 2)
#endif
