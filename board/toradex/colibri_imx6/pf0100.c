// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014-2016, Toradex AG
 */

/*
 * Helpers for Freescale PMIC PF0100
*/

#include <common.h>
#include <i2c.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>

#include "pf0100_otp.inc"
#include "pf0100.h"

/* define for PMIC register dump */
/*#define DEBUG */

/* use GPIO: EXT_IO1 to switch on VPGM, ON: 1 */
static __maybe_unused iomux_v3_cfg_t const pmic_prog_pads[] = {
	MX6_PAD_NANDF_D3__GPIO2_IO03 | MUX_PAD_CTRL(NO_PAD_CTRL),
#	define PMIC_PROG_VOLTAGE IMX_GPIO_NR(2, 3)
};

unsigned pmic_init(void)
{
	unsigned programmed = 0;
	uchar bus = 1;
	uchar devid, revid, val;

	puts("PMIC: ");
	if (!((0 == i2c_set_bus_num(bus)) &&
	      (0 == i2c_probe(PFUZE100_I2C_ADDR)))) {
		puts("i2c bus failed\n");
		return 0;
	}
	/* get device ident */
	if (i2c_read(PFUZE100_I2C_ADDR, PFUZE100_DEVICEID, 1, &devid, 1) < 0) {
		puts("i2c pmic devid read failed\n");
		return 0;
	}
	if (i2c_read(PFUZE100_I2C_ADDR, PFUZE100_REVID, 1, &revid, 1) < 0) {
		puts("i2c pmic revid read failed\n");
		return 0;
	}
	printf("device id: 0x%.2x, revision id: 0x%.2x\n", devid, revid);

#ifdef DEBUG
	{
		unsigned i, j;

		for (i = 0; i < 16; i++)
			printf("\t%x", i);
		for (j = 0; j < 0x80; ) {
			printf("\n%2x", j);
			for (i = 0; i < 16; i++) {
				i2c_read(PFUZE100_I2C_ADDR, j+i, 1, &val, 1);
				printf("\t%2x", val);
			}
			j += 0x10;
		}
		printf("\nEXT Page 1");

		val = PFUZE100_PAGE_REGISTER_PAGE1;
		if (i2c_write(PFUZE100_I2C_ADDR, PFUZE100_PAGE_REGISTER, 1,
			      &val, 1)) {
			puts("i2c write failed\n");
			return 0;
		}

		for (j = 0x80; j < 0x100; ) {
			printf("\n%2x", j);
			for (i = 0; i < 16; i++) {
				i2c_read(PFUZE100_I2C_ADDR, j+i, 1, &val, 1);
				printf("\t%2x", val);
			}
			j += 0x10;
		}
		printf("\nEXT Page 2");

		val = PFUZE100_PAGE_REGISTER_PAGE2;
		if (i2c_write(PFUZE100_I2C_ADDR, PFUZE100_PAGE_REGISTER, 1,
			      &val, 1)) {
			puts("i2c write failed\n");
			return 0;
		}

		for (j = 0x80; j < 0x100; ) {
			printf("\n%2x", j);
			for (i = 0; i < 16; i++) {
				i2c_read(PFUZE100_I2C_ADDR, j+i, 1, &val, 1);
				printf("\t%2x", val);
			}
			j += 0x10;
		}
		printf("\n");
	}
#endif
	/* get device programmed state */
	val = PFUZE100_PAGE_REGISTER_PAGE1;
	if (i2c_write(PFUZE100_I2C_ADDR, PFUZE100_PAGE_REGISTER, 1, &val, 1)) {
		puts("i2c write failed\n");
		return 0;
	}
	if (i2c_read(PFUZE100_I2C_ADDR, PFUZE100_FUSE_POR1, 1, &val, 1) < 0) {
		puts("i2c fuse_por read failed\n");
		return 0;
	}
	if (val & PFUZE100_FUSE_POR_M)
		programmed++;

	if (i2c_read(PFUZE100_I2C_ADDR, PFUZE100_FUSE_POR2, 1, &val, 1) < 0) {
		puts("i2c fuse_por read failed\n");
		return programmed;
	}
	if (val & PFUZE100_FUSE_POR_M)
		programmed++;

	if (i2c_read(PFUZE100_I2C_ADDR, PFUZE100_FUSE_POR3, 1, &val, 1) < 0) {
		puts("i2c fuse_por read failed\n");
		return programmed;
	}
	if (val & PFUZE100_FUSE_POR_M)
		programmed++;

	switch (programmed) {
	case 0:
		printf("PMIC: not programmed\n");
		break;
	case 3:
		printf("PMIC: programmed\n");
		break;
	default:
		printf("PMIC: undefined programming state\n");
		break;
	}

	return programmed;
}

#ifndef CONFIG_SPL_BUILD
static int pf0100_prog(void)
{
	unsigned char bus = 1;
	unsigned char val;
	unsigned int i;

	if (pmic_init() == 3) {
		puts("PMIC already programmed, exiting\n");
		return CMD_RET_FAILURE;
	}
	/* set up gpio to manipulate vprog, initially off */
	imx_iomux_v3_setup_multiple_pads(pmic_prog_pads,
					 ARRAY_SIZE(pmic_prog_pads));
	gpio_direction_output(PMIC_PROG_VOLTAGE, 0);

	if (!((0 == i2c_set_bus_num(bus)) &&
	      (0 == i2c_probe(PFUZE100_I2C_ADDR)))) {
		puts("i2c bus failed\n");
		return CMD_RET_FAILURE;
	}

	for (i = 0; i < ARRAY_SIZE(pmic_otp_prog); i++) {
		switch (pmic_otp_prog[i].cmd) {
		case pmic_i2c:
			val = (unsigned char) (pmic_otp_prog[i].value & 0xff);
			if (i2c_write(PFUZE100_I2C_ADDR, pmic_otp_prog[i].reg,
				      1, &val, 1)) {
				printf("i2c write failed, reg 0x%2x, value 0x%2x\n",
				       pmic_otp_prog[i].reg, val);
				return CMD_RET_FAILURE;
			}
			break;
		case pmic_delay:
			udelay(pmic_otp_prog[i].value * 1000);
			break;
		case pmic_vpgm:
			gpio_direction_output(PMIC_PROG_VOLTAGE,
					      pmic_otp_prog[i].value);
			break;
		case pmic_pwr:
			/* TODO */
			break;
		}
	}
	return CMD_RET_SUCCESS;
}

static int do_pf0100_prog(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	int ret;
	puts("Programming PMIC OTP...");
	ret = pf0100_prog();
	if (ret == CMD_RET_SUCCESS)
		puts("done.\n");
	else
		puts("failed.\n");
	return ret;
}

U_BOOT_CMD(
	pf0100_otp_prog, 1, 0, do_pf0100_prog,
	"Program the OTP fuses on the PMIC PF0100",
	""
);
#endif
