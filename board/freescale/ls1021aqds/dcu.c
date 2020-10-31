// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 * Copyright 2019 NXP
 *
 * FSL DCU Framebuffer driver
 */

#include <asm/global_data.h>
#include <asm/io.h>
#include <common.h>
#include <fsl_dcu_fb.h>
#include <i2c.h>
#include "div64.h"
#include "../common/diu_ch7301.h"
#include "ls1021aqds_qixis.h"

DECLARE_GLOBAL_DATA_PTR;

static int select_i2c_ch_pca9547(u8 ch, int bus_num)
{
	int ret;
#ifdef CONFIG_DM_I2C
	struct udevice *dev;

	ret = i2c_get_chip_for_busnum(bus_num, I2C_MUX_PCA_ADDR_PRI,
				      1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       bus_num);
		return ret;
	}
	ret = dm_i2c_write(dev, 0, &ch, 1);
#else
	ret = i2c_write(I2C_MUX_PCA_ADDR_PRI, 0, 1, &ch, 1);
#endif
	if (ret) {
		puts("PCA: failed to select proper channel\n");
		return ret;
	}

	return 0;
}

unsigned int dcu_set_pixel_clock(unsigned int pixclock)
{
	unsigned long long div;

	div = (unsigned long long)(gd->bus_clk / 1000);
	div *= (unsigned long long)pixclock;
	do_div(div, 1000000000);

	return div;
}

int platform_dcu_init(struct fb_info *fbinfo,
		      unsigned int xres,
		      unsigned int yres,
		      const char *port,
		      struct fb_videomode *dcu_fb_videomode)
{
	const char *name;
	unsigned int pixel_format;
	int ret;
	u8 ch;

	/* Mux I2C3+I2C4 as HSYNC+VSYNC */
#ifdef CONFIG_DM_I2C
	struct udevice *dev;

	/* QIXIS device mount on I2C1 bus*/
	ret = i2c_get_chip_for_busnum(0, CONFIG_SYS_I2C_QIXIS_ADDR,
				      1, &dev);
	if (ret) {
		printf("%s: Cannot find udev for a bus %d\n", __func__,
		       0);
		return ret;
	}
	ret = dm_i2c_read(dev, QIXIS_DCU_BRDCFG5, &ch, 1);
	if (ret) {
		printf("Error: failed to read I2C @%02x\n",
		       CONFIG_SYS_I2C_QIXIS_ADDR);
		return ret;
	}
	ch &= 0x1F;
	ch |= 0xA0;
	ret = dm_i2c_write(dev, QIXIS_DCU_BRDCFG5, &ch, 1);

#else
	ret = i2c_read(CONFIG_SYS_I2C_QIXIS_ADDR, QIXIS_DCU_BRDCFG5,
		       1, &ch, 1);
	if (ret) {
		printf("Error: failed to read I2C @%02x\n",
		       CONFIG_SYS_I2C_QIXIS_ADDR);
		return ret;
	}
	ch &= 0x1F;
	ch |= 0xA0;
	ret = i2c_write(CONFIG_SYS_I2C_QIXIS_ADDR, QIXIS_DCU_BRDCFG5,
			1, &ch, 1);
#endif
	if (ret) {
		printf("Error: failed to write I2C @%02x\n",
		       CONFIG_SYS_I2C_QIXIS_ADDR);
		return ret;
	}

	if (strncmp(port, "hdmi", 4) == 0) {
		unsigned long pixval;

		name = "HDMI";

		pixval = 1000000000 / dcu_fb_videomode->pixclock;
		pixval *= 1000;

#ifndef CONFIG_DM_I2C
		i2c_set_bus_num(CONFIG_SYS_I2C_DVI_BUS_NUM);
#endif
		select_i2c_ch_pca9547(I2C_MUX_CH_CH7301,
				      CONFIG_SYS_I2C_DVI_BUS_NUM);
		diu_set_dvi_encoder(pixval);
		select_i2c_ch_pca9547(I2C_MUX_CH_DEFAULT,
				      CONFIG_SYS_I2C_DVI_BUS_NUM);
	} else {
		return 0;
	}

	printf("DCU: Switching to %s monitor @ %ux%u\n", name, xres, yres);

	pixel_format = 32;
	fsl_dcu_init(fbinfo, xres, yres, pixel_format);

	return 0;
}
