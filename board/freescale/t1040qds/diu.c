/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Author: Priyanka Jain <Priyanka.Jain@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <asm/io.h>
#include <stdio_dev.h>
#include <video_fb.h>
#include <fsl_diu_fb.h>
#include "../common/qixis.h"
#include "t1040qds.h"
#include "t1040qds_qixis.h"
#include <i2c.h>


#define I2C_DVI_INPUT_DATA_FORMAT_REG		0x1F
#define I2C_DVI_PLL_CHARGE_CNTL_REG		0x33
#define I2C_DVI_PLL_DIVIDER_REG			0x34
#define I2C_DVI_PLL_SUPPLY_CNTL_REG		0x35
#define I2C_DVI_PLL_FILTER_REG			0x36
#define I2C_DVI_TEST_PATTERN_REG		0x48
#define I2C_DVI_POWER_MGMT_REG			0x49
#define I2C_DVI_LOCK_STATE_REG			0x4D
#define I2C_DVI_SYNC_POLARITY_REG		0x56

/*
 * Set VSYNC/HSYNC to active high. This is polarity of sync signals
 * from DIU->DVI. The DIU default is active igh, so DVI is set to
 * active high.
 */
#define I2C_DVI_INPUT_DATA_FORMAT_VAL		0x98

#define I2C_DVI_PLL_CHARGE_CNTL_HIGH_SPEED_VAL	0x06
#define I2C_DVI_PLL_DIVIDER_HIGH_SPEED_VAL	0x26
#define I2C_DVI_PLL_FILTER_HIGH_SPEED_VAL	0xA0
#define I2C_DVI_PLL_CHARGE_CNTL_LOW_SPEED_VAL	0x08
#define I2C_DVI_PLL_DIVIDER_LOW_SPEED_VAL	0x16
#define I2C_DVI_PLL_FILTER_LOW_SPEED_VAL	0x60

/* Clear test pattern */
#define I2C_DVI_TEST_PATTERN_VAL		0x18
/* Exit Power-down mode */
#define I2C_DVI_POWER_MGMT_VAL			0xC0

/* Monitor polarity is handled via DVI Sync Polarity Register */
#define I2C_DVI_SYNC_POLARITY_VAL		0x00

/*
 * DIU Area Descriptor
 *
 * Note that we need to byte-swap the value before it's written to the AD
 * register.  So even though the registers don't look like they're in the same
 * bit positions as they are on the MPC8610, the same value is written to the
 * AD register on the MPC8610 and on the P1022.
 */
#define AD_BYTE_F		0x10000000
#define AD_ALPHA_C_SHIFT	25
#define AD_BLUE_C_SHIFT		23
#define AD_GREEN_C_SHIFT	21
#define AD_RED_C_SHIFT		19
#define AD_PIXEL_S_SHIFT	16
#define AD_COMP_3_SHIFT		12
#define AD_COMP_2_SHIFT		8
#define AD_COMP_1_SHIFT		4
#define AD_COMP_0_SHIFT		0

/* Programming of HDMI Chrontel CH7301 connector */
int diu_set_dvi_encoder(unsigned int pixclock)
{
	int ret;
	u8 temp;
	select_i2c_ch_pca9547(I2C_MUX_CH_DIU);

	temp = I2C_DVI_TEST_PATTERN_VAL;
	ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR, I2C_DVI_TEST_PATTERN_REG, 1,
			&temp, 1);
	if (ret) {
		puts("I2C: failed to select proper dvi test pattern\n");
		return ret;
	}
	temp = I2C_DVI_INPUT_DATA_FORMAT_VAL;
	ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR, I2C_DVI_INPUT_DATA_FORMAT_REG,
			1, &temp, 1);
	if (ret) {
		puts("I2C: failed to select dvi input data format\n");
		return ret;
	}

	/* Set Sync polarity register */
	temp = I2C_DVI_SYNC_POLARITY_VAL;
	ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR, I2C_DVI_SYNC_POLARITY_REG, 1,
			&temp, 1);
	if (ret) {
		puts("I2C: failed to select dvi syc polarity\n");
		return ret;
	}

	/* Set PLL registers based on pixel clock rate*/
	if (pixclock > 65000000) {
		temp = I2C_DVI_PLL_CHARGE_CNTL_HIGH_SPEED_VAL;
		ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR,
				I2C_DVI_PLL_CHARGE_CNTL_REG, 1,	&temp, 1);
		if (ret) {
			puts("I2C: failed to select dvi pll charge_cntl\n");
			return ret;
		}
		temp = I2C_DVI_PLL_DIVIDER_HIGH_SPEED_VAL;
		ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR,
				I2C_DVI_PLL_DIVIDER_REG, 1, &temp, 1);
		if (ret) {
			puts("I2C: failed to select dvi pll divider\n");
			return ret;
		}
		temp = I2C_DVI_PLL_FILTER_HIGH_SPEED_VAL;
		ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR,
				I2C_DVI_PLL_FILTER_REG, 1, &temp, 1);
		if (ret) {
			puts("I2C: failed to select dvi pll filter\n");
			return ret;
		}
	} else {
		temp = I2C_DVI_PLL_CHARGE_CNTL_LOW_SPEED_VAL;
		ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR,
				I2C_DVI_PLL_CHARGE_CNTL_REG, 1, &temp, 1);
		if (ret) {
			puts("I2C: failed to select dvi pll charge_cntl\n");
			return ret;
		}
		temp = I2C_DVI_PLL_DIVIDER_LOW_SPEED_VAL;
		ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR,
				I2C_DVI_PLL_DIVIDER_REG, 1, &temp, 1);
		if (ret) {
			puts("I2C: failed to select dvi pll divider\n");
			return ret;
		}
		temp = I2C_DVI_PLL_FILTER_LOW_SPEED_VAL;
		ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR,
				I2C_DVI_PLL_FILTER_REG, 1, &temp, 1);
		if (ret) {
			puts("I2C: failed to select dvi pll filter\n");
			return ret;
		}
	}

	temp = I2C_DVI_POWER_MGMT_VAL;
	ret = i2c_write(CONFIG_SYS_I2C_DVI_ADDR, I2C_DVI_POWER_MGMT_REG, 1,
			&temp, 1);
	if (ret) {
		puts("I2C: failed to select dvi power mgmt\n");
		return ret;
	}

	udelay(500);

	select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT);
	return 0;
}

void diu_set_pixel_clock(unsigned int pixclock)
{
	unsigned long speed_ccb, temp;
	u32 pixval;
	int ret = 0;
	speed_ccb = get_bus_freq(0);
	temp = 1000000000 / pixclock;
	temp *= 1000;
	pixval = speed_ccb / temp;

	/* Program HDMI encoder */
	ret = diu_set_dvi_encoder(temp);
	if (ret) {
		puts("Failed to set DVI encoder\n");
		return;
	}

	/* Program pixel clock */
	out_be32((unsigned *)CONFIG_SYS_FSL_SCFG_PIXCLK_ADDR,
		 ((pixval << PXCK_BITS_START) & PXCK_MASK));
	/* enable clock*/
	out_be32((unsigned *)CONFIG_SYS_FSL_SCFG_PIXCLK_ADDR, PXCKEN_MASK |
		 ((pixval << PXCK_BITS_START) & PXCK_MASK));
}

int platform_diu_init(unsigned int xres, unsigned int yres, const char *port)
{
	u32 pixel_format;
	u8 sw;

	/*Route I2C4 to DIU system as HSYNC/VSYNC*/
	sw = QIXIS_READ(brdcfg[5]);
	QIXIS_WRITE(brdcfg[5],
		    ((sw & ~(BRDCFG5_IMX_MASK)) | (BRDCFG5_IMX_DIU)));

	/*Configure Display ouput port as HDMI*/
	sw = QIXIS_READ(brdcfg[15]);
	QIXIS_WRITE(brdcfg[15],
		    ((sw & ~(BRDCFG15_LCDPD_MASK | BRDCFG15_DIUSEL_MASK))
		      | (BRDCFG15_LCDPD_ENABLED | BRDCFG15_DIUSEL_HDMI)));

	pixel_format = cpu_to_le32(AD_BYTE_F | (3 << AD_ALPHA_C_SHIFT) |
		(0 << AD_BLUE_C_SHIFT) | (1 << AD_GREEN_C_SHIFT) |
		(2 << AD_RED_C_SHIFT) | (8 << AD_COMP_3_SHIFT) |
		(8 << AD_COMP_2_SHIFT) | (8 << AD_COMP_1_SHIFT) |
		(8 << AD_COMP_0_SHIFT) | (3 << AD_PIXEL_S_SHIFT));

	printf("DIU:   Switching to monitor @ %ux%u\n",  xres, yres);


	return fsl_diu_init(xres, yres, pixel_format, 0);
}
