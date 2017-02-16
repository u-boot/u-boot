/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

DECLARE_GLOBAL_DATA_PTR;

/* IO expander I2C device */
#define I2C_IO_EXP_ADDR		0x22
#define I2C_IO_CFG_REG_0	0x6
#define I2C_IO_DATA_OUT_REG_0	0x2
#define I2C_IO_REG_0_SATA_OFF	2
#define I2C_IO_REG_0_USB_H_OFF	1

int board_early_init_f(void)
{
	/* Nothing to do (yet), perhaps later some pin-muxing etc */

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

/* Board specific AHCI / SATA enable code */
int board_ahci_enable(void)
{
	struct udevice *dev;
	int ret;
	u8 buf[8];

	/* Configure IO exander PCA9555: 7bit address 0x22 */
	ret = i2c_get_chip_for_busnum(0, I2C_IO_EXP_ADDR, 1, &dev);
	if (ret) {
		printf("Cannot find PCA9555: %d\n", ret);
		return 0;
	}

	ret = dm_i2c_read(dev, I2C_IO_CFG_REG_0, buf, 1);
	if (ret) {
		printf("Failed to read IO expander value via I2C\n");
		return -EIO;
	}

	/*
	 * Enable SATA power via IO expander connected via I2C by setting
	 * the corresponding bit to output mode to enable power for SATA
	 */
	buf[0] &= ~(1 << I2C_IO_REG_0_SATA_OFF);
	ret = dm_i2c_write(dev, I2C_IO_CFG_REG_0, buf, 1);
	if (ret) {
		printf("Failed to set IO expander via I2C\n");
		return -EIO;
	}

	return 0;
}

/* Board specific xHCI enable code */
int board_xhci_enable(void)
{
	struct udevice *dev;
	int ret;
	u8 buf[8];

	/* Configure IO exander PCA9555: 7bit address 0x22 */
	ret = i2c_get_chip_for_busnum(0, I2C_IO_EXP_ADDR, 1, &dev);
	if (ret) {
		printf("Cannot find PCA9555: %d\n", ret);
		return 0;
	}

	printf("Enable USB VBUS\n");

	/*
	 * Read configuration (direction) and set VBUS pin as output
	 * (reset pin = output)
	 */
	ret = dm_i2c_read(dev, I2C_IO_CFG_REG_0, buf, 1);
	if (ret) {
		printf("Failed to read IO expander value via I2C\n");
		return -EIO;
	}
	buf[0] &= ~(1 << I2C_IO_REG_0_USB_H_OFF);
	ret = dm_i2c_write(dev, I2C_IO_CFG_REG_0, buf, 1);
	if (ret) {
		printf("Failed to set IO expander via I2C\n");
		return -EIO;
	}

	/* Read VBUS output value and disable it */
	ret = dm_i2c_read(dev, I2C_IO_DATA_OUT_REG_0, buf, 1);
	if (ret) {
		printf("Failed to read IO expander value via I2C\n");
		return -EIO;
	}
	buf[0] &= ~(1 << I2C_IO_REG_0_USB_H_OFF);
	ret = dm_i2c_write(dev, I2C_IO_DATA_OUT_REG_0, buf, 1);
	if (ret) {
		printf("Failed to set IO expander via I2C\n");
		return -EIO;
	}

	/*
	 * Required delay for configuration to settle - must wait for
	 * power on port is disabled in case VBUS signal was high,
	 * required 3 seconds delay to let VBUS signal fully settle down
	 */
	mdelay(3000);

	/* Enable VBUS power: Set output value of VBUS pin as enabled */
	buf[0] |= (1 << I2C_IO_REG_0_USB_H_OFF);
	ret = dm_i2c_write(dev, I2C_IO_DATA_OUT_REG_0, buf, 1);
	if (ret) {
		printf("Failed to set IO expander via I2C\n");
		return -EIO;
	}

	mdelay(500); /* required delay to let output value settle */

	return 0;
}
