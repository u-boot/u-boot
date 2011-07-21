/*
 * EMIF programming
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/emif.h>
#include <asm/arch/clocks.h>
#include <asm/arch/sys_proto.h>
#include <asm/omap_common.h>
#include <asm/utils.h>

static inline u32 emif_num(u32 base)
{
	if (base == OMAP44XX_EMIF1)
		return 1;
	else if (base == OMAP44XX_EMIF2)
		return 2;
	else
		return 0;
}

static inline u32 get_mr(u32 base, u32 cs, u32 mr_addr)
{
	u32 mr;
	struct emif_reg_struct *emif = (struct emif_reg_struct *)base;

	mr_addr |= cs << OMAP44XX_REG_CS_SHIFT;
	writel(mr_addr, &emif->emif_lpddr2_mode_reg_cfg);
	if (omap_revision() == OMAP4430_ES2_0)
		mr = readl(&emif->emif_lpddr2_mode_reg_data_es2);
	else
		mr = readl(&emif->emif_lpddr2_mode_reg_data);
	debug("get_mr: EMIF%d cs %d mr %08x val 0x%x\n", emif_num(base),
	      cs, mr_addr, mr);
	return mr;
}

static inline void set_mr(u32 base, u32 cs, u32 mr_addr, u32 mr_val)
{
	struct emif_reg_struct *emif = (struct emif_reg_struct *)base;

	mr_addr |= cs << OMAP44XX_REG_CS_SHIFT;
	writel(mr_addr, &emif->emif_lpddr2_mode_reg_cfg);
	writel(mr_val, &emif->emif_lpddr2_mode_reg_data);
}

void emif_reset_phy(u32 base)
{
	struct emif_reg_struct *emif = (struct emif_reg_struct *)base;
	u32 iodft;

	iodft = readl(&emif->emif_iodft_tlgc);
	iodft |= OMAP44XX_REG_RESET_PHY_MASK;
	writel(iodft, &emif->emif_iodft_tlgc);
}

static void do_lpddr2_init(u32 base, u32 cs)
{
	u32 mr_addr;

	/* Wait till device auto initialization is complete */
	while (get_mr(base, cs, LPDDR2_MR0) & LPDDR2_MR0_DAI_MASK)
		;
	set_mr(base, cs, LPDDR2_MR10, MR10_ZQ_ZQINIT);
	/*
	 * tZQINIT = 1 us
	 * Enough loops assuming a maximum of 2GHz
	 */
	sdelay(2000);
	set_mr(base, cs, LPDDR2_MR1, MR1_BL_8_BT_SEQ_WRAP_EN_NWR_3);
	set_mr(base, cs, LPDDR2_MR16, MR16_REF_FULL_ARRAY);
	/*
	 * Enable refresh along with writing MR2
	 * Encoding of RL in MR2 is (RL - 2)
	 */
	mr_addr = LPDDR2_MR2 | OMAP44XX_REG_REFRESH_EN_MASK;
	set_mr(base, cs, mr_addr, RL_FINAL - 2);
}

static void lpddr2_init(u32 base, const struct emif_regs *regs)
{
	struct emif_reg_struct *emif = (struct emif_reg_struct *)base;

	/* Not NVM */
	clrbits_le32(&emif->emif_lpddr2_nvm_config, OMAP44XX_REG_CS1NVMEN_MASK);

	/*
	 * Keep REG_INITREF_DIS = 1 to prevent re-initialization of SDRAM
	 * when EMIF_SDRAM_CONFIG register is written
	 */
	setbits_le32(&emif->emif_sdram_ref_ctrl, OMAP44XX_REG_INITREF_DIS_MASK);

	/*
	 * Set the SDRAM_CONFIG and PHY_CTRL for the
	 * un-locked frequency & default RL
	 */
	writel(regs->sdram_config_init, &emif->emif_sdram_config);
	writel(regs->emif_ddr_phy_ctlr_1_init, &emif->emif_ddr_phy_ctrl_1);

	do_lpddr2_init(base, CS0);
	if (regs->sdram_config & OMAP44XX_REG_EBANK_MASK)
		do_lpddr2_init(base, CS1);

	writel(regs->sdram_config, &emif->emif_sdram_config);
	writel(regs->emif_ddr_phy_ctlr_1, &emif->emif_ddr_phy_ctrl_1);

	/* Enable refresh now */
	clrbits_le32(&emif->emif_sdram_ref_ctrl, OMAP44XX_REG_INITREF_DIS_MASK);

}

static void emif_update_timings(u32 base, const struct emif_regs *regs)
{
	struct emif_reg_struct *emif = (struct emif_reg_struct *)base;

	writel(regs->ref_ctrl, &emif->emif_sdram_ref_ctrl_shdw);
	writel(regs->sdram_tim1, &emif->emif_sdram_tim_1_shdw);
	writel(regs->sdram_tim2, &emif->emif_sdram_tim_2_shdw);
	writel(regs->sdram_tim3, &emif->emif_sdram_tim_3_shdw);
	if (omap_revision() == OMAP4430_ES1_0) {
		/* ES1 bug EMIF should be in force idle during freq_update */
		writel(0, &emif->emif_pwr_mgmt_ctrl);
	} else {
		writel(EMIF_PWR_MGMT_CTRL, &emif->emif_pwr_mgmt_ctrl);
		writel(EMIF_PWR_MGMT_CTRL_SHDW, &emif->emif_pwr_mgmt_ctrl_shdw);
	}
	writel(regs->read_idle_ctrl, &emif->emif_read_idlectrl_shdw);
	writel(regs->zq_config, &emif->emif_zq_config);
	writel(regs->temp_alert_config, &emif->emif_temp_alert_config);
	writel(regs->emif_ddr_phy_ctlr_1, &emif->emif_ddr_phy_ctrl_1_shdw);
	/*
	 * Workaround:
	 * In a specific situation, the OCP interface between the DMM and
	 * EMIF may hang.
	 * 1. A TILER port is used to perform 2D burst writes of
	 *       width 1 and height 8
	 * 2. ELLAn port is used to perform reads
	 * 3. All accesses are routed to the same EMIF controller
	 *
	 * Work around to avoid this issue REG_SYS_THRESH_MAX value should
	 * be kept higher than default 0x7. As per recommondation 0x0A will
	 * be used for better performance with REG_LL_THRESH_MAX = 0x00
	 */
	if (omap_revision() == OMAP4430_ES1_0) {
		writel(EMIF_L3_CONFIG_VAL_SYS_THRESH_0A_LL_THRESH_00,
		       &emif->emif_l3_config);
	}
}

static void do_sdram_init(u32 base)
{
	const struct emif_regs *regs;
	u32 in_sdram, emif_nr;

	debug(">>do_sdram_init() %x\n", base);

	in_sdram = running_from_sdram();
	emif_nr = (base == OMAP44XX_EMIF1) ? 1 : 2;

	emif_get_reg_dump(emif_nr, &regs);
	if (!regs) {
		debug("EMIF: reg dump not provided\n");
		return;
	}

	/*
	 * Initializing the LPDDR2 device can not happen from SDRAM.
	 * Changing the timing registers in EMIF can happen(going from one
	 * OPP to another)
	 */
	if (!in_sdram)
		lpddr2_init(base, regs);

	/* Write to the shadow registers */
	emif_update_timings(base, regs);

	debug("<<do_sdram_init() %x\n", base);
}

void sdram_init_pads(void)
{
	u32 lpddr2io;
	struct control_lpddr2io_regs *lpddr2io_regs =
		(struct control_lpddr2io_regs *)LPDDR2_IO_REGS_BASE;
	u32 omap4_rev = omap_revision();

	if (omap4_rev == OMAP4430_ES1_0)
		lpddr2io = CONTROL_LPDDR2IO_SLEW_125PS_DRV8_PULL_DOWN;
	else if (omap4_rev == OMAP4430_ES2_0)
		lpddr2io = CONTROL_LPDDR2IO_SLEW_325PS_DRV8_GATE_KEEPER;
	else
		return;		/* Post ES2.1 reset values will work */

	writel(lpddr2io, &lpddr2io_regs->control_lpddr2io1_0);
	writel(lpddr2io, &lpddr2io_regs->control_lpddr2io1_1);
	writel(lpddr2io, &lpddr2io_regs->control_lpddr2io1_2);
	writel(lpddr2io, &lpddr2io_regs->control_lpddr2io2_0);
	writel(lpddr2io, &lpddr2io_regs->control_lpddr2io2_1);
	writel(lpddr2io, &lpddr2io_regs->control_lpddr2io2_2);

	writel(CONTROL_EFUSE_2_NMOS_PMOS_PTV_CODE_1, CONTROL_EFUSE_2);
}

static void emif_post_init_config(u32 base)
{
	struct emif_reg_struct *emif = (struct emif_reg_struct *)base;
	u32 omap4_rev = omap_revision();

	/* reset phy on ES2.0 */
	if (omap4_rev == OMAP4430_ES2_0)
		emif_reset_phy(base);

	/* Put EMIF back in smart idle on ES1.0 */
	if (omap4_rev == OMAP4430_ES1_0)
		writel(0x80000000, &emif->emif_pwr_mgmt_ctrl);
}

static void dmm_init(u32 base)
{
	const struct dmm_lisa_map_regs *lisa_map_regs;

	emif_get_dmm_regs(&lisa_map_regs);

	struct dmm_lisa_map_regs *hw_lisa_map_regs =
	    (struct dmm_lisa_map_regs *)base;

	writel(0, &hw_lisa_map_regs->dmm_lisa_map_3);
	writel(0, &hw_lisa_map_regs->dmm_lisa_map_2);
	writel(0, &hw_lisa_map_regs->dmm_lisa_map_1);
	writel(0, &hw_lisa_map_regs->dmm_lisa_map_0);

	writel(lisa_map_regs->dmm_lisa_map_3,
		&hw_lisa_map_regs->dmm_lisa_map_3);
	writel(lisa_map_regs->dmm_lisa_map_2,
		&hw_lisa_map_regs->dmm_lisa_map_2);
	writel(lisa_map_regs->dmm_lisa_map_1,
		&hw_lisa_map_regs->dmm_lisa_map_1);
	writel(lisa_map_regs->dmm_lisa_map_0,
		&hw_lisa_map_regs->dmm_lisa_map_0);
}

/*
 * SDRAM initialization:
 * SDRAM initialization has two parts:
 * 1. Configuring the SDRAM device
 * 2. Update the AC timings related parameters in the EMIF module
 * (1) should be done only once and should not be done while we are
 * running from SDRAM.
 * (2) can and should be done more than once if OPP changes.
 * Particularly, this may be needed when we boot without SPL and
 * and using Configuration Header(CH). ROM code supports only at 50% OPP
 * at boot (low power boot). So u-boot has to switch to OPP100 and update
 * the frequency. So,
 * Doing (1) and (2) makes sense - first time initialization
 * Doing (2) and not (1) makes sense - OPP change (when using CH)
 * Doing (1) and not (2) doen't make sense
 * See do_sdram_init() for the details
 */
void sdram_init(void)
{
	u32 in_sdram, size_prog, size_detect;

	debug(">>sdram_init()\n");

	if (omap4_hw_init_context() == OMAP_INIT_CONTEXT_UBOOT_AFTER_SPL)
		return;

	in_sdram = running_from_sdram();
	debug("in_sdram = %d\n", in_sdram);

	if (!in_sdram) {
		sdram_init_pads();
		bypass_dpll(&prcm->cm_clkmode_dpll_core);
	}

	do_sdram_init(OMAP44XX_EMIF1);
	do_sdram_init(OMAP44XX_EMIF2);

	if (!in_sdram) {
		dmm_init(OMAP44XX_DMM_LISA_MAP_BASE);
		emif_post_init_config(OMAP44XX_EMIF1);
		emif_post_init_config(OMAP44XX_EMIF2);

	}

	/* for the shadow registers to take effect */
	freq_update_core();

	/* Do some testing after the init */
	if (!in_sdram) {
		size_prog = omap4_sdram_size();
		size_detect = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
						size_prog);
		/* Compare with the size programmed */
		if (size_detect != size_prog) {
			printf("SDRAM: identified size not same as expected"
				" size identified: %x expected: %x\n",
				size_detect,
				size_prog);
		} else
			debug("get_ram_size() successful");
	}

	debug("<<sdram_init()\n");
}
