/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include "designware_i2c.h"

static struct i2c_regs *i2c_get_base(struct i2c_adapter *adap)
{
	switch (adap->hwadapnr) {
#if CONFIG_SYS_I2C_BUS_MAX >= 4
	case 3:
		return (struct i2c_regs *)CONFIG_SYS_I2C_BASE3;
#endif
#if CONFIG_SYS_I2C_BUS_MAX >= 3
	case 2:
		return (struct i2c_regs *)CONFIG_SYS_I2C_BASE2;
#endif
#if CONFIG_SYS_I2C_BUS_MAX >= 2
	case 1:
		return (struct i2c_regs *)CONFIG_SYS_I2C_BASE1;
#endif
	case 0:
		return (struct i2c_regs *)CONFIG_SYS_I2C_BASE;
	default:
		printf("Wrong I2C-adapter number %d\n", adap->hwadapnr);
	}

	return NULL;
}

/*
 * set_speed - Set the i2c speed mode (standard, high, fast)
 * @i2c_spd:	required i2c speed mode
 *
 * Set the i2c speed mode (standard, high, fast)
 */
static void set_speed(struct i2c_adapter *adap, int i2c_spd)
{
	struct i2c_regs *i2c_base = i2c_get_base(adap);
	unsigned int cntl;
	unsigned int hcnt, lcnt;
	unsigned int enbl;

	/* to set speed cltr must be disabled */
	enbl = readl(&i2c_base->ic_enable);
	enbl &= ~IC_ENABLE_0B;
	writel(enbl, &i2c_base->ic_enable);

	cntl = (readl(&i2c_base->ic_con) & (~IC_CON_SPD_MSK));

	switch (i2c_spd) {
	case IC_SPEED_MODE_MAX:
		cntl |= IC_CON_SPD_HS;
		hcnt = (IC_CLK * MIN_HS_SCL_HIGHTIME) / NANO_TO_MICRO;
		writel(hcnt, &i2c_base->ic_hs_scl_hcnt);
		lcnt = (IC_CLK * MIN_HS_SCL_LOWTIME) / NANO_TO_MICRO;
		writel(lcnt, &i2c_base->ic_hs_scl_lcnt);
		break;

	case IC_SPEED_MODE_STANDARD:
		cntl |= IC_CON_SPD_SS;
		hcnt = (IC_CLK * MIN_SS_SCL_HIGHTIME) / NANO_TO_MICRO;
		writel(hcnt, &i2c_base->ic_ss_scl_hcnt);
		lcnt = (IC_CLK * MIN_SS_SCL_LOWTIME) / NANO_TO_MICRO;
		writel(lcnt, &i2c_base->ic_ss_scl_lcnt);
		break;

	case IC_SPEED_MODE_FAST:
	default:
		cntl |= IC_CON_SPD_FS;
		hcnt = (IC_CLK * MIN_FS_SCL_HIGHTIME) / NANO_TO_MICRO;
		writel(hcnt, &i2c_base->ic_fs_scl_hcnt);
		lcnt = (IC_CLK * MIN_FS_SCL_LOWTIME) / NANO_TO_MICRO;
		writel(lcnt, &i2c_base->ic_fs_scl_lcnt);
		break;
	}

	writel(cntl, &i2c_base->ic_con);

	/* Enable back i2c now speed set */
	enbl |= IC_ENABLE_0B;
	writel(enbl, &i2c_base->ic_enable);
}

/*
 * i2c_set_bus_speed - Set the i2c speed
 * @speed:	required i2c speed
 *
 * Set the i2c speed.
 */
static unsigned int dw_i2c_set_bus_speed(struct i2c_adapter *adap,
					 unsigned int speed)
{
	int i2c_spd;

	if (speed >= I2C_MAX_SPEED)
		i2c_spd = IC_SPEED_MODE_MAX;
	else if (speed >= I2C_FAST_SPEED)
		i2c_spd = IC_SPEED_MODE_FAST;
	else
		i2c_spd = IC_SPEED_MODE_STANDARD;

	set_speed(adap, i2c_spd);
	adap->speed = speed;

	return 0;
}

/*
 * i2c_init - Init function
 * @speed:	required i2c speed
 * @slaveaddr:	slave address for the device
 *
 * Initialization function.
 */
static void dw_i2c_init(struct i2c_adapter *adap, int speed,
			int slaveaddr)
{
	struct i2c_regs *i2c_base = i2c_get_base(adap);
	unsigned int enbl;

	/* Disable i2c */
	enbl = readl(&i2c_base->ic_enable);
	enbl &= ~IC_ENABLE_0B;
	writel(enbl, &i2c_base->ic_enable);

	writel((IC_CON_SD | IC_CON_SPD_FS | IC_CON_MM), &i2c_base->ic_con);
	writel(IC_RX_TL, &i2c_base->ic_rx_tl);
	writel(IC_TX_TL, &i2c_base->ic_tx_tl);
	dw_i2c_set_bus_speed(adap, speed);
	writel(IC_STOP_DET, &i2c_base->ic_intr_mask);
	writel(slaveaddr, &i2c_base->ic_sar);

	/* Enable i2c */
	enbl = readl(&i2c_base->ic_enable);
	enbl |= IC_ENABLE_0B;
	writel(enbl, &i2c_base->ic_enable);
}

/*
 * i2c_setaddress - Sets the target slave address
 * @i2c_addr:	target i2c address
 *
 * Sets the target slave address.
 */
static void i2c_setaddress(struct i2c_adapter *adap, unsigned int i2c_addr)
{
	struct i2c_regs *i2c_base = i2c_get_base(adap);
	unsigned int enbl;

	/* Disable i2c */
	enbl = readl(&i2c_base->ic_enable);
	enbl &= ~IC_ENABLE_0B;
	writel(enbl, &i2c_base->ic_enable);

	writel(i2c_addr, &i2c_base->ic_tar);

	/* Enable i2c */
	enbl = readl(&i2c_base->ic_enable);
	enbl |= IC_ENABLE_0B;
	writel(enbl, &i2c_base->ic_enable);
}

/*
 * i2c_flush_rxfifo - Flushes the i2c RX FIFO
 *
 * Flushes the i2c RX FIFO
 */
static void i2c_flush_rxfifo(struct i2c_adapter *adap)
{
	struct i2c_regs *i2c_base = i2c_get_base(adap);

	while (readl(&i2c_base->ic_status) & IC_STATUS_RFNE)
		readl(&i2c_base->ic_cmd_data);
}

/*
 * i2c_wait_for_bb - Waits for bus busy
 *
 * Waits for bus busy
 */
static int i2c_wait_for_bb(struct i2c_adapter *adap)
{
	struct i2c_regs *i2c_base = i2c_get_base(adap);
	unsigned long start_time_bb = get_timer(0);

	while ((readl(&i2c_base->ic_status) & IC_STATUS_MA) ||
	       !(readl(&i2c_base->ic_status) & IC_STATUS_TFE)) {

		/* Evaluate timeout */
		if (get_timer(start_time_bb) > (unsigned long)(I2C_BYTE_TO_BB))
			return 1;
	}

	return 0;
}

static int i2c_xfer_init(struct i2c_adapter *adap, uchar chip, uint addr,
			 int alen)
{
	struct i2c_regs *i2c_base = i2c_get_base(adap);

	if (i2c_wait_for_bb(adap))
		return 1;

	i2c_setaddress(adap, chip);
	while (alen) {
		alen--;
		/* high byte address going out first */
		writel((addr >> (alen * 8)) & 0xff,
		       &i2c_base->ic_cmd_data);
	}
	return 0;
}

static int i2c_xfer_finish(struct i2c_adapter *adap)
{
	struct i2c_regs *i2c_base = i2c_get_base(adap);
	ulong start_stop_det = get_timer(0);

	while (1) {
		if ((readl(&i2c_base->ic_raw_intr_stat) & IC_STOP_DET)) {
			readl(&i2c_base->ic_clr_stop_det);
			break;
		} else if (get_timer(start_stop_det) > I2C_STOPDET_TO) {
			break;
		}
	}

	if (i2c_wait_for_bb(adap)) {
		printf("Timed out waiting for bus\n");
		return 1;
	}

	i2c_flush_rxfifo(adap);

	return 0;
}

/*
 * i2c_read - Read from i2c memory
 * @chip:	target i2c address
 * @addr:	address to read from
 * @alen:
 * @buffer:	buffer for read data
 * @len:	no of bytes to be read
 *
 * Read from i2c memory.
 */
static int dw_i2c_read(struct i2c_adapter *adap, u8 dev, uint addr,
		       int alen, u8 *buffer, int len)
{
	struct i2c_regs *i2c_base = i2c_get_base(adap);
	unsigned long start_time_rx;

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
	dev |= ((addr >> (alen * 8)) & CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW);
	addr &= ~(CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW << (alen * 8));

	debug("%s: fix addr_overflow: dev %02x addr %02x\n", __func__, dev,
	      addr);
#endif

	if (i2c_xfer_init(adap, dev, addr, alen))
		return 1;

	start_time_rx = get_timer(0);
	while (len) {
		if (len == 1)
			writel(IC_CMD | IC_STOP, &i2c_base->ic_cmd_data);
		else
			writel(IC_CMD, &i2c_base->ic_cmd_data);

		if (readl(&i2c_base->ic_status) & IC_STATUS_RFNE) {
			*buffer++ = (uchar)readl(&i2c_base->ic_cmd_data);
			len--;
			start_time_rx = get_timer(0);

		} else if (get_timer(start_time_rx) > I2C_BYTE_TO) {
				return 1;
		}
	}

	return i2c_xfer_finish(adap);
}

/*
 * i2c_write - Write to i2c memory
 * @chip:	target i2c address
 * @addr:	address to read from
 * @alen:
 * @buffer:	buffer for read data
 * @len:	no of bytes to be read
 *
 * Write to i2c memory.
 */
static int dw_i2c_write(struct i2c_adapter *adap, u8 dev, uint addr,
			int alen, u8 *buffer, int len)
{
	struct i2c_regs *i2c_base = i2c_get_base(adap);
	int nb = len;
	unsigned long start_time_tx;

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
	dev |= ((addr >> (alen * 8)) & CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW);
	addr &= ~(CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW << (alen * 8));

	debug("%s: fix addr_overflow: dev %02x addr %02x\n", __func__, dev,
	      addr);
#endif

	if (i2c_xfer_init(adap, dev, addr, alen))
		return 1;

	start_time_tx = get_timer(0);
	while (len) {
		if (readl(&i2c_base->ic_status) & IC_STATUS_TFNF) {
			if (--len == 0) {
				writel(*buffer | IC_STOP,
				       &i2c_base->ic_cmd_data);
			} else {
				writel(*buffer, &i2c_base->ic_cmd_data);
			}
			buffer++;
			start_time_tx = get_timer(0);

		} else if (get_timer(start_time_tx) > (nb * I2C_BYTE_TO)) {
				printf("Timed out. i2c write Failed\n");
				return 1;
		}
	}

	return i2c_xfer_finish(adap);
}

/*
 * i2c_probe - Probe the i2c chip
 */
static int dw_i2c_probe(struct i2c_adapter *adap, u8 dev)
{
	u32 tmp;
	int ret;

	/*
	 * Try to read the first location of the chip.
	 */
	ret = dw_i2c_read(adap, dev, 0, 1, (uchar *)&tmp, 1);
	if (ret)
		dw_i2c_init(adap, adap->speed, adap->slaveaddr);

	return ret;
}

U_BOOT_I2C_ADAP_COMPLETE(dw_0, dw_i2c_init, dw_i2c_probe, dw_i2c_read,
			 dw_i2c_write, dw_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE, 0)

#if CONFIG_SYS_I2C_BUS_MAX >= 2
U_BOOT_I2C_ADAP_COMPLETE(dw_1, dw_i2c_init, dw_i2c_probe, dw_i2c_read,
			 dw_i2c_write, dw_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_SPEED1, CONFIG_SYS_I2C_SLAVE1, 1)
#endif

#if CONFIG_SYS_I2C_BUS_MAX >= 3
U_BOOT_I2C_ADAP_COMPLETE(dw_2, dw_i2c_init, dw_i2c_probe, dw_i2c_read,
			 dw_i2c_write, dw_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_SPEED2, CONFIG_SYS_I2C_SLAVE2, 2)
#endif

#if CONFIG_SYS_I2C_BUS_MAX >= 4
U_BOOT_I2C_ADAP_COMPLETE(dw_3, dw_i2c_init, dw_i2c_probe, dw_i2c_read,
			 dw_i2c_write, dw_i2c_set_bus_speed,
			 CONFIG_SYS_I2C_SPEED3, CONFIG_SYS_I2C_SLAVE3, 3)
#endif
