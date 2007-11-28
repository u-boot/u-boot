/*
 * (C) Copyright 2003-2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2004
 * Mark Jonas, Freescale Semiconductor, mark.jonas@motorola.com.
 *
 * (C) Copyright 2004-2005
 * Martin Krause, TQ-Systems GmbH, martin.krause@tqs.de
 *
 * Adapted to U-Boot 1.2 by:
 *   Bartlomiej Sieka <tur@semihalf.com>:
 *      - HW ID readout from EEPROM
 *      - module detection
 *   Grzegorz Bernacki <gjb@semihalf.com>:
 *      - run-time SDRAM controller configuration
 *      - LIBFDT support
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <mpc5xxx.h>
#include <pci.h>
#include <asm/processor.h>
#include <i2c.h>
#include <linux/ctype.h>

#ifdef CONFIG_OF_LIBFDT
#include <libfdt.h>
#include <libfdt_env.h>
#include <fdt_support.h>
#endif /* CONFIG_OF_LIBFDT */


#include "cm5200.h"
#include "fwupdate.h"

DECLARE_GLOBAL_DATA_PTR;

static hw_id_t hw_id;


#ifndef CFG_RAMBOOT
/*
 * Helper function to initialize SDRAM controller.
 */
static void sdram_start(int hi_addr, mem_conf_t *mem_conf)
{
	long hi_addr_bit = hi_addr ? 0x01000000 : 0;

	/* unlock mode register */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = mem_conf->control | 0x80000000 |
						hi_addr_bit;

	/* precharge all banks */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = mem_conf->control | 0x80000002 |
						hi_addr_bit;

	/* auto refresh */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = mem_conf->control | 0x80000004 |
						hi_addr_bit;

	/* auto refresh, second time */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = mem_conf->control | 0x80000004 |
						hi_addr_bit;

	/* set mode register */
	*(vu_long *)MPC5XXX_SDRAM_MODE = mem_conf->mode;

	/* normal operation */
	*(vu_long *)MPC5XXX_SDRAM_CTRL = mem_conf->control | hi_addr_bit;
}
#endif /* CFG_RAMBOOT */


/*
 * Retrieve memory configuration for a given module. board_type is the index
 * in hw_id_list[] corresponding to the module we are executing on; we return
 * SDRAM controller settings approprate for this module.
 */
static mem_conf_t* get_mem_config(int board_type)
{
	switch(board_type){
		case CM1_QA:
			return memory_config[0];
		case CM11_QA:
		case CMU1_QA:
			return memory_config[1];
		default:
			printf("ERROR: Unknown module, using a default SDRAM "
				"configuration - things may not work!!!.\n");
			return memory_config[0];
	}
}


/*
 * Initalize SDRAM - configure SDRAM controller, detect memory size.
 */
long int initdram(int board_type)
{
	ulong dramsize = 0;
#ifndef CFG_RAMBOOT
	ulong test1, test2;
	mem_conf_t *mem_conf;

	mem_conf = get_mem_config(board_type);

	/* configure SDRAM start/end for detection */
	*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x0000001e; /* 2G at 0x0 */

	/* setup config registers */
	*(vu_long *)MPC5XXX_SDRAM_CONFIG1 = mem_conf->config1;
	*(vu_long *)MPC5XXX_SDRAM_CONFIG2 = mem_conf->config2;

	sdram_start(0, mem_conf);
	test1 = get_ram_size((long *)CFG_SDRAM_BASE, 0x80000000);
	sdram_start(1, mem_conf);
	test2 = get_ram_size((long *)CFG_SDRAM_BASE, 0x80000000);
	if (test1 > test2) {
		sdram_start(0, mem_conf);
		dramsize = test1;
	} else
		dramsize = test2;

	/* memory smaller than 1MB is impossible */
	if (dramsize < (1 << 20))
		dramsize = 0;

	/* set SDRAM CS0 size according to the amount of RAM found */
	if (dramsize > 0) {
		*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0x13 +
			__builtin_ffs(dramsize >> 20) - 1;
	} else
		*(vu_long *)MPC5XXX_SDRAM_CS0CFG = 0; /* disabled */
#else /* CFG_RAMBOOT */
	/* retrieve size of memory connected to SDRAM CS0 */
	dramsize = *(vu_long *)MPC5XXX_SDRAM_CS0CFG & 0xFF;
	if (dramsize >= 0x13)
		dramsize = (1 << (dramsize - 0x13)) << 20;
	else
		dramsize = 0;
#endif /* !CFG_RAMBOOT */

	/*
	 * On MPC5200B we need to set the special configuration delay in the
	 * DDR controller.  Refer to chapter 8.7.5 SDelay--MBAR + 0x0190 of
	 * the MPC5200B User's Manual.
	 */
	*(vu_long *)MPC5XXX_SDRAM_SDELAY = 0x04;
	__asm__ volatile ("sync");

	return dramsize;
}


/*
 * Read module hardware identification data from the I2C EEPROM.
 */
static void read_hw_id(hw_id_t hw_id)
{
	int i;
	for (i = 0; i < HW_ID_ELEM_COUNT; ++i)
		if (i2c_read(CFG_I2C_EEPROM,
				hw_id_format[i].offset,
				2,
				(uchar *)&hw_id[i][0],
				hw_id_format[i].length) != 0)
			printf("ERROR: can't read HW ID from EEPROM\n");
}


/*
 * Identify module we are running on, set gd->board_type to the index in
 * hw_id_list[] corresponding to the module identifed, or to
 * CM5200_UNKNOWN_MODULE if we can't identify the module.
 */
static void identify_module(hw_id_t hw_id)
{
	int i, j, element;
	char match;
	gd->board_type = CM5200_UNKNOWN_MODULE;
	for (i = 0; i < sizeof (hw_id_list) / sizeof (char **); ++i) {
		match = 1;
		for (j = 0; j < sizeof (hw_id_identify) / sizeof (int); ++j) {
			element = hw_id_identify[j];
			if (strncmp(hw_id_list[i][element],
					&hw_id[element][0],
					hw_id_format[element].length) != 0) {
				match = 0;
				break;
			}
		}
		if (match) {
			gd->board_type = i;
			break;
		}
	}
}


/*
 * Compose string with module name.
 * buf is assumed to have enough space, and be null-terminated.
 */
static void compose_module_name(hw_id_t hw_id, char *buf)
{
	char tmp[MODULE_NAME_MAXLEN];
	strncat(buf, &hw_id[PCB_NAME][0], hw_id_format[PCB_NAME].length);
	strncat(buf, ".", 1);
	strncat(buf, &hw_id[FORM][0], hw_id_format[FORM].length);
	strncat(buf, &hw_id[VERSION][0], hw_id_format[VERSION].length);
	strncat(buf, " (", 2);
	strncat(buf, &hw_id[IDENTIFICATION_NUMBER][0],
		hw_id_format[IDENTIFICATION_NUMBER].length);
	sprintf(tmp, " / %u.%u)",
		hw_id[MAJOR_SW_VERSION][0],
		hw_id[MINOR_SW_VERSION][0]);
	strcat(buf, tmp);
}


/*
 * Compose string with hostname.
 * buf is assumed to have enough space, and be null-terminated.
 */
static void compose_hostname(hw_id_t hw_id, char *buf)
{
	char *p;
	strncat(buf, &hw_id[PCB_NAME][0], hw_id_format[PCB_NAME].length);
	strncat(buf, "_", 1);
	strncat(buf, &hw_id[FORM][0], hw_id_format[FORM].length);
	strncat(buf, &hw_id[VERSION][0], hw_id_format[VERSION].length);
	for (p = buf; *p; ++p)
		*p = tolower(*p);

}


#if defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT)
/*
 * Update 'model' and 'memory' properties in the blob according to the module
 * that we are running on.
 */
static void ft_blob_update(void *blob, bd_t *bd)
{
	int len, ret, nodeoffset = 0;
	char module_name[MODULE_NAME_MAXLEN] = {0};
	ulong memory_data[2] = {0};

	compose_module_name(hw_id, module_name);
	len = strlen(module_name) + 1;

	ret = fdt_setprop(blob, nodeoffset, "model", module_name, len);
	if (ret < 0)
	printf("ft_blob_update(): cannot set /model property err:%s\n",
		fdt_strerror(ret));

	memory_data[0] = cpu_to_be32(bd->bi_memstart);
	memory_data[1] = cpu_to_be32(bd->bi_memsize);

	nodeoffset = fdt_find_node_by_path (blob, "/memory");
	if (nodeoffset >= 0) {
		ret = fdt_setprop(blob, nodeoffset, "reg", memory_data,
					sizeof(memory_data));
	if (ret < 0)
		printf("ft_blob_update): cannot set /memory/reg "
			"property err:%s\n", fdt_strerror(ret));
	}
	else {
		/* memory node is required in dts */
		printf("ft_blob_update(): cannot find /memory node "
		"err:%s\n", fdt_strerror(nodeoffset));
	}
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) && defined(CONFIG_OF_LIBFDT) */


/*
 * Read HW ID from I2C EEPROM and detect the modue we are running on. Note
 * that we need to use local variable for readout, because global data is not
 * writable yet (and we'll have to redo the readout later on).
 */
int checkboard(void)
{
	hw_id_t hw_id_tmp;
	char module_name_tmp[MODULE_NAME_MAXLEN] = "";

	/*
	 * We need I2C to access HW ID data from EEPROM, so we call i2c_init()
	 * here despite the fact that it will be called again later on. We
	 * also use a little trick to silence I2C-related output.
	 */
	gd->flags |= GD_FLG_SILENT;
	i2c_init (CFG_I2C_SPEED, CFG_I2C_SLAVE);
	gd->flags &= ~GD_FLG_SILENT;

	read_hw_id(hw_id_tmp);
	identify_module(hw_id_tmp);	/* this sets gd->board_type */
	compose_module_name(hw_id_tmp, module_name_tmp);

	if (gd->board_type != CM5200_UNKNOWN_MODULE)
		printf("Board: %s\n", module_name_tmp);
	else
		printf("Board: unrecognized cm5200 module (%s)\n",
			module_name_tmp);

	return 0;
}


int board_early_init_r(void)
{
	/*
	 * Now, when we are in RAM, enable flash write access for detection
	 * process. Note that CS_BOOT cannot be cleared when executing in
	 * flash.
	 */
	*(vu_long *)MPC5XXX_BOOTCS_CFG &= ~0x1; /* clear RO */

	/* Now that we can write to global data, read HW ID again. */
	read_hw_id(hw_id);
	return 0;
}


#ifdef CONFIG_POST
int post_hotkeys_pressed(void)
{
	return 0;
}
#endif /* CONFIG_POST */


#if defined(CONFIG_POST) || defined(CONFIG_LOGBUFFER)
void post_word_store(ulong a)
{
	vu_long *save_addr = (vu_long *)(MPC5XXX_SRAM + MPC5XXX_SRAM_POST_SIZE);
	*save_addr = a;
}


ulong post_word_load(void)
{
	vu_long *save_addr = (vu_long *)(MPC5XXX_SRAM + MPC5XXX_SRAM_POST_SIZE);
	return *save_addr;
}
#endif /* CONFIG_POST || CONFIG_LOGBUFFER */


#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
	uchar buf[6];
	char str[18];
	char hostname[MODULE_NAME_MAXLEN];

	/* Read ethaddr from EEPROM */
	if (i2c_read(CFG_I2C_EEPROM, CONFIG_MAC_OFFSET, 2, buf, 6) == 0) {
		sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X",
			buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
		/* Check if MAC addr is owned by Schindler */
		if (strstr(str, "00:06:C3") != str)
			printf(LOG_PREFIX "Warning - Illegal MAC address (%s)"
				" in EEPROM.\n", str);
		else {
			printf(LOG_PREFIX "Using MAC (%s) from I2C EEPROM\n",
				str);
			setenv("ethaddr", str);
		}
	} else {
		printf(LOG_PREFIX "Warning - Unable to read MAC from I2C"
			" device at address %02X:%04X\n", CFG_I2C_EEPROM,
			CONFIG_MAC_OFFSET);
	}
#endif /* defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C) */
	if (!getenv("ethaddr"))
		printf(LOG_PREFIX "MAC address not set, networking is not "
					"operational\n");

	/* set the hostname appropriate to the module we're running on */
	hostname[0] = 0x00;
	compose_hostname(hw_id, hostname);
	setenv("hostname", hostname);

	return 0;
}
#endif /* CONFIG_MISC_INIT_R */


#ifdef CONFIG_LAST_STAGE_INIT
int last_stage_init(void)
{
#ifdef CONFIG_USB_STORAGE
	cm5200_fwupdate();
#endif /* CONFIG_USB_STORAGE */
	return 0;
}
#endif /* CONFIG_LAST_STAGE_INIT */


#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	ft_blob_update(blob, bd);
}
#endif /* defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP) */
