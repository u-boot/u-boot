/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include <post.h>
#include <common.h>

/*
 * SYSMON test
 *
 * This test performs the system hardware monitoring.
 * The test passes when all the following voltages and temperatures
 * are within allowed ranges:
 *
 * Board temperature
 * Front temperature
 * +3.3V CPU logic
 * +5V logic
 * +12V PCMCIA
 * +12V CCFL
 * +5V standby
 *
 * CCFL is not enabled if temperature values are not within allowed ranges
 *
 * See the list off all parameters in the sysmon_table below
 */

#include <post.h>
#include <watchdog.h>
#include <i2c.h>

#if CONFIG_POST & CONFIG_SYS_POST_SYSMON

DECLARE_GLOBAL_DATA_PTR;

static int sysmon_temp_invalid = 0;

/* #define DEBUG */

#define	RELOC(x) if (x != NULL) x = (void *) ((ulong) (x) + gd->reloc_off)

typedef struct sysmon_s sysmon_t;
typedef struct sysmon_table_s sysmon_table_t;

static void sysmon_lm87_init (sysmon_t * this);
static void sysmon_pic_init (sysmon_t * this);
static uint sysmon_i2c_read (sysmon_t * this, uint addr);
static uint sysmon_i2c_read_sgn (sysmon_t * this, uint addr);
static void sysmon_ccfl_disable (sysmon_table_t * this);
static void sysmon_ccfl_enable (sysmon_table_t * this);

struct sysmon_s
{
	uchar	chip;
	void	(*init)(sysmon_t *);
	uint	(*read)(sysmon_t *, uint);
};

static sysmon_t sysmon_lm87 =
	{CONFIG_SYS_I2C_SYSMON_ADDR, sysmon_lm87_init, sysmon_i2c_read};
static sysmon_t sysmon_lm87_sgn =
	{CONFIG_SYS_I2C_SYSMON_ADDR, sysmon_lm87_init, sysmon_i2c_read_sgn};
static sysmon_t sysmon_pic =
	{CONFIG_SYS_I2C_PICIO_ADDR, sysmon_pic_init, sysmon_i2c_read};

static sysmon_t * sysmon_list[] =
{
	&sysmon_lm87,
	&sysmon_lm87_sgn,
	&sysmon_pic,
	NULL
};

struct sysmon_table_s
{
	char *		name;
	char *		unit_name;
	sysmon_t *	sysmon;
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

static sysmon_table_t sysmon_table[] =
{
    {"Board temperature", " C", &sysmon_lm87_sgn, NULL, sysmon_ccfl_disable,
     1, 1, -128, 127, 0xFF, 0x58, 0xD5, 0, 0x6C, 0xC6, 0, 0x27},

    {"Front temperature", " C", &sysmon_lm87, NULL, sysmon_ccfl_disable,
     1, 100, -27316, 8984, 0xFF, 0xA4, 0xFC, 0, 0xB2, 0xF1, 0, 0x29},

    {"+3.3V CPU logic", "V", &sysmon_lm87, NULL, NULL,
     100, 1000, 0, 4386, 0xFF, 0xB6, 0xC9, 0, 0xB6, 0xC9, 0, 0x22},

    {"+ 5 V logic", "V", &sysmon_lm87, NULL, NULL,
     100, 1000, 0, 6630, 0xFF, 0xB6, 0xCA, 0, 0xB6, 0xCA, 0, 0x23},

    {"+12 V PCMCIA", "V", &sysmon_lm87, NULL, NULL,
     100, 1000, 0, 15460, 0xFF, 0xBC, 0xD0, 0, 0xBC, 0xD0, 0, 0x21},

    {"+12 V CCFL", "V", &sysmon_lm87, NULL, sysmon_ccfl_enable,
     100, 1000, 0, 15900, 0xFF, 0xB6, 0xCA, 0, 0xB6, 0xCA, 0, 0x24},

    {"+ 5 V standby", "V", &sysmon_pic, NULL, NULL,
     100, 1000, 0, 6040, 0xFF, 0xC8, 0xDE, 0, 0xC8, 0xDE, 0, 0x7C},
};
static int sysmon_table_size = sizeof(sysmon_table) / sizeof(sysmon_table[0]);

static int conversion_done = 0;


int sysmon_init_f (void)
{
	sysmon_t ** l;
	ulong reg;

	/* Power on CCFL, PCMCIA */
	reg = pic_read  (0x60);
	reg |= 0x09;
	pic_write (0x60, reg);

	for (l = sysmon_list; *l; l++) {
		(*l)->init(*l);
	}

	return 0;
}

void sysmon_reloc (void)
{
	sysmon_t ** l;
	sysmon_table_t * t;

	for (l = sysmon_list; *l; l++) {
		RELOC(*l);
		RELOC((*l)->init);
		RELOC((*l)->read);
	}

	for (t = sysmon_table; t < sysmon_table + sysmon_table_size; t ++) {
		RELOC(t->exec_before);
		RELOC(t->exec_after);
		RELOC(t->sysmon);
	}
}

static char *sysmon_unit_value (sysmon_table_t *s, uint val)
{
	static char buf[32];
	int unit_val =
	    s->unit_min + (s->unit_max - s->unit_min) * val / s->val_mask;
	char *p, sign;
	int dec, frac;

	if (val == -1) {
		return "I/O ERROR";
	}

	if (unit_val < 0) {
		sign = '-';
		unit_val = -unit_val;
	} else {
		sign = '+';
	}

	p = buf + sprintf(buf, "%c%2d", sign, unit_val / s->unit_div);


	frac = unit_val % s->unit_div;

	frac /= (s->unit_div / s->unit_precision);

	dec = s->unit_precision;

	if (dec != 1) {
		*p++ = '.';
	}
	for (dec /= 10; dec != 0; dec /= 10) {
		*p++ = '0' + (frac / dec) % 10;
	}
	strcpy(p, s->unit_name);

	return buf;
}

static void sysmon_lm87_init (sysmon_t * this)
{
	uchar val;

	/* Detect LM87 chip */
	if (i2c_read(this->chip, 0x40, 1, &val, 1) || (val & 0x80) != 0 ||
	    i2c_read(this->chip, 0x3E, 1, &val, 1) || val != 0x02) {
		printf("Error: LM87 not found at 0x%02X\n", this->chip);
		return;
	}

	/* Configure pins 5,6 as AIN */
	val = 0x03;
	if (i2c_write(this->chip, 0x16, 1, &val, 1)) {
		printf("Error: can't write LM87 config register\n");
		return;
	}

	/* Start monitoring */
	val = 0x01;
	if (i2c_write(this->chip, 0x40, 1, &val, 1)) {
		printf("Error: can't write LM87 config register\n");
		return;
	}
}

static void sysmon_pic_init (sysmon_t * this)
{
}

static uint sysmon_i2c_read (sysmon_t * this, uint addr)
{
	uchar val;
	uint res = i2c_read(this->chip, addr, 1, &val, 1);

	return res == 0 ? val : -1;
}

static uint sysmon_i2c_read_sgn (sysmon_t * this, uint addr)
{
	uchar val;
	return i2c_read(this->chip, addr, 1, &val, 1) == 0 ?
		128 + (signed char)val : -1;
}

static void sysmon_ccfl_disable (sysmon_table_t * this)
{
	if (!this->val_valid_alt) {
		sysmon_temp_invalid = 1;
	}
}

static void sysmon_ccfl_enable (sysmon_table_t * this)
{
	ulong reg;

	if (!sysmon_temp_invalid) {
		reg = pic_read  (0x60);
		reg |= 0x06;
		pic_write (0x60, reg);
	}
}

int sysmon_post_test (int flags)
{
	int res = 0;
	sysmon_table_t * t;
	uint val;

	/*
	 * The A/D conversion on the LM87 sensor takes 300 ms.
	 */
	if (! conversion_done) {
		while (post_time_ms(gd->post_init_f_time) < 300) WATCHDOG_RESET ();
		conversion_done = 1;
	}

	for (t = sysmon_table; t < sysmon_table + sysmon_table_size; t ++) {
		if (t->exec_before) {
			t->exec_before(t);
		}

		val = t->sysmon->read(t->sysmon, t->addr);
		if (val != -1) {
			t->val_valid = val >= t->val_min && val <= t->val_max;
			t->val_valid_alt = val >= t->val_min_alt && val <= t->val_max_alt;
		} else {
			t->val_valid = 0;
			t->val_valid_alt = 0;
		}

		if (t->exec_after) {
			t->exec_after(t);
		}

		if ((!t->val_valid) || (flags & POST_MANUAL)) {
			printf("%-17s = %-10s ", t->name, sysmon_unit_value(t, val));
			printf("allowed range");
			printf(" %-8s ..", sysmon_unit_value(t, t->val_min));
			printf(" %-8s", sysmon_unit_value(t, t->val_max));
			printf("     %s\n", t->val_valid ? "OK" : "FAIL");
		}

		if (!t->val_valid) {
			res = -1;
		}
	}

	return res;
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_SYSMON */
