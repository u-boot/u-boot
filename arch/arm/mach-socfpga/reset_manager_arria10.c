/*
 * Copyright (C) 2016-2017 Intel Corporation
 *
 * SPDX-License-Identifier:    GPL-2.0
 */

#include <asm/io.h>
#include <asm/arch/fpga_manager.h>
#include <asm/arch/misc.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <wait_bit.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct socfpga_reset_manager *reset_manager_base =
		(void *)SOCFPGA_RSTMGR_ADDRESS;
static const struct socfpga_system_manager *sysmgr_regs =
		(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;

#define ECC_MASK (ALT_RSTMGR_PER0MODRST_EMACECC0_SET_MSK | \
	ALT_RSTMGR_PER0MODRST_EMACECC1_SET_MSK | \
	ALT_RSTMGR_PER0MODRST_EMACECC2_SET_MSK | \
	ALT_RSTMGR_PER0MODRST_NANDECC_SET_MSK | \
	ALT_RSTMGR_PER0MODRST_QSPIECC_SET_MSK | \
	ALT_RSTMGR_PER0MODRST_SDMMCECC_SET_MSK)

void socfpga_reset_uart(int assert)
{
	unsigned int com_port;

	com_port = uart_com_port(gd->fdt_blob);

	if (com_port == SOCFPGA_UART1_ADDRESS)
		socfpga_per_reset(SOCFPGA_RESET(UART1), assert);
	else if (com_port == SOCFPGA_UART0_ADDRESS)
		socfpga_per_reset(SOCFPGA_RESET(UART0), assert);
}

static const u32 per0fpgamasks[] = {
	ALT_RSTMGR_PER0MODRST_EMACECC0_SET_MSK |
	ALT_RSTMGR_PER0MODRST_EMAC0_SET_MSK,
	ALT_RSTMGR_PER0MODRST_EMACECC1_SET_MSK |
	ALT_RSTMGR_PER0MODRST_EMAC1_SET_MSK,
	ALT_RSTMGR_PER0MODRST_EMACECC2_SET_MSK |
	ALT_RSTMGR_PER0MODRST_EMAC2_SET_MSK,
	0, /* i2c0 per1mod */
	0, /* i2c1 per1mod */
	0, /* i2c0_emac */
	0, /* i2c1_emac */
	0, /* i2c2_emac */
	ALT_RSTMGR_PER0MODRST_NANDECC_SET_MSK |
	ALT_RSTMGR_PER0MODRST_NAND_SET_MSK,
	ALT_RSTMGR_PER0MODRST_QSPIECC_SET_MSK |
	ALT_RSTMGR_PER0MODRST_QSPI_SET_MSK,
	ALT_RSTMGR_PER0MODRST_SDMMCECC_SET_MSK |
	ALT_RSTMGR_PER0MODRST_SDMMC_SET_MSK,
	ALT_RSTMGR_PER0MODRST_SPIM0_SET_MSK,
	ALT_RSTMGR_PER0MODRST_SPIM1_SET_MSK,
	ALT_RSTMGR_PER0MODRST_SPIS0_SET_MSK,
	ALT_RSTMGR_PER0MODRST_SPIS1_SET_MSK,
	0, /* uart0 per1mod */
	0, /* uart1 per1mod */
};

static const u32 per1fpgamasks[] = {
	0, /* emac0 per0mod */
	0, /* emac1 per0mod */
	0, /* emac2 per0mod */
	ALT_RSTMGR_PER1MODRST_I2C0_SET_MSK,
	ALT_RSTMGR_PER1MODRST_I2C1_SET_MSK,
	ALT_RSTMGR_PER1MODRST_I2C2_SET_MSK, /* i2c0_emac */
	ALT_RSTMGR_PER1MODRST_I2C3_SET_MSK, /* i2c1_emac */
	ALT_RSTMGR_PER1MODRST_I2C4_SET_MSK, /* i2c2_emac */
	0, /* nand per0mod */
	0, /* qspi per0mod */
	0, /* sdmmc per0mod */
	0, /* spim0 per0mod */
	0, /* spim1 per0mod */
	0, /* spis0 per0mod */
	0, /* spis1 per0mod */
	ALT_RSTMGR_PER1MODRST_UART0_SET_MSK,
	ALT_RSTMGR_PER1MODRST_UART1_SET_MSK,
};

struct bridge_cfg {
	int compat_id;
	u32  mask_noc;
	u32  mask_rstmgr;
};

static const struct bridge_cfg bridge_cfg_tbl[] = {
	{
		COMPAT_ALTERA_SOCFPGA_H2F_BRG,
		ALT_SYSMGR_NOC_H2F_SET_MSK,
		ALT_RSTMGR_BRGMODRST_H2F_SET_MSK,
	},
	{
		COMPAT_ALTERA_SOCFPGA_LWH2F_BRG,
		ALT_SYSMGR_NOC_LWH2F_SET_MSK,
		ALT_RSTMGR_BRGMODRST_LWH2F_SET_MSK,
	},
	{
		COMPAT_ALTERA_SOCFPGA_F2H_BRG,
		ALT_SYSMGR_NOC_F2H_SET_MSK,
		ALT_RSTMGR_BRGMODRST_F2H_SET_MSK,
	},
	{
		COMPAT_ALTERA_SOCFPGA_F2SDR0,
		ALT_SYSMGR_NOC_F2SDR0_SET_MSK,
		ALT_RSTMGR_BRGMODRST_F2SSDRAM0_SET_MSK,
	},
	{
		COMPAT_ALTERA_SOCFPGA_F2SDR1,
		ALT_SYSMGR_NOC_F2SDR1_SET_MSK,
		ALT_RSTMGR_BRGMODRST_F2SSDRAM1_SET_MSK,
	},
	{
		COMPAT_ALTERA_SOCFPGA_F2SDR2,
		ALT_SYSMGR_NOC_F2SDR2_SET_MSK,
		ALT_RSTMGR_BRGMODRST_F2SSDRAM2_SET_MSK,
	},
};

/* Disable the watchdog (toggle reset to watchdog) */
void socfpga_watchdog_disable(void)
{
	/* assert reset for watchdog */
	setbits_le32(&reset_manager_base->per1modrst,
		     ALT_RSTMGR_PER1MODRST_WD0_SET_MSK);
}

/* Release NOC ddr scheduler from reset */
void socfpga_reset_deassert_noc_ddr_scheduler(void)
{
	clrbits_le32(&reset_manager_base->brgmodrst,
		     ALT_RSTMGR_BRGMODRST_DDRSCH_SET_MSK);
}

/* Check whether Watchdog in reset state? */
int socfpga_is_wdt_in_reset(void)
{
	u32 val;

	val = readl(&reset_manager_base->per1modrst);
	val &= ALT_RSTMGR_PER1MODRST_WD0_SET_MSK;

	/* return 0x1 if watchdog in reset */
	return val;
}

/* emacbase: base address of emac to enable/disable reset
 * state: 0 - disable reset, !0 - enable reset
 */
void socfpga_emac_manage_reset(ulong emacbase, u32 state)
{
	ulong eccmask;
	ulong emacmask;

	switch (emacbase) {
	case SOCFPGA_EMAC0_ADDRESS:
		eccmask = ALT_RSTMGR_PER0MODRST_EMACECC0_SET_MSK;
		emacmask = ALT_RSTMGR_PER0MODRST_EMAC0_SET_MSK;
		break;
	case SOCFPGA_EMAC1_ADDRESS:
		eccmask = ALT_RSTMGR_PER0MODRST_EMACECC1_SET_MSK;
		emacmask = ALT_RSTMGR_PER0MODRST_EMAC1_SET_MSK;
		break;
	case SOCFPGA_EMAC2_ADDRESS:
		eccmask = ALT_RSTMGR_PER0MODRST_EMACECC2_SET_MSK;
		emacmask = ALT_RSTMGR_PER0MODRST_EMAC2_SET_MSK;
		break;
	default:
		error("emac base address unexpected! %lx", emacbase);
		hang();
		break;
	}

	if (state) {
		/* Enable ECC OCP first */
		setbits_le32(&reset_manager_base->per0modrst, eccmask);
		setbits_le32(&reset_manager_base->per0modrst, emacmask);
	} else {
		/* Disable ECC OCP first */
		clrbits_le32(&reset_manager_base->per0modrst, emacmask);
		clrbits_le32(&reset_manager_base->per0modrst, eccmask);
	}
}

static int get_bridge_init_val(const void *blob, int compat_id)
{
	int node;

	node = fdtdec_next_compatible(blob, 0, compat_id);
	if (node < 0)
		return 0;

	return fdtdec_get_uint(blob, node, "init-val", 0);
}

/* Enable bridges (hps2fpga, lwhps2fpga, fpga2hps, fpga2sdram) per handoff */
int socfpga_reset_deassert_bridges_handoff(void)
{
	u32 mask_noc = 0, mask_rstmgr = 0;
	int i;

	for (i = 0; i < ARRAY_SIZE(bridge_cfg_tbl); i++) {
		if (get_bridge_init_val(gd->fdt_blob,
					bridge_cfg_tbl[i].compat_id)) {
			mask_noc |= bridge_cfg_tbl[i].mask_noc;
			mask_rstmgr |= bridge_cfg_tbl[i].mask_rstmgr;
		}
	}

	/* clear idle request to all bridges */
	setbits_le32(&sysmgr_regs->noc_idlereq_clr, mask_noc);

	/* Release bridges from reset state per handoff value */
	clrbits_le32(&reset_manager_base->brgmodrst, mask_rstmgr);

	/* Poll until all idleack to 0, timeout at 1000ms */
	return wait_for_bit(__func__, &sysmgr_regs->noc_idleack, mask_noc,
			    false, 1000, false);
}

void socfpga_reset_assert_fpga_connected_peripherals(void)
{
	u32 mask0 = 0;
	u32 mask1 = 0;
	u32 fpga_pinux_addr = SOCFPGA_PINMUX_FPGA_INTERFACE_ADDRESS;
	int i;

	for (i = 0; i < ARRAY_SIZE(per1fpgamasks); i++) {
		if (readl(fpga_pinux_addr)) {
			mask0 |= per0fpgamasks[i];
			mask1 |= per1fpgamasks[i];
		}
		fpga_pinux_addr += sizeof(u32);
	}

	setbits_le32(&reset_manager_base->per0modrst, mask0 & ECC_MASK);
	setbits_le32(&reset_manager_base->per1modrst, mask1);
	setbits_le32(&reset_manager_base->per0modrst, mask0);
}

/* Release L4 OSC1 Watchdog Timer 0 from reset through reset manager */
void socfpga_reset_deassert_osc1wd0(void)
{
	clrbits_le32(&reset_manager_base->per1modrst,
		     ALT_RSTMGR_PER1MODRST_WD0_SET_MSK);
}

/*
 * Assert or de-assert SoCFPGA reset manager reset.
 */
void socfpga_per_reset(u32 reset, int set)
{
	const u32 *reg;
	u32 rstmgr_bank = RSTMGR_BANK(reset);

	switch (rstmgr_bank) {
	case 0:
		reg = &reset_manager_base->mpumodrst;
		break;
	case 1:
		reg = &reset_manager_base->per0modrst;
		break;
	case 2:
		reg = &reset_manager_base->per1modrst;
		break;
	case 3:
		reg = &reset_manager_base->brgmodrst;
		break;
	case 4:
		reg = &reset_manager_base->sysmodrst;
		break;

	default:
		return;
	}

	if (set)
		setbits_le32(reg, 1 << RSTMGR_RESET(reset));
	else
		clrbits_le32(reg, 1 << RSTMGR_RESET(reset));
}

/*
 * Assert reset on every peripheral but L4WD0.
 * Watchdog must be kept intact to prevent glitches
 * and/or hangs.
 * For the Arria10, we disable all the peripherals except L4 watchdog0,
 * L4 Timer 0, and ECC.
 */
void socfpga_per_reset_all(void)
{
	const u32 l4wd0 = (1 << RSTMGR_RESET(SOCFPGA_RESET(L4WD0)) |
			  (1 << RSTMGR_RESET(SOCFPGA_RESET(L4SYSTIMER0))));
	unsigned mask_ecc_ocp =
		ALT_RSTMGR_PER0MODRST_EMACECC0_SET_MSK |
		ALT_RSTMGR_PER0MODRST_EMACECC1_SET_MSK |
		ALT_RSTMGR_PER0MODRST_EMACECC2_SET_MSK |
		ALT_RSTMGR_PER0MODRST_USBECC0_SET_MSK |
		ALT_RSTMGR_PER0MODRST_USBECC1_SET_MSK |
		ALT_RSTMGR_PER0MODRST_NANDECC_SET_MSK |
		ALT_RSTMGR_PER0MODRST_QSPIECC_SET_MSK |
		ALT_RSTMGR_PER0MODRST_SDMMCECC_SET_MSK;

	/* disable all components except ECC_OCP, L4 Timer0 and L4 WD0 */
	writel(~l4wd0, &reset_manager_base->per1modrst);
	setbits_le32(&reset_manager_base->per0modrst, ~mask_ecc_ocp);

	/* Finally disable the ECC_OCP */
	setbits_le32(&reset_manager_base->per0modrst, mask_ecc_ocp);
}

#if defined(CONFIG_SOCFPGA_VIRTUAL_TARGET)
int socfpga_bridges_reset(int enable)
{
	/* For SoCFPGA-VT, this is NOP. */
	return 0;
}
#else
int socfpga_bridges_reset(int enable)
{
	int ret;

	/* Disable all the bridges (hps2fpga, lwhps2fpga, fpga2hps,
	   fpga2sdram) */
	/* set idle request to all bridges */
	writel(ALT_SYSMGR_NOC_H2F_SET_MSK |
		ALT_SYSMGR_NOC_LWH2F_SET_MSK |
		ALT_SYSMGR_NOC_F2H_SET_MSK |
		ALT_SYSMGR_NOC_F2SDR0_SET_MSK |
		ALT_SYSMGR_NOC_F2SDR1_SET_MSK |
		ALT_SYSMGR_NOC_F2SDR2_SET_MSK,
		&sysmgr_regs->noc_idlereq_set);

	/* Enable the NOC timeout */
	writel(ALT_SYSMGR_NOC_TMO_EN_SET_MSK, &sysmgr_regs->noc_timeout);

	/* Poll until all idleack to 1 */
	ret = wait_for_bit(__func__, &sysmgr_regs->noc_idleack,
		     ALT_SYSMGR_NOC_H2F_SET_MSK |
		     ALT_SYSMGR_NOC_LWH2F_SET_MSK |
		     ALT_SYSMGR_NOC_F2H_SET_MSK |
		     ALT_SYSMGR_NOC_F2SDR0_SET_MSK |
		     ALT_SYSMGR_NOC_F2SDR1_SET_MSK |
		     ALT_SYSMGR_NOC_F2SDR2_SET_MSK,
		     true, 10000, false);
	if (ret)
		return ret;

	/* Poll until all idlestatus to 1 */
	ret = wait_for_bit(__func__, &sysmgr_regs->noc_idlestatus,
		     ALT_SYSMGR_NOC_H2F_SET_MSK |
		     ALT_SYSMGR_NOC_LWH2F_SET_MSK |
		     ALT_SYSMGR_NOC_F2H_SET_MSK |
		     ALT_SYSMGR_NOC_F2SDR0_SET_MSK |
		     ALT_SYSMGR_NOC_F2SDR1_SET_MSK |
		     ALT_SYSMGR_NOC_F2SDR2_SET_MSK,
		     true, 10000, false);
	if (ret)
		return ret;

	/* Put all bridges (except NOR DDR scheduler) into reset state */
	setbits_le32(&reset_manager_base->brgmodrst,
		     (ALT_RSTMGR_BRGMODRST_H2F_SET_MSK |
		     ALT_RSTMGR_BRGMODRST_LWH2F_SET_MSK |
		     ALT_RSTMGR_BRGMODRST_F2H_SET_MSK |
		     ALT_RSTMGR_BRGMODRST_F2SSDRAM0_SET_MSK |
		     ALT_RSTMGR_BRGMODRST_F2SSDRAM1_SET_MSK |
		     ALT_RSTMGR_BRGMODRST_F2SSDRAM2_SET_MSK));

	/* Disable NOC timeout */
	writel(0, &sysmgr_regs->noc_timeout);

	return 0;
}
#endif
