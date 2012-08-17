/*
 * Copyright 2009-2011 eXMeritus, A Boeing Company
 * Copyright 2007-2009 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <asm/fsl_ddr_sdram.h>
#include <asm/io.h>
#include <miiphy.h>
#include <libfdt.h>
#include <linux/ctype.h>
#include <fdt_support.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <asm/fsl_law.h>
#include <netdev.h>
#include <malloc.h>
#include <i2c.h>
#include <pca953x.h>

#include "gpios.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	unsigned int gpio_high = 0;
	unsigned int gpio_low  = 0;
	unsigned int gpio_in   = 0;
	unsigned int i;

	puts("Board: HWW-1U-1A ");

	/*
	 * First just figure out which CPU we're on, then use that to
	 * configure the lists of other GPIOs to be programmed.
	 */
	mpc85xx_gpio_set_in(GPIO_CPU_ID);
	if (hww1u1a_is_cpu_a()) {
		puts("CPU A\n");

		/* We want to turn on some LEDs */
		gpio_high |= GPIO_CPUA_CPU_READY;
		gpio_low  |= GPIO_CPUA_DEBUG_LED1;
		gpio_low  |= GPIO_CPUA_DEBUG_LED2;

		/* Disable the unused transmitters */
		gpio_low  |= GPIO_CPUA_TDIS1A;
		gpio_high |= GPIO_CPUA_TDIS1B;
		gpio_low  |= GPIO_CPUA_TDIS2A;
		gpio_high |= GPIO_CPUA_TDIS2B;
	} else {
		puts("CPU B\n");

		/* We want to turn on some LEDs */
		gpio_high |= GPIO_CPUB_CPU_READY;
		gpio_low  |= GPIO_CPUB_DEBUG_LED1;
		gpio_low  |= GPIO_CPUB_DEBUG_LED2;

		/* Enable the appropriate receivers */
		gpio_high |= GPIO_CPUB_RMUX_SEL0A;
		gpio_high |= GPIO_CPUB_RMUX_SEL0B;
		gpio_low  |= GPIO_CPUB_RMUX_SEL1A;
		gpio_low  |= GPIO_CPUB_RMUX_SEL1B;
	}

	/* These GPIOs are common */
	gpio_in   |= IRQ_I2CINT | IRQ_FANINT | IRQ_DIMM_EVENT;
	gpio_low  |= GPIO_RS422_RE;
	gpio_high |= GPIO_RS422_DE;

	/* Ok, now go ahead and program all of those in one go */
	mpc85xx_gpio_set(gpio_high|gpio_low|gpio_in,
			 gpio_high|gpio_low,
			 gpio_high);

	/*
	 * If things have been taken out of reset early (for example, by one
	 * of the BDI3000 debuggers), then we need to put them back in reset
	 * and delay a while before we continue.
	 */
	if (mpc85xx_gpio_get(GPIO_RESETS)) {
		ccsr_ddr_t *ddr = (ccsr_ddr_t *)CONFIG_SYS_MPC85xx_DDR_ADDR;

		puts("Debugger detected... extra device reset enabled!\n");

		/* Put stuff into reset and disable the DDR controller */
		mpc85xx_gpio_set_low(GPIO_RESETS);
		out_be32(&ddr->sdram_cfg, 0x00000000);

		puts("    Waiting 1 sec for reset...");
		for (i = 0; i < 10; i++) {
			udelay(100000);
			puts(".");
		}
		puts("\n");
	}

	/* Now bring everything back out of reset again */
	mpc85xx_gpio_set_high(GPIO_RESETS);
	return 0;
}

/*
 * This little shell function just returns whether or not it's CPU A.
 * It can be used to select the right device-tree when booting, etc.
 */
int do_hww1u1a_test_cpu_a(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	if (argc > 1)
		cmd_usage(cmdtp);

	if (hww1u1a_is_cpu_a())
		return 0;
	else
		return 1;
}
U_BOOT_CMD(
	test_cpu_a, 1, 0, do_hww1u1a_test_cpu_a,
	"Test if this is CPU A (versus B) on the eXMeritus HWW-1U-1A board",
	""
);

/* Create a prompt-like string: "uboot@HOSTNAME% " */
#define PROMPT_PREFIX "uboot@exm"
#define PROMPT_SUFFIX "% "

/* This function returns a PS1 prompt based on the serial number */
static char *hww1u1a_prompt;
const char *hww1u1a_get_ps1(void)
{
	unsigned long len, i, j;
	const char *serialnr;

	/* If our prompt was already set, just use that */
	if (hww1u1a_prompt)
		return hww1u1a_prompt;

	/* Use our serial number if present, otherwise a default */
	serialnr = getenv("serial#");
	if (!serialnr || !serialnr[0])
		serialnr = "999999-X";

	/*
	 * We will turn the serial number into a hostname by:
	 *   (A) Delete all non-alphanumerics.
	 *   (B) Lowercase all letters.
	 *   (C) Prefix "exm".
	 *   (D) Suffix "a" for CPU A and "b" for CPU B.
	 */
	for (i = 0, len = 0; serialnr[i]; i++) {
		if (isalnum(serialnr[i]))
			len++;
	}

	len += sizeof(PROMPT_PREFIX PROMPT_SUFFIX) + 1; /* Includes NUL */
	hww1u1a_prompt = malloc(len);
	if (!hww1u1a_prompt)
		return PROMPT_PREFIX "UNKNOWN(ENOMEM)" PROMPT_SUFFIX;

	/* Now actually fill it in */
	i = 0;

	/* Handle the prefix */
	for (j = 0; j < sizeof(PROMPT_PREFIX) - 1; j++)
		hww1u1a_prompt[i++] = PROMPT_PREFIX[j];

	/* Now the serial# part of the hostname */
	for (j = 0; serialnr[j]; j++)
		if (isalnum(serialnr[j]))
			hww1u1a_prompt[i++] = tolower(serialnr[j]);

	/* Now the CPU id ("a" or "b") */
	hww1u1a_prompt[i++] = hww1u1a_is_cpu_a() ? 'a' : 'b';

	/* Finally the suffix */
	for (j = 0; j < sizeof(PROMPT_SUFFIX); j++)
		hww1u1a_prompt[i++] = PROMPT_SUFFIX[j];

	/* This should all have added up, but just in case */
	hww1u1a_prompt[len - 1] = '\0';

	/* Now we're done */
	return hww1u1a_prompt;
}

void pci_init_board(void)
{
	fsl_pcie_init_board(0);
}

int board_early_init_r(void)
{
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	const u8 flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap bootflash region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	/* invalidate existing TLB entry for FLASH */
	disable_tlb(flash_esel);

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, flash_esel, BOOKE_PAGESZ_256M, 1);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	struct tsec_info_struct tsec_info[4];
	struct fsl_pq_mdio_info mdio_info;

	SET_STD_TSEC_INFO(tsec_info[0], 1);
	SET_STD_TSEC_INFO(tsec_info[1], 2);
	SET_STD_TSEC_INFO(tsec_info[2], 3);

	if (hww1u1a_is_cpu_a())
		tsec_info[2].phyaddr = TSEC3_PHY_ADDR_CPUA;
	else
		tsec_info[2].phyaddr = TSEC3_PHY_ADDR_CPUB;

	mdio_info.regs = (struct tsec_mii_mng *)CONFIG_SYS_MDIO_BASE_ADDR;
	mdio_info.name = DEFAULT_MII_NAME;
	fsl_pq_mdio_init(bis, &mdio_info);

	tsec_eth_init(bis, tsec_info, 3);
	return pci_eth_init(bis);
}

void ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = getenv_bootm_low();
	size = getenv_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

	FT_FSL_PCI_SETUP;
}
