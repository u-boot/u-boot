/*
 *  Copyright (C) 2013 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#include <common.h>
#include <asm/io.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/fpga_manager.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct socfpga_reset_manager *reset_manager_base =
		(void *)SOCFPGA_RSTMGR_ADDRESS;

/* Toggle reset signal to watchdog (WDT is disabled after this operation!) */
void socfpga_watchdog_reset(void)
{
	/* assert reset for watchdog */
	setbits_le32(&reset_manager_base->per_mod_reset,
		     1 << RSTMGR_PERMODRST_L4WD0_LSB);

	/* deassert watchdog from reset (watchdog in not running state) */
	clrbits_le32(&reset_manager_base->per_mod_reset,
		     1 << RSTMGR_PERMODRST_L4WD0_LSB);
}

/*
 * Write the reset manager register to cause reset
 */
void reset_cpu(ulong addr)
{
	/* request a warm reset */
	writel((1 << RSTMGR_CTRL_SWWARMRSTREQ_LSB),
		&reset_manager_base->ctrl);
	/*
	 * infinite loop here as watchdog will trigger and reset
	 * the processor
	 */
	while (1)
		;
}

/*
 * Release peripherals from reset based on handoff
 */
void reset_deassert_peripherals_handoff(void)
{
	writel(0, &reset_manager_base->per_mod_reset);
}

#if defined(CONFIG_SOCFPGA_VIRTUAL_TARGET)
void socfpga_bridges_reset(int enable)
{
	/* For SoCFPGA-VT, this is NOP. */
}
#else

#define L3REGS_REMAP_LWHPS2FPGA_MASK	0x10
#define L3REGS_REMAP_HPS2FPGA_MASK	0x08
#define L3REGS_REMAP_OCRAM_MASK		0x01

void socfpga_bridges_reset(int enable)
{
	const uint32_t l3mask = L3REGS_REMAP_LWHPS2FPGA_MASK |
				L3REGS_REMAP_HPS2FPGA_MASK |
				L3REGS_REMAP_OCRAM_MASK;

	if (enable) {
		/* brdmodrst */
		writel(0xffffffff, &reset_manager_base->brg_mod_reset);
	} else {
		/* Check signal from FPGA. */
		if (fpgamgr_poll_fpga_ready()) {
			/* FPGA not ready. Wait for watchdog timeout. */
			printf("%s: fpga not ready, hanging.\n", __func__);
			hang();
		}

		/* brdmodrst */
		writel(0, &reset_manager_base->brg_mod_reset);

		/* Remap the bridges into memory map */
		writel(l3mask, SOCFPGA_L3REGS_ADDRESS);
	}
}
#endif

/* Change the reset state for EMAC 0 and EMAC 1 */
void socfpga_emac_reset(int enable)
{
	const void *reset = &reset_manager_base->per_mod_reset;

	if (enable) {
		setbits_le32(reset, 1 << RSTMGR_PERMODRST_EMAC0_LSB);
		setbits_le32(reset, 1 << RSTMGR_PERMODRST_EMAC1_LSB);
	} else {
#if (CONFIG_EMAC_BASE == SOCFPGA_EMAC0_ADDRESS)
		clrbits_le32(reset, 1 << RSTMGR_PERMODRST_EMAC0_LSB);
#elif (CONFIG_EMAC_BASE == SOCFPGA_EMAC1_ADDRESS)
		clrbits_le32(reset, 1 << RSTMGR_PERMODRST_EMAC1_LSB);
#endif
	}
}

/* SPI Master enable (its held in reset by the preloader) */
void socfpga_spim_enable(void)
{
	const void *reset = &reset_manager_base->per_mod_reset;

	clrbits_le32(reset, (1 << RSTMGR_PERMODRST_SPIM0_LSB) |
		     (1 << RSTMGR_PERMODRST_SPIM1_LSB));
}

/* Bring UART0 out of reset. */
void socfpga_uart0_enable(void)
{
	const void *reset = &reset_manager_base->per_mod_reset;

	clrbits_le32(reset, 1 << RSTMGR_PERMODRST_UART0_LSB);
}

/* Bring SDRAM controller out of reset. */
void socfpga_sdram_enable(void)
{
	const void *reset = &reset_manager_base->per_mod_reset;

	clrbits_le32(reset, 1 << RSTMGR_PERMODRST_SDR_LSB);
}

/* Bring OSC1 timer out of reset. */
void socfpga_osc1timer_enable(void)
{
	const void *reset = &reset_manager_base->per_mod_reset;

	clrbits_le32(reset, 1 << RSTMGR_PERMODRST_OSC1TIMER0_LSB);
}
