/*
 * Copyright 2018-2022 NXP
 * Copyright 2018 INPHI
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Inphi is a registered trademark of Inphi Corporation
 *
 */
#include <config.h>
#include <malloc.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/stringify.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <dm/device_compat.h>
#include <phy.h>
#include <fsl-mc/ldpaa_wriop.h>
#include <in112525.h>
#ifdef CONFIG_SYS_IN112525_FW_IN_MMC
#include <mmc.h>
#endif

#ifndef CONFIG_PHYLIB_10G
#error The INPHI PHY needs 10G support
#endif

/* lookup table to map the multiply options and MUX selects to the bit values */
unsigned char tx_pll_mpy_map[][2] = {
	/* MPY  MS, LS */
	[10] = {0,  0},
	[20] = {1,  0},
	[40] = {2,  0},
	[8]  = {0,  1},
	[16] = {1,  1},
	[32] = {2,  1},
	[33] = {1,  6},
	[66] = {2,  6},
	[15] = {0,  7},
	[30] = {1,  7},
	[60] = {2,  7},
};

static struct in112525_config inphi_s03_config[] = {
	[INIT_OC192] = { .enable_otu_protocol = 0,
			 .enable_external_refclk = 0,
			 .enable_prescaler = 0,
			 .tx_pll_mpy_ratio = 20,
			 .enable_half_rate = 1,
			 .enable_extended_range = 1,
			 .tx_pll_refclk_source = RECOV_CLK,
			 .ctle_mode = MODE_25_25_10,
			 .rx_common_mode = 3,
			 .rx_odt_override = 0,
			 .l0_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			 .l1_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			 .l2_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			 .l3_phase_adjust_val = IN112525_PHASE_ADJUST_VAL
	},

	[INIT_10GE] = { .enable_otu_protocol = 0,
			.enable_external_refclk = 0,
			.enable_prescaler = 0,
			.tx_pll_mpy_ratio = 20,
			.enable_half_rate = 1,
			.enable_extended_range = 1,
			.tx_pll_refclk_source = RECOV_CLK,
			.ctle_mode = MODE_25_25_10,
			.rx_common_mode = 3,
			.rx_odt_override = 0,
			.l0_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			.l1_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			.l2_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			.l3_phase_adjust_val = IN112525_PHASE_ADJUST_VAL
	},

	[INIT_16GFC] = { .enable_otu_protocol = 0,
			 .enable_external_refclk = 0,
			 .enable_prescaler = 0,
			 .tx_pll_mpy_ratio = 20,
			 .enable_half_rate = 1,
			 .enable_extended_range = 1,
			 .tx_pll_refclk_source = RECOV_CLK,
			 .ctle_mode = MODE_25_25_10,
			 .rx_common_mode = 3,
			 .rx_odt_override = 0,
			 .l0_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			 .l1_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			 .l2_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			 .l3_phase_adjust_val = IN112525_PHASE_ADJUST_VAL
	},

	[INIT_20GE] = { .enable_otu_protocol = 0,
			.enable_external_refclk = 0,
			.enable_prescaler = 0,
			.tx_pll_mpy_ratio = 10,
			.enable_half_rate = 0,
			.enable_extended_range = 1,
			.tx_pll_refclk_source = RECOV_CLK,
			.ctle_mode = MODE_25_25_10,
			.rx_common_mode = 3,
			.rx_odt_override = 0,
			.l0_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			.l1_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			.l2_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			.l3_phase_adjust_val = IN112525_PHASE_ADJUST_VAL
	},

	[INIT_100GE] = { .enable_otu_protocol = 0,
			 .enable_external_refclk = 0,
			 .enable_prescaler = 0,
			 .tx_pll_mpy_ratio = 10,
			 .enable_half_rate = 0,
			 .enable_extended_range = 0,
			 .tx_pll_refclk_source = RECOV_CLK,
			 .ctle_mode = MODE_25_25_10,
			 .rx_common_mode = 3,
			 .rx_odt_override = 0,
			 .l0_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			 .l1_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			 .l2_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			 .l3_phase_adjust_val = IN112525_PHASE_ADJUST_VAL
	},

	[INIT_25GE] = { .enable_otu_protocol = 0,
			.enable_external_refclk = 0,
			.enable_prescaler = 0,
			.tx_pll_mpy_ratio = 10,
			.enable_half_rate = 0,
			.enable_extended_range = 0,
			.tx_pll_refclk_source = RECOV_CLK,
			.ctle_mode = MODE_25_25_10,
			.rx_common_mode = 3,
			.rx_odt_override = 0,
			.l0_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			.l1_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			.l2_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			.l3_phase_adjust_val = IN112525_PHASE_ADJUST_VAL
	},

	[INIT_OTU4] = { .enable_otu_protocol = 1,
			.enable_external_refclk = 0,
			.enable_prescaler = 0,
			.tx_pll_mpy_ratio = 10,
			.enable_half_rate = 0,
			.enable_extended_range = 0,
			.tx_pll_refclk_source = RECOV_CLK,
			.ctle_mode = MODE_25_25_10,
			.rx_common_mode = 3,
			.rx_odt_override = 0,
			.l0_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			.l1_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			.l2_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			.l3_phase_adjust_val = IN112525_PHASE_ADJUST_VAL
	},

	[INIT_32GFC] = { .enable_otu_protocol = 0,
			 .enable_external_refclk = 0,
			 .enable_prescaler = 0,
			 .tx_pll_mpy_ratio = 10,
			 .enable_half_rate = 0,
			 .enable_extended_range = 0,
			 .tx_pll_refclk_source = RECOV_CLK,
			 .ctle_mode = MODE_25_25_10,
			 .rx_common_mode = 3,
			 .rx_odt_override = 0,
			 .l0_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			 .l1_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			 .l2_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			 .l3_phase_adjust_val = IN112525_PHASE_ADJUST_VAL
	},

	[INIT_F28P2G] = { .enable_otu_protocol = 0,
			  .enable_external_refclk = 0,
			  .enable_prescaler = 0,
			  .tx_pll_mpy_ratio = 10,
			  .enable_half_rate = 0,
			  .enable_extended_range = 0,
			  .tx_pll_refclk_source = RECOV_CLK,
			  .ctle_mode = MODE_25_25_10,
			  .rx_common_mode = 3,
			  .rx_odt_override = 0,
			  .l0_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			  .l1_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			  .l2_phase_adjust_val = IN112525_PHASE_ADJUST_VAL,
			  .l3_phase_adjust_val = IN112525_PHASE_ADJUST_VAL
	}
};

static struct in112525_s03_vco_codes s03_vco_codes;

#ifdef CONFIG_IN112525_S03_25G
#define CURRENT_CONFIG (inphi_s03_config[INIT_25GE])
#else
#define CURRENT_CONFIG (inphi_s03_config[INIT_10GE])
#endif

#define ALL_LANES	4
#define mdio_wr(a, b)	phy_write(inphi_phydev, MDIO_MMD_VEND1, (a), (b))
#define mdio_rd(a)	phy_read(inphi_phydev, MDIO_MMD_VEND1, (a))

struct phy_device *inphi_phydev;

int in112525_upload_firmware(struct phy_device *phydev)
{
	char line_temp[0x51] = {0};
	char reg_addr[0x51] = {0};
	char reg_data[0x51] = {0};
	int i, line_cnt = 0, column_cnt = 0;
	struct in112525_reg_config fw_temp;
	char *addr = NULL;

	addr = (char *)IN112525_FW_ADDR;

#if defined(CONFIG_SYS_IN112525_FW_IN_MMC)
	int dev = CONFIG_SYS_MMC_ENV_DEV;
	u32 cnt = IN112525_FW_LENGTH / 512;
	u32 blk = IN112525_FW_ADDR / 512;
	struct mmc *mmc = find_mmc_device(CONFIG_SYS_MMC_ENV_DEV);

	if (!mmc) {
		puts("Failed to find MMC device for IN112525 ucode\n");
	} else {
		addr = malloc(IN112525_FW_LENGTH);
		printf("MMC read: dev # %u, block # %u, count %u ...\n",
		       dev, blk, cnt);
		mmc_init(mmc);
		(void)mmc->block_dev.block_read(&mmc->block_dev, blk, cnt,
						addr);
		/* flush cache after read */
		flush_cache((ulong)addr, cnt * 512);
	}
#endif
	while (*addr != 'Q') {
		i = 0;

		while (*addr != 0xa) {
			line_temp[i++] = *addr++;
			if (i > 0x50) {
				printf("IN112525 ucode not found @ 0x%p\n",
				       (char *)IN112525_FW_ADDR);
				return -1;
			}
		}

		addr++;  /* skip '\n' */
		line_cnt++;
		column_cnt = i;
		line_temp[column_cnt] = '\0';

		if (line_cnt > IN112525_FW_LENGTH) {
			printf("IN112525 ucode not found @ 0x%p\n",
			       (char *)IN112525_FW_ADDR);
			return -1;
		}
		for (i = 0; i < column_cnt; i++) {
			if (isspace(line_temp[i++]))
				break;
		}

		memcpy(reg_addr, line_temp, i);
		memcpy(reg_data, &line_temp[i], column_cnt - i);
		strim(reg_addr);
		strim(reg_data);
		fw_temp.reg_addr = (simple_strtoul(reg_addr, NULL, 0)) & 0xffff;
		fw_temp.reg_value = (simple_strtoul(reg_data, NULL, 0)) &
				     0xffff;
		/* check if garbage is present at ucode location */
		if (fw_temp.reg_addr < 0x700) {
			printf("IN112525 ucode not found @ 0x%p\n",
			       (char *)IN112525_FW_ADDR);
			return -1;
		}
		phy_write(phydev, MDIO_MMD_VEND1, fw_temp.reg_addr,
			  fw_temp.reg_value);
	}
	return 0;
}

int in112525_s05_phy_init(struct phy_device *phydev)
{
	u32 reg_value, ret;
	u32 l0_vco_code, l1_vco_code, l2_vco_code, l3_vco_code;

	phy_write(phydev, MDIO_MMD_VEND1, PHYMISC_REG11, 0);
	phy_write(phydev, MDIO_MMD_VEND1, PHYMISC_REG11, IN112525_FORCE_PC);
	phy_write(phydev, MDIO_MMD_VEND1, PHYMISC_REG11, IN112525_LOL_CTRL);

	/* The S05 retimer seems to work only when ALL lanes are locked.
	 * Datapath is Lane0<->Lane1, Lane2<->Lane3.
	 * For individual lane operation to work, pair 0-1 must be disabled
	 */
	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG0,
		  IN112525_MDIOINIT | IN112525_HRESET |
#if defined(CONFIG_IN112525_S05_10G) || defined(CONFIG_IN112525_S05_25G)
		  IN112525_LANE0_DISABLE | IN112525_LANE1_DISABLE |
#endif
		  IN112525_SRESET
		 );

	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG0,
		  IN112525_HRESET |
#if defined(CONFIG_IN112525_S05_10G) || defined(CONFIG_IN112525_S05_25G)
		  IN112525_LANE0_DISABLE | IN112525_LANE1_DISABLE |
#endif
		  IN112525_SRESET
		  );

	phy_write(phydev, MDIO_MMD_VEND1, PHYMISC_REG3,
		  IN112525_EXT_REFCLK_EN | IN112525_CTLE_10G);

	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG0,
#if defined(CONFIG_IN112525_S05_10G) || defined(CONFIG_IN112525_S05_25G)
		  IN112525_LANE0_DISABLE | IN112525_LANE1_DISABLE |
#endif
		  IN112525_SRESET);

	mdelay(10);

	reg_value = phy_read(phydev, MDIO_MMD_VEND1, IN112525_EFUSE_REG);
	if (!(reg_value & IN112525_EFUSE_DONE)) {
		printf("IN112525 phy init failed: EFUSE Done not set\n");
		return -1;
	}

	udelay(100);

	reg_value = phy_read(phydev, MDIO_MMD_VEND1, PHYMISC_REG2);
	if (!(reg_value & IN112525_CALIBRATION_DONE)) {
		printf("IN112525 phy init failed: CAL DONE not set\n");
		return -1;
	}

	phy_write(phydev, MDIO_MMD_VEND1, PHYMISC_REG2,
		  (IN112525_RX_PLL_RESET | IN112525_TX_PLL_RESET |
		   IN112525_CORE_DATAPATH_RESET | IN112525_TX_SERDES_RESET));

	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG0,
		  IN112525_MANUALRESET_SELECT |
#if defined(CONFIG_IN112525_S05_10G) || defined(CONFIG_IN112525_S05_25G)
		  IN112525_LANE0_DISABLE | IN112525_LANE1_DISABLE |
#endif
		  IN112525_SRESET);

#if defined(CONFIG_IN112525_S05_25G) || defined(CONFIG_IN112525_S05_50G)
	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG15, PHYCTRL_REG15_VAL);
	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG17, PHYCTRL_REG17_VAL);
#else
	/* 100G requires specific extended-range settings */
	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG15, PHYCTRL_REG15_VAL_EXT);
	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG17, PHYCTRL_REG17_VAL_EXT);
#endif

	/* chip internals */
	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG18, 0xff);
	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG19, 0x2d);
	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG19, 0x802d);
	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG19, 0x0);

	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG18, 0xe9);
	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG19, 0x8);
	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG19, 0x8008);
	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG19, 0x0);

	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG11,
		  IN112525_TXPLL_MSDIV | IN112525_TXPLL_IQDIV);

#if defined(CONFIG_IN112525_S05_10G) || defined(CONFIG_IN112525_S05_40G)
	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG14,
		  IN112525_RX_HALFRATE_EN);

	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG13, PHYCTRL_REG13_VAL);

	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG20,
		  IN112525_RX_LOS_EN | IN112525_RX_LOS_10G_THRESHOLD);

	phy_write(phydev, MDIO_MMD_VEND1, PHYMISC_REG30,
		  IN112525_RX_MISC_TRIM1_VAL);
#else
	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG13,
		  PHYCTRL_REG13_VAL | IN112525_LOSD_HYSTERESIS_EN);

	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG20,
		  IN112525_RX_LOS_EN | IN112525_RX_LOS_100G_THRESHOLD);
#endif
	phy_write(phydev, MDIO_MMD_VEND1, PHYMISC_REG8, IN112525_FA_WIN_SIZE);

#if !defined(CONFIG_IN112525_S05_10G) && !defined(CONFIG_IN112525_S05_40G)
	/* specific stuff required when not in half-rate operation before
	 * reading VCO codes
	 */
	phy_write(phydev, MDIO_MMD_VEND1, PHYCTRL_REG2, 0x5000);
	phy_write(phydev, MDIO_MMD_VEND1, 0x501, 0x0200);
	phy_write(phydev, MDIO_MMD_VEND1, 0x510, 0x001F);
	phy_write(phydev, MDIO_MMD_VEND1, 0x517, 0x803F);
#endif

	/* actual VCO codes reading; save codes for later */
	l0_vco_code = phy_read(phydev, MDIO_MMD_VEND1, PHYMISC_REG7);
	l1_vco_code = phy_read(phydev, MDIO_MMD_VEND1, PHYMISC_REG7 + 0x100);
	l2_vco_code = phy_read(phydev, MDIO_MMD_VEND1, PHYMISC_REG7 + 0x200);
	l3_vco_code = phy_read(phydev, MDIO_MMD_VEND1, PHYMISC_REG7 + 0x300);


#if defined(CONFIG_IN112525_S05_10G) || defined(CONFIG_IN112525_S05_40G)
	/* adjust VCOs with 20/25 ratio when in half-rate operation */
	l0_vco_code *= 4 / 5;
	l1_vco_code *= 4 / 5;
	l2_vco_code *= 4 / 5;
	l3_vco_code *= 4 / 5;

#endif

	ret = in112525_upload_firmware(phydev);
	if (ret) {
		printf("IN112525: upload firmware failed\n");
		return -1;
	}

	phy_write(phydev, MDIO_MMD_VEND1, 0x73b, l0_vco_code);
	phy_write(phydev, MDIO_MMD_VEND1, 0x73c, l1_vco_code);
	phy_write(phydev, MDIO_MMD_VEND1, 0x73d, l2_vco_code);
	phy_write(phydev, MDIO_MMD_VEND1, 0x73e, l3_vco_code);

	phy_write(phydev, MDIO_MMD_VEND1, 0x737, IN112525_PHASE_ADJUST_VAL);
	phy_write(phydev, MDIO_MMD_VEND1, 0x738, IN112525_PHASE_ADJUST_VAL);
	phy_write(phydev, MDIO_MMD_VEND1, 0x739, IN112525_PHASE_ADJUST_VAL);
	phy_write(phydev, MDIO_MMD_VEND1, 0x73a, IN112525_PHASE_ADJUST_VAL);

	phy_write(phydev, MDIO_MMD_VEND1, PHYMISC_REG12, IN112525_USEQ_FL);

	printf("IN112525: starting ucode...\n");
	phy_write(phydev, MDIO_MMD_VEND1, PHYMISC_REG11,
		  IN112525_LOL_CTRL | IN112525_USEQ_EN);

	return 0;
}

int in112525_s05_config(struct phy_device *phydev)
{
	return in112525_s05_phy_init(phydev);
}

int tx_pll_lock_test(int lane)
{
	int i, val, locked = 1;

	if (lane == ALL_LANES) {
		for (i = 0; i < ALL_LANES; i++) {
			val = mdio_rd(i * 0x100 + PHYSTAT_REG3);
			locked = locked & bit_test(val, 15);
		}
	} else {
		val = mdio_rd(lane * 0x100 + PHYSTAT_REG3);
		locked = locked & bit_test(val, 15);
	}

	return locked;
}

void tx_pll_assert(int lane)
{
	int val, recal;

	if (lane == ALL_LANES) {
		val = mdio_rd(PHYMISC_REG2);
		recal = (1 << 12);
		mdio_wr(PHYMISC_REG2, val | recal);
	} else {
		val = mdio_rd(lane * 0x100 + PHYCTRL_REG4);
		recal = (1 << 15);
		mdio_wr(lane * 0x100 + PHYCTRL_REG4, val | recal);
	}
}

void tx_pll_de_assert(int lane)
{
	int recal, val;

	if (lane == ALL_LANES) {
		val = mdio_rd(PHYMISC_REG2);
		recal = 0xefff;
		mdio_wr(PHYMISC_REG2, val & recal);
	} else {
		val = mdio_rd(lane * 0x100 + PHYCTRL_REG4);
		recal = 0x7fff;
		mdio_wr(lane * 0x100 + PHYCTRL_REG4, val & recal);
	}
}

void tx_core_assert(int lane)
{
	int recal, val, val2, core_reset;

	if (lane == 4) {
		val = mdio_rd(PHYMISC_REG2);
		recal = 1 << 10;
		mdio_wr(PHYMISC_REG2, val | recal);
	} else {
		val2 = mdio_rd(PHYMISC_REG3);
		core_reset = (1 << (lane + 8));
		mdio_wr(PHYMISC_REG3, val2 | core_reset);
	}
}

void lol_disable(int lane)
{
	int val, mask;

	val = mdio_rd(PHYMISC_REG3);
	mask = 1 << (lane + 4);
	mdio_wr(PHYMISC_REG3, val | mask);
}

void tx_core_de_assert(int lane)
{
	int val, recal, val2, core_reset;

	if (lane == ALL_LANES) {
		val = mdio_rd(PHYMISC_REG2);
		recal = 0xffff - (1 << 10);
		mdio_wr(PHYMISC_REG2, val & recal);
	} else {
		val2 = mdio_rd(PHYMISC_REG3);
		core_reset = 0xffff - (1 << (lane + 8));
		mdio_wr(PHYMISC_REG3, val2 & core_reset);
	}
}

void tx_restart(int lane)
{
	tx_core_assert(lane);
	tx_pll_assert(lane);
	tx_pll_de_assert(lane);
	WAIT(150);
	tx_core_de_assert(lane);
}

void disable_lane(int lane)
{
	rx_reset_assert(lane);
	rx_powerdown_assert(lane);
	tx_core_assert(lane);
	lol_disable(lane);
}

void WAIT(int delay_cycles)
{
	udelay(delay_cycles * 10);
}

int bit_test(int value, int bit_field)
{
	int bit_mask = (1 << bit_field);
	int result;

	result = ((value & bit_mask) == bit_mask);
	return result;
}

void toggle_reset(int lane)
{
	int reg, val, orig;

	if (lane == ALL_LANES) {
		mdio_wr(PHYMISC_REG2, 0x8000);
		WAIT(10);
		mdio_wr(PHYMISC_REG2, 0x0000);
	} else {
		reg = lane * 0x100 + PHYCTRL_REG8;
		val = (1 << 6);
		orig = mdio_rd(reg);
		mdio_wr(reg, orig + val);
		WAIT(10);
		mdio_wr(reg, orig);
	}
}

int az_complete_test(int lane)
{
	int success = 1, value;

	if (lane == 0 || lane == ALL_LANES) {
		value = mdio_rd(PHYCTRL_REG5);
		success = success & bit_test(value, 2);
	}
	if (lane == 1 || lane == ALL_LANES) {
		value = mdio_rd(PHYCTRL_REG5 + 0x100);
		success = success & bit_test(value, 2);
	}
	if (lane == 2 || lane == ALL_LANES) {
		value = mdio_rd(PHYCTRL_REG5 + 0x200);
		success = success & bit_test(value, 2);
	}
	if (lane == 3 || lane == ALL_LANES) {
		value = mdio_rd(PHYCTRL_REG5 + 0x300);
		success = success & bit_test(value, 2);
	}

	return success;
}

void rx_reset_assert(int lane)
{
	int mask, val;

	if (lane == ALL_LANES) {
		val = mdio_rd(PHYMISC_REG2);
		mask = (1 << 15);
		mdio_wr(PHYMISC_REG2, val + mask);
	} else {
		val = mdio_rd(lane * 0x100 + PHYCTRL_REG8);
		mask = (1 << 6);
		mdio_wr(lane * 0x100 + PHYCTRL_REG8, val + mask);
	}
}

void rx_reset_de_assert(int lane)
{
	int mask, val;

	if (lane == ALL_LANES) {
		val = mdio_rd(PHYMISC_REG2);
		mask = 0xffff - (1 << 15);
		mdio_wr(PHYMISC_REG2, val & mask);
	} else {
		val = mdio_rd(lane*0x100 + PHYCTRL_REG8);
		mask = 0xffff - (1 << 6);
		mdio_wr(lane*0x100 + PHYCTRL_REG8, val & mask);
	}
}

void rx_powerdown_assert(int lane)
{
	int mask, val;

	val = mdio_rd(lane * 0x100 + PHYCTRL_REG8);
	mask = (1 << 5);
	mdio_wr(lane * 0x100 + PHYCTRL_REG8, val + mask);
}

void rx_powerdown_de_assert(int lane)
{
	int mask, val;

	val = mdio_rd(lane * 0x100 + PHYCTRL_REG8);
	mask = 0xffff - (1 << 5);
	mdio_wr(lane * 0x100 + PHYCTRL_REG8, val & mask);
}

void save_vco_codes(int lane)
{
	int value0, value1, value2, value3;

	if (lane == 0 || lane == ALL_LANES) {
		value0 = mdio_rd(PHYMISC_REG5);
		mdio_wr(PHYMISC_REG7, value0 + IN112525_RX_VCO_CODE_OFFSET);
		s03_vco_codes.l0_vco_code = value0;
	}
	if (lane == 1 || lane == ALL_LANES) {
		value1 = mdio_rd(PHYMISC_REG5 + 0x100);
		mdio_wr(PHYMISC_REG7 + 0x100,
			value1 + IN112525_RX_VCO_CODE_OFFSET);
		s03_vco_codes.l1_vco_code = value1;
	}
	if (lane == 2 || lane == ALL_LANES) {
		value2 = mdio_rd(PHYMISC_REG5 + 0x200);
		mdio_wr(PHYMISC_REG7 + 0x200,
			value2 + IN112525_RX_VCO_CODE_OFFSET);
		s03_vco_codes.l2_vco_code = value2;
	}
	if (lane == 3 || lane == ALL_LANES) {
		value3 = mdio_rd(PHYMISC_REG5 + 0x300);
		mdio_wr(PHYMISC_REG7 + 0x300,
			value3 + IN112525_RX_VCO_CODE_OFFSET);
		s03_vco_codes.l3_vco_code = value3;
	}
}

void save_az_offsets(int lane)
{
	int i;

#define AZ_OFFSET_LANE_UPDATE(reg, lane) \
	mdio_wr((reg) + (lane) * 0x100,  \
		(mdio_rd((reg) + (lane) * 0x100) >> 8))

	if (lane == ALL_LANES) {
		for (i = 0; i < ALL_LANES; i++) {
			AZ_OFFSET_LANE_UPDATE(PHYMISC_REG20, i);
			AZ_OFFSET_LANE_UPDATE(PHYMISC_REG20 + 1, i);
			AZ_OFFSET_LANE_UPDATE(PHYMISC_REG20 + 2, i);
			AZ_OFFSET_LANE_UPDATE(PHYMISC_REG20 + 3, i);
			AZ_OFFSET_LANE_UPDATE(PHYMISC_REG21, i);
			AZ_OFFSET_LANE_UPDATE(PHYMISC_REG21 + 1, i);
			AZ_OFFSET_LANE_UPDATE(PHYMISC_REG21 + 2, i);
			AZ_OFFSET_LANE_UPDATE(PHYMISC_REG21 + 3, i);
			AZ_OFFSET_LANE_UPDATE(PHYMISC_REG22, i);
		}
	} else {
		AZ_OFFSET_LANE_UPDATE(PHYMISC_REG20, lane);
		AZ_OFFSET_LANE_UPDATE(PHYMISC_REG20 + 1, lane);
		AZ_OFFSET_LANE_UPDATE(PHYMISC_REG20 + 2, lane);
		AZ_OFFSET_LANE_UPDATE(PHYMISC_REG20 + 3, lane);
		AZ_OFFSET_LANE_UPDATE(PHYMISC_REG21, lane);
		AZ_OFFSET_LANE_UPDATE(PHYMISC_REG21 + 1, lane);
		AZ_OFFSET_LANE_UPDATE(PHYMISC_REG21 + 2, lane);
		AZ_OFFSET_LANE_UPDATE(PHYMISC_REG21 + 3, lane);
		AZ_OFFSET_LANE_UPDATE(PHYMISC_REG22, lane);
	}

	mdio_wr(PHYCTRL_REG7, 0x0001);
}

int in112525_s03_lane_recovery(int lane)
{
	int i, value, az_pass;

	switch (lane) {
	case 0:
	case 1:
	case 2:
	case 3:
		rx_reset_assert(lane);
		WAIT(2000);
		break;
	case ALL_LANES:
		mdio_wr(PHYMISC_REG2, 0x9C00);
		WAIT(2000);
		while (1) {
			value = mdio_rd(PHYMISC_REG2);
			if (bit_test(value, 4))
				break;
		}
		break;
	default:
		pr_err("Incorrect usage of APIs in %s driver\n",
		       inphi_phydev->drv->name);
		break;
	}

	if (lane == 0 || lane == ALL_LANES)
		mdio_wr(PHYMISC_REG7, L0_VCO_CODE_trim);
	if (lane == 1 || lane == ALL_LANES)
		mdio_wr(PHYMISC_REG7 + 0x100, L1_VCO_CODE_trim);
	if (lane == 2 || lane == ALL_LANES)
		mdio_wr(PHYMISC_REG7 + 0x200, L2_VCO_CODE_trim);
	if (lane == 3 || lane == ALL_LANES)
		mdio_wr(PHYMISC_REG7 + 0x300, L3_VCO_CODE_trim);

	if (lane == 0 || lane == 4)
		mdio_wr(PHYCTRL_REG5, 0x0418);
	if (lane == 1 || lane == 4)
		mdio_wr(PHYCTRL_REG5 + 0x100, 0x0418);
	if (lane == 2 || lane == 4)
		mdio_wr(PHYCTRL_REG5 + 0x200, 0x0418);
	if (lane == 3 || lane == 4)
		mdio_wr(PHYCTRL_REG5 + 0x300, 0x0418);

	mdio_wr(PHYCTRL_REG7, 0x0000);
	rx_reset_de_assert(lane);

	if (lane == 0 || lane == 4) {
		mdio_wr(PHYCTRL_REG5, 0x0410);
		mdio_wr(PHYCTRL_REG5, 0x0412);
	}
	if (lane == 1 || lane == 4) {
		mdio_wr(PHYCTRL_REG5 + 0x100, 0x0410);
		mdio_wr(PHYCTRL_REG5 + 0x100, 0x0412);
	}
	if (lane == 2 || lane == 4) {
		mdio_wr(PHYCTRL_REG5 + 0x200, 0x0410);
		mdio_wr(PHYCTRL_REG5 + 0x200, 0x0412);
	}
	if (lane == 3 || lane == 4) {
		mdio_wr(PHYCTRL_REG5 + 0x300, 0x0410);
		mdio_wr(PHYCTRL_REG5 + 0x300, 0x0412);
	}

	for (i = 0; i < 64; i++) {
		/* wait 1000 times 10us */
		WAIT(10000);
		az_pass = az_complete_test(lane);
		if (az_pass) {
			save_az_offsets(lane);
			break;
		}
	}

	if (!az_pass) {
		debug("auto-zero calibration timed out for lane %d\n", lane);
		return 0;
	}

	mdio_wr(lane * 0x100 + PHYMISC_REG4, 0x0002);
	mdio_wr(lane * 0x100 + PHYMISC_REG6, 0x2028);
	mdio_wr(lane * 0x100 + PHYCTRL_REG5, 0x0010);
	WAIT(100);
	mdio_wr(lane * 0x100 + PHYCTRL_REG5, 0x0110);
	WAIT(3000);
	mdio_wr(lane * 0x100 + PHYMISC_REG6, 0x3020);

	if (lane == ALL_LANES) {
		mdio_wr(PHYMISC_REG2, 0x1C00);
		mdio_wr(PHYMISC_REG2, 0x0C00);
	} else {
		tx_restart(lane);
		/* delay > 10ms is required */
		WAIT(1100);
	}

	if (lane == ALL_LANES) {
		if (bit_test(mdio_rd(PHYMISC_REG2), 6) == 0)
			debug("TX PLL not locked on ALL lanes\n");
	} else {
		if (tx_pll_lock_test(lane) == 0) {
			debug("TX PLL not locked on lane %d\n", lane);
			return -1;
		}
	}

	save_vco_codes(lane);

	if (lane == ALL_LANES) {
		mdio_wr(PHYMISC_REG2, 0x0400);
		mdio_wr(PHYMISC_REG2, 0x0000);
		value = mdio_rd(PHYCTRL_REG1);
		value = value & 0xffbf;
		mdio_wr(PHYCTRL_REG2, value);
	} else {
		tx_core_de_assert(lane);
	}

	if (lane == ALL_LANES) {
		mdio_wr(PHYMISC_REG1, 0x8000);
		mdio_wr(PHYMISC_REG1, 0x0000);
	}

	mdio_rd(PHYMISC_REG1);
	mdio_rd(PHYMISC_REG1);

	WAIT(100);
	mdio_rd(PHYSTAT_REG1);
	mdio_rd(PHYSTAT_REG2);

	return 0;
}

int in112525_s03_phy_init(struct phy_device *phydev)
{
	u32 reg_value;
	u32 reg;
	int i;
	int tx_pll_MSDIV_value;
	int tx_pll_LSDIV_value;
	int tx_pll_iqdiv;
	int tx_pll_ctrl2_value;

	/* put the chip in hw/sw  and MDIO reset */
	mdio_wr(PHYCTRL_REG0,
		IN112525_HRESET | IN112525_SRESET | IN112525_MDIOINIT);

	/* de-assert MDIO init */
	mdio_wr(PHYCTRL_REG0, IN112525_HRESET | IN112525_SRESET);

	/* apply configuration */
	if (CURRENT_CONFIG.enable_prescaler)
		mdio_wr(IN112525_PRESCALE_20M, 0x0001);

	if (CURRENT_CONFIG.enable_external_refclk)
		mdio_wr(PHYMISC_REG3, (1 << 15));
	else
		mdio_wr(PHYMISC_REG3, 0x0);

	mdio_wr(PHYCTRL_REG0, IN112525_SRESET);

	WAIT(1000);

	reg_value = phy_read(phydev, MDIO_MMD_VEND1, IN112525_EFUSE_REG);
	if (!(reg_value & IN112525_EFUSE_DONE)) {
		puts("IN112525_s03 init failed: EFUSE Done not set\n");
		return -1;
	}
	WAIT(1000);

	reg_value = phy_read(phydev, MDIO_MMD_VEND1, PHYMISC_REG2);
	if (!(reg_value & IN112525_CALIBRATION_DONE)) {
		puts("IN112525_s03 init failed: CALIBRATION_DONE not set\n");
		return -1;
	}

	mdio_wr(PHYMISC_REG2,
		IN112525_RX_PLL_RESET |
		IN112525_TX_PLL_RESET |
		IN112525_TX_SERDES_RESET |
		IN112525_CORE_DATAPATH_RESET);

	if (CURRENT_CONFIG.enable_otu_protocol)
		mdio_wr(PHYCTRL_REG0, 0x8C00);
	else
		mdio_wr(PHYCTRL_REG0, 0x8200);

	if (CURRENT_CONFIG.enable_extended_range) {
		mdio_wr(PHYCTRL_REG10, 0x2032);
		printf("IN112525_s03 possible misconfig [ext range]\n");
		mdio_wr(PHYCTRL_REG12, 0x0007);
	} else {
		mdio_wr(PHYCTRL_REG10, 0xA02D);
		mdio_wr(PHYCTRL_REG12, 0x0005);
	}

	mdio_wr(PHYCTRL_REG18, 0x00ff);
	mdio_wr(PHYCTRL_REG19, 0x002d);
	mdio_wr(PHYCTRL_REG19, 0x802d);
	mdio_wr(PHYCTRL_REG19, 0x0000);
	mdio_wr(PHYCTRL_REG18, 0x00e9);
	mdio_wr(PHYCTRL_REG19, 0x0008);
	mdio_wr(PHYCTRL_REG19, 0x8008);
	mdio_wr(PHYCTRL_REG19, 0x0000);

	tx_pll_MSDIV_value = tx_pll_mpy_map[CURRENT_CONFIG.tx_pll_mpy_ratio][0];
	tx_pll_LSDIV_value = tx_pll_mpy_map[CURRENT_CONFIG.tx_pll_mpy_ratio][1];
	tx_pll_iqdiv = (CURRENT_CONFIG.enable_half_rate) ? 1 : 0;
	tx_pll_ctrl2_value =
		(CURRENT_CONFIG.tx_pll_refclk_source << 11) +
		(tx_pll_iqdiv << 8) +
		(tx_pll_MSDIV_value << 4) +
		tx_pll_LSDIV_value;

	mdio_wr(PHYCTRL_REG11, tx_pll_ctrl2_value);

	if (CURRENT_CONFIG.enable_half_rate)
		mdio_wr(PHYCTRL_REG14, 0x0020);

	/* set the CTLE mode (bw on the front end stages)
	 * for example '25:25:10', '10:10:10' etc.
	 */
	for (i = 0; i < ALL_LANES; i++) {
		reg = i * 0x100 + PHYCTRL_REG8;
		reg_value = phy_read(phydev, MDIO_MMD_VEND1, reg);
		reg_value = reg_value & 0xFF7C;
		/* put bits 7,1,0 for EQ */
		reg_value = reg_value | CURRENT_CONFIG.ctle_mode;
		mdio_wr(reg, reg_value);
	}


	/* rx common code settings */
	mdio_wr(PHYMISC_REG32, CURRENT_CONFIG.rx_common_mode);
	mdio_wr(PHYMISC_REG32 + 0x100, CURRENT_CONFIG.rx_common_mode);
	mdio_wr(PHYMISC_REG32 + 0x200, CURRENT_CONFIG.rx_common_mode - 1);
	mdio_wr(PHYMISC_REG32 + 0x300, CURRENT_CONFIG.rx_common_mode - 1);
	/* mdio_wr(PHYMISC_REG30, CURRENT_CONFIG.rx_common_mode); */

	if (CURRENT_CONFIG.rx_odt_override)
		mdio_wr(PHYMISC_REG31, CURRENT_CONFIG.rx_odt_override);

	s03_vco_codes.l0_vco_code = mdio_rd(PHYMISC_REG7);
	s03_vco_codes.l1_vco_code = mdio_rd(PHYMISC_REG7 + 0x100);
	s03_vco_codes.l2_vco_code = mdio_rd(PHYMISC_REG7 + 0x200);
	s03_vco_codes.l3_vco_code = mdio_rd(PHYMISC_REG7 + 0x300);

	if (CURRENT_CONFIG.enable_extended_range) {
		s03_vco_codes.l0_vco_code =
					(int)(s03_vco_codes.l0_vco_code * 4 / 5);
		s03_vco_codes.l1_vco_code =
					(int)(s03_vco_codes.l1_vco_code * 4 / 5);
		s03_vco_codes.l2_vco_code =
					(int)(s03_vco_codes.l2_vco_code * 4 / 5);
		s03_vco_codes.l3_vco_code =
					(int)(s03_vco_codes.l3_vco_code * 4 / 5);
	}

	mdio_wr(PHYMISC_REG2, 0x0);
	WAIT(10000);

	/* start fresh */
	in112525_s03_lane_recovery(ALL_LANES);

	return 0;
}

int in112525_s03_config(struct phy_device *phydev)
{
	inphi_phydev = phydev;
	return in112525_s03_phy_init(phydev);
}

int in112525_probe(struct phy_device *phydev)
{
	phydev->flags = PHY_FLAG_BROKEN_RESET;
	return 0;
}

int in112525_s03_startup(struct phy_device *phydev)
{
	int reg_value, i;

	phydev->link = 1;

#ifdef CONFIG_IN112525_S03_10G
	phydev->speed = SPEED_10000;
#else
	phydev->speed = SPEED_25000;
#endif
	phydev->duplex = DUPLEX_FULL;

	for (i = 0; i < ALL_LANES; i++) {
		reg_value = mdio_rd(PHYSTAT_REG3 + i * 0x100);
		if (!bit_test(reg_value, 15)) {
			debug("starting recovery for lane %d\n", i);
			in112525_s03_lane_recovery(i);
		}
	}

	return 0;
}

int in112525_s05_startup(struct phy_device *phydev)
{
	phydev->link = 1;

#if defined(CONFIG_IN112525_S05_10G)
	phydev->speed = SPEED_10000;
#elif defined(CONFIG_IN112525_S05_25G)
	phydev->speed = SPEED_25000;
#elif defined(CONFIG_IN112525_S05_40G)
	phydev->speed = SPEED_40000;
#elif defined(CONFIG_IN112525_S05_50G)
	phydev->speed = SPEED_50000;
#elif defined(CONFIG_IN112525_S05_100G)
	phydev->speed = SPEED_100000;
#endif
	phydev->duplex = DUPLEX_FULL;
	return 0;
}

U_BOOT_PHY_DRIVER(in112525_s05) = {
	.name = "Inphi in112525_S05P",
	.uid = PHY_UID_IN112525_S05,
	.mask = 0x0ff0ffff,
	.features = PHY_10G_FEATURES,
	.mmds = MDIO_DEVS_VEND1,
	.config = &in112525_s05_config,
	.probe	= &in112525_probe,
	.startup = &in112525_s05_startup,
	.shutdown = &gen10g_shutdown,
};

U_BOOT_PHY_DRIVER(in112525_s03) = {
	.name = "Inphi in112525_S03P",
	.uid = PHY_UID_IN112525_S03,
	.mask = 0x0ff0fff0,
	.features = PHY_10G_FEATURES,
	.mmds = MDIO_DEVS_VEND1,
	.config = &in112525_s03_config,
	.probe	= &in112525_probe,
	.startup = &in112525_s03_startup,
	.shutdown = &gen10g_shutdown,
};
