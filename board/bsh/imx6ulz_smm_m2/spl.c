// SPDX-License-Identifier: GPL-2.0+

#include <cpu_func.h>
#include <hang.h>
#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6ull_pins.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <linux/libfdt.h>
#include <spl.h>
#include <asm/arch/mx6-ddr.h>

#define UART_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE | \
		       PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
		       PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST | PAD_CTL_HYS)

static const iomux_v3_cfg_t uart4_pads[] = {
	MX6_PAD_UART4_TX_DATA__UART4_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_UART4_RX_DATA__UART4_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static void setup_iomux_uart(void)
{
	imx_iomux_v3_setup_multiple_pads(uart4_pads, ARRAY_SIZE(uart4_pads));
}

struct dram_cfg_param {
	unsigned int reg;
	unsigned int val;
};

struct dram_timing_info {
	const struct dram_cfg_param *ddrc_cfg;
	unsigned int ddrc_cfg_num;
};

static const struct dram_cfg_param ddr_ddrc_cfg_128mb[] = {
	/* IOMUX */

	/* DDR IO Type: */
	{0x020e04b4, 0x000C0000},	/* IOMUXC_SW_PAD_CTL_GRP_DDR_TYPE */
	{0x020e04ac, 0x00000000},	/* IOMUXC_SW_PAD_CTL_GRP_DDRPKE */

	/* Clock: */
	{0x020e027c, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_SDCLK_0 */

	/* Address: */
	{0x020e0250, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_CAS */
	{0x020e024c, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_RAS */
	{0x020e0490, 0x00000028},	/* IOMUXC_SW_PAD_CTL_GRP_ADDDS */

	/* Control: */
	{0x020e0288, 0x000C0028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_RESET */

	{0x020e0270, 0x00000000},	/*
					 * IOMUXC_SW_PAD_CTL_PAD_DRAM_SDBA2 - DSE can be configured
					 * using Group Control Register: IOMUXC_SW_PAD_CTL_GRP_CTLDS
					 */

	{0x020e0260, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_SDODT0 */
	{0x020e0264, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_SDODT1 */
	{0x020e04a0, 0x00000028},	/* IOMUXC_SW_PAD_CTL_GRP_CTLDS */

	/* Data Strobes: */
	{0x020e0494, 0x00020000},	/* IOMUXC_SW_PAD_CTL_GRP_DDRMODE_CTL */
	{0x020e0280, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS0 */
	{0x020e0284, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS1 */

	/* Data: */
	{0x020e04b0, 0x00020000},	/* IOMUXC_SW_PAD_CTL_GRP_DDRMODE */
	{0x020e0498, 0x00000028},	/* IOMUXC_SW_PAD_CTL_GRP_B0DS */
	{0x020e04a4, 0x00000028},	/* IOMUXC_SW_PAD_CTL_GRP_B1DS */

	{0x020e0244, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM0 */
	{0x020e0248, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM1 */

	/*
	 * =============================================================================
	 * DDR Controller Registers
	 * =============================================================================
	 * Manufacturer:ISSI
	 * Device Part Number:IS43TR16640BL-125JBLI
	 * Clock Freq.: 400MHz
	 * Density per CS in Gb: 1
	 * Chip Selects used:1
	 * Number of Banks:8
	 * Row address:    13
	 * Column address: 10
	 * Data bus width16
	 * =============================================================================
	 */

	{0x021b001c, 0x00008000}, /*
				   * MMDC0_MDSCR, set the Configuration request bit during
				   * MMDC set up
				   */

	/*
	 * =============================================================================
	 * Calibration setup
	 * =============================================================================
	 */

	{0x021b0800, 0xA1390003}, /*
				   * DDR_PHY_P0_MPZQHWCTRL, enable both one-time & periodic
				   * HW ZQ calibration.
				   */

	/*
	 * For target board, may need to run write leveling calibration to fine tune these
	 * settings.
	 */
	{0x021b080c, 0x00000000},

	/* Read DQS Gating calibration */
	{0x021b083c, 0x41480148},	/* MPDGCTRL0 PHY0 */

	/* Read calibration */
	{0x021b0848, 0x40403A3E},	/* MPRDDLCTL PHY0 */

	/* Write calibration */
	{0x021b0850, 0x4040362E},	/* MPWRDLCTL PHY0 */

	/*
	 * Read data bit delay: 3 is the recommended default value, although out of reset
	 * value is 0
	 */
	{0x021b081c, 0x33333333},	/* MMDC_MPRDDQBY0DL */
	{0x021b0820, 0x33333333},	/* MMDC_MPRDDQBY1DL */

	/* Write data bit delay: */
	{0x021b082c, 0xF3333333},	/* MMDC_MPWRDQBY0DL */
	{0x021b0830, 0xF3333333},	/* MMDC_MPWRDQBY1DL */

	/* DQS&CLK Duty Cycle */
	{0x021b08c0, 0x00944009},	/* [MMDC_MPDCCR] MMDC Duty Cycle Control Register */

	/* Complete calibration by forced measurement: */
	{0x021b08b8, 0x00000800},	/* DDR_PHY_P0_MPMUR0, frc_msr */

	/*
	 * =============================================================================
	 * Calibration setup end
	 * =============================================================================
	 */

	/* MMDC init: */
	{0x021b0004, 0x0002002D},	/* MMDC0_MDPDC */
	{0x021b0008, 0x1B333030},	/* MMDC0_MDOTC */
	{0x021b000c, 0x2B2F52F3},	/* MMDC0_MDCFG0 */
	{0x021b0010, 0xB66D0B63},	/* MMDC0_MDCFG1 */
	{0x021b0014, 0x01FF00DB},	/* MMDC0_MDCFG2 */

	/*
	 * MDMISC: RALAT kept to the high level of 5.
	 * MDMISC: consider reducing RALAT if your 528MHz board design allow that.
	 * Lower RALAT benefits:
	 * a. better operation at low frequency, for LPDDR2 freq < 100MHz, change RALAT to 3
	 * b. Small performance improvement
	 */
	{0x021b0018, 0x00211740},	/* MMDC0_MDMISC */
	{0x021b001c, 0x00008000},	/*
					 * MMDC0_MDSCR, set the Configuration request
					 * bit during MMDC set up
					 */
	{0x021b002c, 0x000026D2},	/* MMDC0_MDRWD */
	{0x021b0030, 0x002F1023},	/* MMDC0_MDOR */
	{0x021b0040, 0x00000043},	/* Chan0 CS0_END */
	{0x021b0000, 0x82180000},	/* MMDC0_MDCTL */

	{0x021b0890, 0x00400000},	/* MPPDCMPR2 */

	/* Mode register writes */
	{0x021b001c, 0x02808032},	/* MMDC0_MDSCR, MR2 write, CS0 */
	{0x021b001c, 0x00008033},	/* MMDC0_MDSCR, MR3 write, CS0 */
	{0x021b001c, 0x00048031},	/* MMDC0_MDSCR, MR1 write, CS0 */
	{0x021b001c, 0x15208030},	/* MMDC0_MDSCR, MR0write, CS0 */
	{0x021b001c, 0x04008040},	/*
					 * MMDC0_MDSCR, ZQ calibration command sent to
					 * device on CS0
					 */
	{0x021b0020, 0x00007800},	/* MMDC0_MDREF */

	{0x021b0818, 0x00000227},	/* DDR_PHY_P0_MPODTCTRL */

	{0x021b0004, 0x0002552D},	/* MMDC0_MDPDC now SDCTL power down enabled */

	{0x021b0404, 0x00011006},	/*
					 * MMDC0_MAPSR ADOPT power down enabled,
					 * MMDC will enter automatically to self-refresh
					 * while the number of idle cycle reached.
					 */

	{0x021b001c, 0x00000000},	/*
					 * MMDC0_MDSCR, clear this register
					 * (especially the configuration bit as initialization
					 * is complete)
					 */
};

static struct dram_timing_info dram_timing_128mb = {
	.ddrc_cfg = ddr_ddrc_cfg_128mb,
	.ddrc_cfg_num = ARRAY_SIZE(ddr_ddrc_cfg_128mb),
};

static void ddr_cfg_write(const struct dram_timing_info *dram_timing_info)
{
	int i;
	const struct dram_cfg_param *ddrc_cfg = dram_timing_info->ddrc_cfg;
	const int ddrc_cfg_num = dram_timing_info->ddrc_cfg_num;

	for (i = 0; i < ddrc_cfg_num; i++) {
		debug("Writing 0x%x to register 0x%x\n", ddrc_cfg->val,
		      ddrc_cfg->reg);
		writel(ddrc_cfg->val, ddrc_cfg->reg);
		ddrc_cfg++;
	}
}

static void spl_dram_init(void)
{
	struct mmdc_p_regs *mmdc0 = (struct mmdc_p_regs *)MMDC_P0_BASE_ADDR;

	clrbits_le32(&mmdc0->mdctl, 1 << 31);	/* clear SDE_0 */
	clrbits_le32(&mmdc0->mdctl, 1 << 30);	/* clear SDE_1 */

	ddr_cfg_write(&dram_timing_128mb);
}

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0xFFFFFFFF, &ccm->CCGR0);
	writel(0xFFFFFFFF, &ccm->CCGR1);
	writel(0xFFFFFFFF, &ccm->CCGR2);
	writel(0xFFFFFFFF, &ccm->CCGR3);
	writel(0xFFFFFFFF, &ccm->CCGR4);
	writel(0xFFFFFFFF, &ccm->CCGR5);
	writel(0xFFFFFFFF, &ccm->CCGR6);
}

void board_init_f(ulong dummy)
{
	ccgr_init();

	/* DDR initialization */
	spl_dram_init();

	arch_cpu_init();
	timer_init();
	setup_iomux_uart();
	preloader_console_init();
}

void reset_cpu(void)
{
}
