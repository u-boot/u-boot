/*
 * (C) Copyright 2008 Dmitry Rakhchev, EmCraft Systems, rda@emcraft.com
 *
 * Developed for DENX Software Engineering GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <post.h>
#include <common.h>

/*
 * SYSMON test
 *
 * This test performs the system hardware monitoring.
 * The test passes when all the following voltages and temperatures
 * are within allowed ranges:
 *
 * Temperature		  -40 .. +90 C
 * +5V			+4.50 .. +5.50 V
 * +5V standby		+3.50 .. +5.50 V
 *
 * LCD backlight is not enabled if temperature values are not within
 * allowed ranges (-30 .. + 80). The brightness of backlite can be
 * controlled by setting "brightness" environment variable. Default value is 50%
 *
 * See the list of all parameters in the sysmon_table below
 */

#include <post.h>
#include <watchdog.h>
#include <i2c.h>

#if defined(CONFIG_VIDEO)
#include <mb862xx.h>
#endif

#if CONFIG_POST & CONFIG_SYS_POST_SYSMON

DECLARE_GLOBAL_DATA_PTR;

/* from dspic.c */
extern int dspic_read(ushort reg, ushort *data);

#define REG_TEMPERATURE			0x12BC
#define REG_VOLTAGE_5V			0x12CA
#define REG_VOLTAGE_5V_STANDBY		0x12C6

#define TEMPERATURE_MIN			(-40)	/* degr. C */
#define TEMPERATURE_MAX			(+90)	/* degr. C */
#define TEMPERATURE_DISPLAY_MIN		(-35)	/* degr. C */
#define TEMPERATURE_DISPLAY_MAX		(+85)	/* degr. C */

#define VOLTAGE_5V_MIN			(+4500)	/* mV */
#define VOLTAGE_5V_MAX			(+5500)	/* mV */

#define VOLTAGE_5V_STANDBY_MIN		(+3500)	/* mV */
#define VOLTAGE_5V_STANDBY_MAX		(+5500)	/* mV */

typedef struct sysmon_s sysmon_t;
typedef struct sysmon_table_s sysmon_table_t;

static void sysmon_dspic_init(sysmon_t *this);
static int sysmon_dspic_read(sysmon_t *this, uint addr, int *val);
static int sysmon_dspic_read_sgn(sysmon_t *this, uint addr,  int *val);
static void sysmon_backlight_disable(sysmon_table_t *this);

struct sysmon_s {
	uchar	chip;
	void	(*init)(sysmon_t *);
	int	(*read)(sysmon_t *, uint, int *);
};

static sysmon_t sysmon_dspic = {
	CONFIG_SYS_I2C_DSPIC_IO_ADDR,
	sysmon_dspic_init,
	sysmon_dspic_read
};

static sysmon_t sysmon_dspic_sgn = {
	CONFIG_SYS_I2C_DSPIC_IO_ADDR,
	sysmon_dspic_init,
	sysmon_dspic_read_sgn
};

static sysmon_t *sysmon_list[] = {
	&sysmon_dspic,
	NULL
};

struct sysmon_table_s {
	char		*name;
	char		*unit_name;
	sysmon_t	*sysmon;
	void		(*exec_before)(sysmon_table_t *);
	void		(*exec_after)(sysmon_table_t *);

	int		unit_precision;
	int		unit_div;
	int		unit_min;
	int		unit_max;
	uint		val_mask;
	uint		val_min;
	uint		val_max;
	int		val_valid;
	uint		val_min_alt;
	uint		val_max_alt;
	int		val_valid_alt;
	uint		addr;
};

static sysmon_table_t sysmon_table[] = {
	{
		"Temperature", " C", &sysmon_dspic, NULL, sysmon_backlight_disable,
		1, 1, -32768, 32767, 0xFFFF,
		0x8000 + TEMPERATURE_MIN,	  0x8000 + TEMPERATURE_MAX,	    0,
		0x8000 + TEMPERATURE_DISPLAY_MIN, 0x8000 + TEMPERATURE_DISPLAY_MAX, 0,
		REG_TEMPERATURE,
	},

	{
		"+ 5 V", "V", &sysmon_dspic, NULL, NULL,
		100, 1000, -0x8000, 0x7FFF, 0xFFFF,
		0x8000 + VOLTAGE_5V_MIN, 0x8000 + VOLTAGE_5V_MAX, 0,
		0x8000 + VOLTAGE_5V_MIN, 0x8000 + VOLTAGE_5V_MAX, 0,
		REG_VOLTAGE_5V,
	},

	{
		"+ 5 V standby", "V", &sysmon_dspic, NULL, NULL,
		100, 1000, -0x8000, 0x7FFF, 0xFFFF,
		0x8000 + VOLTAGE_5V_STANDBY_MIN, 0x8000 + VOLTAGE_5V_STANDBY_MAX, 0,
		0x8000 + VOLTAGE_5V_STANDBY_MIN, 0x8000 + VOLTAGE_5V_STANDBY_MAX, 0,
		REG_VOLTAGE_5V_STANDBY,
	},

	{
		"Temperature", "°C", &sysmon_dspic_sgn, NULL, sysmon_backlight_disable,
		1, 1, -32768, 32767, 0xFFFF,
		0x8000 + TEMPERATURE_MIN,	  0x8000 + TEMPERATURE_MAX,	    0,
		0x8000 + TEMPERATURE_DISPLAY_MIN, 0x8000 + TEMPERATURE_DISPLAY_MAX, 0,
		REG_TEMPERATURE,
	},
};

int sysmon_init_f(void)
{
	sysmon_t **l;

	for (l = sysmon_list; *l; l++)
		(*l)->init(*l);

	return 0;
}

void sysmon_reloc(void)
{
	/* Do nothing for now, sysmon_reloc() is required by the sysmon post */
}

static char *sysmon_unit_value(sysmon_table_t *s, uint val)
{
	static char buf[32];
	char *p, sign;
	int decimal, frac;
	int unit_val;

	unit_val = s->unit_min + (s->unit_max - s->unit_min) * val / s->val_mask;

	if (val == -1)
		return "I/O ERROR";

	if (unit_val < 0) {
		sign = '-';
		unit_val = -unit_val;
	} else {
		sign = '+';
	}

	p = buf + sprintf(buf, "%c%2d", sign, unit_val / s->unit_div);

	frac = unit_val % s->unit_div;
	frac /= (s->unit_div / s->unit_precision);

	decimal = s->unit_precision;

	if (decimal != 1)
		*p++ = '.';
	for (decimal /= 10; decimal != 0; decimal /= 10)
		*p++ = '0' + (frac / decimal) % 10;
	strcpy(p, s->unit_name);

	return buf;
}

static void sysmon_dspic_init(sysmon_t *this)
{
}

static int sysmon_dspic_read(sysmon_t *this, uint addr, int *val)
{
	ushort data;

	if (dspic_read(addr, &data) == 0){
		/* To fit into the table range we should add 0x8000 */
		*val = data + 0x8000;
		return 0;
	}

	return -1;
}

static int sysmon_dspic_read_sgn(sysmon_t *this, uint addr, int *val)
{
	ushort data;

	if (dspic_read(addr, &data) == 0){
		/* To fit into the table range we should add 0x8000 */
		*val = (signed short)data + 0x8000;
		return 0;
	}

	return -1;
}

static void sysmon_backlight_disable(sysmon_table_t *this)
{
#if defined(CONFIG_VIDEO)
	board_backlight_switch(this->val_valid_alt);
#endif
}

int sysmon_post_test(int flags)
{
	int res = 0;
	sysmon_table_t * t;
	int val;

	for (t = sysmon_table; t < sysmon_table + ARRAY_SIZE(sysmon_table); t++) {
		t->val_valid = 1;
		if (t->exec_before)
			t->exec_before(t);

		if (t->sysmon->read(t->sysmon, t->addr, &val) != 0) {
			t->val_valid = 0;
			t->val_valid_alt = 0;
			post_log(": read failed\n");
			res = 1;
			break;
		}

		if (t->val_valid != 0) {
			t->val_valid = val >= t->val_min && val <= t->val_max;
			t->val_valid_alt = val >= t->val_min_alt && val <= t->val_max_alt;
		}

		if (t->exec_after)
			t->exec_after(t);

		if ((!t->val_valid) || (flags)) {
			post_log("\n\t%-17s = %-10s ", t->name, sysmon_unit_value(t, val));
			post_log("allowed range");
			post_log(" %-8s ..", sysmon_unit_value(t, t->val_min));
			post_log(" %-8s", sysmon_unit_value(t, t->val_max));
			post_log("     %s", t->val_valid ? "OK" : "FAIL");
		}

		if (!t->val_valid) {
			res = 1;
			break;
		}
	}
	post_log("\n");

	return res;
}
#endif /* CONFIG_POST & CONFIG_SYS_POST_SYSMON */
