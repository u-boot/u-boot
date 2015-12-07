/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#if defined(CONFIG_I2C_ENV_EEPROM_BUS)
#include <i2c.h>
#endif
#include <search.h>
#include <errno.h>
#include <linux/compiler.h>	/* for BUG_ON */

DECLARE_GLOBAL_DATA_PTR;

env_t *env_ptr;

char *env_name_spec = "EEPROM";

static int eeprom_bus_read(unsigned dev_addr, unsigned offset,
			   uchar *buffer, unsigned cnt)
{
	int rcode;
#if defined(CONFIG_I2C_ENV_EEPROM_BUS)
	int old_bus = i2c_get_bus_num();

	if (old_bus != CONFIG_I2C_ENV_EEPROM_BUS)
		i2c_set_bus_num(CONFIG_I2C_ENV_EEPROM_BUS);
#endif

	rcode = eeprom_read(dev_addr, offset, buffer, cnt);

#if defined(CONFIG_I2C_ENV_EEPROM_BUS)
	i2c_set_bus_num(old_bus);
#endif

	return rcode;
}

static int eeprom_bus_write(unsigned dev_addr, unsigned offset,
			    uchar *buffer, unsigned cnt)
{
	int rcode;
#if defined(CONFIG_I2C_ENV_EEPROM_BUS)
	int old_bus = i2c_get_bus_num();

	if (old_bus != CONFIG_I2C_ENV_EEPROM_BUS)
		i2c_set_bus_num(CONFIG_I2C_ENV_EEPROM_BUS);
#endif

	rcode = eeprom_write(dev_addr, offset, buffer, cnt);

#if defined(CONFIG_I2C_ENV_EEPROM_BUS)
	i2c_set_bus_num(old_bus);
#endif

	return rcode;
}

uchar env_get_char_spec(int index)
{
	uchar c;
	unsigned int off = CONFIG_ENV_OFFSET;

#ifdef CONFIG_ENV_OFFSET_REDUND
	if (gd->env_valid == 2)
		off = CONFIG_ENV_OFFSET_REDUND;
#endif
	eeprom_bus_read(CONFIG_SYS_DEF_EEPROM_ADDR,
			off + index + offsetof(env_t, data), &c, 1);

	return c;
}

void env_relocate_spec(void)
{
	char buf_env[CONFIG_ENV_SIZE];
	unsigned int off = CONFIG_ENV_OFFSET;

#ifdef CONFIG_ENV_OFFSET_REDUND
	ulong len, crc[2], crc_tmp;
	unsigned int off_env[2];
	uchar rdbuf[64], flags[2];
	int i, crc_ok[2] = {0, 0};

	eeprom_init();	/* prepare for EEPROM read/write */

	off_env[0] = CONFIG_ENV_OFFSET;
	off_env[1] = CONFIG_ENV_OFFSET_REDUND;

	for (i = 0; i < 2; i++) {
		/* read CRC */
		eeprom_bus_read(CONFIG_SYS_DEF_EEPROM_ADDR,
				off_env[i] + offsetof(env_t, crc),
				(uchar *)&crc[i], sizeof(ulong));
		/* read FLAGS */
		eeprom_bus_read(CONFIG_SYS_DEF_EEPROM_ADDR,
				off_env[i] + offsetof(env_t, flags),
				(uchar *)&flags[i], sizeof(uchar));

		crc_tmp = 0;
		len = ENV_SIZE;
		off = off_env[i] + offsetof(env_t, data);
		while (len > 0) {
			int n = (len > sizeof(rdbuf)) ? sizeof(rdbuf) : len;

			eeprom_bus_read(CONFIG_SYS_DEF_EEPROM_ADDR, off,
					rdbuf, n);

			crc_tmp = crc32(crc_tmp, rdbuf, n);
			len -= n;
			off += n;
		}

		if (crc_tmp == crc[i])
			crc_ok[i] = 1;
	}

	if (!crc_ok[0] && !crc_ok[1]) {
		gd->env_addr	= 0;
		gd->env_valid	= 0;
	} else if (crc_ok[0] && !crc_ok[1]) {
		gd->env_valid = 1;
	} else if (!crc_ok[0] && crc_ok[1]) {
		gd->env_valid = 2;
	} else {
		/* both ok - check serial */
		if (flags[0] == ACTIVE_FLAG && flags[1] == OBSOLETE_FLAG)
			gd->env_valid = 1;
		else if (flags[0] == OBSOLETE_FLAG && flags[1] == ACTIVE_FLAG)
			gd->env_valid = 2;
		else if (flags[0] == 0xFF && flags[1] == 0)
			gd->env_valid = 2;
		else if (flags[1] == 0xFF && flags[0] == 0)
			gd->env_valid = 1;
		else /* flags are equal - almost impossible */
			gd->env_valid = 1;
	}

	if (gd->env_valid == 2)
		gd->env_addr = off_env[1] + offsetof(env_t, data);
	else if (gd->env_valid == 1)
		gd->env_addr = off_env[0] + offsetof(env_t, data);

#else /* CONFIG_ENV_OFFSET_REDUND */
	ulong crc, len, new;
	uchar rdbuf[64];

	eeprom_init();	/* prepare for EEPROM read/write */

	/* read old CRC */
	eeprom_bus_read(CONFIG_SYS_DEF_EEPROM_ADDR,
			CONFIG_ENV_OFFSET + offsetof(env_t, crc),
			(uchar *)&crc, sizeof(ulong));

	new = 0;
	len = ENV_SIZE;
	off = offsetof(env_t, data);
	while (len > 0) {
		int n = (len > sizeof(rdbuf)) ? sizeof(rdbuf) : len;

		eeprom_bus_read(CONFIG_SYS_DEF_EEPROM_ADDR,
				CONFIG_ENV_OFFSET + off, rdbuf, n);
		new = crc32(new, rdbuf, n);
		len -= n;
		off += n;
	}

	if (crc == new) {
		gd->env_addr	= offsetof(env_t, data);
		gd->env_valid	= 1;
	} else {
		gd->env_addr	= 0;
		gd->env_valid	= 0;
	}
#endif /* CONFIG_ENV_OFFSET_REDUND */

	off = CONFIG_ENV_OFFSET;
#ifdef CONFIG_ENV_OFFSET_REDUND
	if (gd->env_valid == 2)
		off = CONFIG_ENV_OFFSET_REDUND;
#endif

	eeprom_bus_read(CONFIG_SYS_DEF_EEPROM_ADDR,
		off, (uchar *)buf_env, CONFIG_ENV_SIZE);

	env_import(buf_env, 1);
}

int saveenv(void)
{
	env_t	env_new;
	int	rc;
	unsigned int off	= CONFIG_ENV_OFFSET;
#ifdef CONFIG_ENV_OFFSET_REDUND
	unsigned int off_red	= CONFIG_ENV_OFFSET_REDUND;
	char flag_obsolete	= OBSOLETE_FLAG;
#endif

	BUG_ON(env_ptr != NULL);

	rc = env_export(&env_new);
	if (rc)
		return rc;

#ifdef CONFIG_ENV_OFFSET_REDUND
	if (gd->env_valid == 1) {
		off	= CONFIG_ENV_OFFSET_REDUND;
		off_red	= CONFIG_ENV_OFFSET;
	}

	env_new.flags = ACTIVE_FLAG;
#endif

	rc = eeprom_bus_write(CONFIG_SYS_DEF_EEPROM_ADDR,
			      off, (uchar *)&env_new, CONFIG_ENV_SIZE);

#ifdef CONFIG_ENV_OFFSET_REDUND
	if (rc == 0) {
		eeprom_bus_write(CONFIG_SYS_DEF_EEPROM_ADDR,
				 off_red + offsetof(env_t, flags),
				 (uchar *)&flag_obsolete, 1);

		if (gd->env_valid == 1)
			gd->env_valid = 2;
		else
			gd->env_valid = 1;
	}
#endif
	return rc;
}

/*
 * Initialize Environment use
 *
 * We are still running from ROM, so data use is limited.
 * Use a (moderately small) buffer on the stack
 */
int env_init(void)
{
	gd->env_addr = (ulong)&default_environment[0];
	gd->env_valid = 1;
	return 0;
}
