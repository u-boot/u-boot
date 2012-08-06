/*
 * Freescale i.MX28 Boot PMIC init
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
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

#include <common.h>
#include <config.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>

#include "mxs_init.h"

void mxs_power_clock2xtal(void)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;

	/* Set XTAL as CPU reference clock */
	writel(CLKCTRL_CLKSEQ_BYPASS_CPU,
		&clkctrl_regs->hw_clkctrl_clkseq_set);
}

void mxs_power_clock2pll(void)
{
	struct mxs_clkctrl_regs *clkctrl_regs =
		(struct mxs_clkctrl_regs *)MXS_CLKCTRL_BASE;

	setbits_le32(&clkctrl_regs->hw_clkctrl_pll0ctrl0,
			CLKCTRL_PLL0CTRL0_POWER);
	early_delay(100);
	setbits_le32(&clkctrl_regs->hw_clkctrl_clkseq,
			CLKCTRL_CLKSEQ_BYPASS_CPU);
}

void mxs_power_clear_auto_restart(void)
{
	struct mxs_rtc_regs *rtc_regs =
		(struct mxs_rtc_regs *)MXS_RTC_BASE;

	writel(RTC_CTRL_SFTRST, &rtc_regs->hw_rtc_ctrl_clr);
	while (readl(&rtc_regs->hw_rtc_ctrl) & RTC_CTRL_SFTRST)
		;

	writel(RTC_CTRL_CLKGATE, &rtc_regs->hw_rtc_ctrl_clr);
	while (readl(&rtc_regs->hw_rtc_ctrl) & RTC_CTRL_CLKGATE)
		;

	/*
	 * Due to the hardware design bug of mx28 EVK-A
	 * we need to set the AUTO_RESTART bit.
	 */
	if (readl(&rtc_regs->hw_rtc_persistent0) & RTC_PERSISTENT0_AUTO_RESTART)
		return;

	while (readl(&rtc_regs->hw_rtc_stat) & RTC_STAT_NEW_REGS_MASK)
		;

	setbits_le32(&rtc_regs->hw_rtc_persistent0,
			RTC_PERSISTENT0_AUTO_RESTART);
	writel(RTC_CTRL_FORCE_UPDATE, &rtc_regs->hw_rtc_ctrl_set);
	writel(RTC_CTRL_FORCE_UPDATE, &rtc_regs->hw_rtc_ctrl_clr);
	while (readl(&rtc_regs->hw_rtc_stat) & RTC_STAT_NEW_REGS_MASK)
		;
	while (readl(&rtc_regs->hw_rtc_stat) & RTC_STAT_STALE_REGS_MASK)
		;
}

void mxs_power_set_linreg(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	/* Set linear regulator 25mV below switching converter */
	clrsetbits_le32(&power_regs->hw_power_vdddctrl,
			POWER_VDDDCTRL_LINREG_OFFSET_MASK,
			POWER_VDDDCTRL_LINREG_OFFSET_1STEPS_BELOW);

	clrsetbits_le32(&power_regs->hw_power_vddactrl,
			POWER_VDDACTRL_LINREG_OFFSET_MASK,
			POWER_VDDACTRL_LINREG_OFFSET_1STEPS_BELOW);

	clrsetbits_le32(&power_regs->hw_power_vddioctrl,
			POWER_VDDIOCTRL_LINREG_OFFSET_MASK,
			POWER_VDDIOCTRL_LINREG_OFFSET_1STEPS_BELOW);
}

int mxs_get_batt_volt(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t volt = readl(&power_regs->hw_power_battmonitor);
	volt &= POWER_BATTMONITOR_BATT_VAL_MASK;
	volt >>= POWER_BATTMONITOR_BATT_VAL_OFFSET;
	volt *= 8;
	return volt;
}

int mxs_is_batt_ready(void)
{
	return (mxs_get_batt_volt() >= 3600);
}

int mxs_is_batt_good(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t volt = mxs_get_batt_volt();

	if ((volt >= 2400) && (volt <= 4300))
		return 1;

	clrsetbits_le32(&power_regs->hw_power_5vctrl,
		POWER_5VCTRL_CHARGE_4P2_ILIMIT_MASK,
		0x3 << POWER_5VCTRL_CHARGE_4P2_ILIMIT_OFFSET);
	writel(POWER_5VCTRL_PWD_CHARGE_4P2_MASK,
		&power_regs->hw_power_5vctrl_clr);

	clrsetbits_le32(&power_regs->hw_power_charge,
		POWER_CHARGE_STOP_ILIMIT_MASK | POWER_CHARGE_BATTCHRG_I_MASK,
		POWER_CHARGE_STOP_ILIMIT_10MA | 0x3);

	writel(POWER_CHARGE_PWD_BATTCHRG, &power_regs->hw_power_charge_clr);
	writel(POWER_5VCTRL_PWD_CHARGE_4P2_MASK,
		&power_regs->hw_power_5vctrl_clr);

	early_delay(500000);

	volt = mxs_get_batt_volt();

	if (volt >= 3500)
		return 0;

	if (volt >= 2400)
		return 1;

	writel(POWER_CHARGE_STOP_ILIMIT_MASK | POWER_CHARGE_BATTCHRG_I_MASK,
		&power_regs->hw_power_charge_clr);
	writel(POWER_CHARGE_PWD_BATTCHRG, &power_regs->hw_power_charge_set);

	return 0;
}

void mxs_power_setup_5v_detect(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	/* Start 5V detection */
	clrsetbits_le32(&power_regs->hw_power_5vctrl,
			POWER_5VCTRL_VBUSVALID_TRSH_MASK,
			POWER_5VCTRL_VBUSVALID_TRSH_4V4 |
			POWER_5VCTRL_PWRUP_VBUS_CMPS);
}

void mxs_src_power_init(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	/* Improve efficieny and reduce transient ripple */
	writel(POWER_LOOPCTRL_TOGGLE_DIF | POWER_LOOPCTRL_EN_CM_HYST |
		POWER_LOOPCTRL_EN_DF_HYST, &power_regs->hw_power_loopctrl_set);

	clrsetbits_le32(&power_regs->hw_power_dclimits,
			POWER_DCLIMITS_POSLIMIT_BUCK_MASK,
			0x30 << POWER_DCLIMITS_POSLIMIT_BUCK_OFFSET);

	setbits_le32(&power_regs->hw_power_battmonitor,
			POWER_BATTMONITOR_EN_BATADJ);

	/* Increase the RCSCALE level for quick DCDC response to dynamic load */
	clrsetbits_le32(&power_regs->hw_power_loopctrl,
			POWER_LOOPCTRL_EN_RCSCALE_MASK,
			POWER_LOOPCTRL_RCSCALE_THRESH |
			POWER_LOOPCTRL_EN_RCSCALE_8X);

	clrsetbits_le32(&power_regs->hw_power_minpwr,
			POWER_MINPWR_HALFFETS, POWER_MINPWR_DOUBLE_FETS);

	/* 5V to battery handoff ... FIXME */
	setbits_le32(&power_regs->hw_power_5vctrl, POWER_5VCTRL_DCDC_XFER);
	early_delay(30);
	clrbits_le32(&power_regs->hw_power_5vctrl, POWER_5VCTRL_DCDC_XFER);
}

void mxs_power_init_4p2_params(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	/* Setup 4P2 parameters */
	clrsetbits_le32(&power_regs->hw_power_dcdc4p2,
		POWER_DCDC4P2_CMPTRIP_MASK | POWER_DCDC4P2_TRG_MASK,
		POWER_DCDC4P2_TRG_4V2 | (31 << POWER_DCDC4P2_CMPTRIP_OFFSET));

	clrsetbits_le32(&power_regs->hw_power_5vctrl,
		POWER_5VCTRL_HEADROOM_ADJ_MASK,
		0x4 << POWER_5VCTRL_HEADROOM_ADJ_OFFSET);

	clrsetbits_le32(&power_regs->hw_power_dcdc4p2,
		POWER_DCDC4P2_DROPOUT_CTRL_MASK,
		POWER_DCDC4P2_DROPOUT_CTRL_100MV |
		POWER_DCDC4P2_DROPOUT_CTRL_SRC_SEL);

	clrsetbits_le32(&power_regs->hw_power_5vctrl,
		POWER_5VCTRL_CHARGE_4P2_ILIMIT_MASK,
		0x3f << POWER_5VCTRL_CHARGE_4P2_ILIMIT_OFFSET);
}

void mxs_enable_4p2_dcdc_input(int xfer)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t tmp, vbus_thresh, vbus_5vdetect, pwd_bo;
	uint32_t prev_5v_brnout, prev_5v_droop;

	prev_5v_brnout = readl(&power_regs->hw_power_5vctrl) &
				POWER_5VCTRL_PWDN_5VBRNOUT;
	prev_5v_droop = readl(&power_regs->hw_power_ctrl) &
				POWER_CTRL_ENIRQ_VDD5V_DROOP;

	clrbits_le32(&power_regs->hw_power_5vctrl, POWER_5VCTRL_PWDN_5VBRNOUT);
	writel(POWER_RESET_UNLOCK_KEY | POWER_RESET_PWD_OFF,
		&power_regs->hw_power_reset);

	clrbits_le32(&power_regs->hw_power_ctrl, POWER_CTRL_ENIRQ_VDD5V_DROOP);

	if (xfer && (readl(&power_regs->hw_power_5vctrl) &
			POWER_5VCTRL_ENABLE_DCDC)) {
		return;
	}

	/*
	 * Recording orignal values that will be modified temporarlily
	 * to handle a chip bug. See chip errata for CQ ENGR00115837
	 */
	tmp = readl(&power_regs->hw_power_5vctrl);
	vbus_thresh = tmp & POWER_5VCTRL_VBUSVALID_TRSH_MASK;
	vbus_5vdetect = tmp & POWER_5VCTRL_VBUSVALID_5VDETECT;

	pwd_bo = readl(&power_regs->hw_power_minpwr) & POWER_MINPWR_PWD_BO;

	/*
	 * Disable mechanisms that get erroneously tripped by when setting
	 * the DCDC4P2 EN_DCDC
	 */
	clrbits_le32(&power_regs->hw_power_5vctrl,
		POWER_5VCTRL_VBUSVALID_5VDETECT |
		POWER_5VCTRL_VBUSVALID_TRSH_MASK);

	writel(POWER_MINPWR_PWD_BO, &power_regs->hw_power_minpwr_set);

	if (xfer) {
		setbits_le32(&power_regs->hw_power_5vctrl,
				POWER_5VCTRL_DCDC_XFER);
		early_delay(20);
		clrbits_le32(&power_regs->hw_power_5vctrl,
				POWER_5VCTRL_DCDC_XFER);

		setbits_le32(&power_regs->hw_power_5vctrl,
				POWER_5VCTRL_ENABLE_DCDC);
	} else {
		setbits_le32(&power_regs->hw_power_dcdc4p2,
				POWER_DCDC4P2_ENABLE_DCDC);
	}

	early_delay(25);

	clrsetbits_le32(&power_regs->hw_power_5vctrl,
			POWER_5VCTRL_VBUSVALID_TRSH_MASK, vbus_thresh);

	if (vbus_5vdetect)
		writel(vbus_5vdetect, &power_regs->hw_power_5vctrl_set);

	if (!pwd_bo)
		clrbits_le32(&power_regs->hw_power_minpwr, POWER_MINPWR_PWD_BO);

	while (readl(&power_regs->hw_power_ctrl) & POWER_CTRL_VBUS_VALID_IRQ)
		writel(POWER_CTRL_VBUS_VALID_IRQ,
			&power_regs->hw_power_ctrl_clr);

	if (prev_5v_brnout) {
		writel(POWER_5VCTRL_PWDN_5VBRNOUT,
			&power_regs->hw_power_5vctrl_set);
		writel(POWER_RESET_UNLOCK_KEY,
			&power_regs->hw_power_reset);
	} else {
		writel(POWER_5VCTRL_PWDN_5VBRNOUT,
			&power_regs->hw_power_5vctrl_clr);
		writel(POWER_RESET_UNLOCK_KEY | POWER_RESET_PWD_OFF,
			&power_regs->hw_power_reset);
	}

	while (readl(&power_regs->hw_power_ctrl) & POWER_CTRL_VDD5V_DROOP_IRQ)
		writel(POWER_CTRL_VDD5V_DROOP_IRQ,
			&power_regs->hw_power_ctrl_clr);

	if (prev_5v_droop)
		clrbits_le32(&power_regs->hw_power_ctrl,
				POWER_CTRL_ENIRQ_VDD5V_DROOP);
	else
		setbits_le32(&power_regs->hw_power_ctrl,
				POWER_CTRL_ENIRQ_VDD5V_DROOP);
}

void mxs_power_init_4p2_regulator(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t tmp, tmp2;

	setbits_le32(&power_regs->hw_power_dcdc4p2, POWER_DCDC4P2_ENABLE_4P2);

	writel(POWER_CHARGE_ENABLE_LOAD, &power_regs->hw_power_charge_set);

	writel(POWER_5VCTRL_CHARGE_4P2_ILIMIT_MASK,
		&power_regs->hw_power_5vctrl_clr);
	clrbits_le32(&power_regs->hw_power_dcdc4p2, POWER_DCDC4P2_TRG_MASK);

	/* Power up the 4p2 rail and logic/control */
	writel(POWER_5VCTRL_PWD_CHARGE_4P2_MASK,
		&power_regs->hw_power_5vctrl_clr);

	/*
	 * Start charging up the 4p2 capacitor. We ramp of this charge
	 * gradually to avoid large inrush current from the 5V cable which can
	 * cause transients/problems
	 */
	mxs_enable_4p2_dcdc_input(0);

	if (readl(&power_regs->hw_power_ctrl) & POWER_CTRL_VBUS_VALID_IRQ) {
		/*
		 * If we arrived here, we were unable to recover from mx23 chip
		 * errata 5837. 4P2 is disabled and sufficient battery power is
		 * not present. Exiting to not enable DCDC power during 5V
		 * connected state.
		 */
		clrbits_le32(&power_regs->hw_power_dcdc4p2,
			POWER_DCDC4P2_ENABLE_DCDC);
		writel(POWER_5VCTRL_PWD_CHARGE_4P2_MASK,
			&power_regs->hw_power_5vctrl_set);
		hang();
	}

	/*
	 * Here we set the 4p2 brownout level to something very close to 4.2V.
	 * We then check the brownout status. If the brownout status is false,
	 * the voltage is already close to the target voltage of 4.2V so we
	 * can go ahead and set the 4P2 current limit to our max target limit.
	 * If the brownout status is true, we need to ramp us the current limit
	 * so that we don't cause large inrush current issues. We step up the
	 * current limit until the brownout status is false or until we've
	 * reached our maximum defined 4p2 current limit.
	 */
	clrsetbits_le32(&power_regs->hw_power_dcdc4p2,
			POWER_DCDC4P2_BO_MASK,
			22 << POWER_DCDC4P2_BO_OFFSET);	/* 4.15V */

	if (!(readl(&power_regs->hw_power_sts) & POWER_STS_DCDC_4P2_BO)) {
		setbits_le32(&power_regs->hw_power_5vctrl,
			0x3f << POWER_5VCTRL_CHARGE_4P2_ILIMIT_OFFSET);
	} else {
		tmp = (readl(&power_regs->hw_power_5vctrl) &
			POWER_5VCTRL_CHARGE_4P2_ILIMIT_MASK) >>
			POWER_5VCTRL_CHARGE_4P2_ILIMIT_OFFSET;
		while (tmp < 0x3f) {
			if (!(readl(&power_regs->hw_power_sts) &
					POWER_STS_DCDC_4P2_BO)) {
				tmp = readl(&power_regs->hw_power_5vctrl);
				tmp |= POWER_5VCTRL_CHARGE_4P2_ILIMIT_MASK;
				early_delay(100);
				writel(tmp, &power_regs->hw_power_5vctrl);
				break;
			} else {
				tmp++;
				tmp2 = readl(&power_regs->hw_power_5vctrl);
				tmp2 &= ~POWER_5VCTRL_CHARGE_4P2_ILIMIT_MASK;
				tmp2 |= tmp <<
					POWER_5VCTRL_CHARGE_4P2_ILIMIT_OFFSET;
				writel(tmp2, &power_regs->hw_power_5vctrl);
				early_delay(100);
			}
		}
	}

	clrbits_le32(&power_regs->hw_power_dcdc4p2, POWER_DCDC4P2_BO_MASK);
	writel(POWER_CTRL_DCDC4P2_BO_IRQ, &power_regs->hw_power_ctrl_clr);
}

void mxs_power_init_dcdc_4p2_source(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	if (!(readl(&power_regs->hw_power_dcdc4p2) &
		POWER_DCDC4P2_ENABLE_DCDC)) {
		hang();
	}

	mxs_enable_4p2_dcdc_input(1);

	if (readl(&power_regs->hw_power_ctrl) & POWER_CTRL_VBUS_VALID_IRQ) {
		clrbits_le32(&power_regs->hw_power_dcdc4p2,
			POWER_DCDC4P2_ENABLE_DCDC);
		writel(POWER_5VCTRL_ENABLE_DCDC,
			&power_regs->hw_power_5vctrl_clr);
		writel(POWER_5VCTRL_PWD_CHARGE_4P2_MASK,
			&power_regs->hw_power_5vctrl_set);
	}
}

void mxs_power_enable_4p2(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t vdddctrl, vddactrl, vddioctrl;
	uint32_t tmp;

	vdddctrl = readl(&power_regs->hw_power_vdddctrl);
	vddactrl = readl(&power_regs->hw_power_vddactrl);
	vddioctrl = readl(&power_regs->hw_power_vddioctrl);

	setbits_le32(&power_regs->hw_power_vdddctrl,
		POWER_VDDDCTRL_DISABLE_FET | POWER_VDDDCTRL_ENABLE_LINREG |
		POWER_VDDDCTRL_PWDN_BRNOUT);

	setbits_le32(&power_regs->hw_power_vddactrl,
		POWER_VDDACTRL_DISABLE_FET | POWER_VDDACTRL_ENABLE_LINREG |
		POWER_VDDACTRL_PWDN_BRNOUT);

	setbits_le32(&power_regs->hw_power_vddioctrl,
		POWER_VDDIOCTRL_DISABLE_FET | POWER_VDDIOCTRL_PWDN_BRNOUT);

	mxs_power_init_4p2_params();
	mxs_power_init_4p2_regulator();

	/* Shutdown battery (none present) */
	if (!mxs_is_batt_ready()) {
		clrbits_le32(&power_regs->hw_power_dcdc4p2,
				POWER_DCDC4P2_BO_MASK);
		writel(POWER_CTRL_DCDC4P2_BO_IRQ,
				&power_regs->hw_power_ctrl_clr);
		writel(POWER_CTRL_ENIRQ_DCDC4P2_BO,
				&power_regs->hw_power_ctrl_clr);
	}

	mxs_power_init_dcdc_4p2_source();

	writel(vdddctrl, &power_regs->hw_power_vdddctrl);
	early_delay(20);
	writel(vddactrl, &power_regs->hw_power_vddactrl);
	early_delay(20);
	writel(vddioctrl, &power_regs->hw_power_vddioctrl);

	/*
	 * Check if FET is enabled on either powerout and if so,
	 * disable load.
	 */
	tmp = 0;
	tmp |= !(readl(&power_regs->hw_power_vdddctrl) &
			POWER_VDDDCTRL_DISABLE_FET);
	tmp |= !(readl(&power_regs->hw_power_vddactrl) &
			POWER_VDDACTRL_DISABLE_FET);
	tmp |= !(readl(&power_regs->hw_power_vddioctrl) &
			POWER_VDDIOCTRL_DISABLE_FET);
	if (tmp)
		writel(POWER_CHARGE_ENABLE_LOAD,
			&power_regs->hw_power_charge_clr);
}

void mxs_boot_valid_5v(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	/*
	 * Use VBUSVALID level instead of VDD5V_GT_VDDIO level to trigger a 5V
	 * disconnect event. FIXME
	 */
	writel(POWER_5VCTRL_VBUSVALID_5VDETECT,
		&power_regs->hw_power_5vctrl_set);

	/* Configure polarity to check for 5V disconnection. */
	writel(POWER_CTRL_POLARITY_VBUSVALID |
		POWER_CTRL_POLARITY_VDD5V_GT_VDDIO,
		&power_regs->hw_power_ctrl_clr);

	writel(POWER_CTRL_VBUS_VALID_IRQ | POWER_CTRL_VDD5V_GT_VDDIO_IRQ,
		&power_regs->hw_power_ctrl_clr);

	mxs_power_enable_4p2();
}

void mxs_powerdown(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	writel(POWER_RESET_UNLOCK_KEY, &power_regs->hw_power_reset);
	writel(POWER_RESET_UNLOCK_KEY | POWER_RESET_PWD_OFF,
		&power_regs->hw_power_reset);
}

void mxs_batt_boot(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	clrbits_le32(&power_regs->hw_power_5vctrl, POWER_5VCTRL_PWDN_5VBRNOUT);
	clrbits_le32(&power_regs->hw_power_5vctrl, POWER_5VCTRL_ENABLE_DCDC);

	clrbits_le32(&power_regs->hw_power_dcdc4p2,
			POWER_DCDC4P2_ENABLE_DCDC | POWER_DCDC4P2_ENABLE_4P2);
	writel(POWER_CHARGE_ENABLE_LOAD, &power_regs->hw_power_charge_clr);

	/* 5V to battery handoff. */
	setbits_le32(&power_regs->hw_power_5vctrl, POWER_5VCTRL_DCDC_XFER);
	early_delay(30);
	clrbits_le32(&power_regs->hw_power_5vctrl, POWER_5VCTRL_DCDC_XFER);

	writel(POWER_CTRL_ENIRQ_DCDC4P2_BO, &power_regs->hw_power_ctrl_clr);

	clrsetbits_le32(&power_regs->hw_power_minpwr,
			POWER_MINPWR_HALFFETS, POWER_MINPWR_DOUBLE_FETS);

	mxs_power_set_linreg();

	clrbits_le32(&power_regs->hw_power_vdddctrl,
		POWER_VDDDCTRL_DISABLE_FET | POWER_VDDDCTRL_ENABLE_LINREG);

	clrbits_le32(&power_regs->hw_power_vddactrl,
		POWER_VDDACTRL_DISABLE_FET | POWER_VDDACTRL_ENABLE_LINREG);

	clrbits_le32(&power_regs->hw_power_vddioctrl,
		POWER_VDDIOCTRL_DISABLE_FET);

	setbits_le32(&power_regs->hw_power_5vctrl,
		POWER_5VCTRL_PWD_CHARGE_4P2_MASK);

	setbits_le32(&power_regs->hw_power_5vctrl,
		POWER_5VCTRL_ENABLE_DCDC);

	clrsetbits_le32(&power_regs->hw_power_5vctrl,
		POWER_5VCTRL_CHARGE_4P2_ILIMIT_MASK,
		0x8 << POWER_5VCTRL_CHARGE_4P2_ILIMIT_OFFSET);
}

void mxs_handle_5v_conflict(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t tmp;

	setbits_le32(&power_regs->hw_power_vddioctrl,
			POWER_VDDIOCTRL_BO_OFFSET_MASK);

	for (;;) {
		tmp = readl(&power_regs->hw_power_sts);

		if (tmp & POWER_STS_VDDIO_BO) {
			/*
			 * VDDIO has a brownout, then the VDD5V_GT_VDDIO becomes
			 * unreliable
			 */
			mxs_powerdown();
			break;
		}

		if (tmp & POWER_STS_VDD5V_GT_VDDIO) {
			mxs_boot_valid_5v();
			break;
		} else {
			mxs_powerdown();
			break;
		}

		if (tmp & POWER_STS_PSWITCH_MASK) {
			mxs_batt_boot();
			break;
		}
	}
}

void mxs_5v_boot(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	/*
	 * NOTE: In original IMX-Bootlets, this also checks for VBUSVALID,
	 * but their implementation always returns 1 so we omit it here.
	 */
	if (readl(&power_regs->hw_power_sts) & POWER_STS_VDD5V_GT_VDDIO) {
		mxs_boot_valid_5v();
		return;
	}

	early_delay(1000);
	if (readl(&power_regs->hw_power_sts) & POWER_STS_VDD5V_GT_VDDIO) {
		mxs_boot_valid_5v();
		return;
	}

	mxs_handle_5v_conflict();
}

void mxs_init_batt_bo(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	/* Brownout at 3V */
	clrsetbits_le32(&power_regs->hw_power_battmonitor,
		POWER_BATTMONITOR_BRWNOUT_LVL_MASK,
		15 << POWER_BATTMONITOR_BRWNOUT_LVL_OFFSET);

	writel(POWER_CTRL_BATT_BO_IRQ, &power_regs->hw_power_ctrl_clr);
	writel(POWER_CTRL_ENIRQ_BATT_BO, &power_regs->hw_power_ctrl_clr);
}

void mxs_switch_vddd_to_dcdc_source(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	clrsetbits_le32(&power_regs->hw_power_vdddctrl,
		POWER_VDDDCTRL_LINREG_OFFSET_MASK,
		POWER_VDDDCTRL_LINREG_OFFSET_1STEPS_BELOW);

	clrbits_le32(&power_regs->hw_power_vdddctrl,
		POWER_VDDDCTRL_DISABLE_FET | POWER_VDDDCTRL_ENABLE_LINREG |
		POWER_VDDDCTRL_DISABLE_STEPPING);
}

void mxs_power_configure_power_source(void)
{
	int batt_ready, batt_good;
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	struct mxs_lradc_regs *lradc_regs =
		(struct mxs_lradc_regs *)MXS_LRADC_BASE;

	mxs_src_power_init();

	if (readl(&power_regs->hw_power_sts) & POWER_STS_VDD5V_GT_VDDIO) {
		batt_ready = mxs_is_batt_ready();
		if (batt_ready) {
			/* 5V source detected, good battery detected. */
			mxs_batt_boot();
		} else {
			batt_good = mxs_is_batt_good();
			if (!batt_good) {
				/* 5V source detected, bad battery detected. */
				writel(LRADC_CONVERSION_AUTOMATIC,
					&lradc_regs->hw_lradc_conversion_clr);
				clrbits_le32(&power_regs->hw_power_battmonitor,
					POWER_BATTMONITOR_BATT_VAL_MASK);
			}
			mxs_5v_boot();
		}
	} else {
		/* 5V not detected, booting from battery. */
		mxs_batt_boot();
	}

	mxs_power_clock2pll();

	mxs_init_batt_bo();

	mxs_switch_vddd_to_dcdc_source();
}

void mxs_enable_output_rail_protection(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	writel(POWER_CTRL_VDDD_BO_IRQ | POWER_CTRL_VDDA_BO_IRQ |
		POWER_CTRL_VDDIO_BO_IRQ, &power_regs->hw_power_ctrl_clr);

	setbits_le32(&power_regs->hw_power_vdddctrl,
			POWER_VDDDCTRL_PWDN_BRNOUT);

	setbits_le32(&power_regs->hw_power_vddactrl,
			POWER_VDDACTRL_PWDN_BRNOUT);

	setbits_le32(&power_regs->hw_power_vddioctrl,
			POWER_VDDIOCTRL_PWDN_BRNOUT);
}

int mxs_get_vddio_power_source_off(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t tmp;

	if (readl(&power_regs->hw_power_sts) & POWER_STS_VDD5V_GT_VDDIO) {
		tmp = readl(&power_regs->hw_power_vddioctrl);
		if (tmp & POWER_VDDIOCTRL_DISABLE_FET) {
			if ((tmp & POWER_VDDIOCTRL_LINREG_OFFSET_MASK) ==
				POWER_VDDIOCTRL_LINREG_OFFSET_0STEPS) {
				return 1;
			}
		}

		if (!(readl(&power_regs->hw_power_5vctrl) &
			POWER_5VCTRL_ENABLE_DCDC)) {
			if ((tmp & POWER_VDDIOCTRL_LINREG_OFFSET_MASK) ==
				POWER_VDDIOCTRL_LINREG_OFFSET_0STEPS) {
				return 1;
			}
		}
	}

	return 0;

}

int mxs_get_vddd_power_source_off(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t tmp;

	tmp = readl(&power_regs->hw_power_vdddctrl);
	if (tmp & POWER_VDDDCTRL_DISABLE_FET) {
		if ((tmp & POWER_VDDDCTRL_LINREG_OFFSET_MASK) ==
			POWER_VDDDCTRL_LINREG_OFFSET_0STEPS) {
			return 1;
		}
	}

	if (readl(&power_regs->hw_power_sts) & POWER_STS_VDD5V_GT_VDDIO) {
		if (!(readl(&power_regs->hw_power_5vctrl) &
			POWER_5VCTRL_ENABLE_DCDC)) {
			return 1;
		}
	}

	if (!(tmp & POWER_VDDDCTRL_ENABLE_LINREG)) {
		if ((tmp & POWER_VDDDCTRL_LINREG_OFFSET_MASK) ==
			POWER_VDDDCTRL_LINREG_OFFSET_1STEPS_BELOW) {
			return 1;
		}
	}

	return 0;
}

void mxs_power_set_vddio(uint32_t new_target, uint32_t new_brownout)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t cur_target, diff, bo_int = 0;
	uint32_t powered_by_linreg = 0;

	new_brownout = (new_target - new_brownout + 25) / 50;

	cur_target = readl(&power_regs->hw_power_vddioctrl);
	cur_target &= POWER_VDDIOCTRL_TRG_MASK;
	cur_target *= 50;	/* 50 mV step*/
	cur_target += 2800;	/* 2800 mV lowest */

	powered_by_linreg = mxs_get_vddio_power_source_off();
	if (new_target > cur_target) {

		if (powered_by_linreg) {
			bo_int = readl(&power_regs->hw_power_vddioctrl);
			clrbits_le32(&power_regs->hw_power_vddioctrl,
					POWER_CTRL_ENIRQ_VDDIO_BO);
		}

		setbits_le32(&power_regs->hw_power_vddioctrl,
				POWER_VDDIOCTRL_BO_OFFSET_MASK);
		do {
			if (new_target - cur_target > 100)
				diff = cur_target + 100;
			else
				diff = new_target;

			diff -= 2800;
			diff /= 50;

			clrsetbits_le32(&power_regs->hw_power_vddioctrl,
				POWER_VDDIOCTRL_TRG_MASK, diff);

			if (powered_by_linreg ||
				(readl(&power_regs->hw_power_sts) &
					POWER_STS_VDD5V_GT_VDDIO))
				early_delay(500);
			else {
				while (!(readl(&power_regs->hw_power_sts) &
					POWER_STS_DC_OK))
					;

			}

			cur_target = readl(&power_regs->hw_power_vddioctrl);
			cur_target &= POWER_VDDIOCTRL_TRG_MASK;
			cur_target *= 50;	/* 50 mV step*/
			cur_target += 2800;	/* 2800 mV lowest */
		} while (new_target > cur_target);

		if (powered_by_linreg) {
			writel(POWER_CTRL_VDDIO_BO_IRQ,
				&power_regs->hw_power_ctrl_clr);
			if (bo_int & POWER_CTRL_ENIRQ_VDDIO_BO)
				setbits_le32(&power_regs->hw_power_vddioctrl,
						POWER_CTRL_ENIRQ_VDDIO_BO);
		}
	} else {
		do {
			if (cur_target - new_target > 100)
				diff = cur_target - 100;
			else
				diff = new_target;

			diff -= 2800;
			diff /= 50;

			clrsetbits_le32(&power_regs->hw_power_vddioctrl,
				POWER_VDDIOCTRL_TRG_MASK, diff);

			if (powered_by_linreg ||
				(readl(&power_regs->hw_power_sts) &
					POWER_STS_VDD5V_GT_VDDIO))
				early_delay(500);
			else {
				while (!(readl(&power_regs->hw_power_sts) &
					POWER_STS_DC_OK))
					;

			}

			cur_target = readl(&power_regs->hw_power_vddioctrl);
			cur_target &= POWER_VDDIOCTRL_TRG_MASK;
			cur_target *= 50;	/* 50 mV step*/
			cur_target += 2800;	/* 2800 mV lowest */
		} while (new_target < cur_target);
	}

	clrsetbits_le32(&power_regs->hw_power_vddioctrl,
			POWER_VDDIOCTRL_BO_OFFSET_MASK,
			new_brownout << POWER_VDDIOCTRL_BO_OFFSET_OFFSET);
}

void mxs_power_set_vddd(uint32_t new_target, uint32_t new_brownout)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;
	uint32_t cur_target, diff, bo_int = 0;
	uint32_t powered_by_linreg = 0;

	new_brownout = (new_target - new_brownout + 12) / 25;

	cur_target = readl(&power_regs->hw_power_vdddctrl);
	cur_target &= POWER_VDDDCTRL_TRG_MASK;
	cur_target *= 25;	/* 25 mV step*/
	cur_target += 800;	/* 800 mV lowest */

	powered_by_linreg = mxs_get_vddd_power_source_off();
	if (new_target > cur_target) {
		if (powered_by_linreg) {
			bo_int = readl(&power_regs->hw_power_vdddctrl);
			clrbits_le32(&power_regs->hw_power_vdddctrl,
					POWER_CTRL_ENIRQ_VDDD_BO);
		}

		setbits_le32(&power_regs->hw_power_vdddctrl,
				POWER_VDDDCTRL_BO_OFFSET_MASK);

		do {
			if (new_target - cur_target > 100)
				diff = cur_target + 100;
			else
				diff = new_target;

			diff -= 800;
			diff /= 25;

			clrsetbits_le32(&power_regs->hw_power_vdddctrl,
				POWER_VDDDCTRL_TRG_MASK, diff);

			if (powered_by_linreg ||
				(readl(&power_regs->hw_power_sts) &
					POWER_STS_VDD5V_GT_VDDIO))
				early_delay(500);
			else {
				while (!(readl(&power_regs->hw_power_sts) &
					POWER_STS_DC_OK))
					;

			}

			cur_target = readl(&power_regs->hw_power_vdddctrl);
			cur_target &= POWER_VDDDCTRL_TRG_MASK;
			cur_target *= 25;	/* 25 mV step*/
			cur_target += 800;	/* 800 mV lowest */
		} while (new_target > cur_target);

		if (powered_by_linreg) {
			writel(POWER_CTRL_VDDD_BO_IRQ,
				&power_regs->hw_power_ctrl_clr);
			if (bo_int & POWER_CTRL_ENIRQ_VDDD_BO)
				setbits_le32(&power_regs->hw_power_vdddctrl,
						POWER_CTRL_ENIRQ_VDDD_BO);
		}
	} else {
		do {
			if (cur_target - new_target > 100)
				diff = cur_target - 100;
			else
				diff = new_target;

			diff -= 800;
			diff /= 25;

			clrsetbits_le32(&power_regs->hw_power_vdddctrl,
					POWER_VDDDCTRL_TRG_MASK, diff);

			if (powered_by_linreg ||
				(readl(&power_regs->hw_power_sts) &
					POWER_STS_VDD5V_GT_VDDIO))
				early_delay(500);
			else {
				while (!(readl(&power_regs->hw_power_sts) &
					POWER_STS_DC_OK))
					;

			}

			cur_target = readl(&power_regs->hw_power_vdddctrl);
			cur_target &= POWER_VDDDCTRL_TRG_MASK;
			cur_target *= 25;	/* 25 mV step*/
			cur_target += 800;	/* 800 mV lowest */
		} while (new_target < cur_target);
	}

	clrsetbits_le32(&power_regs->hw_power_vdddctrl,
			POWER_VDDDCTRL_BO_OFFSET_MASK,
			new_brownout << POWER_VDDDCTRL_BO_OFFSET_OFFSET);
}

void mxs_setup_batt_detect(void)
{
	mxs_lradc_init();
	mxs_lradc_enable_batt_measurement();
	early_delay(10);
}

void mxs_power_init(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	mxs_power_clock2xtal();
	mxs_power_clear_auto_restart();
	mxs_power_set_linreg();
	mxs_power_setup_5v_detect();

	mxs_setup_batt_detect();

	mxs_power_configure_power_source();
	mxs_enable_output_rail_protection();

	mxs_power_set_vddio(3300, 3150);

	mxs_power_set_vddd(1350, 1200);

	writel(POWER_CTRL_VDDD_BO_IRQ | POWER_CTRL_VDDA_BO_IRQ |
		POWER_CTRL_VDDIO_BO_IRQ | POWER_CTRL_VDD5V_DROOP_IRQ |
		POWER_CTRL_VBUS_VALID_IRQ | POWER_CTRL_BATT_BO_IRQ |
		POWER_CTRL_DCDC4P2_BO_IRQ, &power_regs->hw_power_ctrl_clr);

	writel(POWER_5VCTRL_PWDN_5VBRNOUT, &power_regs->hw_power_5vctrl_set);

	early_delay(1000);
}

#ifdef	CONFIG_SPL_MX28_PSWITCH_WAIT
void mxs_power_wait_pswitch(void)
{
	struct mxs_power_regs *power_regs =
		(struct mxs_power_regs *)MXS_POWER_BASE;

	while (!(readl(&power_regs->hw_power_sts) & POWER_STS_PSWITCH_MASK))
		;
}
#endif
