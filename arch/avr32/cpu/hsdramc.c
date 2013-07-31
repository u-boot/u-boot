/*
 * Copyright (C) 2005-2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>

#include <asm/io.h>
#include <asm/sdram.h>

#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>

#include "hsdramc1.h"

unsigned long sdram_init(void *sdram_base, const struct sdram_config *config)
{
	unsigned long sdram_size;
	uint32_t cfgreg;
	unsigned int i;

	cfgreg = (HSDRAMC1_BF(NC, config->col_bits - 8)
		       | HSDRAMC1_BF(NR, config->row_bits - 11)
		       | HSDRAMC1_BF(NB, config->bank_bits - 1)
		       | HSDRAMC1_BF(CAS, config->cas)
		       | HSDRAMC1_BF(TWR, config->twr)
		       | HSDRAMC1_BF(TRC, config->trc)
		       | HSDRAMC1_BF(TRP, config->trp)
		       | HSDRAMC1_BF(TRCD, config->trcd)
		       | HSDRAMC1_BF(TRAS, config->tras)
		       | HSDRAMC1_BF(TXSR, config->txsr));

	if (config->data_bits == SDRAM_DATA_16BIT)
		cfgreg |= HSDRAMC1_BIT(DBW);

	hsdramc1_writel(CR, cfgreg);

	/* Send a NOP to turn on the clock (necessary on some chips) */
	hsdramc1_writel(MR, HSDRAMC1_MODE_NOP);
	hsdramc1_readl(MR);
	writel(0, sdram_base);

	/*
	 * Initialization sequence for SDRAM, from the data sheet:
	 *
	 * 1. A minimum pause of 200 us is provided to precede any
	 *    signal toggle.
	 */
	udelay(200);

	/*
	 * 2. A Precharge All command is issued to the SDRAM
	 */
	hsdramc1_writel(MR, HSDRAMC1_MODE_BANKS_PRECHARGE);
	hsdramc1_readl(MR);
	writel(0, sdram_base);

	/*
	 * 3. Eight auto-refresh (CBR) cycles are provided
	 */
	hsdramc1_writel(MR, HSDRAMC1_MODE_AUTO_REFRESH);
	hsdramc1_readl(MR);
	for (i = 0; i < 8; i++)
		writel(0, sdram_base);

	/*
	 * 4. A mode register set (MRS) cycle is issued to program
	 *    SDRAM parameters, in particular CAS latency and burst
	 *    length.
	 *
	 * The address will be chosen by the SDRAMC automatically; we
	 * just have to make sure BA[1:0] are set to 0.
	 */
	hsdramc1_writel(MR, HSDRAMC1_MODE_LOAD_MODE);
	hsdramc1_readl(MR);
	writel(0, sdram_base);

	/*
	 * 5. The application must go into Normal Mode, setting Mode
	 *    to 0 in the Mode Register and performing a write access
	 *    at any location in the SDRAM.
	 */
	hsdramc1_writel(MR, HSDRAMC1_MODE_NORMAL);
	hsdramc1_readl(MR);
	writel(0, sdram_base);

	/*
	 * 6. Write refresh rate into SDRAMC refresh timer count
	 *    register (refresh rate = timing between refresh cycles).
	 */
	hsdramc1_writel(TR, config->refresh_period);

	if (config->data_bits == SDRAM_DATA_16BIT)
		sdram_size = 1 << (config->row_bits + config->col_bits
				   + config->bank_bits + 1);
	else
		sdram_size = 1 << (config->row_bits + config->col_bits
				   + config->bank_bits + 2);

	return sdram_size;
}
