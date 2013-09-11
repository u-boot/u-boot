/*
 * max98095.c -- MAX98095 ALSA SoC Audio driver
 *
 * Copyright 2011 Maxim Integrated Products
 *
 * Modified for uboot by R. Chandrasekar (rcsekar@samsung.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <asm/arch/clk.h>
#include <asm/arch/cpu.h>
#include <asm/arch/power.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <common.h>
#include <div64.h>
#include <fdtdec.h>
#include <i2c.h>
#include <sound.h>
#include "i2s.h"
#include "max98095.h"

enum max98095_type {
	MAX98095,
};

struct max98095_priv {
	enum max98095_type devtype;
	unsigned int sysclk;
	unsigned int rate;
	unsigned int fmt;
};

static struct sound_codec_info g_codec_info;
struct max98095_priv g_max98095_info;
unsigned int g_max98095_i2c_dev_addr;

/* Index 0 is reserved. */
int rate_table[] = {0, 8000, 11025, 16000, 22050, 24000, 32000, 44100, 48000,
		88200, 96000};

/*
 * Writes value to a device register through i2c
 *
 * @param reg	reg number to be write
 * @param data	data to be writen to the above registor
 *
 * @return	int value 1 for change, 0 for no change or negative error code.
 */
static int max98095_i2c_write(unsigned int reg, unsigned char data)
{
	debug("%s: Write Addr : 0x%02X, Data :  0x%02X\n",
	      __func__, reg, data);
	return i2c_write(g_max98095_i2c_dev_addr, reg, 1, &data, 1);
}

/*
 * Read a value from a device register through i2c
 *
 * @param reg	reg number to be read
 * @param data	address of read data to be stored
 *
 * @return	int value 0 for success, -1 in case of error.
 */
static unsigned int max98095_i2c_read(unsigned int reg, unsigned char *data)
{
	int ret;

	ret = i2c_read(g_max98095_i2c_dev_addr, reg, 1, data, 1);
	if (ret != 0) {
		debug("%s: Error while reading register %#04x\n",
		      __func__, reg);
		return -1;
	}

	return 0;
}

/*
 * update device register bits through i2c
 *
 * @param reg	codec register
 * @param mask	register mask
 * @param value	new value
 *
 * @return int value 0 for success, non-zero error code.
 */
static int max98095_update_bits(unsigned int reg, unsigned char mask,
				unsigned char value)
{
	int change, ret = 0;
	unsigned char old, new;

	if (max98095_i2c_read(reg, &old) != 0)
		return -1;
	new = (old & ~mask) | (value & mask);
	change  = (old != new) ? 1 : 0;
	if (change)
		ret = max98095_i2c_write(reg, new);
	if (ret < 0)
		return ret;

	return change;
}

/*
 * codec mclk clock divider coefficients based on sampling rate
 *
 * @param rate sampling rate
 * @param value address of indexvalue to be stored
 *
 * @return	0 for success or negative error code.
 */
static int rate_value(int rate, u8 *value)
{
	int i;

	for (i = 1; i < ARRAY_SIZE(rate_table); i++) {
		if (rate_table[i] >= rate) {
			*value = i;
			return 0;
		}
	}
	*value = 1;

	return -1;
}

/*
 * Sets hw params for max98095
 *
 * @param max98095	max98095 information pointer
 * @param rate		Sampling rate
 * @param bits_per_sample	Bits per sample
 *
 * @return -1 for error  and 0  Success.
 */
static int max98095_hw_params(struct max98095_priv *max98095,
			      enum en_max_audio_interface aif_id,
			      unsigned int rate, unsigned int bits_per_sample)
{
	u8 regval;
	int error;
	unsigned short M98095_DAI_CLKMODE;
	unsigned short M98095_DAI_FORMAT;
	unsigned short M98095_DAI_FILTERS;

	if (aif_id == AIF1) {
		M98095_DAI_CLKMODE = M98095_027_DAI1_CLKMODE;
		M98095_DAI_FORMAT = M98095_02A_DAI1_FORMAT;
		M98095_DAI_FILTERS = M98095_02E_DAI1_FILTERS;
	} else {
		M98095_DAI_CLKMODE = M98095_031_DAI2_CLKMODE;
		M98095_DAI_FORMAT = M98095_034_DAI2_FORMAT;
		M98095_DAI_FILTERS = M98095_038_DAI2_FILTERS;
	}

	switch (bits_per_sample) {
	case 16:
		error = max98095_update_bits(M98095_DAI_FORMAT,
					     M98095_DAI_WS, 0);
		break;
	case 24:
		error = max98095_update_bits(M98095_DAI_FORMAT,
					     M98095_DAI_WS, M98095_DAI_WS);
		break;
	default:
		debug("%s: Illegal bits per sample %d.\n",
		      __func__, bits_per_sample);
		return -1;
	}

	if (rate_value(rate, &regval)) {
		debug("%s: Failed to set sample rate to %d.\n",
		      __func__, rate);
		return -1;
	}
	max98095->rate = rate;

	error |= max98095_update_bits(M98095_DAI_CLKMODE,
				      M98095_CLKMODE_MASK, regval);

	/* Update sample rate mode */
	if (rate < 50000)
		error |= max98095_update_bits(M98095_DAI_FILTERS,
					      M98095_DAI_DHF, 0);
	else
		error |= max98095_update_bits(M98095_DAI_FILTERS,
					      M98095_DAI_DHF, M98095_DAI_DHF);

	if (error < 0) {
		debug("%s: Error setting hardware params.\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * Configures Audio interface system clock for the given frequency
 *
 * @param max98095	max98095 information
 * @param freq		Sampling frequency in Hz
 *
 * @return -1 for error and 0 success.
 */
static int max98095_set_sysclk(struct max98095_priv *max98095,
			       unsigned int freq)
{
	int error = 0;

	/* Requested clock frequency is already setup */
	if (freq == max98095->sysclk)
		return 0;

	/* Setup clocks for slave mode, and using the PLL
	 * PSCLK = 0x01 (when master clk is 10MHz to 20MHz)
	 *	0x02 (when master clk is 20MHz to 40MHz)..
	 *	0x03 (when master clk is 40MHz to 60MHz)..
	 */
	if ((freq >= 10000000) && (freq < 20000000)) {
		error = max98095_i2c_write(M98095_026_SYS_CLK, 0x10);
	} else if ((freq >= 20000000) && (freq < 40000000)) {
		error = max98095_i2c_write(M98095_026_SYS_CLK, 0x20);
	} else if ((freq >= 40000000) && (freq < 60000000)) {
		error = max98095_i2c_write(M98095_026_SYS_CLK, 0x30);
	} else {
		debug("%s: Invalid master clock frequency\n", __func__);
		return -1;
	}

	debug("%s: Clock at %uHz\n", __func__, freq);

	if (error < 0)
		return -1;

	max98095->sysclk = freq;
	return 0;
}

/*
 * Sets Max98095 I2S format
 *
 * @param max98095	max98095 information
 * @param fmt		i2S format - supports a subset of the options defined
 *			in i2s.h.
 *
 * @return -1 for error and 0  Success.
 */
static int max98095_set_fmt(struct max98095_priv *max98095, int fmt,
			    enum en_max_audio_interface aif_id)
{
	u8 regval = 0;
	int error = 0;
	unsigned short M98095_DAI_CLKCFG_HI;
	unsigned short M98095_DAI_CLKCFG_LO;
	unsigned short M98095_DAI_FORMAT;
	unsigned short M98095_DAI_CLOCK;

	if (fmt == max98095->fmt)
		return 0;

	max98095->fmt = fmt;

	if (aif_id == AIF1) {
		M98095_DAI_CLKCFG_HI = M98095_028_DAI1_CLKCFG_HI;
		M98095_DAI_CLKCFG_LO = M98095_029_DAI1_CLKCFG_LO;
		M98095_DAI_FORMAT = M98095_02A_DAI1_FORMAT;
		M98095_DAI_CLOCK = M98095_02B_DAI1_CLOCK;
	} else {
		M98095_DAI_CLKCFG_HI = M98095_032_DAI2_CLKCFG_HI;
		M98095_DAI_CLKCFG_LO = M98095_033_DAI2_CLKCFG_LO;
		M98095_DAI_FORMAT = M98095_034_DAI2_FORMAT;
		M98095_DAI_CLOCK = M98095_035_DAI2_CLOCK;
	}

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFS:
		/* Slave mode PLL */
		error |= max98095_i2c_write(M98095_DAI_CLKCFG_HI,
					0x80);
		error |= max98095_i2c_write(M98095_DAI_CLKCFG_LO,
					0x00);
		break;
	case SND_SOC_DAIFMT_CBM_CFM:
		/* Set to master mode */
		regval |= M98095_DAI_MAS;
		break;
	case SND_SOC_DAIFMT_CBS_CFM:
	case SND_SOC_DAIFMT_CBM_CFS:
	default:
		debug("%s: Clock mode unsupported\n", __func__);
		return -1;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		regval |= M98095_DAI_DLY;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		break;
	default:
		debug("%s: Unrecognized format.\n", __func__);
		return -1;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		break;
	case SND_SOC_DAIFMT_NB_IF:
		regval |= M98095_DAI_WCI;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		regval |= M98095_DAI_BCI;
		break;
	case SND_SOC_DAIFMT_IB_IF:
		regval |= M98095_DAI_BCI | M98095_DAI_WCI;
		break;
	default:
		debug("%s: Unrecognized inversion settings.\n", __func__);
		return -1;
	}

	error |= max98095_update_bits(M98095_DAI_FORMAT,
				      M98095_DAI_MAS | M98095_DAI_DLY |
				      M98095_DAI_BCI | M98095_DAI_WCI,
				      regval);

	error |= max98095_i2c_write(M98095_DAI_CLOCK,
				    M98095_DAI_BSEL64);

	if (error < 0) {
		debug("%s: Error setting i2s format.\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * resets the audio codec
 *
 * @return -1 for error and 0 success.
 */
static int max98095_reset(void)
{
	int i, ret;

	/*
	 * Gracefully reset the DSP core and the codec hardware in a proper
	 * sequence.
	 */
	ret = max98095_i2c_write(M98095_00F_HOST_CFG, 0);
	if (ret != 0) {
		debug("%s: Failed to reset DSP: %d\n", __func__, ret);
		return ret;
	}

	ret = max98095_i2c_write(M98095_097_PWR_SYS, 0);
	if (ret != 0) {
		debug("%s: Failed to reset codec: %d\n", __func__, ret);
		return ret;
	}

	/*
	 * Reset to hardware default for registers, as there is not a soft
	 * reset hardware control register.
	 */
	for (i = M98095_010_HOST_INT_CFG; i < M98095_REG_MAX_CACHED; i++) {
		ret = max98095_i2c_write(i, 0);
		if (ret < 0) {
			debug("%s: Failed to reset: %d\n", __func__, ret);
			return ret;
		}
	}

	return 0;
}

/*
 * Intialise max98095 codec device
 *
 * @param max98095	max98095 information
 *
 * @returns -1 for error  and 0 Success.
 */
static int max98095_device_init(struct max98095_priv *max98095,
				enum en_max_audio_interface aif_id)
{
	unsigned char id;
	int error = 0;

	/* reset the codec, the DSP core, and disable all interrupts */
	error = max98095_reset();
	if (error != 0) {
		debug("Reset\n");
		return error;
	}

	/* initialize private data */
	max98095->sysclk = -1U;
	max98095->rate = -1U;
	max98095->fmt = -1U;

	error = max98095_i2c_read(M98095_0FF_REV_ID, &id);
	if (error < 0) {
		debug("%s: Failure reading hardware revision: %d\n",
		      __func__, id);
		goto err_access;
	}
	debug("%s: Hardware revision: %c\n", __func__, (id - 0x40) + 'A');

	error |= max98095_i2c_write(M98095_097_PWR_SYS, M98095_PWRSV);

	/*
	 * initialize registers to hardware default configuring audio
	 * interface2 to DAC
	 */
	if (aif_id == AIF1)
		error |= max98095_i2c_write(M98095_048_MIX_DAC_LR,
					    M98095_DAI1L_TO_DACL |
					    M98095_DAI1R_TO_DACR);
	else
		error |= max98095_i2c_write(M98095_048_MIX_DAC_LR,
					    M98095_DAI2M_TO_DACL |
					    M98095_DAI2M_TO_DACR);

	error |= max98095_i2c_write(M98095_092_PWR_EN_OUT,
				    M98095_SPK_SPREADSPECTRUM);
	error |= max98095_i2c_write(M98095_04E_CFG_HP, M98095_HPNORMAL);
	if (aif_id == AIF1)
		error |= max98095_i2c_write(M98095_02C_DAI1_IOCFG,
					    M98095_S1NORMAL | M98095_SDATA);
	else
		error |= max98095_i2c_write(M98095_036_DAI2_IOCFG,
					    M98095_S2NORMAL | M98095_SDATA);

	/* take the codec out of the shut down */
	error |= max98095_update_bits(M98095_097_PWR_SYS, M98095_SHDNRUN,
				      M98095_SHDNRUN);
	/* route DACL and DACR output to HO and Spekers */
	error |= max98095_i2c_write(M98095_050_MIX_SPK_LEFT, 0x01); /* DACL */
	error |= max98095_i2c_write(M98095_051_MIX_SPK_RIGHT, 0x01);/* DACR */
	error |= max98095_i2c_write(M98095_04C_MIX_HP_LEFT, 0x01);  /* DACL */
	error |= max98095_i2c_write(M98095_04D_MIX_HP_RIGHT, 0x01); /* DACR */

	/* power Enable */
	error |= max98095_i2c_write(M98095_091_PWR_EN_OUT, 0xF3);

	/* set Volume */
	error |= max98095_i2c_write(M98095_064_LVL_HP_L, 15);
	error |= max98095_i2c_write(M98095_065_LVL_HP_R, 15);
	error |= max98095_i2c_write(M98095_067_LVL_SPK_L, 16);
	error |= max98095_i2c_write(M98095_068_LVL_SPK_R, 16);

	/* Enable DAIs */
	error |= max98095_i2c_write(M98095_093_BIAS_CTRL, 0x30);
	if (aif_id == AIF1)
		error |= max98095_i2c_write(M98095_096_PWR_DAC_CK, 0x01);
	else
		error |= max98095_i2c_write(M98095_096_PWR_DAC_CK, 0x07);

err_access:
	if (error < 0)
		return -1;

	return 0;
}

static int max98095_do_init(struct sound_codec_info *pcodec_info,
			    enum en_max_audio_interface aif_id,
			    int sampling_rate, int mclk_freq,
			    int bits_per_sample)
{
	int ret = 0;

	/* Enable codec clock */
	set_xclkout();

	/* shift the device address by 1 for 7 bit addressing */
	g_max98095_i2c_dev_addr = pcodec_info->i2c_dev_addr >> 1;

	if (pcodec_info->codec_type == CODEC_MAX_98095) {
		g_max98095_info.devtype = MAX98095;
	} else {
		debug("%s: Codec id [%d] not defined\n", __func__,
		      pcodec_info->codec_type);
		return -1;
	}

	ret = max98095_device_init(&g_max98095_info, aif_id);
	if (ret < 0) {
		debug("%s: max98095 codec chip init failed\n", __func__);
		return ret;
	}

	ret = max98095_set_sysclk(&g_max98095_info, mclk_freq);
	if (ret < 0) {
		debug("%s: max98095 codec set sys clock failed\n", __func__);
		return ret;
	}

	ret = max98095_hw_params(&g_max98095_info, aif_id, sampling_rate,
				 bits_per_sample);

	if (ret == 0) {
		ret = max98095_set_fmt(&g_max98095_info,
				       SND_SOC_DAIFMT_I2S |
				       SND_SOC_DAIFMT_NB_NF |
				       SND_SOC_DAIFMT_CBS_CFS,
				       aif_id);
	}

	return ret;
}

static int get_max98095_codec_values(struct sound_codec_info *pcodec_info,
				const void *blob)
{
	int error = 0;
#ifdef CONFIG_OF_CONTROL
	enum fdt_compat_id compat;
	int node;
	int parent;

	/* Get the node from FDT for codec */
	node = fdtdec_next_compatible(blob, 0, COMPAT_MAXIM_98095_CODEC);
	if (node <= 0) {
		debug("EXYNOS_SOUND: No node for codec in device tree\n");
		debug("node = %d\n", node);
		return -1;
	}

	parent = fdt_parent_offset(blob, node);
	if (parent < 0) {
		debug("%s: Cannot find node parent\n", __func__);
		return -1;
	}

	compat = fdtdec_lookup(blob, parent);
	switch (compat) {
	case COMPAT_SAMSUNG_S3C2440_I2C:
		pcodec_info->i2c_bus = i2c_get_bus_num_fdt(parent);
		error |= pcodec_info->i2c_bus;
		debug("i2c bus = %d\n", pcodec_info->i2c_bus);
		pcodec_info->i2c_dev_addr = fdtdec_get_int(blob, node,
							"reg", 0);
		error |= pcodec_info->i2c_dev_addr;
		debug("i2c dev addr = %x\n", pcodec_info->i2c_dev_addr);
		break;
	default:
		debug("%s: Unknown compat id %d\n", __func__, compat);
		return -1;
	}
#else
	pcodec_info->i2c_bus = AUDIO_I2C_BUS;
	pcodec_info->i2c_dev_addr = AUDIO_I2C_REG;
	debug("i2c dev addr = %d\n", pcodec_info->i2c_dev_addr);
#endif
	pcodec_info->codec_type = CODEC_MAX_98095;
	if (error == -1) {
		debug("fail to get max98095 codec node properties\n");
		return -1;
	}

	return 0;
}

/* max98095 Device Initialisation */
int max98095_init(const void *blob, enum en_max_audio_interface aif_id,
		  int sampling_rate, int mclk_freq,
		  int bits_per_sample)
{
	int ret;
	int old_bus = i2c_get_bus_num();
	struct sound_codec_info *pcodec_info = &g_codec_info;

	if (get_max98095_codec_values(pcodec_info, blob) < 0) {
		debug("FDT Codec values failed\n");
		return -1;
	}

	i2c_set_bus_num(pcodec_info->i2c_bus);
	ret = max98095_do_init(pcodec_info, aif_id, sampling_rate, mclk_freq,
			       bits_per_sample);
	i2c_set_bus_num(old_bus);

	return ret;
}
