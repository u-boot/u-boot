/*
 * Copyright (C) 2009 Matthias Kaehlcke <matthias@kaehlcke.net>
 *
 * Copyright (C) 2006 Dominic Rath <Dominic.Rath@gmx.de>
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

#include <asm/io.h>
#include "sdram_cfg.h"
#include "early_udelay.h"

#define PROGRAM_MODE_REG(bank)		(*(volatile uint32_t *)		\
		(SDRAM_BASE_ADDR | SDRAM_BANK_SEL_##bank | SDRAM_MODE_REG_VAL))

#define PRECHARGE_BANK(bank)		(*(volatile uint32_t *)		\
				(SDRAM_BASE_ADDR | SDRAM_BANK_SEL_##bank))

static void force_precharge(void);
static void setup_refresh_timer(void);
static void program_mode_registers(void);

void sdram_cfg(void)
{
	struct sdram_regs *sdram = (struct sdram_regs *)SDRAM_BASE;

	writel(SDRAM_DEVCFG_VAL, &sdram->SDRAM_DEVCFG_REG);

	/* Issue continous NOP commands */
	writel(GLCONFIG_INIT | GLCONFIG_MRS | GLCONFIG_CKE, &sdram->glconfig);

	early_udelay(200);

	force_precharge();

	setup_refresh_timer();

	program_mode_registers();

	/* Select normal operation mode */
	writel(GLCONFIG_CKE, &sdram->glconfig);
}

static void force_precharge(void)
{
	/*
	 * Errata most EP93xx revisions say that PRECHARGE ALL isn't always
	 * issued.
	 *
	 * Do a read from each bank to make sure they're precharged
	 */

	PRECHARGE_BANK(0);
	PRECHARGE_BANK(1);
	PRECHARGE_BANK(2);
	PRECHARGE_BANK(3);
}

static void setup_refresh_timer(void)
{
	struct sdram_regs *sdram = (struct sdram_regs *)SDRAM_BASE;

	/* Load refresh timer with 10 to issue refresh every 10 cycles */
	writel(0x0a, &sdram->refrshtimr);

	/*
	 * Wait at least 80 clock cycles to provide 8 refresh cycles
	 * to all SDRAMs
	 */
	early_udelay(1);

	/*
	 * Program refresh timer with normal value
	 * We need 8192 refresh cycles every 64ms
	 * at 15ns (HCLK >= 66MHz) per cycle:
	 * 64ms / 8192 = 7.8125us
	 * 7.8125us / 15ns = 520 (0x208)
	 */
	/*
	 * TODO: redboot uses 0x1e0 for the slowest possible device
	 * but i don't understand how this value is calculated
	 */
	writel(0x208, &sdram->refrshtimr);
}

static void program_mode_registers(void)
{
	/*
	 * The mode registers are programmed by performing a read from each
	 * SDRAM bank. The value of the address that is read defines the value
	 * that is written into the mode register
	 */

	PROGRAM_MODE_REG(0);

#if (CONFIG_NR_DRAM_BANKS >= 2)
	PROGRAM_MODE_REG(1);
#endif

#if (CONFIG_NR_DRAM_BANKS >= 3)
	PROGRAM_MODE_REG(2);
#endif

#if (CONFIG_NR_DRAM_BANKS == 4)
	PROGRAM_MODE_REG(3);
#endif
}
