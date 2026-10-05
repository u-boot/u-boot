// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001, 2002, 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Keith Outwater, keith_outwater@mvis.com`
 * Steven Scholz, steven.scholz@imc-berlin.de
 */

/*
 * Date & Time support (no alarms) for Dallas Semiconductor (now Maxim)
 * DS1307 and DS1338/9 Real Time Clock (RTC).
 *
 * based on ds1337.c
 */

#include <config.h>
#include <command.h>
#include <dm.h>
#include <linux/delay.h>
#include <log.h>
#include <rtc.h>
#include <i2c.h>

enum ds_type {
	ds_1307,
	ds_1337,
	ds_1339,
	ds_1340,
	m41t11,
	mcp794xx,
	rx_8130,
};

struct rtc_ds1370_data {
	enum ds_type type;
	int offset;
};

/*
 * RTC register addresses
 */
#define RTC_SEC_REG_ADDR	0x00
#define RTC_MIN_REG_ADDR	0x01
#define RTC_HR_REG_ADDR		0x02
#define RTC_DAY_REG_ADDR	0x03
#define RTC_DATE_REG_ADDR	0x04
#define RTC_MON_REG_ADDR	0x05
#define RTC_YR_REG_ADDR		0x06
#define RTC_CTL_REG_ADDR	0x07

#define DS1337_CTL_REG_ADDR	0x0e
#define DS1337_STAT_REG_ADDR	0x0f
#define DS1340_STAT_REG_ADDR	0x09

#define RTC_STAT_BIT_OSF	0x80

#define RTC_SEC_BIT_CH		0x80	/* Clock Halt (in Register 0)   */

/* DS1307-specific bits */
#define RTC_CTL_BIT_RS0		0x01	/* Rate select 0                */
#define RTC_CTL_BIT_RS1		0x02	/* Rate select 1                */
#define RTC_CTL_BIT_SQWE	0x10	/* Square Wave Enable           */
#define RTC_CTL_BIT_OUT		0x80	/* Output Control               */

/* DS1337-specific bits */
#define DS1337_CTL_BIT_RS1	0x08	/* Rate select 1                */
#define DS1337_CTL_BIT_RS2	0x10	/* Rate select 2                */
#define DS1337_CTL_BIT_EOSC	0x80	/* Enable Oscillator            */

/* DS1340-specific bits */
#define DS1340_SEC_BIT_EOSC	0x80	/* Enable Oscillator            */
#define DS1340_CTL_BIT_OUT	0x80	/* Output Control               */

/* MCP7941X-specific bits */
#define MCP7941X_BIT_ST		0x80
#define MCP7941X_BIT_VBATEN	0x08

/* RX8130-specific bits */
#define RX8130_REG_ALARM_MIN		0x17
#define RX8130_REG_ALARM_HOUR		0x18
#define RX8130_REG_ALARM_WEEK_OR_DAY	0x19
#define RX8130_REG_EXTENSION		0x1c
#define RX8130_REG_EXTENSION_TE		BIT(4)
#define RX8130_REG_EXTENSION_USEL	BIT(5)
#define RX8130_REG_EXTENSION_FSEL0	BIT(6)
#define RX8130_REG_EXTENSION_FESL1	BIT(7)
#define RX8130_REG_FLAG			0x1d
#define RX8130_REG_FLAG_VLF		BIT(1)
#define RX8130_REG_FLAG_AF		BIT(3)
#define RX8130_REG_CONTROL0		0x1e
#define RX8130_REG_CONTROL0_AIE		BIT(3)
#define RX8130_REG_CONTROL0_TIE		BIT(4)
#define RX8130_REG_CONTROL0_UIE		BIT(5)
#define RX8130_REG_CONTROL0_STOP	BIT(6)
#define RX8130_REG_CONTROL0_TEST	BIT(7)
#define RX8130_REG_CONTROL1		0x1f
#define RX8130_REG_CONTROL1_INIEN	BIT(4)
#define RX8130_REG_CONTROL1_CHGEN	BIT(5)

static int ds1307_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	int ret;
	uchar buf[7];
	struct rtc_ds1370_data *data = (void*)dev_get_driver_data(dev);

	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
	      tm->tm_hour, tm->tm_min, tm->tm_sec);

	if (tm->tm_year < 1970 || tm->tm_year > 2069)
		printf("WARNING: year should be between 1970 and 2069!\n");

	buf[RTC_YR_REG_ADDR] = bin2bcd(tm->tm_year % 100);
	buf[RTC_MON_REG_ADDR] = bin2bcd(tm->tm_mon);
	/* rx8130 is bit position, not BCD */
	if (data->type == rx_8130)
		buf[RTC_DAY_REG_ADDR] = 1 << tm->tm_wday;
	else
		buf[RTC_DAY_REG_ADDR] = bin2bcd(tm->tm_wday + 1);
	buf[RTC_DATE_REG_ADDR] = bin2bcd(tm->tm_mday);
	buf[RTC_HR_REG_ADDR] = bin2bcd(tm->tm_hour);
	buf[RTC_MIN_REG_ADDR] = bin2bcd(tm->tm_min);
	buf[RTC_SEC_REG_ADDR] = bin2bcd(tm->tm_sec);

	if (data->type == mcp794xx) {
		buf[RTC_DAY_REG_ADDR] |= MCP7941X_BIT_VBATEN;
		buf[RTC_SEC_REG_ADDR] |= MCP7941X_BIT_ST;
	} else if (data->type == rx_8130) {
		ret = dm_i2c_reg_clrset(dev, RX8130_REG_CONTROL0, 0, RX8130_REG_CONTROL0_STOP);
		if (ret < 0)
			return ret;
	}

	ret = dm_i2c_write(dev, data->offset, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	if (data->type == ds_1337) {
		/* Ensure oscillator is enabled */
		dm_i2c_reg_write(dev, data->offset + DS1337_CTL_REG_ADDR, 0);
	} else if (data->type == rx_8130) {
		ret = dm_i2c_reg_clrset(dev, RX8130_REG_CONTROL0, RX8130_REG_CONTROL0_STOP, 0);
		if (ret < 0)
			return ret;

		/* clear Voltage Loss Flag as data is available now */
		ret = dm_i2c_reg_clrset(dev, RX8130_REG_FLAG, RX8130_REG_FLAG_VLF, 0);
		if (ret < 0) {
			printf("RTC: write VLF error %d\n", ret);
			return ret;
		}
	}

	return 0;
}

static int ds1307_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	int ret;
	uchar buf[7];
	struct rtc_ds1370_data *data = (void*)dev_get_driver_data(dev);

	ret = dm_i2c_read(dev, data->offset, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	if (data->type == ds_1337 || data->type == ds_1339 || data->type == ds_1340) {
		uint reg = (data->type == ds_1340) ? DS1340_STAT_REG_ADDR :
					       DS1337_STAT_REG_ADDR;
		int status = dm_i2c_reg_read(dev, data->offset + reg);

		if (status >= 0 && (status & RTC_STAT_BIT_OSF)) {
			printf("### Warning: RTC oscillator has stopped\n");
			/* clear the OSF flag */
			dm_i2c_reg_write(dev, data->offset + reg, status & ~RTC_STAT_BIT_OSF);
		}
	} else if (data->type == rx_8130) {
		ret = dm_i2c_reg_read(dev, RX8130_REG_FLAG);
		if (ret < 0) {
			printf("RTC: read error %d\n", ret);
			return ret;
		}

		if (ret & RX8130_REG_FLAG_VLF) {
			printf("RTC: oscillator failed, set time!\n");
			return -EINVAL;
		}
	}

	tm->tm_sec  = bcd2bin(buf[RTC_SEC_REG_ADDR] & 0x7F);
	tm->tm_min  = bcd2bin(buf[RTC_MIN_REG_ADDR] & 0x7F);
	tm->tm_hour = bcd2bin(buf[RTC_HR_REG_ADDR] & 0x3F);
	tm->tm_mday = bcd2bin(buf[RTC_DATE_REG_ADDR] & 0x3F);
	tm->tm_mon  = bcd2bin(buf[RTC_MON_REG_ADDR] & 0x1F);
	tm->tm_year = bcd2bin(buf[RTC_YR_REG_ADDR]) +
			      (bcd2bin(buf[RTC_YR_REG_ADDR]) >= 70 ?
			       1900 : 2000);
	/* rx8130 is bit position, not BCD */
	if (data->type == rx_8130)
		tm->tm_wday = fls(buf[RTC_DAY_REG_ADDR] & 0x07);
	else
		tm->tm_wday = bcd2bin((buf[RTC_DAY_REG_ADDR] - 1) & 0x07);
	tm->tm_yday = 0;
	tm->tm_isdst = 0;

	debug("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
	      tm->tm_hour, tm->tm_min, tm->tm_sec);

	return 0;
}

int ds1307_rx8130_rtc_reset(struct udevice *dev) {
	/*
	 * RX8130CE manual, 18.2 Software Reset
	 */
	const uint8_t reg_address[]  = { 0x1e, 0x1e, 0x50, 0x53, 0x66, 0x6b, 0x6b };
	const uint8_t reg_sequence[] = { 0x00, 0x80, 0x6c, 0x01, 0x03, 0x02, 0x01 };
	int ret = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(reg_sequence); ++i) {
		ret = dm_i2c_reg_write(dev, reg_address[i], reg_sequence[i]);
		if (ret < 0)
			return ret;
	}

	mdelay(150);

	/* Dummy read before reading FLAG register */
	ret = dm_i2c_reg_read(dev, RX8130_REG_FLAG);
	ret = dm_i2c_reg_read(dev, RX8130_REG_FLAG);
	if (ret < 0)
		return ret;

	while (ret >= 0 && (ret & RX8130_REG_FLAG_VLF)) {
		ret &= ~RX8130_REG_FLAG_VLF;
		dm_i2c_reg_write(dev, RX8130_REG_FLAG, ret);
		mdelay(1);
		ret = dm_i2c_reg_read(dev, RX8130_REG_FLAG);
	}
	printf("rx8130 VLF cleared: %d\n", ret);

	/* Clear TE bit and Disable FOUT */
	if (ret >= 0)
		ret = dm_i2c_reg_clrset(dev, RX8130_REG_EXTENSION, RX8130_REG_EXTENSION_TE,
					RX8130_REG_EXTENSION_FSEL0 | RX8130_REG_EXTENSION_FESL1);

	/* Clear VLF bit */
	if (ret >= 0)
		ret = dm_i2c_reg_clrset(dev, RX8130_REG_FLAG, RX8130_REG_FLAG_VLF, 0);

	/* Clear TEST, AIE, TIE, UIE for inhibit interrupt output of suddenness */
	if (ret >= 0)
		ret = dm_i2c_reg_clrset(dev, RX8130_REG_CONTROL0, RX8130_REG_CONTROL0_TEST |
					RX8130_REG_CONTROL0_AIE |  RX8130_REG_CONTROL0_TIE |
					RX8130_REG_CONTROL0_UIE, 0);

	/* Set INIEN to 1 */
	if (ret >= 0)
		ret = dm_i2c_reg_clrset(dev, RX8130_REG_CONTROL1, 0, RX8130_REG_CONTROL1_INIEN);

	/* Set CHGEN to 0 */
	if (ret >= 0)
		ret = dm_i2c_reg_clrset(dev, RX8130_REG_CONTROL1, RX8130_REG_CONTROL1_CHGEN, 0);

	/* Start clock */
	if (ret >= 0)
		ret = dm_i2c_reg_clrset(dev, RX8130_REG_CONTROL0, RX8130_REG_CONTROL0_STOP, 0);

	return ret;
}

static int ds1307_rtc_reset(struct udevice *dev)
{
	int ret;
	struct rtc_ds1370_data *data = (void*)dev_get_driver_data(dev);

	if (data->type == rx_8130) {
		ret = ds1307_rx8130_rtc_reset(dev);
		if (ret < 0)
			return ret;
	}

	/*
	 * reset clock/oscillator in the seconds register:
	 * on DS1307 bit 7 enables Clock Halt (CH),
	 * on DS1340 bit 7 disables the oscillator (not EOSC)
	 * on MCP794xx bit 7 enables Start Oscillator (ST)
	 */
	ret = dm_i2c_reg_write(dev, data->offset + RTC_SEC_REG_ADDR, 0x00);
	if (ret < 0)
		return ret;

	if (data->type == ds_1307) {
		/* Write control register in order to enable square-wave
		 * output (SQWE) and set a default rate of 32.768kHz (RS1|RS0).
		 */
		ret = dm_i2c_reg_write(dev, data->offset + RTC_CTL_REG_ADDR,
				       RTC_CTL_BIT_SQWE | RTC_CTL_BIT_RS1 |
				       RTC_CTL_BIT_RS0);
	} else if (data->type == ds_1337) {
		/* Write control register in order to enable oscillator output
		 * (not EOSC) and set a default rate of 32.768kHz (RS2|RS1).
		 */
		ret = dm_i2c_reg_write(dev, data->offset + DS1337_CTL_REG_ADDR,
				       DS1337_CTL_BIT_RS2 | DS1337_CTL_BIT_RS1);
	} else if (data->type == ds_1340 || data->type == mcp794xx || data->type == m41t11) {
		/* Reset clock calibration, frequency test and output level. */
		ret = dm_i2c_reg_write(dev, data->offset + RTC_CTL_REG_ADDR, 0x00);
	}

	return ret;
}

static int ds1307_probe(struct udevice *dev)
{
	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS |
			   DM_I2C_CHIP_WR_ADDRESS);

	return 0;
}

static const struct rtc_ops ds1307_rtc_ops = {
	.get = ds1307_rtc_get,
	.set = ds1307_rtc_set,
	.reset = ds1307_rtc_reset,
};

static const struct rtc_ds1370_data ds_1307_data = {
	.type   = ds_1307,
	.offset = 0,
};

static const struct rtc_ds1370_data ds_1337_data = {
	.type   = ds_1337,
	.offset = 0,
};

static const struct rtc_ds1370_data ds_1339_data = {
	.type   = ds_1339,
	.offset = 0,
};

static const struct rtc_ds1370_data ds_1340_data = {
	.type   = ds_1340,
	.offset = 0,
};

static const struct rtc_ds1370_data rx_8130_data = {
	.type   = rx_8130,
	.offset = 0x10,
};

static const struct rtc_ds1370_data mcp794xx_data = {
	.type   = mcp794xx,
	.offset = 0,
};

static const struct rtc_ds1370_data m41t11_data = {
	.type   = m41t11,
	.offset = 0,
};

static const struct udevice_id ds1307_rtc_ids[] = {
	{ .compatible = "dallas,ds1307", .data = (ulong)&ds_1307_data },
	{ .compatible = "dallas,ds1337", .data = (ulong)&ds_1337_data },
	{ .compatible = "dallas,ds1339", .data = (ulong)&ds_1339_data },
	{ .compatible = "dallas,ds1340", .data = (ulong)&ds_1340_data },
	{ .compatible = "epson,rx8130", .data = (ulong)&rx_8130_data },
	{ .compatible = "microchip,mcp7940x", .data = (ulong)&mcp794xx_data },
	{ .compatible = "microchip,mcp7941x", .data = (ulong)&mcp794xx_data },
	{ .compatible = "st,m41t11", .data = (ulong)&m41t11_data },
	{ }
};

U_BOOT_DRIVER(rtc_ds1307) = {
	.name	= "rtc-ds1307",
	.id	= UCLASS_RTC,
	.probe	= ds1307_probe,
	.of_match = ds1307_rtc_ids,
	.ops	= &ds1307_rtc_ops,
};
