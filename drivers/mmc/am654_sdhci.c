// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 *
 * Texas Instruments' K3 SD Host Controller Interface
 */

#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <mmc.h>
#include <power-domain.h>
#include <regmap.h>
#include <sdhci.h>
#include <soc.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/err.h>

/* CTL_CFG Registers */
#define CTL_CFG_2		0x14

#define SLOTTYPE_MASK		GENMASK(31, 30)
#define SLOTTYPE_EMBEDDED	BIT(30)

/* PHY Registers */
#define PHY_CTRL1	0x100
#define PHY_CTRL2	0x104
#define PHY_CTRL3	0x108
#define PHY_CTRL4	0x10C
#define PHY_CTRL5	0x110
#define PHY_CTRL6	0x114
#define PHY_STAT1	0x130
#define PHY_STAT2	0x134

#define IOMUX_ENABLE_SHIFT	31
#define IOMUX_ENABLE_MASK	BIT(IOMUX_ENABLE_SHIFT)
#define OTAPDLYENA_SHIFT	20
#define OTAPDLYENA_MASK		BIT(OTAPDLYENA_SHIFT)
#define OTAPDLYSEL_SHIFT	12
#define OTAPDLYSEL_MASK		GENMASK(15, 12)
#define STRBSEL_SHIFT		24
#define STRBSEL_4BIT_MASK	GENMASK(27, 24)
#define STRBSEL_8BIT_MASK	GENMASK(31, 24)
#define SEL50_SHIFT		8
#define SEL50_MASK		BIT(SEL50_SHIFT)
#define SEL100_SHIFT		9
#define SEL100_MASK		BIT(SEL100_SHIFT)
#define FREQSEL_SHIFT		8
#define FREQSEL_MASK		GENMASK(10, 8)
#define CLKBUFSEL_SHIFT		0
#define CLKBUFSEL_MASK		GENMASK(2, 0)
#define DLL_TRIM_ICP_SHIFT	4
#define DLL_TRIM_ICP_MASK	GENMASK(7, 4)
#define DR_TY_SHIFT		20
#define DR_TY_MASK		GENMASK(22, 20)
#define ENDLL_SHIFT		1
#define ENDLL_MASK		BIT(ENDLL_SHIFT)
#define DLLRDY_SHIFT		0
#define DLLRDY_MASK		BIT(DLLRDY_SHIFT)
#define PDB_SHIFT		0
#define PDB_MASK		BIT(PDB_SHIFT)
#define CALDONE_SHIFT		1
#define CALDONE_MASK		BIT(CALDONE_SHIFT)
#define RETRIM_SHIFT		17
#define RETRIM_MASK		BIT(RETRIM_SHIFT)
#define SELDLYTXCLK_SHIFT	17
#define SELDLYTXCLK_MASK	BIT(SELDLYTXCLK_SHIFT)
#define SELDLYRXCLK_SHIFT	16
#define SELDLYRXCLK_MASK	BIT(SELDLYRXCLK_SHIFT)
#define ITAPDLYSEL_SHIFT	0
#define ITAPDLYSEL_MASK		GENMASK(4, 0)
#define ITAPDLYENA_SHIFT	8
#define ITAPDLYENA_MASK		BIT(ITAPDLYENA_SHIFT)
#define ITAPCHGWIN_SHIFT	9
#define ITAPCHGWIN_MASK		BIT(ITAPCHGWIN_SHIFT)

#define DRIVER_STRENGTH_50_OHM	0x0
#define DRIVER_STRENGTH_33_OHM	0x1
#define DRIVER_STRENGTH_66_OHM	0x2
#define DRIVER_STRENGTH_100_OHM	0x3
#define DRIVER_STRENGTH_40_OHM	0x4

#define AM654_SDHCI_MIN_FREQ	400000
#define CLOCK_TOO_SLOW_HZ	50000000

#define ENABLE	0x1

struct am654_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
	struct regmap *base;
	bool non_removable;
	u32 otap_del_sel[MMC_MODES_END];
	u32 itap_del_sel[MMC_MODES_END];
	u32 itap_del_ena[MMC_MODES_END];
	u32 trm_icp;
	u32 drv_strength;
	u32 strb_sel;
	u32 clkbuf_sel;
	u32 flags;
	bool dll_enable;
#define DLL_PRESENT	BIT(0)
#define IOMUX_PRESENT	BIT(1)
#define FREQSEL_2_BIT	BIT(2)
#define STRBSEL_4_BIT	BIT(3)
#define DLL_CALIB	BIT(4)
	u32 quirks;
#define SDHCI_AM654_QUIRK_FORCE_CDTEST BIT(0)
};

struct timing_data {
	const char *otap_binding;
	const char *itap_binding;
	u32 capability;
};

struct window {
	u8 start;
	u8 end;
	u8 length;
};

static const struct timing_data td[] = {
	[MMC_LEGACY]	= {"ti,otap-del-sel-legacy",
			   "ti,itap-del-sel-legacy",
			   0},
	[MMC_HS]	= {"ti,otap-del-sel-mmc-hs",
			   "ti,itap-del-sel-mms-hs",
			   MMC_CAP(MMC_HS)},
	[SD_HS]		= {"ti,otap-del-sel-sd-hs",
			   "ti,itap-del-sel-sd-hs",
			   MMC_CAP(SD_HS)},
	[UHS_SDR12]	= {"ti,otap-del-sel-sdr12",
			   "ti,itap-del-sel-sdr12",
			   MMC_CAP(UHS_SDR12)},
	[UHS_SDR25]	= {"ti,otap-del-sel-sdr25",
			   "ti,itap-del-sel-sdr25",
			   MMC_CAP(UHS_SDR25)},
	[UHS_SDR50]	= {"ti,otap-del-sel-sdr50",
			   NULL,
			   MMC_CAP(UHS_SDR50)},
	[UHS_SDR104]	= {"ti,otap-del-sel-sdr104",
			   NULL,
			   MMC_CAP(UHS_SDR104)},
	[UHS_DDR50]	= {"ti,otap-del-sel-ddr50",
			   NULL,
			   MMC_CAP(UHS_DDR50)},
	[MMC_DDR_52]	= {"ti,otap-del-sel-ddr52",
			   "ti,itap-del-sel-ddr52",
			   MMC_CAP(MMC_DDR_52)},
	[MMC_HS_200]	= {"ti,otap-del-sel-hs200",
			   NULL,
			   MMC_CAP(MMC_HS_200)},
	[MMC_HS_400]	= {"ti,otap-del-sel-hs400",
			   NULL,
			   MMC_CAP(MMC_HS_400)},
};

struct am654_driver_data {
	const struct sdhci_ops *ops;
	u32 flags;
};

static int am654_sdhci_setup_dll(struct am654_sdhci_plat *plat,
				 unsigned int speed)
{
	int sel50, sel100, freqsel;
	u32 mask, val;
	int ret;

	/* Disable delay chain mode */
	regmap_update_bits(plat->base, PHY_CTRL5,
			   SELDLYTXCLK_MASK | SELDLYRXCLK_MASK, 0);

	if (plat->flags & FREQSEL_2_BIT) {
		switch (speed) {
		case 200000000:
			sel50 = 0;
			sel100 = 0;
			break;
		case 100000000:
			sel50 = 0;
			sel100 = 1;
			break;
		default:
			sel50 = 1;
			sel100 = 0;
		}

		/* Configure PHY DLL frequency */
		mask = SEL50_MASK | SEL100_MASK;
		val = (sel50 << SEL50_SHIFT) | (sel100 << SEL100_SHIFT);
		regmap_update_bits(plat->base, PHY_CTRL5, mask, val);
	} else {
		switch (speed) {
		case 200000000:
			freqsel = 0x0;
			break;
		default:
			freqsel = 0x4;
		}
		regmap_update_bits(plat->base, PHY_CTRL5, FREQSEL_MASK,
				   freqsel << FREQSEL_SHIFT);
	}

	/* Configure DLL TRIM */
	mask = DLL_TRIM_ICP_MASK;
	val = plat->trm_icp << DLL_TRIM_ICP_SHIFT;

	/* Configure DLL driver strength */
	mask |= DR_TY_MASK;
	val |= plat->drv_strength << DR_TY_SHIFT;
	regmap_update_bits(plat->base, PHY_CTRL1, mask, val);

	/* Enable DLL */
	regmap_update_bits(plat->base, PHY_CTRL1, ENDLL_MASK,
			   0x1 << ENDLL_SHIFT);
	/*
	 * Poll for DLL ready. Use a one second timeout.
	 * Works in all experiments done so far
	 */
	ret = regmap_read_poll_timeout(plat->base, PHY_STAT1, val,
				       val & DLLRDY_MASK, 1000, 1000000);

	return ret;
}

static void am654_sdhci_write_itapdly(struct am654_sdhci_plat *plat,
				      u32 itapdly, u32 enable)
{
	regmap_update_bits(plat->base, PHY_CTRL4, ITAPDLYENA_MASK,
			   enable << ITAPDLYENA_SHIFT);
	/* Set ITAPCHGWIN before writing to ITAPDLY */
	regmap_update_bits(plat->base, PHY_CTRL4, ITAPCHGWIN_MASK,
			   1 << ITAPCHGWIN_SHIFT);
	regmap_update_bits(plat->base, PHY_CTRL4, ITAPDLYSEL_MASK,
			   itapdly << ITAPDLYSEL_SHIFT);
	regmap_update_bits(plat->base, PHY_CTRL4, ITAPCHGWIN_MASK, 0);
}

static void am654_sdhci_setup_delay_chain(struct am654_sdhci_plat *plat,
					  int mode)
{
	u32 mask, val;

	val = 1 << SELDLYTXCLK_SHIFT | 1 << SELDLYRXCLK_SHIFT;
	mask = SELDLYTXCLK_MASK | SELDLYRXCLK_MASK;
	regmap_update_bits(plat->base, PHY_CTRL5, mask, val);

	am654_sdhci_write_itapdly(plat, plat->itap_del_sel[mode],
				  plat->itap_del_ena[mode]);
}

static int am654_sdhci_set_ios_post(struct sdhci_host *host)
{
	struct udevice *dev = host->mmc->dev;
	struct am654_sdhci_plat *plat = dev_get_plat(dev);
	unsigned int speed = host->mmc->clock;
	int mode = host->mmc->selected_mode;
	u32 otap_del_sel;
	u32 mask, val;
	int ret;

	/* Reset SD Clock Enable */
	val = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	val &= ~SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, val, SDHCI_CLOCK_CONTROL);

	regmap_update_bits(plat->base, PHY_CTRL1, ENDLL_MASK, 0);

	/* restart clock */
	sdhci_set_clock(host->mmc, speed);

	/* switch phy back on */
	otap_del_sel = plat->otap_del_sel[mode];
	mask = OTAPDLYENA_MASK | OTAPDLYSEL_MASK;
	val = (1 << OTAPDLYENA_SHIFT) |
	      (otap_del_sel << OTAPDLYSEL_SHIFT);

	/* Write to STRBSEL for HS400 speed mode */
	if (host->mmc->selected_mode == MMC_HS_400) {
		if (plat->flags & STRBSEL_4_BIT)
			mask |= STRBSEL_4BIT_MASK;
		else
			mask |= STRBSEL_8BIT_MASK;

		val |= plat->strb_sel << STRBSEL_SHIFT;
	}

	regmap_update_bits(plat->base, PHY_CTRL4, mask, val);

	if ((mode > UHS_SDR25 || mode == MMC_DDR_52) && speed >= CLOCK_TOO_SLOW_HZ) {
		ret = am654_sdhci_setup_dll(plat, speed);
		if (ret)
			return ret;

		plat->dll_enable = true;
		if (mode == MMC_HS_400) {
			plat->itap_del_ena[mode] = ENABLE;
			plat->itap_del_sel[mode] = plat->itap_del_sel[mode - 1];
		}

		am654_sdhci_write_itapdly(plat, plat->itap_del_sel[mode],
					  plat->itap_del_ena[mode]);
	} else {
		am654_sdhci_setup_delay_chain(plat, mode);
		plat->dll_enable = false;
	}

	regmap_update_bits(plat->base, PHY_CTRL5, CLKBUFSEL_MASK,
			   plat->clkbuf_sel);

	return 0;
}

int am654_sdhci_init(struct am654_sdhci_plat *plat)
{
	u32 ctl_cfg_2 = 0;
	u32 mask, val;
	int ret;

	/* Reset OTAP to default value */
	mask = OTAPDLYENA_MASK | OTAPDLYSEL_MASK;
	regmap_update_bits(plat->base, PHY_CTRL4, mask, 0x0);

	if (plat->flags & DLL_CALIB) {
		regmap_read(plat->base, PHY_STAT1, &val);
		if (~val & CALDONE_MASK) {
			/* Calibrate IO lines */
			regmap_update_bits(plat->base, PHY_CTRL1, PDB_MASK,
					   PDB_MASK);
			ret = regmap_read_poll_timeout(plat->base, PHY_STAT1,
						       val, val & CALDONE_MASK,
						       1, 20);
			if (ret)
				return ret;
		}
	}

	/* Enable pins by setting IO mux to 0 */
	if (plat->flags & IOMUX_PRESENT)
		regmap_update_bits(plat->base, PHY_CTRL1, IOMUX_ENABLE_MASK, 0);

	/* Set slot type based on SD or eMMC */
	if (plat->non_removable)
		ctl_cfg_2 = SLOTTYPE_EMBEDDED;

	regmap_update_bits(plat->base, CTL_CFG_2, SLOTTYPE_MASK, ctl_cfg_2);

	return 0;
}

#define MAX_SDCD_DEBOUNCE_TIME 2000
static int am654_sdhci_cd_poll(struct mmc *mmc)
{
	unsigned long start;
	int val;

	/*
	 * The controller takes about 1 second to debounce the card detect line
	 * and doesn't let us power on until that time is up. Instead of waiting
	 * for 1 second at every stage, poll on the CARD_PRESENT bit upto a
	 * maximum of 2 seconds to be safe..
	 */
	start = get_timer(0);
	do {
		if (get_timer(start) > MAX_SDCD_DEBOUNCE_TIME)
			return -ENOMEDIUM;

		val = mmc_getcd(mmc);
	} while (!val);

	return 0;
}

static int am654_sdhci_deferred_probe(struct sdhci_host *host)
{
	struct udevice *dev = host->mmc->dev;
	struct am654_sdhci_plat *plat = dev_get_plat(dev);
	int ret;

	if (!(plat->quirks & SDHCI_AM654_QUIRK_FORCE_CDTEST)) {
		if (am654_sdhci_cd_poll(host->mmc))
			return -ENOMEDIUM;
	}

	am654_sdhci_init(plat);

	ret = sdhci_probe(dev);

	if (plat->quirks & SDHCI_AM654_QUIRK_FORCE_CDTEST) {
		u8 hostctrlreg = sdhci_readb(host, SDHCI_HOST_CONTROL);

		hostctrlreg |= SDHCI_CTRL_CD_TEST_INS | SDHCI_CTRL_CD_TEST;
		sdhci_writeb(host, hostctrlreg, SDHCI_HOST_CONTROL);
	}

	return ret;
}

static void am654_sdhci_write_b(struct sdhci_host *host, u8 val, int reg)
{
	if (reg == SDHCI_HOST_CONTROL) {
		switch (host->mmc->selected_mode) {
		/*
		 * According to the data manual, HISPD bit
		 * should not be set in these speed modes.
		 */
		case SD_HS:
		case MMC_HS:
		case UHS_SDR12:
		case UHS_SDR25:
			val &= ~SDHCI_CTRL_HISPD;
		default:
			break;
		}
	}

	writeb(val, host->ioaddr + reg);
}
#if CONFIG_IS_ENABLED(MMC_SUPPORTS_TUNING)
#define ITAPDLY_LENGTH 32
#define ITAPDLY_LAST_INDEX (ITAPDLY_LENGTH - 1)

static u32 am654_sdhci_calculate_itap(struct udevice *dev, struct window
			  *fail_window, u8 num_fails, bool circular_buffer)
{
	u8 itap = 0, start_fail = 0, end_fail = 0, pass_length = 0;
	u8 first_fail_start = 0, last_fail_end = 0;
	struct window pass_window = {0, 0, 0};
	int prev_fail_end = -1;
	u8 i;

	if (!num_fails)
		return ITAPDLY_LAST_INDEX >> 1;

	if (fail_window->length == ITAPDLY_LENGTH) {
		dev_err(dev, "No passing ITAPDLY, return 0\n");
		return 0;
	}

	first_fail_start = fail_window->start;
	last_fail_end = fail_window[num_fails - 1].end;

	for (i = 0; i < num_fails; i++) {
		start_fail = fail_window[i].start;
		end_fail = fail_window[i].end;
		pass_length = start_fail - (prev_fail_end + 1);

		if (pass_length > pass_window.length) {
			pass_window.start = prev_fail_end + 1;
			pass_window.length = pass_length;
		}
		prev_fail_end = end_fail;
	}

	if (!circular_buffer)
		pass_length = ITAPDLY_LAST_INDEX - last_fail_end;
	else
		pass_length = ITAPDLY_LAST_INDEX - last_fail_end + first_fail_start;

	if (pass_length > pass_window.length) {
		pass_window.start = last_fail_end + 1;
		pass_window.length = pass_length;
	}

	if (!circular_buffer)
		itap = pass_window.start + (pass_window.length >> 1);
	else
		itap = (pass_window.start + (pass_window.length >> 1)) % ITAPDLY_LENGTH;

	return (itap > ITAPDLY_LAST_INDEX) ? ITAPDLY_LAST_INDEX >> 1 : itap;
}

static int am654_sdhci_execute_tuning(struct mmc *mmc, u8 opcode)
{
	struct udevice *dev = mmc->dev;
	struct am654_sdhci_plat *plat = dev_get_plat(dev);
	struct window fail_window[ITAPDLY_LENGTH];
	int mode = mmc->selected_mode;
	u8 curr_pass, itap;
	u8 fail_index = 0;
	u8 prev_pass = 1;

	memset(fail_window, 0, sizeof(fail_window));

	/* Enable ITAPDLY */
	plat->itap_del_ena[mode] = ENABLE;

	for (itap = 0; itap < ITAPDLY_LENGTH; itap++) {
		am654_sdhci_write_itapdly(plat, itap, plat->itap_del_ena[mode]);

		curr_pass = !mmc_send_tuning(mmc, opcode);

		if (!curr_pass && prev_pass)
			fail_window[fail_index].start = itap;

		if (!curr_pass) {
			fail_window[fail_index].end = itap;
			fail_window[fail_index].length++;
		}

		if (curr_pass && !prev_pass)
			fail_index++;

		prev_pass = curr_pass;
	}

	if (fail_window[fail_index].length != 0)
		fail_index++;

	itap = am654_sdhci_calculate_itap(dev, fail_window, fail_index,
					  plat->dll_enable);

	/* Save ITAPDLY */
	plat->itap_del_sel[mode] = itap;

	am654_sdhci_write_itapdly(plat, itap, plat->itap_del_ena[mode]);

	return 0;
}
#endif
const struct sdhci_ops am654_sdhci_ops = {
#if CONFIG_IS_ENABLED(MMC_SUPPORTS_TUNING)
	.platform_execute_tuning = am654_sdhci_execute_tuning,
#endif
	.deferred_probe		= am654_sdhci_deferred_probe,
	.set_ios_post		= &am654_sdhci_set_ios_post,
	.set_control_reg	= sdhci_set_control_reg,
	.write_b		= am654_sdhci_write_b,
};

const struct am654_driver_data am654_drv_data = {
	.ops = &am654_sdhci_ops,
	.flags = DLL_PRESENT | IOMUX_PRESENT | FREQSEL_2_BIT | STRBSEL_4_BIT,
};

const struct am654_driver_data am654_sr1_drv_data = {
	.ops = &am654_sdhci_ops,
	.flags = IOMUX_PRESENT | FREQSEL_2_BIT | DLL_PRESENT | DLL_CALIB |
		 STRBSEL_4_BIT,
};

const struct am654_driver_data j721e_8bit_drv_data = {
	.ops = &am654_sdhci_ops,
	.flags = DLL_PRESENT | DLL_CALIB,
};

static int j721e_4bit_sdhci_set_ios_post(struct sdhci_host *host)
{
	struct udevice *dev = host->mmc->dev;
	struct am654_sdhci_plat *plat = dev_get_plat(dev);
	int mode = host->mmc->selected_mode;
	u32 otap_del_sel;
	u32 itap_del_ena;
	u32 itap_del_sel;
	u32 mask, val;

	otap_del_sel = plat->otap_del_sel[mode];

	mask = OTAPDLYENA_MASK | OTAPDLYSEL_MASK;
	val = (1 << OTAPDLYENA_SHIFT) |
	      (otap_del_sel << OTAPDLYSEL_SHIFT);

	itap_del_ena = plat->itap_del_ena[mode];
	itap_del_sel = plat->itap_del_sel[mode];

	mask |= ITAPDLYENA_MASK | ITAPDLYSEL_MASK;
	val |= (itap_del_ena << ITAPDLYENA_SHIFT) |
	       (itap_del_sel << ITAPDLYSEL_SHIFT);

	regmap_update_bits(plat->base, PHY_CTRL4, ITAPCHGWIN_MASK,
			   1 << ITAPCHGWIN_SHIFT);
	regmap_update_bits(plat->base, PHY_CTRL4, mask, val);
	regmap_update_bits(plat->base, PHY_CTRL4, ITAPCHGWIN_MASK, 0);

	regmap_update_bits(plat->base, PHY_CTRL5, CLKBUFSEL_MASK,
			   plat->clkbuf_sel);

	return 0;
}

const struct sdhci_ops j721e_4bit_sdhci_ops = {
#if CONFIG_IS_ENABLED(MMC_SUPPORTS_TUNING)
	.platform_execute_tuning = am654_sdhci_execute_tuning,
#endif
	.deferred_probe		= am654_sdhci_deferred_probe,
	.set_ios_post		= &j721e_4bit_sdhci_set_ios_post,
	.set_control_reg	= sdhci_set_control_reg,
	.write_b		= am654_sdhci_write_b,
};

const struct am654_driver_data j721e_4bit_drv_data = {
	.ops = &j721e_4bit_sdhci_ops,
	.flags = IOMUX_PRESENT,
};

static const struct am654_driver_data sdhci_am64_8bit_drvdata = {
	.ops = &am654_sdhci_ops,
	.flags = DLL_PRESENT | DLL_CALIB,
};

static const struct am654_driver_data sdhci_am64_4bit_drvdata = {
	.ops = &j721e_4bit_sdhci_ops,
	.flags = IOMUX_PRESENT,
};

const struct soc_attr am654_sdhci_soc_attr[] = {
	{ .family = "AM65X", .revision = "SR1.0", .data = &am654_sr1_drv_data},
	{/* sentinel */}
};

static int sdhci_am654_get_otap_delay(struct udevice *dev,
				      struct mmc_config *cfg)
{
	struct am654_sdhci_plat *plat = dev_get_plat(dev);
	int ret;
	int i;

	/* ti,otap-del-sel-legacy is mandatory */
	ret = dev_read_u32(dev, "ti,otap-del-sel-legacy",
			   &plat->otap_del_sel[0]);
	if (ret)
		return ret;
	/*
	 * Remove the corresponding capability if an otap-del-sel
	 * value is not found
	 */
	for (i = MMC_LEGACY; i <= MMC_HS_400; i++) {
		ret = dev_read_u32(dev, td[i].otap_binding,
				   &plat->otap_del_sel[i]);
		if (ret) {
			dev_dbg(dev, "Couldn't find %s\n", td[i].otap_binding);
			/*
			 * Remove the corresponding capability
			 * if an otap-del-sel value is not found
			 */
			cfg->host_caps &= ~td[i].capability;
		}

		if (td[i].itap_binding) {
			ret = dev_read_u32(dev, td[i].itap_binding,
					   &plat->itap_del_sel[i]);

			if (!ret)
				plat->itap_del_ena[i] = ENABLE;
		}
	}

	return 0;
}

static int am654_sdhci_probe(struct udevice *dev)
{
	struct am654_driver_data *drv_data =
			(struct am654_driver_data *)dev_get_driver_data(dev);
	struct am654_sdhci_plat *plat = dev_get_plat(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	struct mmc_config *cfg = &plat->cfg;
	const struct soc_attr *soc;
	const struct am654_driver_data *soc_drv_data;
	struct clk clk;
	unsigned long clock;
	int ret;

	ret = clk_get_by_name(dev, "clk_xin", &clk);
	if (ret) {
		dev_err(dev, "failed to get clock\n");
		return ret;
	}

	clock = clk_get_rate(&clk);
	if (IS_ERR_VALUE(clock)) {
		dev_err(dev, "failed to get rate\n");
		return clock;
	}

	host->max_clk = clock;
	host->mmc = &plat->mmc;
	host->mmc->dev = dev;
	host->ops = drv_data->ops;
	ret = sdhci_setup_cfg(cfg, host, cfg->f_max,
			      AM654_SDHCI_MIN_FREQ);
	if (ret)
		return ret;

	ret = sdhci_am654_get_otap_delay(dev, cfg);
	if (ret)
		return ret;

	/* Update ops based on SoC revision */
	soc = soc_device_match(am654_sdhci_soc_attr);
	if (soc && soc->data) {
		soc_drv_data = soc->data;
		host->ops = soc_drv_data->ops;
	}

	host->mmc->priv = host;
	upriv->mmc = host->mmc;

	regmap_init_mem_index(dev_ofnode(dev), &plat->base, 1);

	if (plat->quirks & SDHCI_AM654_QUIRK_FORCE_CDTEST)
		am654_sdhci_deferred_probe(host);

	return 0;
}

static int am654_sdhci_of_to_plat(struct udevice *dev)
{
	struct am654_sdhci_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	struct mmc_config *cfg = &plat->cfg;
	u32 drv_strength;
	int ret;

	host->name = dev->name;
	host->ioaddr = dev_read_addr_ptr(dev);
	plat->non_removable = dev_read_bool(dev, "non-removable");

	if (plat->flags & DLL_PRESENT) {
		ret = dev_read_u32(dev, "ti,trm-icp", &plat->trm_icp);
		if (ret)
			return ret;

		ret = dev_read_u32(dev, "ti,driver-strength-ohm",
				   &drv_strength);
		if (ret)
			return ret;

		switch (drv_strength) {
		case 50:
			plat->drv_strength = DRIVER_STRENGTH_50_OHM;
			break;
		case 33:
			plat->drv_strength = DRIVER_STRENGTH_33_OHM;
			break;
		case 66:
			plat->drv_strength = DRIVER_STRENGTH_66_OHM;
			break;
		case 100:
			plat->drv_strength = DRIVER_STRENGTH_100_OHM;
			break;
		case 40:
			plat->drv_strength = DRIVER_STRENGTH_40_OHM;
			break;
		default:
			dev_err(dev, "Invalid driver strength\n");
			return -EINVAL;
		}
	}

	dev_read_u32(dev, "ti,strobe-sel", &plat->strb_sel);
	dev_read_u32(dev, "ti,clkbuf-sel", &plat->clkbuf_sel);
	if (dev_read_bool(dev, "ti,fails-without-test-cd"))
		plat->quirks |= SDHCI_AM654_QUIRK_FORCE_CDTEST;

	ret = mmc_of_parse(dev, cfg);
	if (ret)
		return ret;

	return 0;
}

static int am654_sdhci_bind(struct udevice *dev)
{
	struct am654_driver_data *drv_data =
			(struct am654_driver_data *)dev_get_driver_data(dev);
	struct am654_sdhci_plat *plat = dev_get_plat(dev);
	const struct soc_attr *soc;
	const struct am654_driver_data *soc_drv_data;

	plat->flags = drv_data->flags;

	/* Update flags based on SoC revision */
	soc = soc_device_match(am654_sdhci_soc_attr);
	if (soc && soc->data) {
		soc_drv_data = soc->data;
		plat->flags = soc_drv_data->flags;
	}

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id am654_sdhci_ids[] = {
	{
		.compatible = "ti,am654-sdhci-5.1",
		.data = (ulong)&am654_drv_data,
	},
	{
		.compatible = "ti,j721e-sdhci-8bit",
		.data = (ulong)&j721e_8bit_drv_data,
	},
	{
		.compatible = "ti,j721e-sdhci-4bit",
		.data = (ulong)&j721e_4bit_drv_data,
	},
	{
		.compatible = "ti,am64-sdhci-8bit",
		.data = (ulong)&sdhci_am64_8bit_drvdata,
	},
	{
		.compatible = "ti,am64-sdhci-4bit",
		.data = (ulong)&sdhci_am64_4bit_drvdata,
	},
	{
		.compatible = "ti,am62-sdhci",
		.data = (ulong)&sdhci_am64_4bit_drvdata,
	},
	{ }
};

U_BOOT_DRIVER(am654_sdhci_drv) = {
	.name		= "am654_sdhci",
	.id		= UCLASS_MMC,
	.of_match	= am654_sdhci_ids,
	.of_to_plat = am654_sdhci_of_to_plat,
	.ops		= &sdhci_ops,
	.bind		= am654_sdhci_bind,
	.probe		= am654_sdhci_probe,
	.priv_auto	= sizeof(struct sdhci_host),
	.plat_auto	= sizeof(struct am654_sdhci_plat),
};
