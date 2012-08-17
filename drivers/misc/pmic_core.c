/*
 * Copyright (C) 2011 Samsung Electronics
 * Lukasz Majewski <l.majewski@samsung.com>
 *
 * (C) Copyright 2010
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de
 *
 * (C) Copyright 2008-2009 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <linux/types.h>
#include <pmic.h>

static struct pmic pmic;

int check_reg(u32 reg)
{
	if (reg >= pmic.number_of_regs) {
		printf("<reg num> = %d is invalid. Should be less than %d\n",
		       reg, pmic.number_of_regs);
		return -1;
	}
	return 0;
}

int pmic_set_output(struct pmic *p, u32 reg, int out, int on)
{
	u32 val;

	if (pmic_reg_read(p, reg, &val))
		return -1;

	if (on)
		val |= out;
	else
		val &= ~out;

	if (pmic_reg_write(p, reg, val))
		return -1;

	return 0;
}

static void pmic_show_info(struct pmic *p)
{
	printf("PMIC: %s\n", p->name);
}

static void pmic_dump(struct pmic *p)
{
	int i, ret;
	u32 val;

	pmic_show_info(p);
	for (i = 0; i < p->number_of_regs; i++) {
		ret = pmic_reg_read(p, i, &val);
		if (ret)
			puts("PMIC: Registers dump failed\n");

		if (!(i % 8))
			printf("\n0x%02x: ", i);

		printf("%08x ", val);
	}
	puts("\n");
}

struct pmic *get_pmic(void)
{
	return &pmic;
}

int do_pmic(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 ret, reg, val;
	char *cmd;

	struct pmic *p = &pmic;

	/* at least two arguments please */
	if (argc < 2)
		return cmd_usage(cmdtp);

	cmd = argv[1];
	if (strcmp(cmd, "dump") == 0) {
		pmic_dump(p);
		return 0;
	}

	if (strcmp(cmd, "read") == 0) {
		if (argc < 3)
			return cmd_usage(cmdtp);

		reg = simple_strtoul(argv[2], NULL, 16);

		ret = pmic_reg_read(p, reg, &val);

		if (ret)
			puts("PMIC: Register read failed\n");

		printf("\n0x%02x: 0x%08x\n", reg, val);

		return 0;
	}

	if (strcmp(cmd, "write") == 0) {
		if (argc < 4)
			return cmd_usage(cmdtp);

		reg = simple_strtoul(argv[2], NULL, 16);
		val = simple_strtoul(argv[3], NULL, 16);

		pmic_reg_write(p, reg, val);

		return 0;
	}

	/* No subcommand found */
	return 1;
}

U_BOOT_CMD(
	pmic,	CONFIG_SYS_MAXARGS, 1, do_pmic,
	"PMIC",
	"dump - dump PMIC registers\n"
	"pmic read <reg> - read register\n"
	"pmic write <reg> <value> - write register"
);
