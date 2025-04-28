#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/prcm.h>

#ifdef CONFIG_XPL_BUILD
void clock_init_safe(void)
{
	void *const ccm = (void *)SUNXI_CCM_BASE;
	void *const prcm = (void *)SUNXI_PRCM_BASE;

	if (IS_ENABLED(CONFIG_MACH_SUN50I_H616)) {
		/* this seems to enable PLLs on H616 */
		setbits_le32(prcm + CCU_PRCM_SYS_PWROFF_GATING, 0x10);
		setbits_le32(prcm + CCU_PRCM_RES_CAL_CTRL, 2);
	}

	if (IS_ENABLED(CONFIG_MACH_SUN50I_H616) ||
	    IS_ENABLED(CONFIG_MACH_SUN50I_H6)) {
		clrbits_le32(prcm + CCU_PRCM_RES_CAL_CTRL, 1);
		setbits_le32(prcm + CCU_PRCM_RES_CAL_CTRL, 1);
	}

	if (IS_ENABLED(CONFIG_MACH_SUN50I_H6)) {
		/* set key field for ldo enable */
		setbits_le32(prcm + CCU_PRCM_PLL_LDO_CFG, 0xA7000000);
		/* set PLL VDD LDO output to 1.14 V */
		setbits_le32(prcm + CCU_PRCM_PLL_LDO_CFG, 0x60000);
	}

	clock_set_pll1(408000000);

	writel(CCM_PLL6_DEFAULT, ccm + CCU_H6_PLL6_CFG);
	while (!(readl(ccm + CCU_H6_PLL6_CFG) & CCM_PLL6_LOCK))
		;

	clrsetbits_le32(ccm + CCU_H6_CPU_AXI_CFG,
			CCM_CPU_AXI_APB_MASK | CCM_CPU_AXI_AXI_MASK,
			CCM_CPU_AXI_DEFAULT_FACTORS);

	writel(CCM_PSI_AHB1_AHB2_DEFAULT, ccm + CCU_H6_PSI_AHB1_AHB2_CFG);
#ifdef CCM_AHB3_DEFAULT
	writel(CCM_AHB3_DEFAULT, ccm + CCU_H6_AHB3_CFG);
#endif
	writel(CCM_APB1_DEFAULT, ccm + CCU_H6_APB1_CFG);

	/*
	 * The mux and factor are set, but the clock will be enabled in
	 * DRAM initialization code.
	 */
	writel(MBUS_CLK_SRC_PLL6X2 | MBUS_CLK_M(3), ccm + CCU_H6_MBUS_CFG);
}

void clock_init_uart(void)
{
	void *const ccm = (void *)SUNXI_CCM_BASE;

	/* uart clock source is apb2 */
	writel(APB2_CLK_SRC_OSC24M|
	       APB2_CLK_RATE_N_1|
	       APB2_CLK_RATE_M(1),
	       ccm + CCU_H6_APB2_CFG);

	/* open the clock for uart */
	setbits_le32(ccm + CCU_H6_UART_GATE_RESET,
		     1 << (CONFIG_CONS_INDEX - 1));

	/* deassert uart reset */
	setbits_le32(ccm + CCU_H6_UART_GATE_RESET,
		     1 << (RESET_SHIFT + CONFIG_CONS_INDEX - 1));
}

void clock_set_pll1(unsigned int clk)
{
	void *const ccm = (void *)SUNXI_CCM_BASE;
	u32 val;

	/* Do not support clocks < 288MHz as they need factor P */
	if (clk < 288000000) clk = 288000000;

	/* Switch to 24MHz clock while changing PLL1 */
	val = readl(ccm + CCU_H6_CPU_AXI_CFG);
	val &= ~CCM_CPU_AXI_MUX_MASK;
	val |= CCM_CPU_AXI_MUX_OSC24M;
	writel(val, ccm + CCU_H6_CPU_AXI_CFG);

	/* clk = 24*n/p, p is ignored if clock is >288MHz */
	val = CCM_PLL1_CTRL_EN | CCM_PLL1_LOCK_EN | CCM_PLL1_CLOCK_TIME_2;
	val |= CCM_PLL1_CTRL_N(clk / 24000000);
	if (IS_ENABLED(CONFIG_MACH_SUN50I_H616))
	       val |= CCM_PLL1_OUT_EN;
	if (IS_ENABLED(CONFIG_SUNXI_GEN_NCAT2))
	       val |= CCM_PLL1_OUT_EN | CCM_PLL1_LDO_EN;
	writel(val, ccm + CCU_H6_PLL1_CFG);
	while (!(readl(ccm + CCU_H6_PLL1_CFG) & CCM_PLL1_LOCK)) {}

	/* Switch CPU to PLL1 */
	val = readl(ccm + CCU_H6_CPU_AXI_CFG);
	val &= ~CCM_CPU_AXI_MUX_MASK;
	val |= CCM_CPU_AXI_MUX_PLL_CPUX;
	writel(val, ccm + CCU_H6_CPU_AXI_CFG);
}

int clock_twi_onoff(int port, int state)
{
	void *const ccm = (void *)SUNXI_CCM_BASE;
	void *const prcm = (void *)SUNXI_PRCM_BASE;
	u32 value, *ptr;
	int shift;

	value = BIT(GATE_SHIFT) | BIT (RESET_SHIFT);

	if (port == 5) {
		shift = 0;
		ptr = prcm + CCU_PRCM_I2C_GATE_RESET;
	} else {
		shift = port;
		ptr = ccm + CCU_H6_I2C_GATE_RESET;
	}

	/* set the apb clock gate and reset for twi */
	if (state)
		setbits_le32(ptr, value << shift);
	else
		clrbits_le32(ptr, value << shift);

	return 0;
}
#endif /* CONFIG_XPL_BUILD */

/* PLL_PERIPH0 clock, used by the MMC driver */
unsigned int clock_get_pll6(void)
{
	void *const ccm = (void *)SUNXI_CCM_BASE;
	uint32_t rval = readl(ccm + CCU_H6_PLL6_CFG);
	int n = ((rval & CCM_PLL6_CTRL_N_MASK) >> CCM_PLL6_CTRL_N_SHIFT) + 1;
	int div2 = ((rval & CCM_PLL6_CTRL_DIV2_MASK) >>
		    CCM_PLL6_CTRL_DIV2_SHIFT) + 1;
	int div1, m;

	if (IS_ENABLED(CONFIG_SUNXI_GEN_NCAT2)) {
		div1 = ((rval & CCM_PLL6_CTRL_P0_MASK) >>
			CCM_PLL6_CTRL_P0_SHIFT) + 1;
	} else {
		div1 = ((rval & CCM_PLL6_CTRL_DIV1_MASK) >>
			CCM_PLL6_CTRL_DIV1_SHIFT) + 1;
	}

	/*
	 * The factors encoded in the register describe the doubled clock
	 * frequency, expect for the H6, where it's the quadrupled frequency.
	 * Compensate for that here.
	 */
	if (IS_ENABLED(CONFIG_MACH_SUN50I_H6))
		m = 4;
	else
		m = 2;

	return 24000000U * n / m / div1 / div2;
}
