/*
 * (C) Copyright 2015
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Lokesh Vutla <lokeshvutla@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/utils.h>
#include <asm/arch/dra7xx_iodelay.h>
#include <asm/arch/omap.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/clock.h>
#include <asm/omap_common.h>

static int isolate_io(u32 isolate)
{
	if (isolate) {
		clrsetbits_le32((*ctrl)->control_pbias, SDCARD_PWRDNZ,
				SDCARD_PWRDNZ);
		clrsetbits_le32((*ctrl)->control_pbias, SDCARD_BIAS_PWRDNZ,
				SDCARD_BIAS_PWRDNZ);
	}

	/* Override control on ISOCLKIN signal to IO pad ring. */
	clrsetbits_le32((*prcm)->prm_io_pmctrl, PMCTRL_ISOCLK_OVERRIDE_MASK,
			PMCTRL_ISOCLK_OVERRIDE_CTRL);
	if (!wait_on_value(PMCTRL_ISOCLK_STATUS_MASK, PMCTRL_ISOCLK_STATUS_MASK,
			   (u32 *)(*prcm)->prm_io_pmctrl, LDELAY))
		return ERR_DEISOLATE_IO << isolate;

	/* Isolate/Deisolate IO */
	clrsetbits_le32((*ctrl)->ctrl_core_sma_sw_0, CTRL_ISOLATE_MASK,
			isolate << CTRL_ISOLATE_SHIFT);
	/* Dummy read to add delay t > 10ns */
	readl((*ctrl)->ctrl_core_sma_sw_0);

	/* Return control on ISOCLKIN to hardware */
	clrsetbits_le32((*prcm)->prm_io_pmctrl, PMCTRL_ISOCLK_OVERRIDE_MASK,
			PMCTRL_ISOCLK_NOT_OVERRIDE_CTRL);
	if (!wait_on_value(PMCTRL_ISOCLK_STATUS_MASK,
			   0 << PMCTRL_ISOCLK_STATUS_SHIFT,
			   (u32 *)(*prcm)->prm_io_pmctrl, LDELAY))
		return ERR_DEISOLATE_IO << isolate;

	return 0;
}

static int calibrate_iodelay(u32 base)
{
	u32 reg;

	/* Configure REFCLK period */
	reg = readl(base + CFG_REG_2_OFFSET);
	reg &= ~CFG_REG_REFCLK_PERIOD_MASK;
	reg |= CFG_REG_REFCLK_PERIOD;
	writel(reg, base + CFG_REG_2_OFFSET);

	/* Initiate Calibration */
	clrsetbits_le32(base + CFG_REG_0_OFFSET, CFG_REG_CALIB_STRT_MASK,
			CFG_REG_CALIB_STRT << CFG_REG_CALIB_STRT_SHIFT);
	if (!wait_on_value(CFG_REG_CALIB_STRT_MASK, CFG_REG_CALIB_END,
			   (u32 *)(base + CFG_REG_0_OFFSET), LDELAY))
		return ERR_CALIBRATE_IODELAY;

	return 0;
}

static int update_delay_mechanism(u32 base)
{
	/* Initiate the reload of calibrated values. */
	clrsetbits_le32(base + CFG_REG_0_OFFSET, CFG_REG_ROM_READ_MASK,
			CFG_REG_ROM_READ_START);
	if (!wait_on_value(CFG_REG_ROM_READ_MASK, CFG_REG_ROM_READ_END,
			   (u32 *)(base + CFG_REG_0_OFFSET), LDELAY))
		return ERR_UPDATE_DELAY;

	return 0;
}

void __recalibrate_iodelay(struct pad_conf_entry const *pad, int npads)
{
	int ret = 0;

	/* IO recalibration should be done only from SRAM */
	if (OMAP_INIT_CONTEXT_SPL != omap_hw_init_context()) {
		puts("IODELAY recalibration called from invalid context - use only from SPL in SRAM\n");
		return;
	}

	/* unlock IODELAY CONFIG registers */
	writel(CFG_IODELAY_UNLOCK_KEY, (*ctrl)->iodelay_config_base +
	       CFG_REG_8_OFFSET);

	ret = calibrate_iodelay((*ctrl)->iodelay_config_base);
	if (ret)
		goto err;

	ret = isolate_io(ISOLATE_IO);
	if (ret)
		goto err;

	ret = update_delay_mechanism((*ctrl)->iodelay_config_base);
	if (ret)
		goto err;

	/* Configure Mux settings */
	do_set_mux32((*ctrl)->control_padconf_core_base, pad, npads);

	ret = isolate_io(DEISOLATE_IO);

err:
	/* lock IODELAY CONFIG registers */
	writel(CFG_IODELAY_LOCK_KEY, (*ctrl)->iodelay_config_base +
	       CFG_REG_8_OFFSET);
	/*
	 * UART cannot be used during IO recalibration sequence as IOs are in
	 * isolation. So error handling and debug prints are done after
	 * complete IO delay recalibration sequence
	 */
	switch (ret) {
	case ERR_CALIBRATE_IODELAY:
		puts("IODELAY: IO delay calibration sequence failed\n");
		break;
	case ERR_ISOLATE_IO:
		puts("IODELAY: Isolation of Device IOs failed\n");
		break;
	case ERR_UPDATE_DELAY:
		puts("IODELAY: Delay mechanism update with new calibrated values failed\n");
		break;
	case ERR_DEISOLATE_IO:
		puts("IODELAY: De-isolation of Device IOs failed\n");
		break;
	default:
		debug("IODELAY: IO delay recalibration successfully completed\n");
	}
}
