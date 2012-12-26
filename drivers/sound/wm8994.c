/*
 * Copyright (C) 2012 Samsung Electronics
 * R. Chandrasekar <rcsekar@samsung.com>
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
#include <asm/arch/clk.h>
#include <asm/arch/cpu.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <common.h>
#include <div64.h>
#include <fdtdec.h>
#include <i2c.h>
#include <i2s.h>
#include <sound.h>
#include <asm/arch/sound.h>
#include "wm8994.h"
#include "wm8994_registers.h"

/* defines for wm8994 system clock selection */
#define SEL_MCLK1	0x00
#define SEL_MCLK2	0x08
#define SEL_FLL1	0x10
#define SEL_FLL2	0x18

/* fll config to configure fll */
struct wm8994_fll_config {
	int src;	/* Source */
	int in;		/* Input frequency in Hz */
	int out;	/* output frequency in Hz */
};

/* codec private data */
struct wm8994_priv {
	enum wm8994_type type;		/* codec type of wolfson */
	int revision;			/* Revision */
	int sysclk[WM8994_MAX_AIF];	/* System clock frequency in Hz  */
	int mclk[WM8994_MAX_AIF];	/* master clock frequency in Hz */
	int aifclk[WM8994_MAX_AIF];	/* audio interface clock in Hz   */
	struct wm8994_fll_config fll[2]; /* fll config to configure fll */
};

/* wm 8994 supported sampling rate values */
static unsigned int src_rate[] = {
			 8000, 11025, 12000, 16000, 22050, 24000,
			 32000, 44100, 48000, 88200, 96000
};

/* op clock divisions */
static int opclk_divs[] = { 10, 20, 30, 40, 55, 60, 80, 120, 160 };

/* lr clock frame size ratio */
static int fs_ratios[] = {
	64, 128, 192, 256, 348, 512, 768, 1024, 1408, 1536
};

/* bit clock divisors */
static int bclk_divs[] = {
	10, 15, 20, 30, 40, 50, 60, 80, 110, 120, 160, 220, 240, 320, 440, 480,
	640, 880, 960, 1280, 1760, 1920
};

static struct wm8994_priv g_wm8994_info;
static unsigned char g_wm8994_i2c_dev_addr;
static struct sound_codec_info g_codec_info;

/*
 * Initialise I2C for wm 8994
 *
 * @param bus no	i2c bus number in which wm8994 is connected
 */
static void wm8994_i2c_init(int bus_no)
{
	i2c_set_bus_num(bus_no);
}

/*
 * Writes value to a device register through i2c
 *
 * @param reg	reg number to be write
 * @param data	data to be writen to the above registor
 *
 * @return	int value 1 for change, 0 for no change or negative error code.
 */
static int wm8994_i2c_write(unsigned int reg, unsigned short data)
{
	unsigned char val[2];

	val[0] = (unsigned char)((data >> 8) & 0xff);
	val[1] = (unsigned char)(data & 0xff);
	debug("Write Addr : 0x%04X, Data :  0x%04X\n", reg, data);

	return i2c_write(g_wm8994_i2c_dev_addr, reg, 2, val, 2);
}

/*
 * Read a value from a device register through i2c
 *
 * @param reg	reg number to be read
 * @param data	address of read data to be stored
 *
 * @return	int value 0 for success, -1 in case of error.
 */
static unsigned int  wm8994_i2c_read(unsigned int reg , unsigned short *data)
{
	unsigned char val[2];
	int ret;

	ret = i2c_read(g_wm8994_i2c_dev_addr, reg, 2, val, 2);
	if (ret != 0) {
		debug("%s: Error while reading register %#04x\n",
		      __func__, reg);
		return -1;
	}

	*data = val[0];
	*data <<= 8;
	*data |= val[1];

	return 0;
}

/*
 * update device register bits through i2c
 *
 * @param reg	codec register
 * @param mask	register mask
 * @param value	new value
 *
 * @return int value 1 if change in the register value,
 * 0 for no change or negative error code.
 */
static int wm8994_update_bits(unsigned int reg, unsigned short mask,
						unsigned short value)
{
	int change , ret = 0;
	unsigned short old, new;

	if (wm8994_i2c_read(reg, &old) != 0)
		return -1;
	new = (old & ~mask) | (value & mask);
	change  = (old != new) ? 1 : 0;
	if (change)
		ret = wm8994_i2c_write(reg, new);
	if (ret < 0)
		return ret;

	return change;
}

/*
 * Sets i2s set format
 *
 * @param aif_id	Interface ID
 * @param fmt		i2S format
 *
 * @return -1 for error and 0  Success.
 */
int wm8994_set_fmt(int aif_id, unsigned int fmt)
{
	int ms_reg;
	int aif_reg;
	int ms = 0;
	int aif = 0;
	int aif_clk = 0;
	int error = 0;

	switch (aif_id) {
	case 1:
		ms_reg = WM8994_AIF1_MASTER_SLAVE;
		aif_reg = WM8994_AIF1_CONTROL_1;
		aif_clk = WM8994_AIF1_CLOCKING_1;
		break;
	case 2:
		ms_reg = WM8994_AIF2_MASTER_SLAVE;
		aif_reg = WM8994_AIF2_CONTROL_1;
		aif_clk = WM8994_AIF2_CLOCKING_1;
		break;
	default:
		debug("%s: Invalid audio interface selection\n", __func__);
		return -1;
	}

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFS:
		break;
	case SND_SOC_DAIFMT_CBM_CFM:
		ms = WM8994_AIF1_MSTR;
		break;
	default:
		debug("%s: Invalid i2s master selection\n", __func__);
		return -1;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_DSP_B:
		aif |= WM8994_AIF1_LRCLK_INV;
	case SND_SOC_DAIFMT_DSP_A:
		aif |= 0x18;
		break;
	case SND_SOC_DAIFMT_I2S:
		aif |= 0x10;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		aif |= 0x8;
		break;
	default:
		debug("%s: Invalid i2s format selection\n", __func__);
		return -1;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_DSP_A:
	case SND_SOC_DAIFMT_DSP_B:
		/* frame inversion not valid for DSP modes */
		switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
		case SND_SOC_DAIFMT_NB_NF:
			break;
		case SND_SOC_DAIFMT_IB_NF:
			aif |= WM8994_AIF1_BCLK_INV;
			break;
		default:
			debug("%s: Invalid i2s frame inverse selection\n",
			      __func__);
			return -1;
		}
		break;

	case SND_SOC_DAIFMT_I2S:
	case SND_SOC_DAIFMT_RIGHT_J:
	case SND_SOC_DAIFMT_LEFT_J:
		switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
		case SND_SOC_DAIFMT_NB_NF:
			break;
		case SND_SOC_DAIFMT_IB_IF:
			aif |= WM8994_AIF1_BCLK_INV | WM8994_AIF1_LRCLK_INV;
			break;
		case SND_SOC_DAIFMT_IB_NF:
			aif |= WM8994_AIF1_BCLK_INV;
			break;
		case SND_SOC_DAIFMT_NB_IF:
			aif |= WM8994_AIF1_LRCLK_INV;
			break;
		default:
			debug("%s: Invalid i2s clock polarity selection\n",
			      __func__);
			return -1;
		}
		break;
	default:
		debug("%s: Invalid i2s format selection\n", __func__);
		return -1;
	}

	error = wm8994_update_bits(aif_reg, WM8994_AIF1_BCLK_INV |
			WM8994_AIF1_LRCLK_INV_MASK | WM8994_AIF1_FMT_MASK, aif);

	error |= wm8994_update_bits(ms_reg, WM8994_AIF1_MSTR_MASK, ms);
	error |= wm8994_update_bits(aif_clk, WM8994_AIF1CLK_ENA_MASK,
						WM8994_AIF1CLK_ENA);
	if (error < 0) {
		debug("%s: codec register access error\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * Sets hw params FOR WM8994
 *
 * @param wm8994		wm8994 information pointer
 * @param aif_id		Audio interface ID
 * @param sampling_rate		Sampling rate
 * @param bits_per_sample	Bits per sample
 * @param Channels		Channels in the given audio input
 *
 * @return -1 for error  and 0  Success.
 */
static int wm8994_hw_params(struct wm8994_priv *wm8994, int aif_id,
		unsigned int sampling_rate, unsigned int bits_per_sample,
		unsigned int channels)
{
	int aif1_reg;
	int aif2_reg;
	int bclk_reg;
	int bclk = 0;
	int rate_reg;
	int aif1 = 0;
	int aif2 = 0;
	int rate_val = 0;
	int id = aif_id - 1;
	int i, cur_val, best_val, bclk_rate, best;
	unsigned short reg_data;
	int ret = 0;

	switch (aif_id) {
	case 1:
		aif1_reg = WM8994_AIF1_CONTROL_1;
		aif2_reg = WM8994_AIF1_CONTROL_2;
		bclk_reg = WM8994_AIF1_BCLK;
		rate_reg = WM8994_AIF1_RATE;
		break;
	case 2:
		aif1_reg = WM8994_AIF2_CONTROL_1;
		aif2_reg = WM8994_AIF2_CONTROL_2;
		bclk_reg = WM8994_AIF2_BCLK;
		rate_reg = WM8994_AIF2_RATE;
		break;
	default:
		return -1;
	}

	bclk_rate = sampling_rate * 32;
	switch (bits_per_sample) {
	case 16:
		bclk_rate *= 16;
		break;
	case 20:
		bclk_rate *= 20;
		aif1 |= 0x20;
		break;
	case 24:
		bclk_rate *= 24;
		aif1 |= 0x40;
		break;
	case 32:
		bclk_rate *= 32;
		aif1 |= 0x60;
		break;
	default:
		return -1;
	}

	/* Try to find an appropriate sample rate; look for an exact match. */
	for (i = 0; i < ARRAY_SIZE(src_rate); i++)
		if (src_rate[i] == sampling_rate)
			break;

	if (i == ARRAY_SIZE(src_rate)) {
		debug("%s: Could not get the best matching samplingrate\n",
		      __func__);
		return -1;
	}

	rate_val |= i << WM8994_AIF1_SR_SHIFT;

	/* AIFCLK/fs ratio; look for a close match in either direction */
	best = 0;
	best_val = abs((fs_ratios[0] * sampling_rate)
						- wm8994->aifclk[id]);

	for (i = 1; i < ARRAY_SIZE(fs_ratios); i++) {
		cur_val = abs((fs_ratios[i] * sampling_rate)
					- wm8994->aifclk[id]);
		if (cur_val >= best_val)
			continue;
		best = i;
		best_val = cur_val;
	}

	rate_val |= best;

	/*
	 * We may not get quite the right frequency if using
	 * approximate clocks so look for the closest match that is
	 * higher than the target (we need to ensure that there enough
	 * BCLKs to clock out the samples).
	 */
	best = 0;
	for (i = 0; i < ARRAY_SIZE(bclk_divs); i++) {
		cur_val = (wm8994->aifclk[id] * 10 / bclk_divs[i]) - bclk_rate;
		if (cur_val < 0) /* BCLK table is sorted */
			break;
		best = i;
	}

	if (i ==  ARRAY_SIZE(bclk_divs)) {
		debug("%s: Could not get the best matching bclk division\n",
		      __func__);
		return -1;
	}

	bclk_rate = wm8994->aifclk[id] * 10 / bclk_divs[best];
	bclk |= best << WM8994_AIF1_BCLK_DIV_SHIFT;

	if (wm8994_i2c_read(aif1_reg, &reg_data) != 0) {
		debug("%s: AIF1 register read Failed\n", __func__);
		return -1;
	}

	if ((channels == 1) && ((reg_data & 0x18) == 0x18))
		aif2 |= WM8994_AIF1_MONO;

	if (wm8994->aifclk[id] == 0) {
		debug("%s:Audio interface clock not set\n", __func__);
		return -1;
	}

	ret = wm8994_update_bits(aif1_reg, WM8994_AIF1_WL_MASK, aif1);
	ret |= wm8994_update_bits(aif2_reg, WM8994_AIF1_MONO, aif2);
	ret |= wm8994_update_bits(bclk_reg, WM8994_AIF1_BCLK_DIV_MASK, bclk);
	ret |= wm8994_update_bits(rate_reg, WM8994_AIF1_SR_MASK |
				WM8994_AIF1CLK_RATE_MASK, rate_val);

	debug("rate vale = %x , bclk val= %x\n", rate_val, bclk);

	if (ret < 0) {
		debug("%s: codec register access error\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * Configures Audio interface Clock
 *
 * @param wm8994	wm8994 information pointer
 * @param aif		Audio Interface ID
 *
 * @return -1 for error  and 0  Success.
 */
static int configure_aif_clock(struct wm8994_priv *wm8994, int aif)
{
	int rate;
	int reg1 = 0;
	int offset;
	int ret;

	/* AIF(1/0) register adress offset calculated */
	if (aif)
		offset = 4;
	else
		offset = 0;

	switch (wm8994->sysclk[aif]) {
	case WM8994_SYSCLK_MCLK1:
		reg1 |= SEL_MCLK1;
		rate = wm8994->mclk[0];
		break;

	case WM8994_SYSCLK_MCLK2:
		reg1 |= SEL_MCLK2;
		rate = wm8994->mclk[1];
		break;

	case WM8994_SYSCLK_FLL1:
		reg1 |= SEL_FLL1;
		rate = wm8994->fll[0].out;
		break;

	case WM8994_SYSCLK_FLL2:
		reg1 |= SEL_FLL2;
		rate = wm8994->fll[1].out;
		break;

	default:
		debug("%s: Invalid input clock selection [%d]\n",
		      __func__, wm8994->sysclk[aif]);
		return -1;
	}

	/* if input clock frequenct is more than 135Mhz then divide */
	if (rate >= WM8994_MAX_INPUT_CLK_FREQ) {
		rate /= 2;
		reg1 |= WM8994_AIF1CLK_DIV;
	}

	wm8994->aifclk[aif] = rate;

	ret = wm8994_update_bits(WM8994_AIF1_CLOCKING_1 + offset,
				WM8994_AIF1CLK_SRC_MASK | WM8994_AIF1CLK_DIV,
				reg1);

	ret |= wm8994_update_bits(WM8994_CLOCKING_1,
			WM8994_SYSCLK_SRC | WM8994_AIF2DSPCLK_ENA_MASK |
			WM8994_SYSDSPCLK_ENA_MASK, WM8994_SYSCLK_SRC |
			WM8994_AIF2DSPCLK_ENA | WM8994_SYSDSPCLK_ENA);

	if (ret < 0) {
		debug("%s: codec register access error\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * Configures Audio interface  for the given frequency
 *
 * @param wm8994	wm8994 information
 * @param aif_id	Audio Interface
 * @param clk_id	Input Clock ID
 * @param freq		Sampling frequency in Hz
 *
 * @return -1 for error and 0 success.
 */
static int wm8994_set_sysclk(struct wm8994_priv *wm8994, int aif_id,
				int clk_id, unsigned int freq)
{
	int i;
	int ret = 0;

	wm8994->sysclk[aif_id - 1] = clk_id;

	switch (clk_id) {
	case WM8994_SYSCLK_MCLK1:
		wm8994->mclk[0] = freq;
		if (aif_id == 2) {
			ret = wm8994_update_bits(WM8994_AIF1_CLOCKING_2 ,
			WM8994_AIF2DAC_DIV_MASK , 0);
		}
		break;

	case WM8994_SYSCLK_MCLK2:
		/* TODO: Set GPIO AF */
		wm8994->mclk[1] = freq;
		break;

	case WM8994_SYSCLK_FLL1:
	case WM8994_SYSCLK_FLL2:
		break;

	case WM8994_SYSCLK_OPCLK:
		/*
		 * Special case - a division (times 10) is given and
		 * no effect on main clocking.
		 */
		if (freq) {
			for (i = 0; i < ARRAY_SIZE(opclk_divs); i++)
				if (opclk_divs[i] == freq)
					break;
			if (i == ARRAY_SIZE(opclk_divs)) {
				debug("%s frequency divisor not found\n",
					__func__);
				return -1;
			}
			ret = wm8994_update_bits(WM8994_CLOCKING_2,
					    WM8994_OPCLK_DIV_MASK, i);
			ret |= wm8994_update_bits(WM8994_POWER_MANAGEMENT_2,
					    WM8994_OPCLK_ENA, WM8994_OPCLK_ENA);
		} else {
			ret |= wm8994_update_bits(WM8994_POWER_MANAGEMENT_2,
					    WM8994_OPCLK_ENA, 0);
		}

	default:
		debug("%s Invalid input clock selection [%d]\n",
		      __func__, clk_id);
		return -1;
	}

	ret |= configure_aif_clock(wm8994, aif_id - 1);

	if (ret < 0) {
		debug("%s: codec register access error\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * Initializes Volume for AIF2 to HP path
 *
 * @returns -1 for error  and 0 Success.
 *
 */
static int wm8994_init_volume_aif2_dac1(void)
{
	int ret;

	/* Unmute AIF2DAC */
	ret = wm8994_update_bits(WM8994_AIF2_DAC_FILTERS_1,
			WM8994_AIF2DAC_MUTE_MASK, 0);


	ret |= wm8994_update_bits(WM8994_AIF2_DAC_LEFT_VOLUME,
			WM8994_AIF2DAC_VU_MASK | WM8994_AIF2DACL_VOL_MASK,
			WM8994_AIF2DAC_VU | 0xff);

	ret |= wm8994_update_bits(WM8994_AIF2_DAC_RIGHT_VOLUME,
			WM8994_AIF2DAC_VU_MASK | WM8994_AIF2DACR_VOL_MASK,
			WM8994_AIF2DAC_VU | 0xff);


	ret |= wm8994_update_bits(WM8994_DAC1_LEFT_VOLUME,
			WM8994_DAC1_VU_MASK | WM8994_DAC1L_VOL_MASK |
			WM8994_DAC1L_MUTE_MASK, WM8994_DAC1_VU | 0xc0);

	ret |= wm8994_update_bits(WM8994_DAC1_RIGHT_VOLUME,
			WM8994_DAC1_VU_MASK | WM8994_DAC1R_VOL_MASK |
			WM8994_DAC1R_MUTE_MASK, WM8994_DAC1_VU | 0xc0);
	/* Head Phone Volume */
	ret |= wm8994_i2c_write(WM8994_LEFT_OUTPUT_VOLUME, 0x12D);
	ret |= wm8994_i2c_write(WM8994_RIGHT_OUTPUT_VOLUME, 0x12D);

	if (ret < 0) {
		debug("%s: codec register access error\n", __func__);
		return -1;
	}

	return 0;
}

/*
 * Intialise wm8994 codec device
 *
 * @param wm8994	wm8994 information
 *
 * @returns -1 for error  and 0 Success.
 */
static int wm8994_device_init(struct wm8994_priv *wm8994)
{
	const char *devname;
	unsigned short reg_data;
	int ret;

	wm8994_i2c_write(WM8994_SOFTWARE_RESET, WM8994_SW_RESET);/* Reset */

	ret = wm8994_i2c_read(WM8994_SOFTWARE_RESET, &reg_data);
	if (ret < 0) {
		debug("Failed to read ID register\n");
		goto err;
	}

	if (reg_data == WM8994_ID) {
		devname = "WM8994";
		debug("Device registered as type %d\n", wm8994->type);
		wm8994->type = WM8994;
	} else {
		debug("Device is not a WM8994, ID is %x\n", ret);
		ret = -1;
		goto err;
	}

	ret = wm8994_i2c_read(WM8994_CHIP_REVISION, &reg_data);
	if (ret < 0) {
		debug("Failed to read revision register: %d\n", ret);
		goto err;
	}
	wm8994->revision = reg_data;
	debug("%s revision %c\n", devname, 'A' + wm8994->revision);

	/* VMID Selection */
	ret |= wm8994_update_bits(WM8994_POWER_MANAGEMENT_1,
			WM8994_VMID_SEL_MASK | WM8994_BIAS_ENA_MASK, 0x3);

	/* Charge Pump Enable */
	ret |= wm8994_update_bits(WM8994_CHARGE_PUMP_1, WM8994_CP_ENA_MASK,
					WM8994_CP_ENA);

	/* Head Phone Power Enable */
	ret |= wm8994_update_bits(WM8994_POWER_MANAGEMENT_1,
			WM8994_HPOUT1L_ENA_MASK, WM8994_HPOUT1L_ENA);

	ret |= wm8994_update_bits(WM8994_POWER_MANAGEMENT_1,
				WM8994_HPOUT1R_ENA_MASK, WM8994_HPOUT1R_ENA);

	/* Power enable for AIF2 and DAC1 */
	ret |= wm8994_update_bits(WM8994_POWER_MANAGEMENT_5,
		WM8994_AIF2DACL_ENA_MASK | WM8994_AIF2DACR_ENA_MASK |
		WM8994_DAC1L_ENA_MASK | WM8994_DAC1R_ENA_MASK,
		WM8994_AIF2DACL_ENA | WM8994_AIF2DACR_ENA | WM8994_DAC1L_ENA |
		WM8994_DAC1R_ENA);

	/* Head Phone Initialisation */
	ret |= wm8994_update_bits(WM8994_ANALOGUE_HP_1,
		WM8994_HPOUT1L_DLY_MASK | WM8994_HPOUT1R_DLY_MASK,
		WM8994_HPOUT1L_DLY | WM8994_HPOUT1R_DLY);

	ret |= wm8994_update_bits(WM8994_DC_SERVO_1,
			WM8994_DCS_ENA_CHAN_0_MASK |
			WM8994_DCS_ENA_CHAN_1_MASK , WM8994_DCS_ENA_CHAN_0 |
			WM8994_DCS_ENA_CHAN_1);

	ret |= wm8994_update_bits(WM8994_ANALOGUE_HP_1,
			WM8994_HPOUT1L_DLY_MASK |
			WM8994_HPOUT1R_DLY_MASK | WM8994_HPOUT1L_OUTP_MASK |
			WM8994_HPOUT1R_OUTP_MASK |
			WM8994_HPOUT1L_RMV_SHORT_MASK |
			WM8994_HPOUT1R_RMV_SHORT_MASK, WM8994_HPOUT1L_DLY |
			WM8994_HPOUT1R_DLY | WM8994_HPOUT1L_OUTP |
			WM8994_HPOUT1R_OUTP | WM8994_HPOUT1L_RMV_SHORT |
			WM8994_HPOUT1R_RMV_SHORT);

	/* MIXER Config DAC1 to HP */
	ret |= wm8994_update_bits(WM8994_OUTPUT_MIXER_1,
			WM8994_DAC1L_TO_HPOUT1L_MASK, WM8994_DAC1L_TO_HPOUT1L);

	ret |= wm8994_update_bits(WM8994_OUTPUT_MIXER_2,
			WM8994_DAC1R_TO_HPOUT1R_MASK, WM8994_DAC1R_TO_HPOUT1R);

	/* Routing AIF2 to DAC1 */
	ret |= wm8994_update_bits(WM8994_DAC1_LEFT_MIXER_ROUTING,
			WM8994_AIF2DACL_TO_DAC1L_MASK,
			WM8994_AIF2DACL_TO_DAC1L);

	ret |= wm8994_update_bits(WM8994_DAC1_RIGHT_MIXER_ROUTING,
			WM8994_AIF2DACR_TO_DAC1R_MASK,
			WM8994_AIF2DACR_TO_DAC1R);

	 /* GPIO Settings for AIF2 */
	 /* B CLK */
	ret |= wm8994_update_bits(WM8994_GPIO_3, WM8994_GPIO_DIR_MASK |
				WM8994_GPIO_FUNCTION_MASK ,
				WM8994_GPIO_DIR_OUTPUT |
				WM8994_GPIO_FUNCTION_I2S_CLK);

	/* LR CLK */
	ret |= wm8994_update_bits(WM8994_GPIO_4, WM8994_GPIO_DIR_MASK |
				WM8994_GPIO_FUNCTION_MASK,
				WM8994_GPIO_DIR_OUTPUT |
				WM8994_GPIO_FUNCTION_I2S_CLK);

	/* DATA */
	ret |= wm8994_update_bits(WM8994_GPIO_5, WM8994_GPIO_DIR_MASK |
				WM8994_GPIO_FUNCTION_MASK,
				WM8994_GPIO_DIR_OUTPUT |
				WM8994_GPIO_FUNCTION_I2S_CLK);

	ret |= wm8994_init_volume_aif2_dac1();
	if (ret < 0)
		goto err;

	debug("%s: Codec chip init ok\n", __func__);
	return 0;
err:
	debug("%s: Codec chip init error\n", __func__);
	return -1;
}

/*
 * Gets fdt values for wm8994 config parameters
 *
 * @param pcodec_info	codec information structure
 * @param blob		FDT blob
 * @return		int value, 0 for success
 */
static int get_codec_values(struct sound_codec_info *pcodec_info,
			const void *blob)
{
	int error = 0;
#ifdef CONFIG_OF_CONTROL
	enum fdt_compat_id compat;
	int node;
	int parent;

	/* Get the node from FDT for codec */
	node = fdtdec_next_compatible(blob, 0, COMPAT_WOLFSON_WM8994_CODEC);
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
		debug("i2c dev addr = %d\n", pcodec_info->i2c_dev_addr);
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

	pcodec_info->codec_type = CODEC_WM_8994;

	if (error == -1) {
		debug("fail to get wm8994 codec node properties\n");
		return -1;
	}

	return 0;
}

/*wm8994 Device Initialisation */
int wm8994_init(const void *blob, enum en_audio_interface aif_id,
			int sampling_rate, int mclk_freq,
			int bits_per_sample, unsigned int channels)
{
	int ret = 0;
	struct sound_codec_info *pcodec_info = &g_codec_info;

	/* Get the codec Values */
	if (get_codec_values(pcodec_info, blob) < 0) {
		debug("FDT Codec values failed\n");
		return -1;
	}

	/* shift the device address by 1 for 7 bit addressing */
	g_wm8994_i2c_dev_addr = pcodec_info->i2c_dev_addr;
	wm8994_i2c_init(pcodec_info->i2c_bus);

	if (pcodec_info->codec_type == CODEC_WM_8994)
		g_wm8994_info.type = WM8994;
	else {
		debug("%s: Codec id [%d] not defined\n", __func__,
				pcodec_info->codec_type);
		return -1;
	}

	ret = wm8994_device_init(&g_wm8994_info);
	if (ret < 0) {
		debug("%s: wm8994 codec chip init failed\n", __func__);
		return ret;
	}

	ret =  wm8994_set_sysclk(&g_wm8994_info, aif_id, WM8994_SYSCLK_MCLK1,
							mclk_freq);
	if (ret < 0) {
		debug("%s: wm8994 codec set sys clock failed\n", __func__);
		return ret;
	}

	ret = wm8994_hw_params(&g_wm8994_info, aif_id, sampling_rate,
						bits_per_sample, channels);

	if (ret == 0) {
		ret = wm8994_set_fmt(aif_id, SND_SOC_DAIFMT_I2S |
						SND_SOC_DAIFMT_NB_NF |
						SND_SOC_DAIFMT_CBS_CFS);
	}
	return ret;
}
