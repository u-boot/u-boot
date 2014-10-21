/*
 *  Copyright (C) 2011-2012 Samsung Electronics
 *  Lukasz Majewski <l.majewski@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CORE_PMIC_H_
#define __CORE_PMIC_H_

#include <linux/list.h>
#include <i2c.h>
#include <power/power_chrg.h>

enum { PMIC_I2C, PMIC_SPI, PMIC_NONE};
enum { I2C_PMIC, I2C_NUM, };
enum { PMIC_READ, PMIC_WRITE, };
enum { PMIC_SENSOR_BYTE_ORDER_LITTLE, PMIC_SENSOR_BYTE_ORDER_BIG, };

enum {
	PMIC_CHARGER_DISABLE,
	PMIC_CHARGER_ENABLE,
};

struct p_i2c {
	unsigned char addr;
	unsigned char *buf;
	unsigned char tx_num;
};

struct p_spi {
	unsigned int cs;
	unsigned int mode;
	unsigned int bitlen;
	unsigned int clk;
	unsigned int flags;
	u32 (*prepare_tx)(u32 reg, u32 *val, u32 write);
};

struct pmic;
struct power_fg {
	int (*fg_battery_check) (struct pmic *p, struct pmic *bat);
	int (*fg_battery_update) (struct pmic *p, struct pmic *bat);
};

struct power_chrg {
	int (*chrg_type) (struct pmic *p);
	int (*chrg_bat_present) (struct pmic *p);
	int (*chrg_state) (struct pmic *p, int state, int current);
};

struct power_battery {
	struct battery *bat;
	int (*battery_init) (struct pmic *bat, struct pmic *p1,
			     struct pmic *p2, struct pmic *p3);
	int (*battery_charge) (struct pmic *bat);
	/* Keep info about power devices involved with battery operation */
	struct pmic *chrg, *fg, *muic;
};

struct pmic {
	const char *name;
	unsigned char bus;
	unsigned char interface;
	unsigned char sensor_byte_order;
	unsigned int number_of_regs;
	union hw {
		struct p_i2c i2c;
		struct p_spi spi;
	} hw;

	void (*low_power_mode) (void);
	struct power_battery *pbat;
	struct power_chrg *chrg;
	struct power_fg *fg;

	struct pmic *parent;
	struct list_head list;
};

int pmic_init(unsigned char bus);
int power_init_board(void);
int pmic_dialog_init(unsigned char bus);
int check_reg(struct pmic *p, u32 reg);
struct pmic *pmic_alloc(void);
struct pmic *pmic_get(const char *s);
int pmic_probe(struct pmic *p);
int pmic_reg_read(struct pmic *p, u32 reg, u32 *val);
int pmic_reg_write(struct pmic *p, u32 reg, u32 val);
int pmic_set_output(struct pmic *p, u32 reg, int ldo, int on);

#define pmic_i2c_addr (p->hw.i2c.addr)
#define pmic_i2c_tx_num (p->hw.i2c.tx_num)

#define pmic_spi_bitlen (p->hw.spi.bitlen)
#define pmic_spi_flags (p->hw.spi.flags)

#endif /* __CORE_PMIC_H_ */
